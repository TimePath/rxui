#include "rxui-efl.h"

#include <algorithm>

#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT
// #define EFL_NOLEGACY_API_SUPPORT

#include <Elementary.h>


EflRoot::EflRoot(void *container) : container{container}
{
    this->size = sizeof(EflRoot);
    this->name = "EflRoot";
}

void EflRoot::init(rxui::Root &self, void *props)
{
    auto lazy = static_cast<Lazy *>(props);
    auto w = lazy->get(reinterpret_cast<Eo *>(container));
    new(&self) EflRoot(w);
}

void EflRoot::beginChildren()
{
    auto self = reinterpret_cast<Eo *>(container);
    Eo *data;
    auto itr = efl_content_iterate(self);
    EINA_ITERATOR_FOREACH(itr, data) {
        refs.push_back(Ref{data, -1});
    }
    eina_iterator_free(itr);
}

void EflRoot::push(void *it)
{
    auto self = reinterpret_cast<Eo *>(container);
    auto w = static_cast<Lazy *>(it)->get(self);
    evas_object_show(w);
    auto ref = std::find_if(std::begin(refs), std::end(refs), [&](Ref &it) {
        return it.w == w;
    });
    if (ref != std::end(refs)) {
        ref->status = 0;
    } else {
        refs.push_back(Ref{w, 1});
    }
}

void EflRoot::endChildren()
{
    auto self = reinterpret_cast<Eo *>(container);
    for (const auto &ref : refs) {
        auto w = reinterpret_cast<Eo *>(ref.w);
        if (ref.status < 0) {
            efl_pack_unpack(self, w);
        } else if (ref.status > 0) {
            if (efl_isa(self, ELM_SCROLLER_CLASS)) {
                elm_object_content_set(self, w);
            } else {
                efl_pack(self, w);
            }
        }
    }
    refs.clear();
}

Lazy::Lazy(std::function<Eo * (Eo * )> construct) : construct{std::move(construct)}
{
}

Eo *Lazy::get(void *parent)
{
    if (!val) {
        val = construct(reinterpret_cast<Eo *>(parent));
    }
    return val;
}

HBox::HBox() : handle{std::make_shared<Lazy>([](Eo *parent) {
    return efl_add(EFL_UI_BOX_CLASS, parent);
})}
{}

rxui::Element<> HBox::render()
{
    auto self = this;
    auto handle = reinterpret_cast<Lazy *>(self->handle.get())->get(nullptr);
    efl_orientation_set(handle, EFL_ORIENT_RIGHT);
    return rxui::intrinsic(self->handle, rxui::Flags::Portal | rxui::Flags::Fragment);
}

VBox::VBox() : handle{std::make_shared<Lazy>([](Eo *parent) {
    return efl_add(EFL_UI_BOX_CLASS, parent);
})}
{}

rxui::Element<> VBox::render()
{
    auto self = this;
    auto handle = reinterpret_cast<Lazy *>(self->handle.get())->get(nullptr);
    efl_orientation_set(handle, EFL_ORIENT_DOWN);
    return rxui::intrinsic(self->handle, rxui::Flags::Portal | rxui::Flags::Fragment);
}

Scrolled::Scrolled() : handle{std::make_shared<Lazy>([](Eo *parent) {
    return efl_add(EFL_UI_BOX_FLOW_CLASS, parent);
})}
{}

rxui::Element<> Scrolled::render()
{
    auto self = this;
    auto handle = reinterpret_cast<Lazy *>(self->handle.get())->get(nullptr);
    (void) handle;
    return rxui::intrinsic(self->handle, rxui::Flags::Portal | rxui::Flags::Fragment);
}

Button::Button() : handle{std::make_shared<Lazy>([this](Eo *parent) {
    auto x = efl_add(EFL_UI_BUTTON_CLASS, parent);
    efl_event_callback_add(x, EFL_UI_EVENT_CLICKED, +[](void *data, const Efl_Event *event EINA_UNUSED) {
        auto self = static_cast<Button *>(data);
        if (self->props->onClick) {
            self->props->onClick();
        }
    }, this);
    return x;
})}
{}

