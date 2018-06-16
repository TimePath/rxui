#pragma once

#include "../../src/rxui-efl.h"

#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT

#include <Elementary.h>

EAPI_MAIN void efl_main(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED);

EFL_MAIN()

static rxui::Element<> render(State &state, std::function<void()> &refresh);

EAPI_MAIN void efl_main(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
    int w = 800;
    int h = 600;
    elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
    auto window = efl_add(EFL_UI_WIN_CLASS, nullptr);
    efl_ui_win_autodel_set(window, EINA_TRUE);
    efl_text_set(window, "Window");
    evas_object_size_hint_min_set(window, w, h);
    evas_object_show(window);

    auto bx = efl_add(EFL_UI_BOX_CLASS, window);
    evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_win_resize_object_add(window, bx);
    evas_object_show(bx);

    auto state = new State;
    auto root = new EflRoot(bx);
    auto refresh = new std::function<void()>{};
    *refresh = [=]() {
        auto element = render(*state, *refresh);
        rxui::render(element, *root);
    };
    (*refresh)();
}
