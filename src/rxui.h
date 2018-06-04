#pragma once

#include <map>
#include <memory>
#include <string>
#include <typeinfo>
#include <vector>

#define make(T, ...) ([&]() -> T { T ${}; __VA_ARGS__; return $; }())

namespace rxui {
    namespace detail {
        template<class T>
        struct TypeTraits {
            static constexpr const std::type_info *const info{&typeid(T)};

            static const std::size_t size = sizeof(T);

            static void construct(void *out)
            {
                new(reinterpret_cast<T *>(out)) T{};
            }
        };

        template<>
        struct TypeTraits<void> {
            static constexpr const std::type_info *const info{&typeid(void)};

            static const std::size_t size = 0;

            static constexpr void (*const construct)(void *out) = nullptr;
        };

        template<class T>
        class TypeRef {
        private:
            using Traits = TypeTraits<T>;
            const std::type_info *_info = Traits::info;
            std::size_t _size = Traits::size;

            void (*_construct)(void *out) = Traits::construct;

        public:
            operator const TypeRef<void>() const // NOLINT
            {
                return *reinterpret_cast<const TypeRef<void> *>(this);
            }

            const std::type_info &info() const
            {
                return *this->_info;
            }

            const std::size_t &size() const
            {
                return this->_size;
            }

            void construct(void *out) const
            {
                this->_construct(out);
            }

            bool operator==(TypeRef<void> &other)
            {
                return this->info() == other.info();
            }
        };
    }

    class Top;

    enum Flags : std::uint8_t {
        Intrinsic = 1 << 0,
        Portal = 1 << 1,
        Fragment = 1 << 2,
    };

    template<class Props>
    class Element {
    public:
        std::string key;
        std::shared_ptr<Props> props;
        std::vector<Element<void>> children;
        detail::TypeRef<void> type;
        std::uint8_t _flags;

        operator Element<void> &()
        {
            return *reinterpret_cast<Element<void> *>(this);
        }
    };

    class Root;

    template<class Self, class Props>
    class Component {
        friend class Root;

        friend class Top;

    public:
        using _self = Self;
        using _props = Props;
        static constexpr detail::TypeRef<_self> T{};

    protected:
        detail::TypeRef<void> *type;
        const Props *props;

        // componentWillUnmount
        virtual ~Component() = default;

        virtual rxui::Element<void> render() = 0;

        // fixme: don't like this
        virtual void componentWillMount(Root *root)
        {
            (void) root;
        };

        void componentDidMount()
        {
            // after initial render
        }

        bool shouldComponentUpdate()
        {
            return true;
        }

        void componentDidUpdate()
        {
            // after update render
        }

    };

    class Root {
        std::map<std::string, Element<void>> elements;
        std::map<std::string, Component<void, void> *> components;
    protected:
        const char *name;
        std::size_t size;
    public:

        virtual ~Root() = default;

        struct ComponentState {
            bool rootInit;
        };

        Component<void, void> *lookup(Element<void> &_elem, bool &added)
        {
            elements[_elem.key] = _elem;
            printf("lookup[%s]\n", _elem.key.c_str());
            Element<void> &elem = elements[_elem.key];

            Component<void, void> *c;
            auto found = components.find(_elem.key);
            bool notEnd = found != components.end();
            bool typeMatch = notEnd && *found->second->type == _elem.type;
            printf("nend: %d, types: %d\n", notEnd, typeMatch);
            if (notEnd && typeMatch) {
                added = false;
                c = found->second;
                printf("re-used a %s\n", elem.type.info().name());
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
                printf("mount %s\n", elem.type.info().name());
                c->componentWillMount(this);
            }
            return c;
        };

        virtual void beginChildren()
        {};

        virtual void push(void *) = 0;

        virtual void endChildren()
        {};

        Root *newroot(Component<void, void> &instance, void *props)
        {
            auto ptr = reinterpret_cast<std::uint8_t *>(&instance) - this->size - sizeof(ComponentState);
            auto root = reinterpret_cast<Root *>(ptr);
            if (!reinterpret_cast<ComponentState *>(ptr)->rootInit) {
                init(*root, props);
            }
            return root;
        }

    protected:

        virtual void init(Root &self, void *props) = 0;

    };

    class Top {
    public:
        static void render(Element<void> &elem, Root *root, bool force = false)
        {
            bool first = true;
            auto &component = *root->lookup(elem, first);

            printf("force: %d, first: %d, update: %d\n", force, first, component.shouldComponentUpdate());
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
                for (auto &it : elem.children) { // fixme: buttonBox not passing children
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
    };

    template<class Component, class... E>
    Element<typename Component::_props>
    createElement(detail::TypeRef<Component>, typename Component::_props props, E ...children)
    {
        return make(Element<typename Component::_props>, {
            $.key = "";
            $.props = std::make_shared<typename Component::_props>(props);
            $.children = {children...};
            for (std::size_t i = 0; i < $.children.size(); ++i) {
                auto &it = $.children[i];
                if (it.key == "") {
                    it.key = std::to_string(i);
                }
            }
            $.type = Component::T;
        });
    }

    template<class T>
    Element<void> intrinsic(std::shared_ptr<T> props, std::uint8_t flags = 0)
    {
        return make(Element<T>, {
            $._flags = Flags::Intrinsic | flags;
            $.key = "";
            $.props = props;
            $.children = {};
            $.type = detail::TypeRef<void>{};
        });
    }

    void render(Element<void> &elem, Root &root)
    {
        root.beginChildren();
        Top::render(elem, &root);
        root.endChildren();
    }
}
