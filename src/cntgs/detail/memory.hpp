// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_DETAIL_MEMORY_HPP
#define CNTGS_DETAIL_MEMORY_HPP

#include "cntgs/detail/attributes.hpp"
#include "cntgs/detail/iterator.hpp"
#include "cntgs/detail/range.hpp"
#include "cntgs/detail/typeTraits.hpp"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <limits>
#include <memory>

namespace cntgs::detail
{
using Byte = std::underlying_type_t<std::byte>;

template <std::size_t N>
struct Aligned
{
    alignas(N) std::byte v[N];
};

template <class T>
auto memcpy(const T* CNTGS_RESTRICT source, std::byte* CNTGS_RESTRICT target, std::size_t size) noexcept
{
    std::memcpy(target, source, size * sizeof(T));
    return target + size * sizeof(T);
}

template <class Range, class TargetIterator>
constexpr auto uninitialized_move(Range&& source, TargetIterator&& target)
{
#ifdef __cpp_lib_ranges
    return std::ranges::uninitialized_move(
               std::forward<Range>(source),
               std::ranges::subrange{std::forward<TargetIterator>(target), std::unreachable_sentinel})
        .out;
#else
    return std::uninitialized_move(std::begin(source), std::end(source), std::forward<TargetIterator>(target));
#endif
}

template <class Range, class TargetIterator>
constexpr auto uninitialized_copy(Range&& source, TargetIterator&& target)
{
#ifdef __cpp_lib_ranges
    return std::ranges::uninitialized_copy(
               std::forward<Range>(source),
               std::ranges::subrange{std::forward<TargetIterator>(target), std::unreachable_sentinel})
        .out;
#else
    return std::uninitialized_copy(std::begin(source), std::end(source), std::forward<TargetIterator>(target));
#endif
}

template <class SourceIterator, class DifferenceType, class TargetIterator>
constexpr auto uninitialized_copy_n(SourceIterator&& source, DifferenceType count, TargetIterator&& target)
{
#ifdef __cpp_lib_ranges
    return std::ranges::uninitialized_copy_n(std::forward<SourceIterator>(source),
                                             static_cast<std::iter_difference_t<SourceIterator>>(count),
                                             std::forward<TargetIterator>(target), std::unreachable_sentinel)
        .out;
#else
    return std::uninitialized_copy_n(std::forward<SourceIterator>(source), count, std::forward<TargetIterator>(target));
#endif
}

template <bool IgnoreAliasing, class TargetType, class Range>
auto uninitialized_range_construct(Range&& range, TargetType* address)
{
    using RangeValueType = typename std::iterator_traits<decltype(std::begin(range))>::value_type;
    if constexpr (IgnoreAliasing && detail::HAS_DATA_AND_SIZE<std::decay_t<Range>> &&
                  detail::MEMCPY_COMPATIBLE<TargetType, RangeValueType>)
    {
        return detail::memcpy(std::data(range), reinterpret_cast<std::byte*>(address), std::size(range));
    }
    else
    {
        if constexpr (!std::is_lvalue_reference_v<Range>)
        {
            return reinterpret_cast<std::byte*>(detail::uninitialized_move(std::forward<Range>(range), address));
        }
        else
        {
            return reinterpret_cast<std::byte*>(detail::uninitialized_copy(std::forward<Range>(range), address));
        }
    }
}

template <bool IgnoreAliasing, class TargetType, class Range>
auto uninitialized_construct(Range&& range, TargetType* address,
                             std::size_t) -> std::enable_if_t<detail::IS_RANGE<Range>, std::byte*>
{
    return detail::uninitialized_range_construct<IgnoreAliasing>(std::forward<Range>(range), address);
}

template <bool IgnoreAliasing, class TargetType, class Iterator>
auto uninitialized_construct(const Iterator& iterator, TargetType* address,
                             std::size_t size) -> std::enable_if_t<!detail::IS_RANGE<Iterator>, std::byte*>
{
    using IteratorValueType = typename std::iterator_traits<Iterator>::value_type;
    if constexpr (IgnoreAliasing && std::is_pointer_v<Iterator> &&
                  detail::MEMCPY_COMPATIBLE<TargetType, IteratorValueType>)
    {
        return detail::memcpy(iterator, reinterpret_cast<std::byte*>(address), size);
    }
    else if constexpr (IgnoreAliasing && detail::CONTIGUOUS_ITERATOR_V<Iterator> &&
                       detail::MEMCPY_COMPATIBLE<TargetType, IteratorValueType>)
    {
        return detail::memcpy(iterator.operator->(), reinterpret_cast<std::byte*>(address), size);
    }
    else
    {
        return reinterpret_cast<std::byte*>(detail::uninitialized_copy_n(iterator, size, address));
    }
}

#ifdef __cpp_lib_ranges
using std::construct_at;
#else
template <class T, class... Args>
constexpr T* construct_at(T* ptr, Args&&... args)
{
    return ::new (const_cast<void*>(static_cast<const volatile void*>(ptr))) T(std::forward<Args>(args)...);
}
#endif

#ifdef __cpp_lib_assume_aligned
using std::assume_aligned;
#else
template <std::size_t Alignment, class T>
[[nodiscard]] constexpr T* assume_aligned(T* const ptr) noexcept
{
    return static_cast<T*>(::__builtin_assume_aligned(ptr, Alignment));
}
#endif

[[nodiscard]] inline bool is_aligned(void* ptr, size_t alignment) noexcept
{
    void* void_ptr = ptr;
    auto size = std::numeric_limits<size_t>::max();
    std::align(alignment, 0, void_ptr, size);
    return void_ptr == ptr;
}

[[nodiscard]] constexpr auto align(std::uintptr_t position, std::size_t alignment) noexcept
{
    return (position - 1u + alignment) & (alignment * std::numeric_limits<std::size_t>::max());
}

template <std::size_t Alignment>
[[nodiscard]] constexpr auto align(std::uintptr_t position) noexcept
{
    if constexpr (Alignment > 1)
    {
        return detail::align(position, Alignment);
    }
    else
    {
        return position;
    }
}

template <std::size_t Alignment, class T>
[[nodiscard]] constexpr T* align(T* ptr) noexcept
{
    if constexpr (Alignment > 1)
    {
        const auto uintptr = reinterpret_cast<std::uintptr_t>(ptr);
        const auto aligned = detail::align<Alignment>(uintptr);
        return detail::assume_aligned<Alignment>(reinterpret_cast<T*>(aligned));
    }
    else
    {
        return ptr;
    }
}

template <bool NeedsAlignment, std::size_t Alignment, class T>
[[nodiscard]] constexpr auto align_if(T* ptr) noexcept
{
    if constexpr (NeedsAlignment && Alignment > 1)
    {
        ptr = detail::align<Alignment>(ptr);
    }
    return detail::assume_aligned<Alignment>(ptr);
}

template <bool NeedsAlignment, std::size_t Alignment>
[[nodiscard]] constexpr auto align_if(std::uintptr_t position) noexcept
{
    if constexpr (NeedsAlignment && Alignment > 1)
    {
        position = detail::align<Alignment>(position);
    }
    return position;
}

template <class T>
[[nodiscard]] constexpr T extract_lowest_set_bit(T value) noexcept
{
    return value & (~value + T{1});
}

[[nodiscard]] constexpr std::size_t trailing_alignment(std::size_t byte_size, std::size_t alignment) noexcept
{
    return (std::min)(detail::extract_lowest_set_bit(byte_size), alignment);
}

inline constexpr auto SIZE_T_TRAILING_ALIGNMENT = detail::trailing_alignment(sizeof(std::size_t), alignof(std::size_t));
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_MEMORY_HPP
