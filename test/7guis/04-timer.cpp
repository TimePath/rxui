#include <iostream>
#include <cstring>

struct State {
};

#include "00-common.h"

static rxui::Element<> render(State &state, std::function<void()> &refresh)
{
    return rxui::createElement(VBox::T, make(VBoxProps, {}));
}
