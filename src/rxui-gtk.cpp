#include "rxui-gtk.h"

#include <algorithm>
#include <gtk/gtk.h>

GtkRoot::GtkRoot(void *container) : container{GTK_CONTAINER(container)}
{
    this->size = sizeof(GtkRoot);
    this->name = "GtkRoot";
}

void GtkRoot::init(rxui::Root &self, void *props)
{
    new(&self) GtkRoot(props);
}

void GtkRoot::beginChildren()
{
    auto list = gtk_container_get_children(GTK_CONTAINER(container));
    for (auto iter = list; iter; iter = g_list_next(iter)) {
        auto w = GTK_WIDGET(iter->data);
        refs.push_back(Ref{w, -1});
    }
    g_list_free(list);
}

void GtkRoot::push(void *it)
{
    auto w = GTK_WIDGET(it);
    gtk_widget_show(w);
    auto ref = std::find_if(std::begin(refs), std::end(refs), [&](Ref &it) {
        return it.w == w;
    });
    if (ref != std::end(refs)) {
        ref->status = 0;
    } else {
        refs.push_back(Ref{w, 1});
    }
}

void GtkRoot::endChildren()
{
    for (const auto &ref : refs) {
        if (ref.status < 0) {
            gtk_container_remove(GTK_CONTAINER(container), GTK_WIDGET(ref.w));
        } else if (ref.status > 0) {
            gtk_container_add(GTK_CONTAINER(container), GTK_WIDGET(ref.w));
        }
    }
    refs.clear();
}

HBox::HBox() : handle{gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0), gtk_widget_destroy}
{
}

rxui::Element<> HBox::render()
{
    auto self = this;
    return rxui::intrinsic(self->handle, rxui::Flags::Portal | rxui::Flags::Fragment);
}

VBox::VBox() : handle{gtk_box_new(GTK_ORIENTATION_VERTICAL, 0), gtk_widget_destroy}
{
}

rxui::Element<> VBox::render()
{
    auto self = this;
    return rxui::intrinsic(self->handle, rxui::Flags::Portal | rxui::Flags::Fragment);
}

Scrolled::Scrolled() : handle{gtk_scrolled_window_new(nullptr, nullptr), gtk_widget_destroy}
{
}

