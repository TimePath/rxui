#pragma once

#include "rxui.h"

class TkRoot : public rxui::Root {
public:
    void *container;

    explicit TkRoot(void *container);

    void init(Root &self, void *props) override;

    void beginChildren() override;

    void push(void *it) override;

    void endChildren() override;
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
