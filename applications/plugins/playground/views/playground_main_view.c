#include "playground_main_view.h"
#include "../playground_i.h"

#include <input/input.h>
#include <gui/elements.h>
#include "assets_icons.h"
#include <gui/icon.h>
#include <m-array.h>

#define STATUS_BAR_Y_SHIFT 14
#define TAG "PlayGroundMainView"

typedef struct {
    string_t item_str;
    uint8_t type;
} PlaygroundTextItem;

ARRAY_DEF(PlaygroundTextItemArray, PlaygroundTextItem, M_POD_OPLIST)

#define M_OPL_PlaygroundTextItemArray_t() ARRAY_OPLIST(PlaygroundTextItemArray, M_POD_OPLIST)

struct PlaygroundText {
    PlaygroundTextItemArray_t data;
};

typedef struct PlaygroundText PlaygroundText;

struct PlayGroundMainView {
    View* view;
    PlayGroundMainViewCallback callback;
    void* context;
};

typedef struct {
    uint8_t index;
    PlaygroundText* items;
} PlayGroundMainViewModel;

void playground_main_view_set_callback(
    PlayGroundMainView* instance,
    PlayGroundMainViewCallback callback,
    void* context) {
    furi_assert(instance);
    furi_assert(callback);

    instance->callback = callback;
    instance->context = context;
}

void playground_main_view_draw(Canvas* canvas, PlayGroundMainViewModel* model) {
    furi_assert(model);
    PlayGroundMainViewModel* m = model;
    furi_assert(m);

    // Title
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_box(canvas, 0, 0, canvas_width(canvas), STATUS_BAR_Y_SHIFT);
    canvas_invert_color(canvas);
    canvas_draw_str_aligned(canvas, 64, 3, AlignCenter, AlignTop, "Playground");
    canvas_invert_color(canvas);

    size_t size = PlaygroundTextItemArray_size(m->items->data);
    const uint8_t line_height = 10;

    if(size > 0) {
        for(size_t item_position = 0; item_position < size; item_position++) {
            PlaygroundTextItem* current = PlaygroundTextItemArray_get(m->items->data, item_position);
            canvas_draw_str_aligned(
                canvas,
                4,
                9 + (item_position * line_height) + STATUS_BAR_Y_SHIFT,
                AlignLeft,
                AlignCenter,
                furi_string_get_cstr(current->item_str));
        }
    }
}

bool playground_main_view_input(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
#ifdef FURI_DEBUG
    FURI_LOG_D(TAG, "InputKey: %d", event->key);
#endif
    bool consumed = false;

    if(event->key == InputKeyBack && event->type == InputTypeShort) {
        return consumed;
    }

    PlayGroundMainView* instance = context;
    uint8_t index = 0;

    with_view_model(
        instance->view, (PlayGroundMainViewModel * model) {
            index = model->index;
            return false;
        })

    furi_assert(index != 100);
    if((event->type == InputTypeShort) || (event->type == InputTypeRepeat)) {
        with_view_model(
            instance->view, (PlayGroundMainViewModel * model) {
                model->index = index;
                return true;
            })

        if(event->key == InputKeyOk && event->type == InputTypeShort) {
            if(index == 255) {
                instance->callback(PlayGroundCustomEventTypeLoadFile, instance->context);
            } else {
                instance->callback(PlayGroundCustomEventTypeMenuSelected, instance->context);
            }
        }

        consumed = true;
    }

    return consumed;
}

void playground_main_view_enter(void* context) {
    furi_assert(context);
}

void playground_main_view_exit(void* context) {
    furi_assert(context);
}

PlayGroundMainView* playground_main_view_alloc() {
    PlayGroundMainView* instance = malloc(sizeof(PlayGroundMainView));
    instance->view = view_alloc();
    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(PlayGroundMainViewModel));
    view_set_context(instance->view, instance);
    view_set_draw_callback(instance->view, (ViewDrawCallback)playground_main_view_draw);
    view_set_input_callback(instance->view, playground_main_view_input);
    view_set_enter_callback(instance->view, playground_main_view_enter);
    view_set_exit_callback(instance->view, playground_main_view_exit);

    with_view_model(
        instance->view, (PlayGroundMainViewModel * model) {
            model->index = 0;
            model->items = malloc(sizeof(PlaygroundText));
            PlaygroundTextItemArray_init(model->items->data);
            return true;
        })

    return instance;
}

void playground_main_view_free(PlayGroundMainView* instance) {
    furi_assert(instance);

    with_view_model(
        instance->view, (PlayGroundMainViewModel * model) {
                for
                    M_EACH(item_menu, model->items->data, PlaygroundTextItemArray_t) {
                        furi_string_free(item_menu->item_str);
                        item_menu->type = 0;
                    }
                PlaygroundTextItemArray_clear(model->items->data);
                free(model->items);
                return false;
        })

    view_free(instance->view);
    free(instance);
}

View* playground_main_view_get_view(PlayGroundMainView* instance) {
    furi_assert(instance);
    return instance->view;
}

void playground_main_view_set_index(PlayGroundMainView* instance, uint8_t idx) {
    furi_assert(instance);
    furi_assert(idx);
    with_view_model(
        instance->view, (PlayGroundMainViewModel * model) {
            model->index = idx;
            return true;
        })
}

void playground_main_view_add_item(PlayGroundMainView* instance, const char* name, uint8_t type) {
    furi_assert(instance);
    with_view_model(
        instance->view, (PlayGroundMainViewModel * model) {
            PlaygroundTextItem* item_menu = PlaygroundTextItemArray_push_raw(model->items->data);
            item_menu->item_str = furi_string_alloc_printf("%s", name);
            item_menu->type = type;
            model->index++;

            return true;
        })
}

uint8_t playground_main_view_get_index(PlayGroundMainView* instance) {
    furi_assert(instance);

    uint8_t idx = 0;
    with_view_model(
        instance->view, (PlayGroundMainViewModel * model) {
            idx = model->index;
            return false;
        })

    return idx;
}