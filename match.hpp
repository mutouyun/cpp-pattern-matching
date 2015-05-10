#pragma once

#include "capo/preprocessor.hpp"

#include <utility>     // std::forward
#include <tuple>       // std::tuple
#include <type_traits> // std::add_pointer, std::remove_reference, ...
#include <cstddef>     // size_t

namespace match {

// To remove reference and cv qualification from a type.

template <typename T>
struct underly : std::remove_cv<typename std::remove_reference<T>::type> {};

// The helper meta-predicate capable of distinguishing all our patterns.

template <typename T>
struct is_pattern : std::false_type {};

template <typename T>
struct pattern_checker : is_pattern<typename underly<T>::type> {};

/*
 * Constant pattern
*/

template <typename T>
struct constant
{
    const T& t_;

    template <typename U>
    bool operator()(U&& tar) const
    {
        return (std::forward<U>(tar) == t_);
    }
};

template <typename T>
struct is_pattern<constant<T>> : std::true_type {};

/*
 * Variable pattern
*/

template <typename T>
struct variable
{
    T& t_;

    template <typename U>
    bool operator()(U&& tar) const
    {
        t_ = std::forward<U>(tar);
        return (t_ == std::forward<U>(tar));
    }
};

template <typename T>
struct is_pattern<variable<T>> : std::true_type {};

/*
 * Wildcard pattern
*/

struct wildcard
{
    template <typename U>
    bool operator()(U&&) const
    {
        return true;
    }
};

template <>
struct is_pattern<wildcard> : std::true_type{};

#if defined(_MSC_VER)
__pragma(warning(suppress:4100)) static wildcard _;
#elif defined(__GNUC__)
__attribute__((__unused__)) static wildcard _;
#else
static wildcard _;
#endif

/*
 * Predicate pattern, for lambda-expressions or other callable objects.
*/

template <typename F>
struct predicate
{
    F judge_;

    template <typename U>
    bool operator()(U&& x) const
    {
        return !!(this->judge_(x));
    }
};

template <typename T>
struct is_pattern<predicate<T>> : std::true_type {};

struct is_functor_checker_
{
    template <typename T> static std::true_type  check(decltype(&T::operator())*);
    template <typename T> static std::false_type check(...);
};
template <typename T>
struct is_functor : decltype(is_functor_checker_::check<T>(nullptr)) {};

template <typename T, bool = std::is_function<typename std::remove_pointer<T>::type>::value || 
                                  is_functor<T>::value>
struct is_closure_;
template <typename T> struct is_closure_<T, true>  : std::true_type  {};
template <typename T> struct is_closure_<T, false> : std::false_type {};

template <typename T>
struct is_closure : is_closure_<typename underly<T>::type> {};

template <typename T>
inline auto converter(T&& arg)
    -> typename std::enable_if<is_closure<T>::value, predicate<T&&>>::type
{
    return { std::forward<T>(arg) };
}

/*
 * Type pattern
*/

template <typename T> inline const T* addr(const T* t) { return t; }
template <typename T> inline       T* addr(      T* t) { return t; }
template <typename T> inline const T* addr(const T& t) { return std::addressof(t); }
template <typename T> inline       T* addr(      T& t) { return std::addressof(t); }

template <typename T, bool = std::is_polymorphic<typename underly<T>::type>::value>
struct type;

template <>
struct type<wildcard, false>
{
    template <typename U>
    bool operator()(U&&) const
    {
        return true;
    }
};

template <typename T>
struct type<T, false>
{
    template <typename U>
    bool operator()(const U&) const
    {
        return std::is_same<U, T>::value;
    }
};

template <typename T>
struct type<T, true>
{
    template <typename U>
    bool operator()(const U& tar) const
    {
        using p_t = typename std::add_pointer<
                    typename std::add_const<typename underly<T>::type>::type>::type;
        return (dynamic_cast<p_t>(addr(tar)) != nullptr);
    }
};

template <typename T, bool Cond>
struct is_pattern<type<T, Cond>> : std::true_type{};

#define Type(...) match::type<__VA_ARGS__>{}

/*
 * Constructor pattern
*/

template <typename...>
struct model;

template <>
struct model<> {};

template <typename T1, typename... T>
struct model<T1, T...> : model<T...> { T1 m_; };

template <typename Tp, size_t N>
struct model_strip;
template <typename T1, typename... T, size_t N>
struct model_strip<model<T1, T...>, N> : model_strip<model<T...>, N - 1> {};
template <typename T1, typename... T>
struct model_strip<model<T1, T...>, 0> { using type = model<T1, T...>; };
template <typename Tp>
struct model_strip<Tp, 0>              { using type = Tp; };

template <typename Tp>
struct model_length              : std::integral_constant<size_t, 0> {};
template <typename... T>
struct model_length<model<T...>> : std::integral_constant<size_t, sizeof...(T)> {};

template <typename T, typename U>
struct model_append                 { using type = model<T, U>; };
template <typename... T, typename U>
struct model_append<model<T...>, U> { using type = model<T..., U>; };

template <typename Tp>
struct model_reverse { using type = Tp; };
template <typename T1, typename... T>
struct model_reverse<model<T1, T...>>
{
    using head = typename model_reverse<model<T...>>::type;
    using type = typename model_append<head, T1>::type;
};

template <typename... T>
struct layout
{
    using model_t = typename model_reverse<model<T...>>::type;

