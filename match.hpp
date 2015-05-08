#pragma once

#include "capo/preprocessor.hpp"

#include <utility>     // std::forward
#include <tuple>       // std::tuple
#include <memory>      // std::addressof
#include <type_traits> // std::add_pointer, std::remove_reference, ...
#include <regex>       // std::regex, std::regex_match
#include <cstdarg>     // va_list, va_start, ...

namespace match {

template <typename T> inline const T* addr(const T* t) { return t; }
template <typename T> inline       T* addr(      T* t) { return t; }
template <typename T> inline const T* addr(const T& t) { return std::addressof(t); }
template <typename T> inline       T* addr(      T& t) { return std::addressof(t); }

template <typename T>
struct underly : std::remove_cv<typename std::remove_reference<T>::type> {};

template <typename T, bool = std::is_polymorphic<typename underly<T>::type>::value>
class type;

template <typename T>
class type<T, false>
{
public:
    template <typename U>
    bool operator==(U const &)
    {
        return std::is_same<U, T>::value;
    }
};

template <typename T>
class type<T, true>
{
public:
    template <typename U>
    bool operator==(U const & x)
    {
        using p_t = typename std::add_pointer<
                    typename std::add_const<typename underly<T>::type>::type>::type;
        return (dynamic_cast<p_t>(addr(x)) != nullptr);
    }
};

class regex
{
    std::regex pattern_;

public:
    template <typename T>
    regex(T&& str)
        : pattern_(std::forward<T>(str))
    {}

    template <typename T>
    bool operator==(T const & x)
    {
        return std::regex_match(x, this->pattern_);
    }
};

template <typename F>
class closure
{
    F judge_;

public:
    template <typename F_>
    closure(F_&& f)
        : judge_(std::forward<F_>(f))
    {}

    template <typename T>
    bool operator==(T const & x)
    {
        return !!( this->judge_(x) );
    }
};

struct is_functor_checker_
{
    template <typename T> static std::true_type  check(decltype(&T::operator())*);
    template <typename T> static std::false_type check(...);
};
template <typename T>
struct is_functor : decltype(is_functor_checker_::check<T>(nullptr)) {};

template <typename T, bool = std::is_function<T>::value || is_functor<T>::value>
struct is_closure_;

template <typename T> struct is_closure_<T , true>  : std::true_type  {};
template <typename T> struct is_closure_<T , false> : std::false_type {};
template <typename T> struct is_closure_<T*, false> : is_closure_<T>  {};

template <typename T>
struct is_closure : is_closure_<typename underly<T>::type> {};

template <typename T>
inline auto filter(T&& arg)
    -> typename std::enable_if<is_closure<T>::value, closure<T&&>>::type
{
    return { std::forward<T>(arg) };
}

inline void filter(...);

template <typename T>
inline T&& filter_value(T&& arg)
{
    return std::forward<T>(arg);
}

template <typename T>
inline auto tester(T&& arg)
    -> typename std::enable_if<std::is_same<decltype(filter(std::forward<T>(arg))), void>::value, T&&>::type
{
    return filter_value(std::forward<T>(arg));
}

template <typename T>
inline auto tester(T&& arg)
    -> typename std::enable_if<!std::is_same<decltype(filter(std::forward<T>(arg))), void>::value, 
                                             decltype(filter(std::forward<T>(arg)))>::type
{
    return filter(std::forward<T>(arg));
}

template <size_t N, typename... T>
inline decltype(auto) get(std::tuple<T...>       & tp) { return std::get<N>(tp); }
template <size_t N, typename... T>
inline decltype(auto) get(std::tuple<T...> const & tp) { return std::get<N>(tp); }
template <size_t N, typename... T>
inline decltype(auto) get(std::tuple<T...>      && tp) { return std::get<N>(tp); }
template <size_t N, typename T>
inline bool get(T&&) { return true; }

template <typename... P>
inline auto match_info(P&&... args)
{
    return std::forward_as_tuple(std::forward<P>(args)...);
}

inline auto match_info(void)
{
    return nullptr;
}

} // namespace match

#define Type(...)  match::type<__VA_ARGS__>{}
#define Regex(...) match::regex(__VA_ARGS__)

#define Match(...)                                     \
    {                                                  \
        auto target_ = match::match_info(__VA_ARGS__); \
        if (false)

#define MATCH_CASE_(N, ...) && ( match::tester( CAPO_PP_A_(N, __VA_ARGS__) ) == match::get<N - 1>(target_) )
#define Case(...) \
        } else if (true CAPO_PP_REPEAT_(CAPO_PP_COUNT_(__VA_ARGS__), MATCH_CASE_, __VA_ARGS__)) {

#define Otherwise() \
        } else {

#define EndMatch \
    }
