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

    class Root;

    template<class Self, class Props>
    class Component;

    template<class Props = void>
    class Element;

    enum Flags : std::uint8_t {
        Intrinsic = 1 << 0,
        Portal = 1 << 1,
        Fragment = 1 << 2,
    };

    void render(Element<> &elem, Root &root);

    template<class Component, class... E>
    Element<typename Component::_props>
    createElement(detail::TypeRef<Component>, typename Component::_props props, E ...children);

    template<class T>
    Element<> intrinsic(std::shared_ptr<T> props, std::uint8_t flags = 0);

    // impl

    class Top {
    public:
        static void render(Element<> &elem, Root *root, bool force = false);
    };

    class Root {
        std::map<std::string, Element<>> elements;
        std::map<std::string, Component<void, void> *> components;
    protected:
        const char *name;
        std::size_t size;
    public:

        virtual ~Root() = default;

        Component<void, void> *lookup(Element<> &_elem, bool &added);

        virtual void beginChildren()
        {}

        virtual void push(void *) = 0;

        virtual void endChildren()
        {}

        Root *newroot(Component<void, void> &instance, void *props);

    protected:

        virtual void init(Root &self, void *props) = 0;

    };

    template<class Self, class Props>
    class Component {
        friend class Top;

        friend class Root;

    public:
        using _self = Self;
        using _props = Props;
        static constexpr detail::TypeRef<_self> T{};

    protected:
        detail::TypeRef<void> *type;
        const Props *props;

        // componentWillUnmount
        virtual ~Component() = default;

        virtual rxui::Element<> render() = 0;

        // fixme: don't like this
        virtual void componentWillMount(Root *root)
        {
            (void) root;
        }

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

    template<class Props>
    class Element {
    public:
        std::string key;
        std::shared_ptr<Props> props;
        std::vector<Element<>> children;
        detail::TypeRef<void> type;
        std::uint8_t _flags = 0;

        operator Element<> &() // NOLINT
        {
            return *reinterpret_cast<Element<> *>(this);
        }
    };

    template<class Component, class... E>
    Element<typename Component::_props>
    createElement(detail::TypeRef<Component>, typename Component::_props props, E... children)
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
    Element<> intrinsic(std::shared_ptr<T> props, uint8_t flags)
    {
        return make(Element<T>, {
            $._flags = Flags::Intrinsic | flags;
            $.key = "";
            $.props = props;
            $.children = {};
            $.type = detail::TypeRef<void>{};
        });
    }
}
