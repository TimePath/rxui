#pragma once

#include "rxui.h"

#include <functional>

class EflRoot : public rxui::Root {
public:
    void *container;

    explicit EflRoot(void *container);

    void init(Root &self, void *props) override;

    void push(void *it) override;
};

typedef struct _Eo_Opaque Eo;

class Lazy {
    using T = Eo;
    std::function<T *(T *)> construct;
    T *val = nullptr;
public:
    explicit Lazy(std::function<T *(T *)> construct);

    T *get(void *parent);
};

template<class Self, class Props>
class NativeComponent : public rxui::Component<Self, Props> {
    void componentWillMount(rxui::Root *root) override
    {
        reinterpret_cast<Lazy *>(static_cast<Self *>(this)->handle.get())->get(
                dynamic_cast<EflRoot *>(root)->container
        );
    }
};

typedef struct _Elm_Gen_Item_Class Elm_Gen_Item_Class;
typedef Elm_Gen_Item_Class Elm_Genlist_Item_Class;

struct ComboState {
    Elm_Genlist_Item_Class *itc;
};

struct EntryState {
    bool changing = false;
};

struct ListState {
};

#include "rxui-widgets-gtk.h"
