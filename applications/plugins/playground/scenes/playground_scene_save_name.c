#include <m-string.h>
#include <subghz/types.h>
#include <lib/toolbox/random_name.h>
#include <gui/modules/validators.h>
#include <lib/toolbox/path.h>

#include "../playground_i.h"
#include "../playground_custom_event.h"

#define TAG "PlayGroundSceneSaveFile"

void playground_scene_save_name_on_enter(void* context) {
    PlayGroundState* instance = (PlayGroundState*)context;

    // Setup view
    TextInput* text_input = instance->text_input;
    set_random_name(instance->text_store, sizeof(instance->text_store));

    text_input_set_header_text(text_input, "Name of file");
    text_input_set_result_callback(
        text_input,
        playground_text_input_callback,
        instance,
        instance->text_store,
        PLAYGROUND_MAX_LEN_NAME,
        true);

    string_set_str(instance->save_path, PLAYGROUND_PATH);

    ValidatorIsFile* validator_is_file = validator_is_file_alloc_init(
        string_get_cstr(instance->save_path), PLAYGROUND_FILE_EXT, "");
    text_input_set_validator(text_input, validator_is_file_callback, validator_is_file);

    view_dispatcher_switch_to_view(instance->view_dispatcher, PlayGroundViewTextInput);
}

bool playground_scene_save_name_on_event(void* context, SceneManagerEvent event) {
    PlayGroundState* instance = (PlayGroundState*)context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeBack) {
        scene_manager_previous_scene(instance->scene_manager);
        return true;
    } else if(
        event.type == SceneManagerEventTypeCustom &&
        event.event == PlayGroundCustomEventTypeTextEditDone) {
#ifdef FURI_DEBUG
        FURI_LOG_D(TAG, "Saving: %s", instance->text_store);
#endif
        bool success = false;
        if(strcmp(instance->text_store, "")) {
            string_cat_printf(
                instance->save_path, "/%s%s", instance->text_store, PLAYGROUND_FILE_EXT);

            //if(playground_device_save_file(instance, string_get_cstr(instance->save_path))) {
                scene_manager_next_scene(instance->scene_manager, PlayGroundSceneSaveSuccess);
                success = true;
                consumed = true;
            //}
        }

        if(!success) {
            dialog_message_show_storage_error(instance->dialogs, "Error during saving!");
            consumed = scene_manager_search_and_switch_to_previous_scene(
                instance->scene_manager, PlayGroundSceneStart);
        }
    }
    return consumed;
}

void playground_scene_save_name_on_exit(void* context) {
    PlayGroundState* instance = (PlayGroundState*)context;

    // Clear view
    void* validator_context = text_input_get_validator_callback_context(instance->text_input);
    text_input_set_validator(instance->text_input, NULL, NULL);
    validator_is_file_free(validator_context);

    text_input_reset(instance->text_input);

    string_reset(instance->save_path);
}
