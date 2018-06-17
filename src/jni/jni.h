#pragma once

#include <jni.h>
#include <cassert>

namespace compile {
#if defined NDEBUG
# define ASSERT(CHECK) void(0)
#else
# define ASSERT(CHECK) ((CHECK) ? void(0) : []{assert(!#CHECK);}())
#endif

    namespace detail {
        template<std::size_t... I>
        struct sequence {};

        template<std::size_t I>
        struct _make_sequence;

        template<std::size_t I>
        using make_sequence = typename _make_sequence<I>::type;

        template<>
        struct _make_sequence<0> {
            using type = sequence<>;
        };

        template<class T>
        struct _sequence_append;

        template<std::size_t... I>
        struct _sequence_append<sequence<I...>> {
            using type = sequence<I..., sizeof...(I)>;
        };

        template<std::size_t I>
        struct _make_sequence : _sequence_append<make_sequence<I - 1>> {
        };
    }

    template<std::size_t N>
    class string {
        const char data[N + 1];

        template<std::size_t... PACK>
        constexpr string(const char (&s)[N + 1], detail::sequence<PACK...>)
                : data{s[PACK]..., '\0'}
        {}

        template<std::size_t N1, std::size_t... PACK1, std::size_t... PACK2>
        constexpr string(const string<N1> &a, detail::sequence<PACK1...>,
                         const string<N - N1> &b, detail::sequence<PACK2...>)
                : data{a[PACK1]..., b[PACK2]..., '\0'}
        {}

    public:
        constexpr explicit string(const char (&data)[N + 1]) : string{data, detail::make_sequence<N>{}}
        {}

        template<std::size_t N1>
        constexpr string(const string<N1> &a, const string<N - N1> &b)
                : string{a, detail::make_sequence<N1>{}, b, detail::make_sequence<N - N1>{}}
        {}

        constexpr char operator[](std::size_t i) const
        { return ASSERT(i >= 0 && i < N), data[i]; }

        constexpr std::size_t size() const
        { return N; }

        constexpr const char *c_str() const
        { return data; }

        constexpr operator const char *() const // NOLINT
        { return c_str(); }

        template<std::size_t M>
        constexpr string<N + M> operator+(const string<M> &other) const
        {
            return {*this, other};
        }
    };
}

template<std::size_t N_Z>
constexpr const compile::string<N_Z - 1> const_string(const char (&lit)[N_Z])
{
    return compile::string<N_Z - 1>(lit);
}

#define VALUE(expr) \
    static constexpr auto value() -> decltype(expr) \
    { return expr; }

template<class T>
struct sig {
    VALUE(const_string("L") + T::fqn + const_string(";"))
};

template<class T>
struct sig<T[]> {
    VALUE(const_string("[") + sig<T>::value())
};

struct sum {
    template<class T>
    static constexpr T apply(T head)
    {
        return head;
    }

    template<class T, class... Rest>
    static constexpr auto apply(T head, Rest &&...tail) -> decltype(head + sum::apply(std::forward<Rest>(tail)...))
    {
        return head + sum::apply(std::forward<Rest>(tail)...);
    }
};

template<class R, class... Args>
struct sig<R(Args...)> {
    VALUE(const_string("(") + sum::apply(sig<Args>::value()...) + const_string(")") + sig<R>::value())
};

template<class R>
struct sig<R()> {
    VALUE(const_string("(") + const_string(")") + sig<R>::value())
};

template<>
struct sig<void> {
    VALUE(const_string("V"))
};

template<>
struct sig<jboolean> {
    VALUE(const_string("Z"))
};

template<>
struct sig<jbyte> {
    VALUE(const_string("B"))
};

template<>
struct sig<jchar> {
    VALUE(const_string("C"))
};

template<>
struct sig<jshort> {
    VALUE(const_string("S"))
};

template<>
struct sig<jint> {
    VALUE(const_string("I"))
};

template<>
struct sig<jlong> {
    VALUE(const_string("J"))
};

template<>
struct sig<jfloat> {
    VALUE(const_string("F"))
};

template<>
struct sig<jdouble> {
    VALUE(const_string("D"))
};

template<class T>
struct SIG {
    static constexpr auto &&str = sig<T>::value();
};

namespace jni {
    class Env {
    public:
        JNIEnv *env;

        JNIEnv *operator->()
        {
            return env;
        }

        template<class T>
        jclass FindClass()
        {
            return env->FindClass(T::fqn);
        }

        template<class T>
        jobject GetStaticObjectField(jclass clazz, const char *name)
        {
            auto field = env->GetStaticFieldID(clazz, name, SIG<T>::str);
            return env->GetStaticObjectField(clazz, field);
        }

        template<class T>
        jmethodID GetMethodID(jclass clazz, const char *name)
        {
            return env->GetMethodID(clazz, name, SIG<T>::str);
        }
    };
}
