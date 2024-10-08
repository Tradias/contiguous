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
#include "cntgs/detail/utility.hpp"

#include <cstddef>
#include <cstring>
#include <tuple>

namespace cntgs
{
template <class... Parameter>
using ContiguousReference = cntgs::BasicContiguousReference<false, Parameter...>;

template <class... Parameter>
using ContiguousConstReference = cntgs::BasicContiguousReference<true, Parameter...>;

/// Reference type
template <bool IsConst, class... Parameter>
class BasicContiguousReference
{
  private:
    using ListTraits = detail::ParameterListTraits<Parameter...>;
    using ElementTraits = detail::ElementTraitsT<Parameter...>;
    using PointerTuple = detail::ToTupleOfContiguousPointer<Parameter...>;

    static constexpr auto IS_CONST = IsConst;

  public:
    PointerTuple tuple_;

    BasicContiguousReference() = default;
    ~BasicContiguousReference() = default;

    BasicContiguousReference(const BasicContiguousReference&) = default;
    BasicContiguousReference(BasicContiguousReference&&) = default;

    template <bool OtherIsConst>
    /*implicit*/ constexpr BasicContiguousReference(
        const cntgs::BasicContiguousReference<OtherIsConst, Parameter...>& other) noexcept
        : tuple_(other.tuple_)
    {
    }

    template <class Allocator>
    /*implicit*/ constexpr BasicContiguousReference(
        const cntgs::BasicContiguousElement<Allocator, Parameter...>& other) noexcept
        : BasicContiguousReference(other.reference_)
    {
    }

    template <class Allocator>
    /*implicit*/ constexpr BasicContiguousReference(
        cntgs::BasicContiguousElement<Allocator, Parameter...>& other) noexcept
        : BasicContiguousReference(other.reference_)
    {
    }

    constexpr BasicContiguousReference& operator=(const BasicContiguousReference& other) noexcept(
        ListTraits::IS_NOTHROW_COPY_ASSIGNABLE)
    {
        assign(other);
        return *this;
    }

    template <bool OtherIsConst>
    constexpr BasicContiguousReference& operator=(const cntgs::BasicContiguousReference<OtherIsConst, Parameter...>&
                                                      other) noexcept(ListTraits::IS_NOTHROW_COPY_ASSIGNABLE)
    {
        assign(other);
        return *this;
    }

    template <class Allocator>
    constexpr BasicContiguousReference& operator=(const cntgs::BasicContiguousElement<Allocator, Parameter...>&
                                                      other) noexcept(ListTraits::IS_NOTHROW_COPY_ASSIGNABLE)
    {
        assign(other.reference);
        return *this;
    }

    constexpr BasicContiguousReference& operator=(BasicContiguousReference&& other) noexcept(
        ListTraits::IS_NOTHROW_MOVE_ASSIGNABLE)
    {
        assign(other);
        return *this;
    }

    template <bool OtherIsConst>
    constexpr BasicContiguousReference& operator=(cntgs::BasicContiguousReference<OtherIsConst, Parameter...>&&
                                                      other) noexcept(OtherIsConst
                                                                          ? ListTraits::IS_NOTHROW_COPY_ASSIGNABLE
                                                                          : ListTraits::IS_NOTHROW_MOVE_ASSIGNABLE)
    {
        assign(other);
        return *this;
    }

    template <class Allocator>
    constexpr BasicContiguousReference& operator=(
        cntgs::BasicContiguousElement<Allocator, Parameter...>&& other) noexcept(ListTraits::IS_NOTHROW_MOVE_ASSIGNABLE)
    {
        assign(other.reference_);
        return *this;
    }

    [[nodiscard]] constexpr std::size_t size_in_bytes() const noexcept { return data_end() - data_begin(); }

    [[nodiscard]] constexpr auto data_begin() const noexcept
    {
        return ElementTraits::template ParameterTraitsAt<0>::data_begin(cntgs::get<0>(*this));
    }

    [[nodiscard]] constexpr auto data_end() const noexcept
    {
        return ElementTraits::template ParameterTraitsAt<sizeof...(Parameter) - 1>::data_end(
            cntgs::get<sizeof...(Parameter) - 1>(*this));
    }

    friend constexpr void swap(const BasicContiguousReference& lhs,
                               const BasicContiguousReference& rhs) noexcept(ListTraits::IS_NOTHROW_SWAPPABLE)
    {
        ElementTraits::swap(rhs, lhs);
    }

