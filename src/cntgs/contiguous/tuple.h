#pragma once

#include "cntgs/contiguous/detail/forward.h"
#include "cntgs/contiguous/detail/tuple.h"
#include "cntgs/contiguous/detail/tupleQualifier.h"

#include <cstddef>
#include <tuple>

namespace cntgs
{
template <detail::ContiguousTupleQualifier Qualifier, class... Types>
class ContiguousTuple
{
  public:
    using Tuple = std::conditional_t<(Qualifier == detail::ContiguousTupleQualifier::REFERENCE),
                                     detail::ToContiguousTupleOfReferenceReturnType<std::tuple<Types...>>,
                                     detail::ToContiguousTupleOfConstReferenceReturnType<std::tuple<Types...>>>;

    Tuple tuple;

    ContiguousTuple() = default;

    ContiguousTuple(const ContiguousTuple&) = default;
    ContiguousTuple(ContiguousTuple&&) = default;

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

    template <detail::ContiguousTupleQualifier TQualifier>
    explicit constexpr ContiguousTuple(const ContiguousTuple<TQualifier, Types...>& other) : tuple(other.tuple)
    {
    }

    explicit constexpr ContiguousTuple(const cntgs::ContiguousElement<Types...>& other) : tuple(other.tuple) {}

    template <detail::ContiguousTupleQualifier TQualifier>
    explicit constexpr ContiguousTuple(ContiguousTuple<TQualifier, Types...>&& other) : tuple(std::move(other.tuple))
    {
    }

    explicit constexpr ContiguousTuple(cntgs::ContiguousElement<Types...>&& other) : tuple(std::move(other.tuple)) {}

    template <detail::ContiguousTupleQualifier TQualifier>
    constexpr ContiguousTuple& operator=(const ContiguousTuple<TQualifier, Types...>& other)
    {
        if (static_cast<const void*>(this) != static_cast<const void*>(std::addressof(other)))
        {
            this->assign(other, std::make_index_sequence<sizeof...(Types)>{});
        }
        return *this;
    }

    constexpr ContiguousTuple& operator=(const cntgs::ContiguousElement<Types...>& other)
    {
        this->assign(other.tuple, std::make_index_sequence<sizeof...(Types)>{});
        return *this;
    }

    template <detail::ContiguousTupleQualifier TQualifier>
    constexpr ContiguousTuple& operator=(ContiguousTuple<TQualifier, Types...>&& other)
    {
        if (static_cast<const void*>(this) != static_cast<const void*>(std::addressof(other)))
        {
            this->assign(other, std::make_index_sequence<sizeof...(Types)>{});
        }
        return *this;
    }

    constexpr ContiguousTuple& operator=(cntgs::ContiguousElement<Types...>&& other)
    {
        this->assign(other.tuple, std::make_index_sequence<sizeof...(Types)>{});
        return *this;
    }

    template <detail::ContiguousTupleQualifier TQualifier, std::size_t... I>
    constexpr void swap(ContiguousTuple<TQualifier, Types...>& other, std::index_sequence<I...>)
    {
        (detail::ParameterTraits<Types>::swap(std::get<I>(this->tuple), std::get<I>(other.tuple)), ...);
    }

    [[nodiscard]] constexpr auto size_in_bytes() const noexcept
    {
        using TraitsOfFirst = detail::ParameterTraits<std::tuple_element_t<0, std::tuple<Types...>>>;
        using TraitsOfLast = detail::ParameterTraits<std::tuple_element_t<sizeof...(Types) - 1, std::tuple<Types...>>>;
        return TraitsOfLast::end_address(std::get<sizeof...(Types) - 1>(this->tuple)) -
               TraitsOfFirst::start_address(std::get<0>(this->tuple));
    }

  private:
    template <detail::ContiguousTupleQualifier TQualifier, std::size_t... I>
    constexpr void assign(const ContiguousTuple<TQualifier, Types...>& other, std::index_sequence<I...>)
    {
        (detail::ParameterTraits<Types>::copy(std::get<I>(other.tuple), std::get<I>(this->tuple)), ...);
    }

    template <detail::ContiguousTupleQualifier TQualifier, std::size_t... I>
    constexpr void assign(ContiguousTuple<TQualifier, Types...>& other, std::index_sequence<I...>)
    {
        (detail::ParameterTraits<Types>::move(std::get<I>(other.tuple), std::get<I>(this->tuple)), ...);
    }
};

template <detail::ContiguousTupleQualifier Qualifier, class... T>
constexpr void swap(cntgs::ContiguousTuple<Qualifier, T...> lhs, cntgs::ContiguousTuple<Qualifier, T...> rhs)
{
    lhs.swap(rhs, std::make_index_sequence<sizeof...(T)>{});
}

template <std::size_t I, detail::ContiguousTupleQualifier Qualifier, class... Types>
[[nodiscard]] constexpr decltype(auto) get(cntgs::ContiguousTuple<Qualifier, Types...>& tuple) noexcept
{
    return std::get<I>(tuple.tuple);
}

template <std::size_t I, detail::ContiguousTupleQualifier Qualifier, class... Types>
[[nodiscard]] constexpr decltype(auto) get(const cntgs::ContiguousTuple<Qualifier, Types...>& tuple) noexcept
{
    return std::get<I>(tuple.tuple);
}

template <std::size_t I, detail::ContiguousTupleQualifier Qualifier, class... Types>
[[nodiscard]] constexpr decltype(auto) get(cntgs::ContiguousTuple<Qualifier, Types...>&& tuple) noexcept
{
    return std::get<I>(std::move(tuple.tuple));
}

template <std::size_t I, detail::ContiguousTupleQualifier Qualifier, class... Types>
[[nodiscard]] constexpr decltype(auto) get(const cntgs::ContiguousTuple<Qualifier, Types...>&& tuple) noexcept
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
