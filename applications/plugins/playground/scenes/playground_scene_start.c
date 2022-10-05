#include "../playground_i.h"
#include "../playground_custom_event.h"
#include "../views/playground_main_view.h"

#define TAG "PlayGroundSceneStart"

uint32_t rand_range(uint32_t n, uint32_t m) {
    // size of range, inclusive
    const uint32_t length_of_range = m - n + 1;

    // add n so that we don't return a number below our range
    return (uint32_t)(rand() % length_of_range + n);
}

void playground_scene_start_callback(PlayGroundCustomEvent event, void* context) {
    furi_assert(context);

    PlayGroundState* instance = (PlayGroundState*)context;
    view_dispatcher_send_custom_event(instance->view_dispatcher, event);
}

void playground_scene_start_on_enter(void* context) {
    furi_assert(context);
    PlayGroundState* instance = (PlayGroundState*)context;
    PlayGroundMainView* view = instance->view_main;

    instance->current_view = PlayGroundViewMain;
    playground_main_view_set_callback(view, playground_scene_start_callback, instance);
    playground_main_view_set_index(view, 0);

    view_dispatcher_switch_to_view(instance->view_dispatcher, instance->current_view);
}

void playground_scene_start_on_exit(void* context) {
    UNUSED(context);
}

bool playground_scene_start_on_event(void* context, SceneManagerEvent event) {
    PlayGroundState* instance = (PlayGroundState*)context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == PlayGroundCustomEventTypeMenuSelected) {
            // uint8_t index = playground_main_view_get_index(instance->view_main);

            // playground_main_view_set_index(instance->view_main, index);
            // scene_manager_next_scene(instance->scene_manager, PlayGroundSceneStart);
            string_t value;
            string_init_printf(value, "%d", rand_range(100, 600));
            playground_main_view_add_item(instance->view_main, string_get_cstr(value), 0);

            string_clear(value);

            consumed = true;
        } else if(event.event == PlayGroundCustomEventTypeLoadFile) {
            scene_manager_next_scene(instance->scene_manager, PlayGroundSceneLoadFile);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        //exit app
        scene_manager_stop(instance->scene_manager);
        view_dispatcher_stop(instance->view_dispatcher);
        consumed = true;
    }

    return consumed;
}