#include <string>
#include <vector>
#include <algorithm>

struct State {
    struct Row {
        std::size_t id;
        std::string name;
        std::string surname;
    };
    std::size_t autoid = 0;
    Row active = {};
    std::vector<Row> rows = {
            {++autoid, "Hans",  "Emil"},
            {++autoid, "Max",   "Mustermann"},
            {++autoid, "Tisch", "Roman"},
    };
    std::string filter;
};

#include "00-common.h"

static rxui::Element<void> render(State &state, std::function<void()> &refresh)
{
    return rxui::createElement(
            VBox::T, make(VBoxProps, {}),
            rxui::createElement(
                    HBox::T, make(HBoxProps, {}),
                    rxui::createElement(Label::T, make(LabelProps, {
                        $.text = "Filter prefix:";
                    })),
                    rxui::createElement(Entry::T, make(EntryProps, {
                        $.text = state.filter;
                        $.onChange = [&](std::string value) {
                            state.filter = value;
                            refresh();
                        };
                    }))
            ),
            rxui::createElement(
                    HBox::T, make(HBoxProps, {}),
                    rxui::createElement(
                            Scrolled::T, make(ScrolledProps, {}),
                            rxui::createElement(List::T, make(ListProps, {
                                $.selected = std::to_string(state.active.id);
                                $.onChange = [&](std::string value) {
                                    state.active.id = std::stoul(value);
                                    refresh();
                                };
                                for (auto &it : state.rows) {
                                    if (it.surname.find(state.filter) == 0) {
                                        $.children.push_back({std::to_string(it.id), it.surname + ", " + it.name});
                                    }
                                }
                            }))
                    ),
                    rxui::createElement(
                            VBox::T, make(VBoxProps, {}),
                            rxui::createElement(
                                    HBox::T, make(HBoxProps, {}),
                                    rxui::createElement(Label::T, make(LabelProps, {
                                        $.text = "Name:";
                                    })),
                                    rxui::createElement(Entry::T, make(EntryProps, {
                                        $.text = state.active.name;
                                        $.onChange = [&](std::string value) {
                                            state.active.name = value;
                                            refresh();
                                        };
                                    }))
                            ),
                            rxui::createElement(
                                    HBox::T, make(HBoxProps, {}),
                                    rxui::createElement(Label::T, make(LabelProps, {
                                        $.text = "Surname:";
                                    })),
                                    rxui::createElement(Entry::T, make(EntryProps, {
                                        $.text = state.active.surname;
                                        $.onChange = [&](std::string value) {
                                            state.active.surname = value;
                                            refresh();
                                        };
                                    }))
                            )
                    )
            ),
            rxui::createElement(
                    HBox::T, make(HBoxProps, {}),
                    rxui::createElement(Button::T, make(ButtonProps, {
                        $.label = "Create";
                        $.onClick = [&]() {
                            state.active.id = ++state.autoid;
                            state.rows.push_back(state.active);
                            refresh();
                        };
                    })),
                    rxui::createElement(Button::T, make(ButtonProps, {
                        $.label = "Update";
                        $.onClick = [&]() {
                            auto it = std::find_if(std::begin(state.rows), std::end(state.rows), [&](State::Row &it) {
                                return it.id == state.active.id;
                            });
                            if (it != std::end(state.rows)) {
                                *it = state.active;
                                refresh();
                            }
                        };
                    })),
                    rxui::createElement(Button::T, make(ButtonProps, {
                        $.label = "Delete";
                        $.onClick = [&]() {
                            state.rows.erase(
                                    std::remove_if(std::begin(state.rows), std::end(state.rows), [&](State::Row &it) {
                                        return it.id == state.active.id;
                                    }),
                                    std::end(state.rows)
                            );
                            refresh();
                        };
                    }))
            )
    );
}
