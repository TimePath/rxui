#pragma once

#include <jni.h>
#include <cassert>

#if defined NDEBUG
# define ASSERT(CHECK) void(0)
#else
# define ASSERT(CHECK) ((CHECK) ? void(0) : []{assert(!#CHECK);}())
#endif

namespace compile {

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

#define JNIX_TYPES(_) \
    _(jobject, Object, sig<java::lang::Object>::value()) \
    _(jboolean, Boolean, const_string("Z")) \
    _(jbyte, Byte, const_string("B")) \
    _(jchar, Char, const_string("C")) \
    _(jshort, Short, const_string("S")) \
    _(jint, Int, const_string("I")) \
    _(jlong, Long, const_string("J")) \
    _(jfloat, Float, const_string("F")) \
    _(jdouble, Double, const_string("D")) \
    _(void, Void, const_string("V")) \
    /**/

namespace java {
    namespace lang {
        struct Object {
            static constexpr auto &fqn = const_string("java/lang/Object");
        };

        struct String {
            static constexpr auto &fqn = const_string("java/lang/String");
        };
    }
}

namespace signatures {

#define JNIX_VALUE(expr) \
    static constexpr auto value() -> decltype(expr) \
    { return expr; }

    template<class T>
    struct sig {
        JNIX_VALUE(const_string("L") + T::fqn + const_string(";"))
    };

    template<class T>
    struct sig<T[]> {
        JNIX_VALUE(const_string("[") + sig<T>::value())
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
        JNIX_VALUE(const_string("(") + sum::apply(sig<Args>::value()...) + const_string(")") + sig<R>::value())
    };

    template<class R>
    struct sig<R()> {
        JNIX_VALUE(const_string("(") + const_string(")") + sig<R>::value())
    };

#define X(T, Name, S) \
    template<> \
    struct sig<T> { \
        JNIX_VALUE(S) \
    };

    JNIX_TYPES(X)

#undef X

    template<>
    struct sig<jstring> {
        JNIX_VALUE(sig<java::lang::String>::value())
    };
}

namespace jni {
    template<class T>
    struct sig {
        static constexpr auto &&str = signatures::sig<T>::value();
    };

    template<class... Args>
    class Constructor;

    template<class T>
    class Method;

    class Class {
    public:
        JNIEnv *env;
        jclass clazz;

        template<class... Args>
        Constructor<Args...> ctor()
        {
            return {env, clazz, env->GetMethodID(clazz, "<init>", sig<void(Args...)>::str)};
        }

        template<class T>
        Method<T> method(const char *name)
        {
            return {env, env->GetMethodID(clazz, name, sig<T>::str)};
        }
    };

    template<class... Args>
    class Constructor {
    public:
        JNIEnv *env;
        jclass clazz;
        jmethodID id;

        jobject operator()(Args &&...args) {
            return env->NewObject(clazz, id, args...);
        }
    };

    template<class T>
    struct MethodCall;

    template<class R, class... Args>
    class Method<R(Args...)> {
    public:
        JNIEnv *env;
        jmethodID id;

        R operator()(jobject self, Args &&...args) {
            return MethodCall<R>::call(env, self, id, args...);
        }
    };

#define MethodCall(T, F) \
    template <> \
    struct MethodCall<T> { \
        template<class... Args> \
        static T call(JNIEnv *env, Args &&...args) { \
            return env->F(args...); \
        } \
    };
#define X(T, Name, S) MethodCall(T, Call ## Name ## Method)
    JNIX_TYPES(X)
#undef X
#undef MethodCall

    class Env {
    public:
        JNIEnv *env;

        JNIEnv *operator->()
        {
            return env;
        }

        template<class T>
        Class FindClass()
        {
            return {env, env->FindClass(T::fqn)};
        }

        template<class T>
        jobject GetStaticObjectField(jclass clazz, const char *name)
        {
            auto field = env->GetStaticFieldID(clazz, name, sig<T>::str);
            return env->GetStaticObjectField(clazz, field);
        }
    };
}
