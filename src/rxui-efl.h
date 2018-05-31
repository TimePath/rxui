#pragma once

#include <functional>

#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT
// #define EFL_NOLEGACY_API_SUPPORT

#include <Elementary.h>

#include "rxui.h"

using Eo_ptr = Eo *;

class LazyWidget {
    std::function<Eo_ptr(Eo_ptr)> construct;

    Eo *val = nullptr;

public:

    explicit LazyWidget(std::function<Eo_ptr(Eo_ptr)> construct) : construct{construct}
    {
    }

    Eo *get(Eo *parent)
    {
        if (!val) {
            val = construct(parent);
        }
        return val;
    }
};

class EflRoot : public rxui::Root {
public:
    Eo *container;

    explicit EflRoot(Eo *container) : container{container}
    {
        this->size = sizeof(EflRoot);
        this->name = "EflRoot";
    }

    void init(Root &self, void *props) override
    {
        auto lazy = static_cast<LazyWidget *>(props);
        auto w = lazy->get(container);
        new(&self) EflRoot(w);
    }

    void push(void *it) override
    {
        auto lazy = static_cast<LazyWidget *>(it);
        auto w = lazy->get(container);
        if (efl_isa(container, ELM_SCROLLER_CLASS)) {
            elm_object_content_set(container, w);
        } else {
            efl_pack(container, w);
        }
        evas_object_show(w);
    }
};

struct HBoxProps {
};

class HBox : public rxui::Component<HBox, HBoxProps> {
    const std::shared_ptr<LazyWidget> handle{init()};

    LazyWidget *init()
    {
        return new LazyWidget{[](Eo *parent) {
            return efl_add(EFL_UI_BOX_CLASS, parent);
        }};
    }

    void componentWillMount(rxui::Root *root) override
    {
        handle->get(((EflRoot *) root)->container);
    }

public:
    rxui::Element<void> render() override
    {
        auto self = this;
        auto handle = (&*self->handle)->get(nullptr);
        efl_orientation_set(handle, EFL_ORIENT_RIGHT);
        return rxui::intrinsic(self->handle, rxui::Flags::Portal | rxui::Flags::Fragment);
    }
};

struct VBoxProps {
};

class VBox : public rxui::Component<VBox, VBoxProps> {
    const std::shared_ptr<LazyWidget> handle{init()};

    LazyWidget *init()
    {
        return new LazyWidget{[](Eo *parent) {
            return efl_add(EFL_UI_BOX_CLASS, parent);
        }};
    }

    void componentWillMount(rxui::Root *root) override
    {
        handle->get(((EflRoot *) root)->container);
    }

public:
    rxui::Element<void> render() override
    {
        auto self = this;
        auto handle = (&*self->handle)->get(nullptr);
        efl_orientation_set(handle, EFL_ORIENT_DOWN);
        return rxui::intrinsic(self->handle, rxui::Flags::Portal | rxui::Flags::Fragment);
    }
};

struct ScrolledProps {
};

class Scrolled : public rxui::Component<Scrolled, ScrolledProps> {
    const std::shared_ptr<LazyWidget> handle{init()};

    LazyWidget *init()
    {
        return new LazyWidget{[](Eo *parent) {
            return efl_add(EFL_UI_BOX_FLOW_CLASS, parent);
        }};
    }

    void componentWillMount(rxui::Root *root) override
    {
        handle->get(((EflRoot *) root)->container);
    }

public:
    rxui::Element<void> render() override
    {
        auto self = this;
        auto handle = (&*self->handle)->get(nullptr);
        (void) handle;
        return rxui::intrinsic(self->handle, rxui::Flags::Portal | rxui::Flags::Fragment);
    }
};

struct ButtonProps {
    bool disabled;
    std::string label;
    std::function<void()> onClick;
};

class Button : public rxui::Component<Button, ButtonProps> {
    const std::shared_ptr<LazyWidget> handle{init()};

    LazyWidget *init()
    {
        return new LazyWidget{[this](Eo *parent) {
            auto x = efl_add(EFL_UI_BUTTON_CLASS, parent);
            efl_event_callback_add(x, EFL_UI_EVENT_CLICKED, +[](void *data, const Efl_Event *event EINA_UNUSED) {
                auto self = static_cast<Button *>(data);
                if (self->props->onClick) {
                    self->props->onClick();
                }
            }, this);
            return x;
        }};
    }

    void componentWillMount(rxui::Root *root) override
    {
        handle->get(((EflRoot *) root)->container);
    }

public:
    rxui::Element<void> render() override
    {
        auto self = this;
        auto handle = (&*self->handle)->get(nullptr);
        efl_text_set(handle, self->props->label.c_str());
        return rxui::intrinsic(self->handle);
    }
};

struct ComboProps {
    struct Option {
        std::string value;
        std::string label;
    };
    std::vector<Option> children;
    std::string selected;
    std::function<void(std::string)> onChange;
};

class Combo : public rxui::Component<Combo, ComboProps> {
    const std::shared_ptr<LazyWidget> handle{init()};

    LazyWidget *init()
    {
        return new LazyWidget{[this](Eo *parent) {
            auto x = efl_add(ELM_COMBOBOX_CLASS, parent);
            evas_object_size_hint_weight_set(x, EVAS_HINT_EXPAND, 0);
            evas_object_size_hint_align_set(x, EVAS_HINT_FILL, 0);
            elm_obj_entry_editable_set(x, EINA_FALSE);

            efl_event_callback_add(x, ELM_COMBOBOX_EVENT_ITEM_PRESSED, +[](void *data, const Efl_Event *event) {
                auto self = static_cast<Combo *>(data);
                auto handle = (&*self->handle)->get(nullptr);

                int idx = elm_obj_genlist_item_index_get((Elm_Genlist_Item *) event->info) - 1;
                auto &it = self->props->children[idx];
                elm_object_text_set(handle, it.label.c_str());
                elm_obj_entry_cursor_end_set(handle);
                elm_obj_combobox_hover_end(handle);
                if (self->props->onChange) {
                    self->props->onChange(it.value);
                }
            }, this);
            return x;
        }};
    }

