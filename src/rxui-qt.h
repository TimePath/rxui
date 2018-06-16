#pragma once

#include "rxui.h"

class QtRoot : public rxui::Root {
public:
    void *container;

    explicit QtRoot(void *container);

    void init(Root &self, void *props) override;

    void push(void *it) override;
};

template<class Self, class Props>
class NativeComponent : public rxui::Component<Self, Props> {
};

struct ComboState {
};

struct EntryState {
};

struct ListState {
};

#include "rxui-widgets-gtk.h"
