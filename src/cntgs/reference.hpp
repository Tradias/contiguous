// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_CNTGS_REFERENCE_HPP
#define CNTGS_CNTGS_REFERENCE_HPP

#include "cntgs/detail/attributes.hpp"
#include "cntgs/detail/elementTraits.hpp"
#include "cntgs/detail/forward.hpp"
#include "cntgs/detail/parameterListTraits.hpp"
#include "cntgs/detail/reference.hpp"
#include "cntgs/detail/tuple.hpp"
#include "cntgs/detail/typeUtils.hpp"

#include <cstddef>
#include <cstring>
#include <tuple>

namespace cntgs
{
template <class... Types>
using ContiguousReference = cntgs::BasicContiguousReference<false, Types...>;

template <class... Types>
using ContiguousConstReference = cntgs::BasicContiguousReference<true, Types...>;

/// Reference type
template <bool IsConst, class... Types>
class BasicContiguousReference
{
  private:
    using ListTraits = detail::ParameterListTraits<Types...>;
    using ElementTraits = detail::ElementTraitsT<Types...>;
    using PointerTuple = detail::ToTupleOfContiguousPointer<Types...>;
    using Tuple = detail::ToTupleOfContiguousReferences<IsConst, Types...>;

    static constexpr auto IS_CONST = IsConst;

  public:
    PointerTuple tuple;

    BasicContiguousReference() = default;
    ~BasicContiguousReference() = default;

    BasicContiguousReference(const BasicContiguousReference&) = default;
    BasicContiguousReference(BasicContiguousReference&&) = default;

    template <bool OtherIsConst>
    /*implicit*/ constexpr BasicContiguousReference(
        const cntgs::BasicContiguousReference<OtherIsConst, Types...>& other) noexcept
        : tuple(other.tuple)
    {
    }

    template <class Allocator>
    /*implicit*/ constexpr BasicContiguousReference(
        const cntgs::BasicContiguousElement<Allocator, Types...>& other) noexcept
        : BasicContiguousReference(other.reference)
    {
    }

    template <class Allocator>
    /*implicit*/ constexpr BasicContiguousReference(cntgs::BasicContiguousElement<Allocator, Types...>& other) noexcept
        : BasicContiguousReference(other.reference)
    {
    }

    constexpr BasicContiguousReference& operator=(const BasicContiguousReference& other) noexcept(
        ListTraits::IS_NOTHROW_COPY_ASSIGNABLE)
    {
        this->assign(other);
        return *this;
    }

    template <bool OtherIsConst>
    constexpr BasicContiguousReference& operator=(const cntgs::BasicContiguousReference<OtherIsConst, Types...>&
                                                      other) noexcept(ListTraits::IS_NOTHROW_COPY_ASSIGNABLE)
    {
        this->assign(other);
        return *this;
    }

    template <class Allocator>
    constexpr BasicContiguousReference& operator=(const cntgs::BasicContiguousElement<Allocator, Types...>&
                                                      other) noexcept(ListTraits::IS_NOTHROW_COPY_ASSIGNABLE)
    {
        this->assign(other.reference);
        return *this;
    }

    constexpr BasicContiguousReference& operator=(BasicContiguousReference&& other) noexcept(
        ListTraits::IS_NOTHROW_MOVE_ASSIGNABLE)
    {
        this->assign(other);
        return *this;
    }

    template <bool OtherIsConst>
    constexpr BasicContiguousReference& operator=(cntgs::BasicContiguousReference<OtherIsConst, Types...>&&
                                                      other) noexcept(OtherIsConst
                                                                          ? ListTraits::IS_NOTHROW_COPY_ASSIGNABLE
                                                                          : ListTraits::IS_NOTHROW_MOVE_ASSIGNABLE)
    {
        this->assign(other);
        return *this;
    }

    template <class Allocator>
    constexpr BasicContiguousReference& operator=(cntgs::BasicContiguousElement<Allocator, Types...>&& other) noexcept(
        ListTraits::IS_NOTHROW_MOVE_ASSIGNABLE)
    {
        this->assign(other.reference);
        return *this;
    }

    [[nodiscard]] constexpr std::size_t size_in_bytes() const noexcept { return this->data_end() - this->data_begin(); }

    [[nodiscard]] constexpr auto data_begin() const noexcept
    {
        return ElementTraits::template ParameterTraitsAt<0>::data_begin(cntgs::get<0>(*this));
    }

    [[nodiscard]] constexpr auto data_end() const noexcept
    {
        return ElementTraits::template ParameterTraitsAt<sizeof...(Types) - 1>::data_end(
            cntgs::get<sizeof...(Types) - 1>(*this));
    }

