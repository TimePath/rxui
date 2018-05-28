#pragma once

#include "../../src/rxui-gtk.h"

static void activate(GtkApplication *app);

int main(int argc, char **argv)
{
    auto app = G_APPLICATION(gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE));
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(app, argc, argv);
    g_object_unref(app);
    return status;
}

static rxui::Element<void> render(State &state, std::function<void()> &refresh);

static void activate(GtkApplication *app)
{
    auto window = GTK_WINDOW(gtk_application_window_new(app));
    gtk_window_set_title(window, "Window");
    gtk_window_set_default_size(window, 0, 0);
    gtk_window_set_position(window, GTK_WIN_POS_CENTER);

    auto state = new State;
    auto root = new GtkRoot(GTK_CONTAINER(window));
    auto refresh = new std::function<void()>{};
    *refresh = [=]() {
        auto parent = root->container;
        auto list = gtk_container_get_children(parent);
        if (list) {
            gtk_container_remove(parent, GTK_WIDGET(list->data));
        }

        auto element = render(*state, *refresh);
        rxui::render(element, *root);
        gtk_widget_show_all(GTK_WIDGET(parent));
    };
    (*refresh)();
}
