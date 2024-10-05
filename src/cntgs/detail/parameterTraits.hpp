// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_DETAIL_PARAMETERTRAITS_HPP
#define CNTGS_DETAIL_PARAMETERTRAITS_HPP

#include "cntgs/detail/memory.hpp"
#include "cntgs/detail/parameterType.hpp"
#include "cntgs/parameter.hpp"
#include "cntgs/span.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <memory>
#include <new>
#include <type_traits>

namespace cntgs::detail
{
struct AlignedSizeInMemory
{
    std::size_t offset;
    std::size_t size;
    std::size_t padding;
};

template <class T>
struct VaryingSizeAddresses
{
    T* value;
    std::size_t* size;
};

template <class T>
struct ParameterTraits : detail::ParameterTraits<cntgs::AlignAs<T, 1>>
{
};

template <class T, std::size_t Alignment>
struct ParameterTraits<cntgs::AlignAs<T, Alignment>>
{
    using ValueType = T;
    using PointerType = std::add_pointer_t<T>;
    using ReferenceType = std::add_lvalue_reference_t<T>;
    using ConstReferenceType = std::add_lvalue_reference_t<std::add_const_t<T>>;

    static constexpr auto TYPE = detail::ParameterType::PLAIN;
    static constexpr auto ALIGNMENT = Alignment;
    static constexpr auto VALUE_ALIGNMENT = Alignment;
    static constexpr auto VALUE_BYTES = sizeof(T);
    static constexpr auto TRAILING_ALIGNMENT = detail::trailing_alignment(VALUE_BYTES, ALIGNMENT);

    template <std::size_t PreviousAlignment, bool>
    static auto load(std::byte* address, std::size_t) noexcept
    {
        address = detail::align_if<(PreviousAlignment < ALIGNMENT), ALIGNMENT>(address);
        assert(detail::is_aligned(address, ALIGNMENT));
        auto result = std::launder(reinterpret_cast<PointerType>(address));
        return std::pair{result, address + VALUE_BYTES};
    }

    template <std::size_t PreviousAlignment, bool, class Arg>
    static std::byte* store(Arg&& arg, std::byte* address, std::size_t)
    {
        address = detail::align_if<(PreviousAlignment < ALIGNMENT), ALIGNMENT>(address);
        assert(detail::is_aligned(address, ALIGNMENT));
        detail::construct_at(reinterpret_cast<T*>(address), std::forward<Arg>(arg));
        return address + VALUE_BYTES;
    }

    template <std::size_t PreviousAlignment, std::size_t NextAlignment>
    static constexpr AlignedSizeInMemory aligned_size_in_memory(std::size_t offset, std::size_t) noexcept
    {
        const auto alignment_offset = detail::align_if<(PreviousAlignment < ALIGNMENT), ALIGNMENT>(offset);
        const auto size = alignment_offset - offset + VALUE_BYTES;
        const auto new_offset = offset + size;
        const auto padding_offset = detail::align_if<(TRAILING_ALIGNMENT < NextAlignment), NextAlignment>(new_offset);
        return {new_offset, size, padding_offset - new_offset};
    }

    static auto data_begin(ConstReferenceType reference) noexcept
    {
        return detail::assume_aligned<ALIGNMENT>(reinterpret_cast<const std::byte*>(std::addressof(reference)));
    }

    static auto data_begin(ReferenceType reference) noexcept
    {
        return detail::assume_aligned<ALIGNMENT>(reinterpret_cast<std::byte*>(std::addressof(reference)));
    }

    static auto data_end(ConstReferenceType reference) noexcept { return data_begin(reference) + VALUE_BYTES; }

    static auto data_end(ReferenceType reference) noexcept { return data_begin(reference) + VALUE_BYTES; }

    static constexpr void copy(ConstReferenceType source,
                               ReferenceType target) noexcept(std::is_nothrow_copy_assignable_v<T>)
    {
        target = source;
    }

    static constexpr void move(T&& source, ReferenceType target) noexcept(std::is_nothrow_move_assignable_v<T>)
    {
        target = std::move(source);
    }

    static constexpr void move(ReferenceType source,
                               ReferenceType target) noexcept(std::is_nothrow_move_assignable_v<T>)
    {
        target = std::move(source);
    }

