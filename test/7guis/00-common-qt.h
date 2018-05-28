#pragma once

#include "../../src/rxui-qt.h"

static void activate(QApplication *app);

int main(int argc, char **argv)
{
    QApplication app{argc, argv};
    activate(&app);
    int status = app.exec();
    return status;
}

static rxui::Element<void> render(State &state, std::function<void()> &refresh);

static void activate(QApplication *)
{
    auto window = new QMainWindow;
    window->setWindowTitle("Window");

    auto rootLayout = new QFormLayout;
    auto w = new QWidget;
    w->setLayout(rootLayout);
    window->setCentralWidget(w);

    auto state = new State;
    auto root = new QtRoot(rootLayout);
    auto refresh = new std::function<void()>{};
    *refresh = [=]() {
        while (rootLayout->count()) {
            auto item = rootLayout->takeAt(0);
            delete item->widget();
            delete item;
        }
        auto element = render(*state, *refresh);
        rxui::render(element, *root);
    };
    (*refresh)();
    auto _refresh = new std::function<void()>{*refresh};
    *refresh = [=]() { QTimer::singleShot(0, *_refresh); };
    window->show();
}