    template <bool OtherIsConst>
    [[nodiscard]] constexpr auto operator==(const cntgs::BasicContiguousReference<OtherIsConst, Parameter...>& other)
        const noexcept(ListTraits::IS_NOTHROW_EQUALITY_COMPARABLE)
    {
        return ElementTraits::equal(*this, other);
    }

    template <class Allocator>
    [[nodiscard]] constexpr auto operator==(const cntgs::BasicContiguousElement<Allocator, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTHROW_EQUALITY_COMPARABLE)
    {
        return *this == other.reference_;
    }

    template <bool OtherIsConst>
    [[nodiscard]] constexpr auto operator!=(const cntgs::BasicContiguousReference<OtherIsConst, Parameter...>& other)
        const noexcept(ListTraits::IS_NOTHROW_EQUALITY_COMPARABLE)
    {
        return !(*this == other);
    }

    template <class Allocator>
    [[nodiscard]] constexpr auto operator!=(const cntgs::BasicContiguousElement<Allocator, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTHROW_EQUALITY_COMPARABLE)
    {
        return !(*this == other.reference_);
    }

    template <bool OtherIsConst>
    [[nodiscard]] constexpr auto operator<(const cntgs::BasicContiguousReference<OtherIsConst, Parameter...>& other)
        const noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return ElementTraits::lexicographical_compare(*this, other);
    }

    template <class Allocator>
    [[nodiscard]] constexpr auto operator<(const cntgs::BasicContiguousElement<Allocator, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return *this < other.reference_;
    }

    template <bool OtherIsConst>
    [[nodiscard]] constexpr auto operator<=(const cntgs::BasicContiguousReference<OtherIsConst, Parameter...>& other)
        const noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return !(other < *this);
    }

    template <class Allocator>
    [[nodiscard]] constexpr auto operator<=(const cntgs::BasicContiguousElement<Allocator, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return !(other.reference_ < *this);
    }

    template <bool OtherIsConst>
    [[nodiscard]] constexpr auto operator>(const cntgs::BasicContiguousReference<OtherIsConst, Parameter...>& other)
        const noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return other < *this;
    }

    template <class Allocator>
    [[nodiscard]] constexpr auto operator>(const cntgs::BasicContiguousElement<Allocator, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return other.reference_ < *this;
    }

    template <bool OtherIsConst>
    [[nodiscard]] constexpr auto operator>=(const cntgs::BasicContiguousReference<OtherIsConst, Parameter...>& other)
        const noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return !(*this < other);
    }

    template <class Allocator>
    [[nodiscard]] constexpr auto operator>=(const cntgs::BasicContiguousElement<Allocator, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return !(*this < other.reference_);
    }

  private:
    friend cntgs::BasicContiguousReference<!IsConst, Parameter...>;

    template <class, class...>
    friend class BasicContiguousVector;

    template <class, class...>
    friend class BasicContiguousElement;

    template <bool, class, class...>
    friend class ContiguousVectorIterator;

    constexpr explicit BasicContiguousReference(std::byte* CNTGS_RESTRICT address,
                                                const typename ListTraits::FixedSizesArray& fixed_sizes = {}) noexcept
        : BasicContiguousReference(ElementTraits::load_element_at(address, fixed_sizes))
    {
    }

    constexpr explicit BasicContiguousReference(const PointerTuple& tuple) noexcept : tuple_(tuple) {}

    template <class Reference>
    void assign(Reference& other) const
    {
        static constexpr auto USE_MOVE = !std::is_const_v<Reference> && !Reference::IS_CONST;
        ElementTraits::template assign<USE_MOVE>(other, *this);
    }
};

template <std::size_t I, bool IsConst, class... Parameter>
[[nodiscard]] constexpr std::tuple_element_t<I, cntgs::BasicContiguousReference<IsConst, Parameter...>> get(
    const cntgs::BasicContiguousReference<IsConst, Parameter...>& reference) noexcept
{
    if constexpr (IsConst)
    {
        return detail::as_const_ref(std::get<I>(reference.tuple_));
    }
    else
    {
        return detail::as_ref(std::get<I>(reference.tuple_));
    }
}
}  // namespace cntgs

#endif  // CNTGS_CNTGS_REFERENCE_HPP
