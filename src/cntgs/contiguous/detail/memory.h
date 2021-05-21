#pragma once

#include "cntgs/contiguous/detail/iterator.h"
#include "cntgs/contiguous/detail/range.h"
#include "cntgs/contiguous/span.h"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <limits>
#include <memory>

namespace cntgs::detail
{
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

template <class Iterator>
auto copy_iterator_ignore_aliasing(const Iterator& iterator, std::byte* address, std::size_t size)
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

template <class T>
struct MaybeOwnedPtr
{
    std::unique_ptr<T> ptr;
    bool is_owned{};

    MaybeOwnedPtr() = default;

    MaybeOwnedPtr(std::unique_ptr<T>&& ptr) noexcept : ptr(std::move(ptr)), is_owned(true) {}

    MaybeOwnedPtr(cntgs::Span<std::remove_extent_t<T>> span) noexcept : ptr(span.data()) {}

    MaybeOwnedPtr(MaybeOwnedPtr&& other) = default;

    MaybeOwnedPtr& operator=(MaybeOwnedPtr&& other) noexcept
    {
        if (this != &other)
        {
            release_if_not_owned();
            ptr = std::move(other.ptr);
            is_owned = std::move(other.is_owned);
        }
        return *this;
    }

    ~MaybeOwnedPtr() noexcept { release_if_not_owned(); }

    decltype(auto) get() const noexcept { return this->ptr.get(); }

    explicit operator bool() const noexcept { return bool(this->ptr); }

    void release_if_not_owned() noexcept
    {
        if (!this->is_owned)
        {
            this->ptr.release();
        }
    }
};

template <class T>
auto acquire_or_create_new(detail::MaybeOwnedPtr<T>&& ptr, std::size_t memory_size)
{
    if (ptr)
    {
        return std::move(ptr);
    }
    return detail::MaybeOwnedPtr<T>{detail::make_unique_for_overwrite<std::byte[]>(memory_size)};
}

inline void* align(std::size_t alignment, void* ptr) noexcept
{
    const auto intptr = reinterpret_cast<std::uintptr_t>(ptr);
    const auto aligned = (intptr - 1u + alignment) & (alignment * std::numeric_limits<std::size_t>::max());
    return ptr = reinterpret_cast<void*>(aligned);
}
}  // namespace cntgs::detail