    template <bool OtherIsConst>
    [[nodiscard]] constexpr auto operator==(const cntgs::BasicContiguousReference<OtherIsConst, Types...>& other) const
        noexcept(ListTraits::IS_NOTHROW_EQUALITY_COMPARABLE)
    {
        return ElementTraits::equal(*this, other);
    }

    template <class Allocator>
    [[nodiscard]] constexpr auto operator==(const cntgs::BasicContiguousElement<Allocator, Types...>& other) const
        noexcept(ListTraits::IS_NOTHROW_EQUALITY_COMPARABLE)
    {
        return *this == other.reference;
    }

    template <bool OtherIsConst>
    [[nodiscard]] constexpr auto operator!=(const cntgs::BasicContiguousReference<OtherIsConst, Types...>& other) const
        noexcept(ListTraits::IS_NOTHROW_EQUALITY_COMPARABLE)
    {
        return !(*this == other);
    }

    template <class Allocator>
    [[nodiscard]] constexpr auto operator!=(const cntgs::BasicContiguousElement<Allocator, Types...>& other) const
        noexcept(ListTraits::IS_NOTHROW_EQUALITY_COMPARABLE)
    {
        return !(*this == other.reference);
    }

    template <bool OtherIsConst>
    [[nodiscard]] constexpr auto operator<(const cntgs::BasicContiguousReference<OtherIsConst, Types...>& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return ElementTraits::lexicographical_compare(*this, other);
    }

    template <class Allocator>
    [[nodiscard]] constexpr auto operator<(const cntgs::BasicContiguousElement<Allocator, Types...>& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return *this < other.reference;
    }

    template <bool OtherIsConst>
    [[nodiscard]] constexpr auto operator<=(const cntgs::BasicContiguousReference<OtherIsConst, Types...>& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return !(other < *this);
    }

    template <class Allocator>
    [[nodiscard]] constexpr auto operator<=(const cntgs::BasicContiguousElement<Allocator, Types...>& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return !(other.reference < *this);
    }

    template <bool OtherIsConst>
    [[nodiscard]] constexpr auto operator>(const cntgs::BasicContiguousReference<OtherIsConst, Types...>& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return other < *this;
    }

    template <class Allocator>
    [[nodiscard]] constexpr auto operator>(const cntgs::BasicContiguousElement<Allocator, Types...>& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return other.reference < *this;
    }

    template <bool OtherIsConst>
    [[nodiscard]] constexpr auto operator>=(const cntgs::BasicContiguousReference<OtherIsConst, Types...>& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return !(*this < other);
    }

    template <class Allocator>
    [[nodiscard]] constexpr auto operator>=(const cntgs::BasicContiguousElement<Allocator, Types...>& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return !(*this < other.reference);
    }

  private:
    friend cntgs::BasicContiguousReference<!IsConst, Types...>;

    template <class, class...>
    friend class BasicContiguousVector;

    template <class, class...>
    friend class BasicContiguousElement;

    template <bool, class, class...>
    friend class ContiguousVectorIterator;

    explicit constexpr BasicContiguousReference(std::byte* CNTGS_RESTRICT address,
                                                const typename ListTraits::FixedSizesArray& fixed_sizes = {}) noexcept
        : BasicContiguousReference(ElementTraits::load_element_at(address, fixed_sizes))
    {
    }

    explicit constexpr BasicContiguousReference(const PointerTuple& tuple) noexcept : tuple(tuple) {}

    template <class Reference>
    void assign(Reference& other) const
    {
        static constexpr auto USE_MOVE = !std::is_const_v<Reference> && !Reference::IS_CONST;
        ElementTraits::template assign<USE_MOVE>(other, *this);
    }
};

template <bool IsConst, class... Types>
constexpr void swap(const cntgs::BasicContiguousReference<IsConst, Types...>& lhs,
                    const cntgs::BasicContiguousReference<IsConst, Types...>&
                        rhs) noexcept(detail::ParameterListTraits<Types...>::IS_NOTHROW_SWAPPABLE)
{
    detail::ElementTraitsT<Types...>::swap(rhs, lhs);
}

template <std::size_t I, bool IsConst, class... Types>
[[nodiscard]] constexpr std::tuple_element_t<I, cntgs::BasicContiguousReference<IsConst, Types...>> get(
    const cntgs::BasicContiguousReference<IsConst, Types...>& reference) noexcept
{
    if constexpr (IsConst)
    {
        return detail::as_const_ref(std::get<I>(reference.tuple));
    }
    else
    {
        return detail::as_ref(std::get<I>(reference.tuple));
    }
}
}  // namespace cntgs

#endif  // CNTGS_CNTGS_REFERENCE_HPP
