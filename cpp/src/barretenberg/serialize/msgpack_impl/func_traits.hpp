#pragma once
#include <tuple>
#include "../msgpack.hpp"

template <typename Func> struct func_traits;

template <typename R, typename... Vs> struct func_traits<R (*)(Vs...)> {
    typedef std::tuple<Vs...> Args;
    Args args;
    R ret;
    MSGPACK_FIELDS(args, ret);
};
template <typename R, typename... Vs> struct func_traits<R (&)(Vs...)> {
    typedef std::tuple<Vs...> Args;
    Args args;
    R ret;
    MSGPACK_FIELDS(args, ret);
};

template <typename R, typename T, typename... Vs> struct func_traits<R (T::*)(Vs...) const> {
    typedef std::tuple<Vs...> Args;
    Args args;
    R ret;
    MSGPACK_FIELDS(args, ret);
};

// Template metamagic for figuring out the parameters and return type of a function
template <typename T>
concept Callable =
    requires() { typename std::enable_if_t<std::is_member_function_pointer_v<decltype(&T::operator())>, void>; };

template <Callable T> constexpr auto get_func_traits()
{
    return func_traits<decltype(&T::operator())>();
}

template <typename T> constexpr auto get_func_traits()
{
    return func_traits<T>();
}