    template <typename U, typename V>
    struct rep;
    template <typename U, typename V>
    struct rep<U&, V>        { using type = V&; };
    template <typename U, typename V>
    struct rep<U&&, V>       { using type = V&&; };
    template <typename U, typename V>
    struct rep<const U&, V>  { using type = const V&; };
    template <typename U, typename V>
    struct rep<const U&&, V> { using type = const V&&; };

    template <size_t N, typename U>
    static auto & get(U&& tar)
    {
        decltype(auto) mm = reinterpret_cast<typename rep<U&&, model_t>::type>(tar);
        return static_cast<typename model_strip<model_t, model_length<model_t>::value - N - 1>::type &>(mm).m_;
    }
};

template <class Bind>
struct bindings_base
{
    template <size_t N, typename T, typename U>
    static auto apply(const T&, U&&)
        -> typename std::enable_if<(std::tuple_size<T>::value <= N), bool>::type
    {
        return true;
    }

    template <size_t N, typename T, typename U>
    static auto apply(const T& tp, U&& tar)
        -> typename std::enable_if<(std::tuple_size<T>::value > N), bool>::type
    {
        using layout_t = typename Bind::layout_t;
        if ( std::get<N>(tp)(layout_t::template get<N>(tar)) )
        {
            return apply<N + 1>(tp, std::forward<U>(tar));
        }
        return false;
    }

    template <typename T, typename U>
    static auto apply(const T& tp, U&& tar)
        -> typename std::enable_if<std::is_pointer<typename underly<U>::type>::value, bool>::type
    {
        return apply<0>(tp, *std::forward<U>(tar));
    }

    template <typename T, typename U>
    static auto apply(const T& tp, U&& tar)
        -> typename std::enable_if<!std::is_pointer<typename underly<U>::type>::value, bool>::type
    {
        return apply<0>(tp, std::forward<U>(tar));
    }
};

template <class C>
struct bindings;

template <typename C, typename... T>
struct constructor : type<C>
{
    std::tuple<T...> tp_;

    template <typename... U>
    constructor(U&&... args)
        : tp_(std::forward<U>(args)...)
    {}