rxui::Element<> Button::render()
{
    auto self = this;
    auto handle = reinterpret_cast<Lazy *>(self->handle.get())->get(nullptr);
    efl_text_set(handle, self->props->label.c_str());
    return rxui::intrinsic(self->handle);
}

Combo::Combo() : handle{std::make_shared<Lazy>([this](Eo *parent) {
    auto x = efl_add(ELM_COMBOBOX_CLASS, parent);
    evas_object_size_hint_weight_set(x, EVAS_HINT_EXPAND, 0);
    evas_object_size_hint_align_set(x, EVAS_HINT_FILL, 0);
    elm_obj_entry_editable_set(x, EINA_FALSE);

    efl_event_callback_add(x, ELM_COMBOBOX_EVENT_ITEM_PRESSED, +[](void *data, const Efl_Event *event) {
        auto self = static_cast<Combo *>(data);
        auto handle = reinterpret_cast<Lazy *>(self->handle.get())->get(nullptr);

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
})}
{
    itc = elm_genlist_item_class_new();
    itc->func.text_get = [](void *data, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED) {
        auto it = static_cast<ComboProps::Option *>(data);
        return strdup(it->label.c_str());
    };
}

rxui::Element<> Combo::render()
{
    auto self = this;
    auto handle = reinterpret_cast<Lazy *>(self->handle.get())->get(nullptr);

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

Entry::Entry() : handle{std::make_shared<Lazy>([this](Eo *parent) {
    auto x = efl_add(EFL_UI_TEXT_EDITABLE_CLASS, parent);
    bool boxhack = false;
    if (boxhack) {
        efl_ui_text_scrollable_set(x, EINA_TRUE);
        evas_object_size_hint_min_set(x, 20, 20);
        evas_object_size_hint_max_set(x, 9999, 20);
    }

    auto f = +[](void *data, const Efl_Event *) {
        auto self = static_cast<Entry *>(data);
        auto handle = reinterpret_cast<Lazy *>(self->handle.get())->get(nullptr);
        if (self->changing) {
            return;
        }
        auto p = efl_loop_job(efl_loop_get(handle), nullptr);
        efl_future_then(p, +[](void *data, const Efl_Event *obj EINA_UNUSED) {
            auto self = static_cast<Entry *>(data);
            auto handle = reinterpret_cast<Lazy *>(self->handle.get())->get(nullptr);
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
})}
{}

rxui::Element<> Entry::render()
{
    auto self = this;
    auto handle = reinterpret_cast<Lazy *>(self->handle.get())->get(nullptr);
    elm_object_disabled_set(handle, self->props->disabled ? EINA_TRUE : EINA_FALSE);
    changing = true;
    efl_text_set(handle, self->props->text.c_str());
    changing = false;
    return rxui::intrinsic(self->handle);
}

Label::Label() : handle{std::make_shared<Lazy>([](Eo *parent) {
    return efl_add(EFL_UI_TEXT_CLASS, parent);
})}
{}

rxui::Element<> Label::render()
{
    auto self = this;
    auto handle = reinterpret_cast<Lazy *>(self->handle.get())->get(nullptr);
    efl_text_set(handle, self->props->text.c_str());
    return rxui::intrinsic(self->handle);
}

List::List() : handle{std::make_shared<Lazy>([](Eo *parent) {
    return efl_add(ELM_LIST_CLASS, parent);
})}
{}

rxui::Element<> List::render()
{
    auto self = this;
    auto handle = reinterpret_cast<Lazy *>(self->handle.get())->get(nullptr);
    evas_object_size_hint_weight_set(handle, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(handle, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_obj_list_clear(handle);
    for (const auto &it : self->props->children) {
        elm_obj_list_item_append(handle, it.label.c_str(), nullptr, nullptr, nullptr, nullptr);
    }
    elm_obj_list_go(handle);
    return rxui::intrinsic(self->handle);
}