    static constexpr void uninitialized_copy(ConstReferenceType source,
                                             T* target) noexcept(std::is_nothrow_copy_constructible_v<T>)
    {
        detail::construct_at(target, source);
    }

    static constexpr void uninitialized_move(ReferenceType source,
                                             T* target) noexcept(std::is_nothrow_move_constructible_v<T>)
    {
        detail::construct_at(target, std::move(source));
    }

    static constexpr void swap(ReferenceType lhs, ReferenceType rhs) noexcept(std::is_nothrow_swappable_v<T>)
    {
        using std::swap;
        swap(lhs, rhs);
    }

    static constexpr auto equal(ConstReferenceType lhs, ConstReferenceType rhs) { return lhs == rhs; }

    static constexpr auto lexicographical_compare(ConstReferenceType lhs, ConstReferenceType rhs) { return lhs < rhs; }

    static constexpr void destroy(ReferenceType value) noexcept { value.~T(); }
};

template <class T, std::size_t Alignment>
struct BaseContiguousParameterTraits
{
    using Self = BaseContiguousParameterTraits<T, Alignment>;

    static constexpr auto VALUE_BYTES = sizeof(T);

    static auto data_end(const cntgs::Span<std::add_const_t<T>>& value) noexcept
    {
        return reinterpret_cast<const std::byte*>(std::end(value));
    }

    static auto data_end(const cntgs::Span<T>& value) noexcept { return reinterpret_cast<std::byte*>(std::end(value)); }

    static constexpr void uninitialized_copy(
        const cntgs::Span<std::add_const_t<T>>& source,
        const cntgs::Span<T>& target) noexcept(std::is_nothrow_copy_constructible_v<T>)
    {
        std::uninitialized_copy(Self::begin(source), std::end(source), Self::begin(target));
    }

    static constexpr void uninitialized_copy(const cntgs::Span<T>& source, const cntgs::Span<T>& target) noexcept(
        std::is_nothrow_copy_constructible_v<T>)
    {
        std::uninitialized_copy(Self::begin(source), std::end(source), Self::begin(target));
    }

    static constexpr void uninitialized_move(const cntgs::Span<T>& source, const cntgs::Span<T>& target) noexcept(
        std::is_nothrow_move_constructible_v<T>)
    {
        std::uninitialized_copy(std::make_move_iterator(Self::begin(source)), std::make_move_iterator(std::end(source)),
                                Self::begin(target));
    }

    static constexpr auto equal(const cntgs::Span<T>& source, const cntgs::Span<T>& target)
    {
        return std::equal(Self::begin(source), std::end(source), Self::begin(target));
    }

    static constexpr auto equal(const cntgs::Span<T>& source, const cntgs::Span<std::add_const_t<T>>& target)
    {
        return std::equal(Self::begin(source), std::end(source), Self::begin(target));
    }

    static constexpr auto equal(const cntgs::Span<std::add_const_t<T>>& source, const cntgs::Span<T>& target)
    {
        return std::equal(Self::begin(source), std::end(source), Self::begin(target));
    }

    static constexpr auto equal(const cntgs::Span<std::add_const_t<T>>& source,
                                const cntgs::Span<std::add_const_t<T>>& target)
    {
        return std::equal(Self::begin(source), std::end(source), Self::begin(target));
    }

    static constexpr auto lexicographical_compare(const cntgs::Span<T>& source, const cntgs::Span<T>& target)
    {
        return std::lexicographical_compare(Self::begin(source), std::end(source), Self::begin(target),
                                            std::end(target));
    }

    static constexpr auto lexicographical_compare(const cntgs::Span<T>& source,
                                                  const cntgs::Span<std::add_const_t<T>>& target)
    {
        return std::lexicographical_compare(Self::begin(source), std::end(source), Self::begin(target),
                                            std::end(target));
    }

    static constexpr auto lexicographical_compare(const cntgs::Span<std::add_const_t<T>>& source,
                                                  const cntgs::Span<T>& target)
    {
        return std::lexicographical_compare(Self::begin(source), std::end(source), Self::begin(target),
                                            std::end(target));
    }

    static constexpr auto lexicographical_compare(const cntgs::Span<std::add_const_t<T>>& source,
                                                  const cntgs::Span<std::add_const_t<T>>& target)
    {
        return std::lexicographical_compare(Self::begin(source), std::end(source), Self::begin(target),
                                            std::end(target));
    }

