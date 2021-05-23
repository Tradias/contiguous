#pragma once

#include <cstddef>
#include <tuple>

namespace cntgs
{
template <class... Types>
class ContiguousTuple
{
  public:
    using Tuple = std::tuple<Types...>;

    std::tuple<Types...> tuple;

    ContiguousTuple() = default;

    template <class Arg>
    explicit constexpr ContiguousTuple(Arg&& arg) : tuple(std::forward<Arg>(arg))
    {
    }

    template <class Arg1, class Arg2>
    constexpr ContiguousTuple(Arg1&& arg1, Arg2&& arg2) : tuple(std::forward<Arg1>(arg1), std::forward<Arg2>(arg2))
    {
    }

    template <class Arg1, class Arg2, class... Args>
    constexpr ContiguousTuple(Arg1&& arg1, Arg2&& arg2, Args&&... args)
        : tuple(std::forward<Arg1>(arg1), std::forward<Arg2>(arg2), std::forward<Args>(args)...)
    {
    }

    template <class... T>
    constexpr ContiguousTuple(const ContiguousTuple<T...>& other) : tuple(other.tuple)
    {
    }

    template <class... T>
    constexpr ContiguousTuple(ContiguousTuple<T...>&& other) : tuple(std::move(other.tuple))
    {
    }

    ContiguousTuple(const ContiguousTuple&) = default;
    ContiguousTuple(ContiguousTuple&&) = default;

    ContiguousTuple& operator=(const ContiguousTuple&) = default;
    ContiguousTuple& operator=(ContiguousTuple&&) = default;

    template <class... T>
    constexpr ContiguousTuple& operator=(const ContiguousTuple<T...>& other)
    {
        if (this != &other)
        {
            this->assign(other, std::make_index_sequence<sizeof...(Types)>{});
        }
        return *this;
    }

    template <class... T>
    constexpr ContiguousTuple& operator=(ContiguousTuple<T...>&& other)
    {
        if (this != &other)
        {
            this->assign(std::move(other), std::make_index_sequence<sizeof...(Types)>{});
        }
        return *this;
    }

  private:
    template <class T>
    constexpr void assign(const ContiguousTuple<T...>& other)
    {
        (
            [&]() mutable {
                if constexpr (detail::ParameterTraits<Types>::IS_CONTIGUOUS)
                {
                    std::copy(std::begin(cntgs::get<I>(other)), std::end(cntgs::get<I>(other)),
                              std::begin(cntgs::get<I>(this->tuple)));
                }
                else
                {
                    cntgs::get<I>(this->tuple) = cntgs::get<I>(other.tuple);
                }
            }(),
            ...);
    }

    template <class T>
    constexpr void assign(ContiguousTuple<T...>&& other)
    {
        (
            [&]() mutable {
                if constexpr (detail::ParameterTraits<Types>::IS_CONTIGUOUS)
                {
                    std::move(std::begin(cntgs::get<I>(other)), std::end(cntgs::get<I>(other)),
                              std::begin(cntgs::get<I>(this->tuple)));
                }
                else
                {
                    cntgs::get<I>(this->tuple) = std::move(cntgs::get<I>(other.tuple));
                }
            }(),
            ...);
    }
};

template <std::size_t I, class... Types>
constexpr decltype(auto) get(cntgs::ContiguousTuple<Types...>& tuple) noexcept
{
    return std::get<I>(tuple.tuple);
}

template <std::size_t I, class... Types>
constexpr decltype(auto) get(const cntgs::ContiguousTuple<Types...>& tuple) noexcept
{
    return std::get<I>(tuple.tuple);
}

template <std::size_t I, class... Types>
constexpr decltype(auto) get(cntgs::ContiguousTuple<Types...>&& tuple) noexcept
{
    return std::get<I>(std::move(tuple.tuple));
}

template <std::size_t I, class... Types>
constexpr decltype(auto) get(const cntgs::ContiguousTuple<Types...>&& tuple) noexcept
{
    return std::get<I>(std::move(tuple.tuple));
}
}  // namespace cntgs

namespace std
{
template <std::size_t I, class... Types>
struct tuple_element<I, ::cntgs::ContiguousTuple<Types...>>
    : std::tuple_element<I, typename ::cntgs::ContiguousTuple<Types...>::Tuple>
{
};

template <class... Types>
struct tuple_size<::cntgs::ContiguousTuple<Types...>> : std::integral_constant<std::size_t, sizeof...(Types)>
{
};
}  // namespace std