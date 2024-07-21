#pragma once

#include "../playground_custom_event.h"
#include <gui/view.h>
#include "assets_icons.h"
#include <input/input.h>
#include <gui/elements.h>
#include <gui/icon.h>

typedef void (*PlayGroundMainViewCallback)(PlayGroundCustomEvent event, void* context);
typedef struct PlayGroundMainView PlayGroundMainView;

void playground_main_view_set_callback(
    PlayGroundMainView* instance,
    PlayGroundMainViewCallback callback,
    void* context);

PlayGroundMainView* playground_main_view_alloc();
void playground_main_view_free(PlayGroundMainView* instance);
View* playground_main_view_get_view(PlayGroundMainView* instance);
void playground_main_view_set_index(PlayGroundMainView* instance, uint8_t idx);
void playground_main_view_add_item(PlayGroundMainView* instance, const char* name, uint8_t type);
uint8_t playground_main_view_get_index(PlayGroundMainView* instance);
void playground_attack_view_enter(void* context);
void playground_attack_view_exit(void* context);
bool playground_attack_view_input(InputEvent* event, void* context);
void playground_attack_view_draw(Canvas* canvas, void* context);