    static constexpr void destroy(const cntgs::Span<T>& value) noexcept
    {
        std::destroy(Self::begin(value), std::end(value));
    }

    template <class U>
    static constexpr auto begin(const cntgs::Span<U>& value) noexcept
    {
        return detail::assume_aligned<Alignment>(std::begin(value));
    }
};

template <class T>
struct ParameterTraits<cntgs::VaryingSize<T>> : ParameterTraits<cntgs::VaryingSize<cntgs::AlignAs<T, 1>>>
{
};

template <class T, std::size_t Alignment>
struct ParameterTraits<cntgs::VaryingSize<cntgs::AlignAs<T, Alignment>>> : BaseContiguousParameterTraits<T, Alignment>
{
    using ValueType = T;
    using PointerType = cntgs::Span<T>;
    using ReferenceType = cntgs::Span<T>;
    using ConstReferenceType = cntgs::Span<std::add_const_t<T>>;
    using IteratorType = T*;

    static constexpr auto TYPE = detail::ParameterType::VARYING_SIZE;
    static constexpr auto ALIGNMENT = alignof(std::size_t);
    static constexpr auto VALUE_ALIGNMENT = (std::max)(detail::SIZE_T_TRAILING_ALIGNMENT, Alignment);

    template <std::size_t PreviousAlignment, bool IsSizeProvided>
    static auto load(std::byte* address, std::size_t size) noexcept
    {
        const auto [value_address, size_address] = get_addresses<PreviousAlignment>(address);
        if constexpr (!IsSizeProvided)
        {
            size = *size_address;
        }
        const auto first = std::launder(reinterpret_cast<IteratorType>(value_address));
        const auto last = std::launder(reinterpret_cast<IteratorType>(value_address) + size);
        return std::pair{PointerType{first, last}, reinterpret_cast<std::byte*>(last)};
    }

    template <std::size_t PreviousAlignment, bool IgnoreAliasing, class Range>
    static std::byte* store(Range&& range, std::byte* address, std::size_t)
    {
        const auto [value_address, size_address] = get_addresses<PreviousAlignment>(address);
        auto* new_address =
            detail::uninitialized_range_construct<IgnoreAliasing>(std::forward<Range>(range), value_address);
        *size_address = reinterpret_cast<IteratorType>(new_address) - value_address;
        return new_address;
    }

    template <std::size_t PreviousAlignment, std::size_t NextAlignment>
    static constexpr AlignedSizeInMemory aligned_size_in_memory(std::size_t offset, std::size_t) noexcept
    {
        const auto size_t_alignment_offset = detail::align_if<(PreviousAlignment < ALIGNMENT), ALIGNMENT>(offset);
        const auto value_alignment_offset =
            detail::align_if<(detail::SIZE_T_TRAILING_ALIGNMENT < VALUE_ALIGNMENT), VALUE_ALIGNMENT>(
                size_t_alignment_offset + sizeof(std::size_t));
        const auto trailing_alignment =
            detail::trailing_alignment(sizeof(T), detail::extract_lowest_set_bit(value_alignment_offset));
        const auto next_alignment_difference =
            trailing_alignment < NextAlignment ? NextAlignment - trailing_alignment : 0;
        return {0, value_alignment_offset - offset + next_alignment_difference, 0};
    }

    static auto data_begin(const cntgs::Span<std::add_const_t<T>>& value) noexcept
    {
        return reinterpret_cast<const std::byte*>(ParameterTraits::begin(value)) - sizeof(std::size_t);
    }

    static auto data_begin(const cntgs::Span<T>& value) noexcept
    {
        return reinterpret_cast<std::byte*>(ParameterTraits::begin(value)) - sizeof(std::size_t);
    }

