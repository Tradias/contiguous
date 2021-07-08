#pragma once

#include "cntgs/contiguous/detail/attributes.h"
#include "cntgs/contiguous/detail/elementTraits.h"
#include "cntgs/contiguous/detail/forward.h"
#include "cntgs/contiguous/detail/parameterListTraits.h"
#include "cntgs/contiguous/detail/tuple.h"
#include "cntgs/contiguous/detail/tupleQualifier.h"
#include "cntgs/contiguous/detail/typeUtils.h"

#include <cstddef>
#include <cstring>
#include <tuple>

namespace cntgs
{
template <detail::ContiguousTupleQualifier Qualifier, class... Types>
class ContiguousTuple
{
  private:
    using ListTraits = detail::ParameterListTraits<Types...>;
    using ElementTraits = detail::ElementTraitsT<Types...>;
    using PointerTuple = detail::ToContiguousTupleOfPointerReturnType<std::tuple<Types...>>;

  public:
    using Tuple = detail::ConditionalT<(Qualifier == detail::ContiguousTupleQualifier::REFERENCE),
                                       detail::ToContiguousTupleOfReferenceReturnType<std::tuple<Types...>>,
                                       detail::ToContiguousTupleOfConstReferenceReturnType<std::tuple<Types...>>>;

    static constexpr auto IS_CONST = detail::ContiguousTupleQualifier::CONST_REFERENCE == Qualifier;

    Tuple tuple;

    ContiguousTuple() = default;
    ~ContiguousTuple() = default;

    ContiguousTuple(const ContiguousTuple&) = default;
    ContiguousTuple(ContiguousTuple&&) = default;

    explicit constexpr ContiguousTuple(std::byte* CNTGS_RESTRICT address,
                                       const typename ListTraits::FixedSizes& fixed_sizes = {}) noexcept
        : ContiguousTuple(ElementTraits::load_element_at(address, fixed_sizes))
    {
    }

    explicit constexpr ContiguousTuple(const PointerTuple& tuple) noexcept
        : tuple(detail::convert_tuple_to<Tuple>(tuple))
    {
    }

    template <detail::ContiguousTupleQualifier TQualifier>
    /*implicit*/ constexpr ContiguousTuple(const ContiguousTuple<TQualifier, Types...>& other) noexcept
        : tuple(other.tuple)
    {
    }

    template <class Allocator>
    /*implicit*/ constexpr ContiguousTuple(const cntgs::BasicContiguousElement<Allocator, Types...>& other) noexcept
        : tuple(other.tuple)
    {
    }

    template <detail::ContiguousTupleQualifier TQualifier>
    /*implicit*/ constexpr ContiguousTuple(ContiguousTuple<TQualifier, Types...>&& other) noexcept
        : tuple(std::move(other.tuple))
    {
    }

    template <class Allocator>
    /*implicit*/ constexpr ContiguousTuple(cntgs::BasicContiguousElement<Allocator, Types...>&& other) noexcept
        : tuple(std::move(other.tuple))
    {
    }

    constexpr ContiguousTuple& operator=(const ContiguousTuple& other) noexcept(ListTraits::IS_NOTHROW_COPY_ASSIGNABLE)
    {
        this->assign(other);
        return *this;
    }

    template <detail::ContiguousTupleQualifier TQualifier>
    constexpr ContiguousTuple& operator=(const ContiguousTuple<TQualifier, Types...>& other) noexcept(
        ListTraits::IS_NOTHROW_COPY_ASSIGNABLE)
    {
        this->assign(other);
        return *this;
    }

    template <class Allocator>
    constexpr ContiguousTuple& operator=(const cntgs::BasicContiguousElement<Allocator, Types...>& other) noexcept(
        ListTraits::IS_NOTHROW_COPY_ASSIGNABLE)
    {
        this->assign(other.tuple);
        return *this;
    }

    constexpr ContiguousTuple& operator=(ContiguousTuple&& other) noexcept(ListTraits::IS_NOTHROW_MOVE_ASSIGNABLE)
    {
        this->assign(other);
        return *this;
    }

    template <detail::ContiguousTupleQualifier TQualifier>
    constexpr ContiguousTuple& operator=(ContiguousTuple<TQualifier, Types...>&& other) noexcept(
        ContiguousTuple<TQualifier, Types...>::IS_CONST ? ListTraits::IS_NOTHROW_MOVE_ASSIGNABLE
                                                        : ListTraits::IS_NOTHROW_COPY_ASSIGNABLE)
    {
        this->assign(other);
        return *this;
    }

    template <class Allocator>
    constexpr ContiguousTuple& operator=(cntgs::BasicContiguousElement<Allocator, Types...>&& other) noexcept(
        ListTraits::IS_NOTHROW_MOVE_ASSIGNABLE)
    {
        this->assign(other.tuple);
        return *this;
    }

    constexpr void swap(const ContiguousTuple& other) const noexcept(ListTraits::IS_NOTHROW_SWAPPABLE)
    {
        ElementTraits::swap(other, *this);
    }

    [[nodiscard]] constexpr auto size_in_bytes() const noexcept { return this->end_address() - this->start_address(); }

    [[nodiscard]] constexpr auto start_address() const noexcept
    {
        return ElementTraits::template ParameterTraitsAt<0>::start_address(std::get<0>(this->tuple));
    }

    [[nodiscard]] constexpr auto end_address() const noexcept
    {
        return ElementTraits::template ParameterTraitsAt<sizeof...(Types) - 1>::end_address(
            std::get<sizeof...(Types) - 1>(this->tuple));
    }

  private:
    template <class Tuple>
    void assign(Tuple& other) const
    {
        static constexpr auto USE_MOVE = !std::is_const_v<Tuple> && !Tuple::IS_CONST;
        ElementTraits::template assign<USE_MOVE>(other, *this);
    }
};

template <detail::ContiguousTupleQualifier Qualifier, class... T>
constexpr void swap(const cntgs::ContiguousTuple<Qualifier, T...>& lhs,
                    const cntgs::ContiguousTuple<Qualifier, T...>&
                        rhs) noexcept(detail::ParameterListTraits<T...>::IS_NOTHROW_SWAPPABLE)
{
    lhs.swap(rhs);
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
