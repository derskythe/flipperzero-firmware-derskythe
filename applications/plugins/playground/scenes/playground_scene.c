#include "playground_scene.h"

// Generate scene on_enter handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_enter,
void (*const playground_on_enter_handlers[])(void*) = {
#include "playground_scene_config.h"
};
#undef ADD_SCENE

// Generate scene on_event handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_event,
bool (*const playground_on_event_handlers[])(void* context, SceneManagerEvent event) = {
#include "playground_scene_config.h"
};
#undef ADD_SCENE

// Generate scene on_exit handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_exit,
void (*const playground_on_exit_handlers[])(void* context) = {
#include "playground_scene_config.h"
};
#undef ADD_SCENE

// Initialize scene handlers configuration structure
const SceneManagerHandlers playground_scene_handlers = {
    .on_enter_handlers = playground_on_enter_handlers,
    .on_event_handlers = playground_on_event_handlers,
    .on_exit_handlers = playground_on_exit_handlers,
    .scene_num = PlayGroundSceneNum,
};
