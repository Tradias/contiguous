#pragma once

#include "cntgs/contiguous/detail/tuple.h"
#include "cntgs/contiguous/detail/typeUtils.h"

#include <cstddef>
#include <tuple>

namespace cntgs
{
template <detail::ContiguousTupleQualifier Qualifier, class... Types>
class ContiguousTuple
{
  public:
    using Tuple = std::conditional_t<
        (Qualifier == detail::ContiguousTupleQualifier::NONE),
        detail::ToContiguousTupleOfValueReturnType<std::tuple<Types...>>,
        std::conditional_t<(Qualifier == detail::ContiguousTupleQualifier::REFERENCE),
                           detail::ToContiguousTupleOfReferenceReturnType<std::tuple<Types...>>,
                           std::conditional_t<(Qualifier == detail::ContiguousTupleQualifier::CONST_REFERENCE),
                                              detail::ToContiguousTupleOfConstReferenceReturnType<std::tuple<Types...>>,
                                              detail::ToContiguousTupleOfPointerReturnType<std::tuple<Types...>>>>>;

    Tuple tuple;

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

    template <detail::ContiguousTupleQualifier Qualifier, class... T>
    constexpr ContiguousTuple(const ContiguousTuple<Qualifier, T...>& other) : tuple(other.tuple)
    {
    }

    template <detail::ContiguousTupleQualifier Qualifier, class... T>
    constexpr ContiguousTuple(ContiguousTuple<Qualifier, T...>&& other) : tuple(std::move(other.tuple))
    {
    }

    ContiguousTuple(const ContiguousTuple&) = default;
    ContiguousTuple(ContiguousTuple&&) = default;

    ContiguousTuple& operator=(const ContiguousTuple&) = default;
    ContiguousTuple& operator=(ContiguousTuple&&) = default;

    template <detail::ContiguousTupleQualifier Qualifier, class... T>
    constexpr ContiguousTuple& operator=(const ContiguousTuple<Qualifier, T...>& other)
    {
        if (this != &other)
        {
            this->assign(other, std::make_index_sequence<sizeof...(Types)>{});
        }
        return *this;
    }

    template <detail::ContiguousTupleQualifier Qualifier, class... T>
    constexpr ContiguousTuple& operator=(ContiguousTuple<Qualifier, T...>&& other)
    {
        if (this != &other)
        {
            this->assign(std::move(other), std::make_index_sequence<sizeof...(Types)>{});
        }
        return *this;
    }

    template <detail::ContiguousTupleQualifier Qualifier, class... T, std::size_t... I>
    constexpr void swap(ContiguousTuple<Qualifier, T...>& other, std::index_sequence<I...>)
    {
        (
            [&]() mutable {
                if constexpr (detail::ParameterTraits<Types>::IS_CONTIGUOUS)
                {
                    std::swap_ranges(std::begin(std::get<I>(other.tuple)), std::end(std::get<I>(other.tuple)),
                                     std::begin(std::get<I>(this->tuple)));
                }
                else
                {
                    std::swap(std::get<I>(this->tuple), std::get<I>(other.tuple));
                }
            }(),
            ...);
    }

  private:
    template <detail::ContiguousTupleQualifier Qualifier, class... T, std::size_t... I>
    constexpr void assign(const ContiguousTuple<Qualifier, T...>& other, std::index_sequence<I...>)
    {
        (
            [&]() mutable {
                if constexpr (detail::ParameterTraits<Types>::IS_CONTIGUOUS)
                {
                    std::copy(std::begin(std::get<I>(other.tuple)), std::end(std::get<I>(other.tuple)),
                              std::begin(std::get<I>(this->tuple)));
                }
                else
                {
                    std::get<I>(this->tuple) = std::get<I>(other.tuple);
                }
            }(),
            ...);
    }

    template <detail::ContiguousTupleQualifier Qualifier, class... T, std::size_t... I>
    constexpr void assign(ContiguousTuple<Qualifier, T...>&& other, std::index_sequence<I...>)
    {
        (
            [&]() mutable {
                if constexpr (detail::ParameterTraits<Types>::IS_CONTIGUOUS)
                {
                    std::move(std::begin(std::get<I>(other.tuple)), std::end(std::get<I>(other.tuple)),
                              std::begin(std::get<I>(this->tuple)));
                }
                else
                {
                    std::get<I>(this->tuple) = std::move(std::get<I>(other.tuple));
                }
            }(),
            ...);
    }
};

template <detail::ContiguousTupleQualifier Qualifier, class... T, class... U>
constexpr void swap(cntgs::ContiguousTuple<Qualifier, T...> lhs, cntgs::ContiguousTuple<Qualifier, U...> rhs,
                    std::enable_if_t<(Qualifier != detail::ContiguousTupleQualifier::NONE)>* = nullptr)
{
    lhs.swap(rhs, std::make_index_sequence<sizeof...(T)>{});
}

template <class... T, class... U>
constexpr void swap(cntgs::ContiguousTuple<detail::ContiguousTupleQualifier::NONE, T...>& lhs,
                    cntgs::ContiguousTuple<detail::ContiguousTupleQualifier::NONE, U...>& rhs)
{
    lhs.swap(rhs, std::make_index_sequence<sizeof...(T)>{});
}

template <std::size_t I, detail::ContiguousTupleQualifier Qualifier, class... Types>
constexpr decltype(auto) get(cntgs::ContiguousTuple<Qualifier, Types...>& tuple) noexcept
{
    return std::get<I>(tuple.tuple);
}

template <std::size_t I, detail::ContiguousTupleQualifier Qualifier, class... Types>
constexpr decltype(auto) get(const cntgs::ContiguousTuple<Qualifier, Types...>& tuple) noexcept
{
    return std::get<I>(tuple.tuple);
}

template <std::size_t I, detail::ContiguousTupleQualifier Qualifier, class... Types>
constexpr decltype(auto) get(cntgs::ContiguousTuple<Qualifier, Types...>&& tuple) noexcept
{
    return std::get<I>(std::move(tuple.tuple));
}

template <std::size_t I, detail::ContiguousTupleQualifier Qualifier, class... Types>
constexpr decltype(auto) get(const cntgs::ContiguousTuple<Qualifier, Types...>&& tuple) noexcept
{
    return std::get<I>(std::move(tuple.tuple));
}
}  // namespace cntgs

namespace std
{
template <std::size_t I, ::cntgs::detail::ContiguousTupleQualifier Qualifier, class... Types>
struct tuple_element<I, ::cntgs::ContiguousTuple<Qualifier, Types...>>
    : std::tuple_element<I, typename ::cntgs::ContiguousTuple<Qualifier, Types...>::Tuple>
{
};

template <::cntgs::detail::ContiguousTupleQualifier Qualifier, class... Types>
struct tuple_size<::cntgs::ContiguousTuple<Qualifier, Types...>> : std::integral_constant<std::size_t, sizeof...(Types)>
{
};
}  // namespace std