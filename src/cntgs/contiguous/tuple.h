#pragma once

#include "cntgs/contiguous/detail/elementLocator.h"
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
  private:
    using Locator = detail::BaseElementLocatorT<Types...>;

    static constexpr auto TYPE_COUNT = sizeof...(Types);

  public:
    using Tuple = std::conditional_t<(Qualifier == detail::ContiguousTupleQualifier::REFERENCE),
                                     detail::ToContiguousTupleOfReferenceReturnType<std::tuple<Types...>>,
                                     detail::ToContiguousTupleOfConstReferenceReturnType<std::tuple<Types...>>>;

    static constexpr auto IS_CONST = detail::ContiguousTupleQualifier::CONST_REFERENCE == Qualifier;

    Tuple tuple;

    ContiguousTuple() = default;

    ContiguousTuple(const ContiguousTuple&) = default;
    ContiguousTuple(ContiguousTuple&&) = default;

    template <class Arg>
    explicit constexpr ContiguousTuple(Arg&& arg) : tuple(std::forward<Arg>(arg))
    {
    }

    template <class Arg1, class Arg2, class... Args>
    constexpr ContiguousTuple(Arg1&& arg1, Arg2&& arg2, Args&&... args)
        : tuple(std::forward<Arg1>(arg1), std::forward<Arg2>(arg2), std::forward<Args>(args)...)
    {
    }

    template <detail::ContiguousTupleQualifier TQualifier>
    /*implicit*/ constexpr ContiguousTuple(const ContiguousTuple<TQualifier, Types...>& other) : tuple(other.tuple)
    {
    }

    /*implicit*/ constexpr ContiguousTuple(const cntgs::ContiguousElement<Types...>& other) : tuple(other.tuple) {}

    template <detail::ContiguousTupleQualifier TQualifier>
    /*implicit*/ constexpr ContiguousTuple(ContiguousTuple<TQualifier, Types...>&& other)
        : tuple(std::move(other.tuple))
    {
    }

    /*implicit*/ constexpr ContiguousTuple(cntgs::ContiguousElement<Types...>&& other) : tuple(std::move(other.tuple))
    {
    }

    constexpr ContiguousTuple& operator=(const ContiguousTuple& other)
    {
        if (static_cast<const void*>(this) != static_cast<const void*>(std::addressof(other)))
        {
            this->assign(other, std::make_index_sequence<TYPE_COUNT>{});
        }
        return *this;
    }

    template <detail::ContiguousTupleQualifier TQualifier>
    constexpr ContiguousTuple& operator=(const ContiguousTuple<TQualifier, Types...>& other)
    {
        if (static_cast<const void*>(this) != static_cast<const void*>(std::addressof(other)))
        {
            this->assign(other, std::make_index_sequence<TYPE_COUNT>{});
        }
        return *this;
    }

    constexpr ContiguousTuple& operator=(const cntgs::ContiguousElement<Types...>& other)
    {
        this->assign(other.tuple, std::make_index_sequence<TYPE_COUNT>{});
        return *this;
    }

    constexpr ContiguousTuple& operator=(ContiguousTuple&& other)
    {
        if (static_cast<const void*>(this) != static_cast<const void*>(std::addressof(other)))
        {
            this->assign(other, std::make_index_sequence<TYPE_COUNT>{});
        }
        return *this;
    }

    template <detail::ContiguousTupleQualifier TQualifier>
    constexpr ContiguousTuple& operator=(ContiguousTuple<TQualifier, Types...>&& other)
    {
        if (static_cast<const void*>(this) != static_cast<const void*>(std::addressof(other)))
        {
            this->assign(other, std::make_index_sequence<TYPE_COUNT>{});
        }
        return *this;
    }

    constexpr ContiguousTuple& operator=(cntgs::ContiguousElement<Types...>&& other)
    {
        this->assign(other.tuple, std::make_index_sequence<TYPE_COUNT>{});
        return *this;
    }

    template <detail::ContiguousTupleQualifier TQualifier, std::size_t... I>
    constexpr void swap(ContiguousTuple<TQualifier, Types...>& other, std::index_sequence<I...>)
    {
        (detail::ParameterTraits<Types>::swap(std::get<I>(this->tuple), std::get<I>(other.tuple)), ...);
    }

    [[nodiscard]] constexpr auto size_in_bytes() const noexcept { return this->end_address() - this->start_address(); }

    [[nodiscard]] constexpr auto start_address() const noexcept
    {
        return Locator::template ParameterTraitsAt<0>::start_address(std::get<0>(this->tuple));
    }

    [[nodiscard]] constexpr auto end_address() const noexcept
    {
        return Locator::template ParameterTraitsAt<TYPE_COUNT - 1>::end_address(std::get<TYPE_COUNT - 1>(this->tuple));
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