    template <typename U>
    bool operator()(U&& tar) const
    {
        if ( type<C>::operator()(std::forward<U>(tar)) )
        {
            return bindings<typename underly<U>::type>::apply(tp_, std::forward<U>(tar));
        }
        return false;
    }
};

template <typename C, typename... T>
struct is_pattern<constructor<C, T...>> : std::true_type{};

#define MATCH_REGIST_TYPE(TYPE, ...)                                      \
    namespace match                                                       \
    {                                                                     \
        template <> struct bindings<TYPE> : bindings_base<bindings<TYPE>> \
        {                                                                 \
            using layout_t = layout<__VA_ARGS__>;                         \
        };                                                                \
        template <> struct bindings<TYPE*> : bindings<TYPE> {};           \
    }

/*
 * Sequence pattern
*/

template <typename... T>
struct sequence
{
    std::tuple<T...> tp_;

    template <typename... U>
    sequence(U&&... args)
        : tp_(std::forward<U>(args)...)
    {}

    template <size_t N, typename U, typename It>
    auto apply(U&&, It&&) const
        -> typename std::enable_if<(sizeof...(T) <= N), bool>::type
    {
        return true;
    }

    template <size_t N, typename U, typename It>
    auto apply(U&& tar, It&& it) const
        -> typename std::enable_if<(sizeof...(T) > N), bool>::type
    {
        if ( it == tar.end() )       return false;
        if ( std::get<N>(tp_)(*it) ) return apply<N + 1>(std::forward<U>(tar), ++it);
        return false;
    }

    template <typename U>
    bool operator()(U&& tar) const
    {
        return apply<0>(std::forward<U>(tar), tar.begin());
    }
};

template <typename... T>
struct is_pattern<sequence<T...>> : std::true_type{};

/*
 * "filter" is a common function used to provide convenience to the users by converting 
 * constant values into constant patterns and regular variables into variable patterns.
 * If the users defined a suitable converter for a specific type, the filter would choose
 * the custom converter for wrapping that variable of the type.
*/

void converter(...);

template <typename T>
inline auto filter(T&& arg)
    -> typename std::enable_if<pattern_checker<T>::value, T&&>::type
{
    return std::forward<T>(arg);
}

template <typename T>
inline auto filter(const T& arg)
    -> typename std::enable_if<!pattern_checker<T>::value && 
                                std::is_same<decltype(converter(arg)), void>::value, 
                                constant<T>>::type
{
    return { arg };
}

template <typename T>
inline auto filter(T& arg)
    -> typename std::enable_if<!pattern_checker<T>::value && 
                                std::is_same<decltype(converter(arg)), void>::value, 
                                variable<T>>::type
{
    return { arg };
}

template <typename T>
inline auto filter(T&& arg)
    -> typename std::enable_if<!pattern_checker<T>::value && 
                               !std::is_same<decltype(converter(std::forward<T>(arg))), void>::value, 
                                             decltype(converter(std::forward<T>(arg)))>::type
{
    return converter(std::forward<T>(arg));
}

/*
 * Here is a part of the constructor & sequence pattern.
 * I have to implement it here, because gcc needs the filter be declared before it.
*/

template <typename T = wildcard, typename... P>
inline auto C(P&&... args)
    -> constructor<T, decltype(filter(std::forward<P>(args)))...>
{
    return { filter(std::forward<P>(args))... };
}

template <typename... P>
inline auto S(P&&... args)
    -> sequence<decltype(filter(std::forward<P>(args)))...>
{
    return { filter(std::forward<P>(args))... };
}

} // namespace match

#define Match(...)                                         \
    {                                                      \
        auto target_ = std::forward_as_tuple(__VA_ARGS__); \
        if (false)

#define MATCH_CASE_ARG_(N, ...) && ( match::filter( CAPO_PP_A_(N, __VA_ARGS__) )( std::get<N - 1>(target_) ) )
#define P(...)                  ( true CAPO_PP_REPEAT_(CAPO_PP_COUNT_(__VA_ARGS__), MATCH_CASE_ARG_, __VA_ARGS__) )

#define With(...) \
        } else if (__VA_ARGS__) {

#define Case(...) With( P(__VA_ARGS__) )

#define Otherwise() \
        } else {

#define EndMatch \
    }
