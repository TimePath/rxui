#include <iostream>
#include <cstring>

struct State {
    bool oneway = true;
    std::string t0 = "27.03.2014";
    std::string t1 = "27.03.2014";
};

#include "00-common.h"

static bool is_date(const std::string &s)
{
    return (
            s.length() == 2 + 1 + 2 + 1 + 4
            && isdigit(s[0]) && isdigit(s[1])
            && '.' == s[2]
            && isdigit(s[3]) && isdigit(s[4])
            && '.' == s[5]
            && isdigit(s[6]) && isdigit(s[7]) && isdigit(s[8]) && isdigit(s[9])
    );
}

static rxui::Element<void> render(State &state, std::function<void()> &refresh)
{
    return rxui::createElement(
            VBox::T, make(VBoxProps, {}),
            rxui::createElement(Combo::T, make(ComboProps, {
                $.selected = state.oneway ? "0" : "1";
                $.onChange = [&](std::string value) {
                    state.oneway = value == "0";
                    refresh();
                };
                $.children = {
                        {"0", "one-way flight"},
                        {"1", "return flight"},
                };
            })),
            rxui::createElement(Entry::T, make(EntryProps, {
                $.text = state.t0;
                $.background = is_date(state.t0) ? "" : "#f00";
                $.onChange = [&](std::string s) {
                    state.t0 = s;
                    refresh();
                };
            })),
            rxui::createElement(Entry::T, make(EntryProps, {
                $.text = state.t1;
                $.disabled = state.oneway;
                $.background = state.oneway || is_date(state.t1) ? "" : "#f00";
                $.onChange = [&](std::string s) {
                    state.t1 = s;
                    refresh();
                };
            })),
            rxui::createElement(Button::T, make(ButtonProps, {
                $.label = "Book";
                $.disabled = !(is_date(state.t0) && is_date(state.t1));
                $.onClick = [&]() {
                    auto kind = state.oneway ? "one-way flight" : "return flight";
                    auto date = state.t0;
                    std::cout << "You have booked a " << kind << " on " << date << "." << std::endl;
                };
            }))
    );
}
