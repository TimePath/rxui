struct State {
    int count = 0;
};

#include "00-common.h"

static rxui::Element<> render(State &state, std::function<void()> &refresh)
{
    return rxui::createElement(
            HBox::T, make(HBoxProps, {}),
            rxui::createElement(Label::T, make(LabelProps, {
                $.text = std::to_string(state.count);
            })),
            rxui::createElement(Button::T, make(ButtonProps, {
                $.label = "Count";
                $.onClick = [&]() {
                    state.count += 1;
                    refresh();
                };
            }))
    );
}