    template <std::size_t PreviousAlignment>
    static VaryingSizeAddresses<T> get_addresses(std::byte* address) noexcept
    {
        address = detail::align_if<(PreviousAlignment < ALIGNMENT), ALIGNMENT>(address);
        assert(detail::is_aligned(address, ALIGNMENT));
        auto size = reinterpret_cast<std::size_t*>(address);
        address += sizeof(std::size_t);
        const auto aligned_address = reinterpret_cast<IteratorType>(
            detail::align_if<(detail::SIZE_T_TRAILING_ALIGNMENT < VALUE_ALIGNMENT), VALUE_ALIGNMENT>(address));
        assert(detail::is_aligned(aligned_address, VALUE_ALIGNMENT));
        return {aligned_address, size};
    }
};

template <class T>
struct ParameterTraits<cntgs::FixedSize<T>> : ParameterTraits<cntgs::FixedSize<cntgs::AlignAs<T, 1>>>
{
};

template <class T, std::size_t Alignment>
struct ParameterTraits<cntgs::FixedSize<cntgs::AlignAs<T, Alignment>>> : BaseContiguousParameterTraits<T, Alignment>
{
    using ValueType = T;
    using PointerType = cntgs::Span<T>;
    using ReferenceType = cntgs::Span<T>;
    using ConstReferenceType = cntgs::Span<std::add_const_t<T>>;
    using IteratorType = T*;

    static constexpr auto TYPE = detail::ParameterType::FIXED_SIZE;
    static constexpr auto ALIGNMENT = Alignment;
    static constexpr auto VALUE_ALIGNMENT = Alignment;
    using ParameterTraits::BaseContiguousParameterTraits::VALUE_BYTES;
    static constexpr auto TRAILING_ALIGNMENT = detail::trailing_alignment(VALUE_BYTES, ALIGNMENT);

    template <std::size_t PreviousAlignment, bool>
    static auto load(std::byte* address, std::size_t size) noexcept
    {
        const auto first = std::launder(
            reinterpret_cast<IteratorType>(detail::align_if<(PreviousAlignment < ALIGNMENT), ALIGNMENT>(address)));
        assert(detail::is_aligned(first, ALIGNMENT));
        const auto last = first + size;
        return std::pair{PointerType{first, last}, reinterpret_cast<std::byte*>(last)};
    }

    template <std::size_t PreviousAlignment, bool IgnoreAliasing, class RangeOrIterator>
    static std::byte* store(RangeOrIterator&& range_or_iterator, std::byte* address, std::size_t size)
    {
        const auto aligned_address =
            reinterpret_cast<IteratorType>(detail::align_if<(PreviousAlignment < ALIGNMENT), ALIGNMENT>(address));
        assert(detail::is_aligned(aligned_address, ALIGNMENT));
        return detail::uninitialized_construct<IgnoreAliasing>(std::forward<RangeOrIterator>(range_or_iterator),
                                                               aligned_address, size);
    }

    template <std::size_t PreviousAlignment, std::size_t NextAlignment>
    static constexpr AlignedSizeInMemory aligned_size_in_memory(std::size_t offset, std::size_t fixed_size) noexcept
    {
        const auto alignment_offset = detail::align_if<(PreviousAlignment < ALIGNMENT), ALIGNMENT>(offset);
        const auto size = alignment_offset - offset + VALUE_BYTES * fixed_size;
        const auto new_offset = offset + size;
        const auto padding_offset = detail::align_if<(TRAILING_ALIGNMENT < NextAlignment), NextAlignment>(new_offset);
        return {new_offset, size, padding_offset - new_offset};
    }

    static auto data_begin(const cntgs::Span<std::add_const_t<T>>& value) noexcept
    {
        return reinterpret_cast<const std::byte*>(ParameterTraits::begin(value));
    }

    static auto data_begin(const cntgs::Span<T>& value) noexcept
    {
        return reinterpret_cast<std::byte*>(ParameterTraits::begin(value));
    }

    static void copy(const cntgs::Span<std::add_const_t<T>>& source,
                     const cntgs::Span<T>& target) noexcept(std::is_nothrow_copy_assignable_v<T>)
    {
        std::copy(std::begin(source), std::end(source), std::begin(target));
    }

    static void copy(const cntgs::Span<T>& source,
                     const cntgs::Span<T>& target) noexcept(std::is_nothrow_copy_assignable_v<T>)
    {
        std::copy(std::begin(source), std::end(source), std::begin(target));
    }

    static void move(const cntgs::Span<T>& source,
                     const cntgs::Span<T>& target) noexcept(std::is_nothrow_move_assignable_v<T>)
    {
        std::move(std::begin(source), std::end(source), std::begin(target));
    }

    static void swap(const cntgs::Span<T>& lhs, const cntgs::Span<T>& rhs) noexcept(std::is_nothrow_swappable_v<T>)
    {
        std::swap_ranges(std::begin(lhs), std::end(lhs), std::begin(rhs));
    }
};
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_PARAMETERTRAITS_HPP
