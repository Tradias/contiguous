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
    std::size_t alignment;
};

struct ForwardSizeInMemory
{
    std::size_t offset;
};

struct BackwardSizeInMemory
{
    std::size_t offset;
    std::size_t padding;
};

struct TrailingAlignmentResult
{
    std::size_t offset;
    std::size_t alignment_bracket;
    std::size_t trailing_alignment;
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
    static constexpr auto VALUE_BYTES = sizeof(T);
    static constexpr auto TRAILING_ALIGNMENT = detail::trailing_alignment(VALUE_BYTES, ALIGNMENT);

    template <std::size_t PreviousTrailingAlignment, class SizeType>
    static auto load(std::byte* address, const SizeType&) noexcept
    {
        address = detail::align_if<(PreviousTrailingAlignment < ALIGNMENT), ALIGNMENT>(address);
        assert(detail::is_aligned(address, ALIGNMENT));
        auto result = std::launder(reinterpret_cast<PointerType>(address));
        return std::pair{result, address + VALUE_BYTES};
    }

    template <std::size_t PreviousTrailingAlignment, bool, class Arg>
    static std::byte* store(Arg&& arg, std::byte* address, std::size_t)
    {
        address = detail::align_if<(PreviousTrailingAlignment < ALIGNMENT), ALIGNMENT>(address);
        assert(detail::is_aligned(address, ALIGNMENT));
        detail::construct_at(reinterpret_cast<T*>(address), static_cast<Arg&&>(arg));
        return address + VALUE_BYTES;
    }

    static constexpr TrailingAlignmentResult trailing_alignment(std::size_t offset, std::size_t alignment) noexcept
    {
        std::size_t new_offset{};
        if (alignment < ALIGNMENT)
        {
            new_offset = VALUE_BYTES;
        }
        else
        {
            const auto alignment_offset = detail::align(offset, ALIGNMENT);
            const auto size = alignment_offset - offset + VALUE_BYTES;
            new_offset = offset + size;
        }
        alignment = (std::max)(alignment, ALIGNMENT);
        const auto trailing_alignment = detail::trailing_alignment(new_offset, alignment);
        return {new_offset, alignment, trailing_alignment};
    }

    template <std::size_t PreviousTrailingAlignment, std::size_t NextAlignment>
    static constexpr AlignedSizeInMemory aligned_size_in_memory(std::size_t offset, std::size_t alignment,
                                                                std::size_t) noexcept
    {
        std::size_t new_offset{};
        std::size_t size{};
        if (alignment < ALIGNMENT)
        {
            const auto alignment_offset = detail::align(offset, alignment) + ALIGNMENT - alignment;
            size = alignment_offset - offset + VALUE_BYTES;
            new_offset = VALUE_BYTES;
        }
        else
        {
            const auto alignment_offset = detail::align_if<(PreviousTrailingAlignment < ALIGNMENT), ALIGNMENT>(offset);
            size = alignment_offset - offset + VALUE_BYTES;
            new_offset = offset + size;
        }
        const auto padding_offset = detail::align_if<(TRAILING_ALIGNMENT < NextAlignment), NextAlignment>(new_offset);
        return {new_offset, size, padding_offset - new_offset, (std::max)(alignment, ALIGNMENT)};
    }

    static constexpr ForwardSizeInMemory forward_size_in_memory(std::size_t offset, std::size_t) noexcept
    {
        const auto padding = detail::align(offset, ALIGNMENT) - offset;
        const auto next_offset = offset + padding + VALUE_BYTES;
        return {next_offset};
    }

