#include "../playground_i.h"
#include "../playground_custom_event.h"
#include "../views/playground_main_view.h"

#define TAG "PlayGroundSceneStart"

void playground_scene_load_select_callback(PlayGroundCustomEvent event, void* context) {
    furi_assert(context);

    PlayGroundState* instance = (PlayGroundState*)context;
    view_dispatcher_send_custom_event(instance->view_dispatcher, event);
}

void playground_scene_load_select_on_enter(void* context) {
    furi_assert(context);
    PlayGroundState* instance = (PlayGroundState*)context;
    PlayGroundMainView* view = instance->view_main;

    instance->current_view = PlayGroundViewMain;
    playground_main_view_set_callback(view, playground_scene_load_select_callback, instance);

    view_dispatcher_switch_to_view(instance->view_dispatcher, instance->current_view);
}

void playground_scene_load_select_on_exit(void* context) {
    UNUSED(context);
}

bool playground_scene_load_select_on_event(void* context, SceneManagerEvent event) {
    PlayGroundState* instance = (PlayGroundState*)context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == PlayGroundCustomEventTypeIndexSelected) {
            scene_manager_next_scene(instance->scene_manager, PlayGroundSceneStart);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        if(!scene_manager_search_and_switch_to_previous_scene(
               instance->scene_manager, PlayGroundSceneStart)) {
            scene_manager_next_scene(instance->scene_manager, PlayGroundSceneStart);
        }
        consumed = true;
    }

    return consumed;
}