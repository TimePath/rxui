#include "rxui.h"

namespace rxui {
    FILE *debug = stdout;

    void Top::render(Element<> &elem, Root *root, bool force)
    {
        bool first = true;
        auto &component = *root->lookup(elem, first);

        fprintf(debug, "force: %d, first: %d, update: %d\n", force, first, component.shouldComponentUpdate());
        if (!force && !first && !component.shouldComponentUpdate()) {
            return;
        }

        auto next = component.render();
        void *nextProps = next.props.get();
        if (next._flags & Flags::Intrinsic) {
            root->push(nextProps);
        }
        if (next._flags & Flags::Portal) {
            root = root->newroot(component, nextProps);
        }
        if (next._flags & Flags::Fragment) {
            root->beginChildren();
            for (auto &it : elem.children) {
                render(it, root, force);
            }
            root->endChildren();
        }
        if (first) {
            component.componentDidMount();
        } else {
            component.componentDidUpdate();
        }
    }

    struct ComponentState {
        bool rootInit;
    };

    Component<void, void> *Root::lookup(Element<> &_elem, bool &added)
    {
        elements[_elem.key] = _elem;
        fprintf(debug, "lookup[%s]\n", _elem.key.c_str());
        Element<> &elem = elements[_elem.key];

        Component<void, void> *c;
        auto found = components.find(_elem.key);
        bool notEnd = found != components.end();
        bool typeMatch = notEnd && *found->second->type == _elem.type;
        fprintf(debug, "nend: %d, types: %d\n", notEnd, typeMatch);
        if (notEnd && typeMatch) {
            added = false;
            c = found->second;
            fprintf(debug, "re-used a %s\n", elem.type.info().name());
        } else {
            added = true;
            auto &T = elem.type;
            auto mem = malloc(sizeof(ComponentState) + this->size + T.size());
            auto priv = ComponentState{};
            priv.rootInit = false;
            *reinterpret_cast<ComponentState *>(mem) = priv;
            auto ptr = reinterpret_cast<std::uint8_t *>(mem) + sizeof(ComponentState) + this->size;
            c = reinterpret_cast<Component<void, void> *>(ptr);
            T.construct(c);
            components[_elem.key] = c;
        }
        c->type = &elem.type;
        c->props = elem.props.get();
        if (added) {
            fprintf(debug, "mount %s\n", elem.type.info().name());
            c->componentWillMount(this);
        }
        return c;
    }

    Root *Root::newroot(Component<void, void> &instance, void *props)
    {
        auto ptr = reinterpret_cast<std::uint8_t *>(&instance) - this->size - sizeof(ComponentState);
        auto root = reinterpret_cast<Root *>(ptr);
        if (!reinterpret_cast<ComponentState *>(ptr)->rootInit) {
            init(*root, props);
        }
        return root;
    }

    void render(Element<> &elem, Root &root)
    {
        root.beginChildren();
        Top::render(elem, &root);
        root.endChildren();
    }
}
