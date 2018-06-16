#pragma once

#include "rxui.h"

#include <vector>

class GtkRoot : public rxui::Root {
public:
    void *container;

    explicit GtkRoot(void *container);

    void init(Root &self, void *props) override;

    void beginChildren() override;

    void push(void *it) override;

    void endChildren() override;
};

template<class Self, class Props>
class NativeComponent : public rxui::Component<Self, Props> {
};

using gulong = unsigned long;

struct ComboState
{
    gulong changed;
};

struct EntryState
{
    gulong changed;
};

struct ListState
{
    gulong changed;
};

#include "rxui-widgets-gtk.h"
