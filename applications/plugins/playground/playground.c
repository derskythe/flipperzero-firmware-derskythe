#include <furi.h>
#include <furi_hal.h>

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/view_stack.h>
#include <gui/modules/text_input.h>
#include <gui/modules/popup.h>
#include <gui/modules/widget.h>
#include <gui/modules/loading.h>

#include <dialogs/dialogs.h>

#include "playground.h"
#include "playground_i.h"
#include "playground_custom_event.h"

#define TAG "PlayGroundApp"

static bool playground_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    PlayGroundState* instance = context;
    return scene_manager_handle_custom_event(instance->scene_manager, event);
}

static bool playground_back_event_callback(void* context) {
    furi_assert(context);
    PlayGroundState* instance = context;
    return scene_manager_handle_back_event(instance->scene_manager);
}

static void playground_tick_event_callback(void* context) {
    furi_assert(context);
    PlayGroundState* instance = context;
    scene_manager_handle_tick_event(instance->scene_manager);
}

PlayGroundState* playground_alloc() {
    PlayGroundState* instance = malloc(sizeof(PlayGroundState));

    instance->save_path = furi_string_alloc();
    instance->scene_manager = scene_manager_alloc(&playground_scene_handlers, instance);
    instance->view_dispatcher = view_dispatcher_alloc();

    instance->gui = furi_record_open(RECORD_GUI);
    instance->storage = furi_record_open(RECORD_STORAGE);

    view_dispatcher_enable_queue(instance->view_dispatcher);
    view_dispatcher_set_event_callback_context(instance->view_dispatcher, instance);
    view_dispatcher_set_custom_event_callback(
        instance->view_dispatcher, playground_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        instance->view_dispatcher, playground_back_event_callback);
    view_dispatcher_set_tick_event_callback(
        instance->view_dispatcher, playground_tick_event_callback, 500);

    //Dialog
    instance->dialogs = furi_record_open(RECORD_DIALOGS);

    // Notifications
    instance->notifications = furi_record_open(RECORD_NOTIFICATION);

    // TextInput
    instance->text_input = text_input_alloc();
    view_dispatcher_add_view(
        instance->view_dispatcher,
        PlayGroundViewTextInput,
        text_input_get_view(instance->text_input));

    // Custom Widget
    instance->widget = widget_alloc();
    view_dispatcher_add_view(
        instance->view_dispatcher, PlayGroundViewWidget, widget_get_view(instance->widget));

    // Popup
    instance->popup = popup_alloc();
    view_dispatcher_add_view(
        instance->view_dispatcher, PlayGroundViewPopup, popup_get_view(instance->popup));

    // ViewStack
    instance->view_stack = view_stack_alloc();
    view_dispatcher_add_view(
        instance->view_dispatcher, PlayGroundViewStack, view_stack_get_view(instance->view_stack));

    // PlayGroundMainView
    instance->view_main = playground_main_view_alloc();
    view_dispatcher_add_view(
        instance->view_dispatcher,
        PlayGroundViewMain,
        playground_main_view_get_view(instance->view_main));

    // Loading
    instance->loading = loading_alloc();

    return instance;
}

void playground_free(PlayGroundState* instance) {
    furi_assert(instance);

    // Notifications
    notification_message(instance->notifications, &sequence_blink_stop);
    furi_record_close(RECORD_NOTIFICATION);
    instance->notifications = NULL;

    // Loading
    loading_free(instance->loading);

    // View Main
    view_dispatcher_remove_view(instance->view_dispatcher, PlayGroundViewMain);
    playground_main_view_free(instance->view_main);

    // TextInput
    view_dispatcher_remove_view(instance->view_dispatcher, PlayGroundViewTextInput);
    text_input_free(instance->text_input);

    // Custom Widget
    view_dispatcher_remove_view(instance->view_dispatcher, PlayGroundViewWidget);
    widget_free(instance->widget);

    // Popup
    view_dispatcher_remove_view(instance->view_dispatcher, PlayGroundViewPopup);
    popup_free(instance->popup);

    // ViewStack
    view_dispatcher_remove_view(instance->view_dispatcher, PlayGroundViewStack);
    view_stack_free(instance->view_stack);

    //Dialog
    furi_record_close(RECORD_DIALOGS);
    instance->dialogs = NULL;

    // Scene manager
    scene_manager_free(instance->scene_manager);

    // View Dispatcher
    view_dispatcher_free(instance->view_dispatcher);

    // GUI
    furi_record_close(RECORD_GUI);
    instance->gui = NULL;

    // Storage
    furi_record_close(RECORD_STORAGE);
    instance->storage = NULL;

    furi_string_free(instance->save_path);

    // The rest
    free(instance);
}

void playground_show_loading_popup(void* context, bool show) {
    TaskHandle_t timer_task = xTaskGetHandle(configTIMER_SERVICE_TASK_NAME);
    PlayGroundState* instance = context;
    ViewStack* view_stack = instance->view_stack;
    Loading* loading = instance->loading;

    if(show) {
        // Raise timer priority so that animations can play
        vTaskPrioritySet(timer_task, configMAX_PRIORITIES - 1);
        view_stack_add_view(view_stack, loading_get_view(loading));
    } else {
        view_stack_remove_view(view_stack, loading_get_view(loading));
        // Restore default timer priority
        vTaskPrioritySet(timer_task, configTIMER_TASK_PRIORITY);
    }
}

void playground_text_input_callback(void* context) {
    furi_assert(context);
    PlayGroundState* instance = context;
    view_dispatcher_send_custom_event(
        instance->view_dispatcher, PlayGroundCustomEventTypeTextEditDone);
}

void playground_popup_closed_callback(void* context) {
    furi_assert(context);
    PlayGroundState* instance = context;
    view_dispatcher_send_custom_event(
        instance->view_dispatcher, PlayGroundCustomEventTypePopupClosed);
}

// ENTRYPOINT
int32_t playground_app(void* p) {
    UNUSED(p);

    PlayGroundState* instance = playground_alloc();
    view_dispatcher_attach_to_gui(
        instance->view_dispatcher, instance->gui, ViewDispatcherTypeFullscreen);
    scene_manager_next_scene(instance->scene_manager, PlayGroundSceneStart);

    furi_hal_power_suppress_charge_enter();
    notification_message(instance->notifications, &sequence_display_backlight_on);
    view_dispatcher_run(instance->view_dispatcher);
    furi_hal_power_suppress_charge_exit();
    playground_free(instance);

    return 0;
}