#pragma once

#include "cntgs/contiguous/detail/iterator.h"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <memory>

namespace cntgs::detail
{
template <class, class = std::void_t<>>
struct HasDataAndSize : std::false_type
{
};

template <class T>
struct HasDataAndSize<T, std::void_t<decltype(std::data(std::declval<T&>())), decltype(std::size(std::declval<T&>()))>>
    : std::true_type
{
};

template <class, class = std::void_t<>>
struct IsRange : std::false_type
{
};

template <class T>
struct IsRange<T, std::void_t<decltype(std::begin(std::declval<T&>())), decltype(std::end(std::declval<T&>()))>>
    : std::true_type
{
};

template <class T>
auto copy_using_memcpy(const T* source, std::byte* target, std::size_t size)
{
    std::memcpy(target, source, size * sizeof(T));
    return target + size * sizeof(T);
}

template <class Range>
auto copy_range_ignore_aliasing(const Range& range, std::byte* address)
{
    using RangeValueType = typename std::iterator_traits<decltype(std::begin(range))>::value_type;
    if constexpr (std::is_trivially_copyable_v<RangeValueType> && detail::HasDataAndSize<Range>{})
    {
        const auto size = std::size(range);
        std::memcpy(address, std::data(range), size * sizeof(RangeValueType));
        return address + size * sizeof(RangeValueType);
    }
    else
    {
        const auto prev_address = reinterpret_cast<RangeValueType*>(address);
        return reinterpret_cast<std::byte*>(std::uninitialized_copy(std::begin(range), std::end(range), prev_address));
    }
}

template <class Range>
auto copy_ignore_aliasing(const Range& range, std::byte* address, std::size_t)
    -> std::enable_if_t<IsRange<Range>::value, std::byte*>
{
    return copy_range_ignore_aliasing(range, address);
}

template <class Iterator>
auto copy_ignore_aliasing(const Iterator& iterator, std::byte* address, std::size_t size)
    -> std::enable_if_t<!IsRange<Iterator>::value, std::byte*>
{
    using IteratorValueType = typename std::iterator_traits<Iterator>::value_type;
    if constexpr (std::is_pointer_v<Iterator> && std::is_trivially_copyable_v<IteratorValueType>)
    {
        return detail::copy_using_memcpy(iterator, address, size);
    }
    else if constexpr (detail::ContiguousIterator<Iterator> && std::is_trivially_copyable_v<IteratorValueType>)
    {
        return detail::copy_using_memcpy(iterator.operator->(), address, size);
    }
    else
    {
        const auto prev_address = reinterpret_cast<IteratorValueType*>(address);
        return reinterpret_cast<std::byte*>(std::uninitialized_copy_n(iterator, size, prev_address));
    }
}

template <class T>
auto make_unique_for_overwrite(std::size_t size)
{
#ifdef __cpp_lib_smart_ptr_for_overwrite
    return std::make_unique_for_overwrite<T>(size);
#else
    return std::unique_ptr<T>(new std::remove_extent_t<T>[size]);
#endif
}
}  // namespace cntgs::detail