    static constexpr BackwardSizeInMemory backward_size_in_memory(std::size_t offset, std::size_t) noexcept
    {
        const auto next_offset = offset + VALUE_BYTES;
        const auto padding = detail::align(next_offset, ALIGNMENT) - next_offset;
        return {next_offset, padding};
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

    template <class U>
    static constexpr auto begin(const cntgs::Span<U>& value) noexcept
    {
        return detail::assume_aligned<Alignment>(std::begin(value));
    }

    static auto data_begin(const cntgs::Span<std::add_const_t<T>>& value) noexcept
    {
        return reinterpret_cast<const std::byte*>(Self::begin(value));
    }

    static auto data_begin(const cntgs::Span<T>& value) noexcept
    {
        return reinterpret_cast<std::byte*>(Self::begin(value));
    }

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
    static constexpr auto ALIGNMENT = Alignment;

    template <std::size_t PreviousTrailingAlignment, class SizeType>
    static auto load(std::byte* address, const SizeType& size) noexcept
    {
        const auto first = std::launder(reinterpret_cast<IteratorType>(
            detail::align_if<(PreviousTrailingAlignment < ALIGNMENT), ALIGNMENT>(address)));
        assert(detail::is_aligned(first, ALIGNMENT));
        const auto last = first + size;
        return std::pair{PointerType{first, last}, reinterpret_cast<std::byte*>(last)};
    }

    template <std::size_t PreviousTrailingAlignment, bool IgnoreAliasing, class Range>
    static std::byte* store(Range&& range, std::byte* address, std::size_t)
    {
        const auto aligned_address = reinterpret_cast<IteratorType>(
            detail::align_if<(PreviousTrailingAlignment < ALIGNMENT), ALIGNMENT>(address));
        assert(detail::is_aligned(aligned_address, ALIGNMENT));
        return detail::uninitialized_construct<IgnoreAliasing>(static_cast<Range&&>(range), aligned_address, {});
    }

    static constexpr TrailingAlignmentResult trailing_alignment(std::size_t offset, std::size_t alignment) noexcept
    {
        const auto alignment_offset = detail::align(offset, ALIGNMENT);
        alignment = (std::max)(alignment, ALIGNMENT);
        const auto leading_alignment = (std::max)(ALIGNMENT, detail::trailing_alignment(alignment_offset, alignment));
        const auto trailing_alignment = detail::trailing_alignment(sizeof(T), leading_alignment);
        return {{}, trailing_alignment, trailing_alignment};
    }

    template <std::size_t PreviousTrailingAlignment, std::size_t NextAlignment>
    static constexpr AlignedSizeInMemory aligned_size_in_memory(std::size_t offset, std::size_t alignment,
                                                                std::size_t) noexcept
    {
        std::size_t alignment_offset{};
        if (alignment < ALIGNMENT)
        {
            alignment_offset = detail::align(offset, alignment) + ALIGNMENT - alignment;
        }
        else
        {
            alignment_offset = detail::align_if<(PreviousTrailingAlignment < ALIGNMENT), ALIGNMENT>(offset);
        }
        const auto trailing_alignment = (std::min)(
            alignment, detail::trailing_alignment(sizeof(T), detail::extract_lowest_set_bit(alignment_offset)));
        const auto next_alignment_difference =
            trailing_alignment < NextAlignment ? NextAlignment - trailing_alignment : 0;
        return {{}, alignment_offset - offset, next_alignment_difference, trailing_alignment};
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
    using ParameterTraits::BaseContiguousParameterTraits::VALUE_BYTES;
    static constexpr auto TRAILING_ALIGNMENT = detail::trailing_alignment(VALUE_BYTES, ALIGNMENT);

    template <std::size_t PreviousTrailingAlignment, class SizeType>
    static auto load(std::byte* address, const SizeType& size) noexcept
    {
        const auto first = std::launder(reinterpret_cast<IteratorType>(
            detail::align_if<(PreviousTrailingAlignment < ALIGNMENT), ALIGNMENT>(address)));
        assert(detail::is_aligned(first, ALIGNMENT));
        const auto last = first + size;
        return std::pair{PointerType{first, last}, reinterpret_cast<std::byte*>(last)};
    }

    template <std::size_t PreviousTrailingAlignment, bool IgnoreAliasing, class RangeOrIterator>
    static std::byte* store(RangeOrIterator&& range_or_iterator, std::byte* address, std::size_t size)
    {
        const auto aligned_address = reinterpret_cast<IteratorType>(
            detail::align_if<(PreviousTrailingAlignment < ALIGNMENT), ALIGNMENT>(address));
        assert(detail::is_aligned(aligned_address, ALIGNMENT));
        return detail::uninitialized_construct<IgnoreAliasing>(static_cast<RangeOrIterator&&>(range_or_iterator),
                                                               aligned_address, size);
    }

    static constexpr TrailingAlignmentResult trailing_alignment(std::size_t offset, std::size_t alignment) noexcept
    {
        const auto alignment_offset = detail::align(offset, ALIGNMENT);
        alignment = (std::max)(alignment, ALIGNMENT);
        const auto leading_alignment = (std::max)(ALIGNMENT, detail::trailing_alignment(alignment_offset, alignment));
        const auto trailing_alignment = detail::trailing_alignment(sizeof(T), leading_alignment);
        return {{}, trailing_alignment, trailing_alignment};
    }

    template <std::size_t PreviousTrailingAlignment, std::size_t NextAlignment>
    static constexpr AlignedSizeInMemory aligned_size_in_memory(std::size_t offset, std::size_t alignment,
                                                                std::size_t fixed_size) noexcept
    {
        const auto value_size = VALUE_BYTES * fixed_size;
        std::size_t new_offset{};
        std::size_t size{};
        if (alignment < ALIGNMENT)
        {
            const auto alignment_offset = detail::align(offset, alignment) + ALIGNMENT - alignment;
            size = alignment_offset - offset + value_size;
            new_offset = value_size;
        }
        else
        {
            const auto alignment_offset = detail::align_if<(PreviousTrailingAlignment < ALIGNMENT), ALIGNMENT>(offset);
            size = alignment_offset - offset + value_size;
            new_offset = offset + size;
        }
        const auto padding_offset = detail::align_if<(TRAILING_ALIGNMENT < NextAlignment), NextAlignment>(new_offset);
        return {new_offset, size, padding_offset - new_offset, (std::max)(alignment, ALIGNMENT)};
    }

    static constexpr ForwardSizeInMemory forward_size_in_memory(std::size_t offset, std::size_t fixed_size) noexcept
    {
        const auto padding = detail::align(offset, ALIGNMENT) - offset;
        const auto next_offset = offset + padding + fixed_size * VALUE_BYTES;
        return {next_offset};
    }

    static constexpr BackwardSizeInMemory backward_size_in_memory(std::size_t offset, std::size_t fixed_size) noexcept
    {
        const auto next_offset = offset + fixed_size * VALUE_BYTES;
        const auto padding = detail::align(next_offset, ALIGNMENT) - next_offset;
        return {next_offset, padding};
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
