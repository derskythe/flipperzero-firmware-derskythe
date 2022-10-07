#pragma once

#include <furi.h>
#include <furi_hal.h>
#include <input/input.h>

#include "lib/toolbox/path.h"
#include <notification/notification.h>
#include <notification/notification_messages.h>
#include <string.h>

#include <lib/toolbox/stream/stream.h>
#include <stream_buffer.h>

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/view_stack.h>
#include <gui/scene_manager.h>
#include <gui/modules/text_input.h>
#include <gui/modules/popup.h>
#include <gui/modules/widget.h>
#include <gui/modules/loading.h>

#include <dialogs/dialogs.h>

#include <lib/subghz/protocols/base.h>
#include <lib/subghz/transmitter.h>
#include <lib/subghz/receiver.h>
#include <lib/subghz/environment.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>

#include "playground.h"
#include "scenes/playground_scene.h"
#include "views/playground_main_view.h"

#define PLAYGROUND_TEXT_STORE_SIZE 256

#define PLAYGROUND_MAX_LEN_NAME 64
#define PLAYGROUND_PATH EXT_PATH("subghz")
#define PLAYGROUND_FILE_EXT ".sub"

typedef enum {
    PlayGroundViewNone,
    PlayGroundViewMain,
    PlayGroundViewAttack,
    PlayGroundViewTextInput,
    PlayGroundViewDialogEx,
    PlayGroundViewPopup,
    PlayGroundViewWidget,
    PlayGroundViewStack,
} PlayGroundView;

struct PlayGroundState {
    // GUI elements
    NotificationApp* notifications;
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    ViewStack* view_stack;
    TextInput* text_input;
    Popup* popup;
    Widget* widget;
    DialogsApp* dialogs;
    Loading* loading;

    // Other
    char text_store[PLAYGROUND_MAX_LEN_NAME];
    FuriString* save_path;

    // Views
    PlayGroundMainView* view_main;
    PlayGroundView current_view;

    // Scene
    SceneManager* scene_manager;

    // Storage
    Storage* storage;
};

void playground_show_loading_popup(void* context, bool show);
void playground_text_input_callback(void* context);
void playground_popup_closed_callback(void* context);