    void componentWillMount(rxui::Root *root) override
    {
        handle->get(((EflRoot *) root)->container);
    }

    Elm_Genlist_Item_Class *itc = [] {
        auto itc = elm_genlist_item_class_new();
        itc->func.text_get = [](void *data, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED) {
            auto it = static_cast<ComboProps::Option *>(data);
            return strdup(it->label.c_str());
        };
        return itc;
    }();

public:
    rxui::Element<void> render() override
    {
        auto self = this;
        auto handle = (&*self->handle)->get(nullptr);

        elm_genlist_clear(handle);
        for (const auto &it : self->props->children) {
            auto item = elm_obj_genlist_item_append(handle, itc, &it, nullptr, ELM_GENLIST_ITEM_NONE, nullptr, nullptr);
            if (self->props->selected == it.value) {
                elm_object_text_set(handle, it.label.c_str());
                elm_obj_genlist_item_selected_set(item, EINA_TRUE);
            }
        }

        return rxui::intrinsic(self->handle);
    }
};

struct EntryProps {
    bool disabled;
    std::string text;
    std::function<void(std::string)> onChange;
    std::string background;
};

class Entry : public rxui::Component<Entry, EntryProps> {
    const std::shared_ptr<LazyWidget> handle{init()};
    bool changing = false;

    LazyWidget *init()
    {
        return new LazyWidget{[this](Eo *parent) {
            auto x = efl_add(EFL_UI_TEXT_EDITABLE_CLASS, parent);
            bool boxhack = false;
            if (boxhack) {
                efl_ui_text_scrollable_set(x, EINA_TRUE);
                evas_object_size_hint_min_set(x, 20, 20);
                evas_object_size_hint_max_set(x, 9999, 20);
            }

            auto f = +[](void *data, const Efl_Event *) {
                auto self = static_cast<Entry *>(data);
                auto handle = (&*self->handle)->get(nullptr);
                if (self->changing) {
                    return;
                }
                auto p = efl_loop_job(efl_loop_get(handle), nullptr);
                efl_future_then(p, +[](void *data, const Efl_Event *obj EINA_UNUSED) {
                    auto self = static_cast<Entry *>(data);
                    auto handle = (&*self->handle)->get(nullptr);
                    auto s = std::string{efl_text_get(handle)};
                    self->changing = true;
                    efl_text_set(handle, self->props->text.c_str());
                    self->changing = false;
                    if (self->props->onChange) {
                        self->props->onChange(s);
                    }
                }, nullptr, nullptr, data);
            };
            efl_event_callback_add(x, EFL_UI_TEXT_EVENT_CHANGED, f, this);
            efl_event_callback_add(x, EFL_UI_EVENT_SELECTION_CUT, f, this);
            efl_event_callback_add(x, EFL_UI_EVENT_SELECTION_PASTE, f, this);
            return x;
        }};
    }

    void componentWillMount(rxui::Root *root) override
    {
        handle->get(((EflRoot *) root)->container);
    }

public:
    rxui::Element<void> render() override
    {
        auto self = this;
        auto handle = (&*self->handle)->get(nullptr);
        elm_object_disabled_set(handle, self->props->disabled ? EINA_TRUE : EINA_FALSE);
        changing = true;
        efl_text_set(handle, self->props->text.c_str());
        changing = false;
        return rxui::intrinsic(self->handle);
    }
};

struct LabelProps {
    std::string text;
};

class Label : public rxui::Component<Label, LabelProps> {
    const std::shared_ptr<LazyWidget> handle{init()};

    LazyWidget *init()
    {
        return new LazyWidget{[](Eo *parent) {
            return efl_add(EFL_UI_TEXT_CLASS, parent);
        }};
    }

    void componentWillMount(rxui::Root *root) override
    {
        handle->get(((EflRoot *) root)->container);
    }

public:
    rxui::Element<void> render() override
    {
        auto self = this;
        auto handle = (&*self->handle)->get(nullptr);
        efl_text_set(handle, self->props->text.c_str());
        return rxui::intrinsic(self->handle);
    }
};

struct ListProps {
    struct Option {
        std::string value;
        std::string label;
    };
    std::vector<Option> children;
    std::string selected;
    std::function<void(std::string)> onChange;
};

class List : public rxui::Component<List, ListProps> {
    const std::shared_ptr<LazyWidget> handle{init()};

    LazyWidget *init()
    {
        return new LazyWidget{[](Eo *parent) {
            auto x = efl_add(ELM_LIST_CLASS, parent);
            return x;
        }};
    }

    void componentWillMount(rxui::Root *root) override
    {
        handle->get(((EflRoot *) root)->container);
    }

public:
    rxui::Element<void> render() override
    {
        auto self = this;
        auto handle = (&*self->handle)->get(nullptr);
        evas_object_size_hint_weight_set(handle, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(handle, EVAS_HINT_FILL, EVAS_HINT_FILL);
        elm_obj_list_clear(handle);
        for (const auto &it : self->props->children) {
            elm_obj_list_item_append(handle, it.label.c_str(), nullptr, nullptr, nullptr, nullptr);
        }
        elm_obj_list_go(handle);
        return rxui::intrinsic(self->handle);
    }
};
