#pragma once

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
        friend class Top;

    public:
        using _self = Self;
        using _props = Props;
        static constexpr detail::TypeRef<_self> T{};

    protected:
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
    protected:
        const char *name;
        std::size_t size;
    public:

        virtual ~Root() = default;

        Component<void, void> *lookup(Element<void> &elem)
        {
            // todo: re-use existing instances
            auto &T = elem.type;
            auto *mem = malloc(this->size + T.size());
            auto *obj = reinterpret_cast<void *>(reinterpret_cast<std::uint8_t *>(mem) + this->size);
            T.construct(obj);
            return reinterpret_cast<Component<void, void> *>(obj);
        };

        virtual void push(void *) = 0;

        Root &newroot(Component<void, void> &instance, void *props)
        {
            auto &root = *reinterpret_cast<Root *>(reinterpret_cast<std::uint8_t *>(&instance) - size);
            init(root, props);
            return root;
        }

    protected:

        virtual void init(Root &self, void *props) = 0;
    };

    class Top {
    public:
        static void render(Element<void> &_elem, Root *root, bool force = false)
        {
            // copy to heap
            // todo: track these
            Element<void> &elem = *new Element<void>(_elem);
            auto &it = *root->lookup(elem);
            it.props = elem.props.get();
            it.componentWillMount(root);

            const bool first = true;
            if (!force && !first && !it.shouldComponentUpdate()) {
                return;
            }

            auto next = it.render();
            void *nextProps = next.props.get();
            if (next._flags & Flags::Intrinsic) {
                root->push(nextProps);
            }
            if (next._flags & Flags::Portal) {
                root = &root->newroot(it, nextProps);
            }
            if (next._flags & Flags::Fragment) {
                for (auto &it : elem.children) { // fixme: buttonBox not passing children
                    render(it, root, force);
                }
            }
            if (first) {
                it.componentDidMount();
            } else {
                it.componentDidUpdate();
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
        Top::render(elem, &root);
    }
}