rxui::Element<> Scrolled::render()
{
    auto self = this;
    auto handle = GTK_SCROLLED_WINDOW(self->handle.get());
    gtk_scrolled_window_set_policy(handle, GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
    return rxui::intrinsic(self->handle, rxui::Flags::Portal | rxui::Flags::Fragment);
}

Button::Button() : handle{gtk_button_new(), gtk_widget_destroy}
{
    auto self = this;
    auto handle = GTK_BUTTON(self->handle.get());
    g_signal_connect(handle, "clicked", G_CALLBACK(+[](GtkWidget *, Button *self) {
        if (self->props->onClick) {
            self->props->onClick();
        }
    }), self);
}

rxui::Element<> Button::render()
{
    auto self = this;
    auto handle = GTK_BUTTON(self->handle.get());
    gtk_widget_set_sensitive(GTK_WIDGET(handle), !self->props->disabled);
    gtk_button_set_label(handle, self->props->label.c_str());
    return rxui::intrinsic(self->handle);
}

Combo::Combo() : handle{gtk_combo_box_new(), gtk_widget_destroy}
{
    auto self = this;
    auto handle = GTK_COMBO_BOX(self->handle.get());
    g_object_ref(handle); // need an extra ref
    self->changed = g_signal_connect(handle, "changed", G_CALLBACK(+[](GtkComboBox *handle, Combo *self) {
        auto value = gtk_combo_box_get_active_id(handle);
        g_signal_handler_block(handle, self->changed);
        gtk_combo_box_set_active_id(handle, self->props->selected.c_str());
        g_signal_handler_unblock(handle, self->changed);
        if (self->props->onChange) {
            self->props->onChange(value);
        }
    }), self);

    auto layout = GTK_CELL_LAYOUT(handle);
    auto renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(layout, renderer, TRUE);
    gtk_cell_layout_add_attribute(layout, renderer, "text", 1);
}

rxui::Element<> Combo::render()
{
    auto self = this;
    auto handle = GTK_COMBO_BOX(self->handle.get());

    auto store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
    for (auto &it : this->props->children) {
        gtk_list_store_insert_with_values(store, nullptr, -1,
                                          0, it.value.c_str(),
                                          1, it.label.c_str(),
                                          -1);
    }
    gtk_combo_box_set_model(handle, GTK_TREE_MODEL(store));
    g_object_unref(store);
    gtk_combo_box_set_id_column(handle, 0);

    g_signal_handler_block(handle, self->changed);
    gtk_combo_box_set_active_id(handle, self->props->selected.c_str());
    g_signal_handler_unblock(handle, self->changed);

    return rxui::intrinsic(self->handle);
}

Entry::Entry() : handle{gtk_entry_new(), gtk_widget_destroy}
{
    auto self = this;
    auto handle = GTK_ENTRY(self->handle.get());
    self->changed = g_signal_connect(handle, "changed", G_CALLBACK(+[](GtkEntry *handle, Entry *self) {
        auto s = std::string{gtk_entry_get_text(handle)};
        g_signal_handler_block(handle, self->changed);
        gtk_entry_set_text(handle, self->props->text.c_str());
        g_signal_handler_unblock(handle, self->changed);
        if (self->props->onChange) {
            self->props->onChange(s);
        }
    }), self);
}

rxui::Element<> Entry::render()
{
    auto self = this;
    auto handle = GTK_ENTRY(self->handle.get());
    gtk_widget_set_sensitive(GTK_WIDGET(handle), !self->props->disabled);
    g_signal_handler_block(handle, self->changed);
    gtk_entry_set_text(handle, self->props->text.c_str());
    g_signal_handler_unblock(handle, self->changed);
    GdkRGBA *background = nullptr;
    GdkRGBA rgba;
    if (!self->props->background.empty()) {
        gdk_rgba_parse(&rgba, self->props->background.c_str());
        background = &rgba;
    }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    gtk_widget_override_background_color(GTK_WIDGET(handle), GTK_STATE_FLAG_NORMAL, background);
#pragma GCC diagnostic pop
    return rxui::intrinsic(self->handle);
}

Label::Label() : handle{gtk_label_new(nullptr), gtk_widget_destroy}
{
}

rxui::Element<> Label::render()
{
    auto self = this;
    auto handle = GTK_LABEL(self->handle.get());
    gtk_label_set_text(handle, self->props->text.c_str());
    return rxui::intrinsic(self->handle);
}

List::List() : handle{gtk_tree_view_new(), gtk_widget_destroy}
{
    auto self = this;
    auto handle = GTK_TREE_VIEW(self->handle.get());
    auto selection = gtk_tree_view_get_selection(handle);
    self->changed = g_signal_connect(selection, "changed", G_CALLBACK(+[](GtkTreeSelection *selection, List *self) {
        GtkTreeIter iter;
        GtkTreeModel *model;
        if (!gtk_tree_selection_get_selected(selection, &model, &iter)) {
            return;
        }
        const char *value;
        gtk_tree_model_get(model, &iter, 0, &value, -1);
        if (self->props->onChange) {
            self->props->onChange(std::string{value});
        }
    }), self);

    auto renderer = gtk_cell_renderer_text_new();
    auto column = gtk_tree_view_column_new_with_attributes("", renderer, "text", 1, nullptr);
    gtk_tree_view_append_column(handle, column);
}

rxui::Element<> List::render()
{
    auto self = this;
    auto handle = GTK_TREE_VIEW(self->handle.get());

    auto selection = gtk_tree_view_get_selection(handle);
    gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);

    auto store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
    int active = -1;
    int i = -1;
    for (auto &it : this->props->children) {
        i += 1;
        GtkTreeIter iter;
        gtk_list_store_insert_with_values(store, &iter, -1,
                                          0, it.value.c_str(),
                                          1, it.label.c_str(),
                                          -1);
        if (active < 0 && it.value == this->props->selected) {
            active = i;
        }
    }
    gtk_tree_view_set_model(handle, GTK_TREE_MODEL(store));
    g_object_unref(store);

    if (active >= 0) {
        auto path = gtk_tree_path_new_from_indices(active, -1);
        g_signal_handler_block(selection, self->changed);
        gtk_tree_view_set_cursor(handle, path, nullptr, false);
        g_signal_handler_unblock(selection, self->changed);
        gtk_tree_path_free(path);
    }

    gtk_tree_view_set_headers_visible(handle, false);

    return rxui::intrinsic(self->handle);
}
