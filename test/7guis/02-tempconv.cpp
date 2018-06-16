#include <sstream>

struct State {
    float celsius = 5;
};

#include "00-common.h"

namespace temp {
    static float c_to_f(float c)
    {
        return c * (9.0f / 5) + 32;
    }

    static float f_to_c(float f)
    {
        return (f - 32) * (5.0f / 9);
    }
}

static rxui::Element<> render(State &state, std::function<void()> &refresh)
{
    return rxui::createElement(
            HBox::T, make(HBoxProps, {}),
            rxui::createElement(Entry::T, make(EntryProps, {
                std::ostringstream oss;
                oss << std::noshowpoint << state.celsius;
                $.text = oss.str();
                $.onChange = [&](std::string s) {
                    try {
                        auto val = std::stof(s);
                        state.celsius = val;
                        refresh();
                    } catch (std::invalid_argument &) {
                        // ignore
                    }
                };
            })),
            rxui::createElement(Label::T, make(LabelProps, {
                $.text = "Celsius =";
            })),
            rxui::createElement(Entry::T, make(EntryProps, {
                std::ostringstream oss;
                oss << std::noshowpoint << temp::c_to_f(state.celsius);
                $.text = oss.str();
                $.onChange = [&](std::string s) {
                    try {
                        auto val = std::stof(s);
                        state.celsius = temp::f_to_c(val);
                        refresh();
                    } catch (std::invalid_argument &) {
                        // ignore
                    }
                };
            })),
            rxui::createElement(Label::T, make(LabelProps, {
                $.text = "Fahrenheit";
            }))
    );
}
