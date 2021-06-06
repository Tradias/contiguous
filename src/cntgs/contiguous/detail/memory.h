#pragma once

#include "cntgs/contiguous/detail/attributes.h"
#include "cntgs/contiguous/detail/iterator.h"
#include "cntgs/contiguous/detail/range.h"
#include "cntgs/contiguous/span.h"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <limits>
#include <memory>
#include <version>

namespace cntgs::detail
{
template <class T, class U>
static constexpr auto MEMCPY_COMPATIBLE =
    sizeof(T) == sizeof(U) && std::is_trivially_copyable_v<T>&& std::is_trivially_copyable_v<
                                  U>&& std::is_floating_point_v<T> == std::is_floating_point_v<U>;

template <class T>
auto copy_using_memcpy(const T* CNTGS_RESTRICT source, std::byte* CNTGS_RESTRICT target, std::size_t size) noexcept
{
    std::memcpy(target, source, size * sizeof(T));
    return target + size * sizeof(T);
}

template <class TargetType, class Range>
auto copy_range_ignore_aliasing(Range&& range, std::byte* CNTGS_RESTRICT address)
{
    using RangeValueType = typename std::iterator_traits<decltype(std::begin(range))>::value_type;
    if constexpr (detail::HasDataAndSize<std::decay_t<Range>>{} &&
                  detail::MEMCPY_COMPATIBLE<TargetType, RangeValueType>)
    {
        const auto size = std::size(range);
        std::memcpy(address, std::data(range), size * sizeof(TargetType));
        return address + size * sizeof(TargetType);
    }
    else
    {
        const auto prev_address = reinterpret_cast<std::add_pointer_t<TargetType>>(address);
        if constexpr (!std::is_lvalue_reference_v<Range>)
        {
            return reinterpret_cast<std::byte*>(
                std::uninitialized_move(std::begin(range), std::end(range), prev_address));
        }
        else
        {
            return reinterpret_cast<std::byte*>(
                std::uninitialized_copy(std::begin(range), std::end(range), prev_address));
        }
    }
}

template <class TargetType, class Range>
auto copy_ignore_aliasing(Range&& range, std::byte* CNTGS_RESTRICT address, std::size_t)
    -> std::enable_if_t<detail::IsRange<Range>::value, std::byte*>
{
    return copy_range_ignore_aliasing<TargetType>(std::forward<Range>(range), address);
}

template <class TargetType, class Iterator>
auto copy_ignore_aliasing(const Iterator& iterator, std::byte* CNTGS_RESTRICT address, std::size_t size)
    -> std::enable_if_t<!detail::IsRange<Iterator>::value, std::byte*>
{
    using IteratorValueType = typename std::iterator_traits<Iterator>::value_type;
    if constexpr (std::is_pointer_v<Iterator> && detail::MEMCPY_COMPATIBLE<TargetType, IteratorValueType>)
    {
        return detail::copy_using_memcpy(iterator, address, size);
    }
    else if constexpr (detail::ContiguousIterator<Iterator> && detail::MEMCPY_COMPATIBLE<TargetType, IteratorValueType>)
    {
        return detail::copy_using_memcpy(iterator.operator->(), address, size);
    }
    else
    {
        const auto prev_address = reinterpret_cast<std::add_pointer_t<TargetType>>(address);
        return reinterpret_cast<std::byte*>(std::uninitialized_copy_n(iterator, size, prev_address));
    }
}

template <class T>
[[nodiscard]] auto make_unique_for_overwrite(std::size_t size)
{
#ifdef __cpp_lib_smart_ptr_for_overwrite
    return std::make_unique_for_overwrite<T>(size);
#else
    return std::unique_ptr<T>(new std::remove_extent_t<T>[size]);
#endif
}

template <class T, class... Args>
constexpr T* construct_at(T* ptr, Args&&... args)
{
#ifdef __cpp_lib_ranges
    return std::construct_at(ptr, std::forward<Args>(args)...);
#else
    return ::new (const_cast<void*>(static_cast<const volatile void*>(ptr))) T(std::forward<Args>(args)...);
#endif
}

template <std::size_t N>
struct alignas(N) AlignedByte
{
    std::byte byte;
};

template <std::size_t N>
using AlignedByteT = std::conditional_t<(N == 0), std::byte, AlignedByte<N>>;

template <class T>
struct MaybeOwnedPtr
{
    using pointer = typename std::unique_ptr<T>::pointer;
    using element_type = typename std::unique_ptr<T>::element_type;
    using deleter_type = typename std::unique_ptr<T>::deleter_type;

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
[[nodiscard]] auto make_maybe_owned_ptr(std::size_t memory_size)
{
    return detail::MaybeOwnedPtr<T>{detail::make_unique_for_overwrite<T>(memory_size)};
}

template <class T>
[[nodiscard]] auto acquire_or_create_new(detail::MaybeOwnedPtr<T>&& ptr, std::size_t memory_size)
{
    if (ptr)
    {
        return std::move(ptr);
    }
    return detail::make_maybe_owned_ptr<T>(memory_size);
}

template <std::size_t Alignment, class T>
[[nodiscard]] constexpr T* assume_aligned(T* const ptr) noexcept
{
#ifdef __cpp_lib_assume_aligned
    return std::assume_aligned<Alignment>(ptr);
#endif
    return static_cast<T*>(__builtin_assume_aligned(ptr, Alignment));
}

[[nodiscard]] constexpr auto align(std::size_t alignment, std::uintptr_t position) noexcept
{
    return (position - 1u + alignment) & (alignment * std::numeric_limits<std::size_t>::max());
}

template <std::size_t Alignment>
[[nodiscard]] constexpr auto align(std::uintptr_t position) noexcept
{
    if constexpr (Alignment == 0)
    {
        return position;
    }
    else
    {
        return detail::align(Alignment, position);
    }
}

template <std::size_t Alignment>
[[nodiscard]] void* align(void* ptr) noexcept
{
    const auto uintptr = reinterpret_cast<std::uintptr_t>(ptr);
    const auto aligned = detail::align<Alignment>(uintptr);
    return reinterpret_cast<void*>(aligned);
}

template <bool NeedsAlignment, std::size_t Alignment>
[[nodiscard]] void* align_if(void* ptr) noexcept
{
    if constexpr (NeedsAlignment)
    {
        return detail::align<Alignment>(ptr);
    }
    else
    {
        if constexpr (Alignment == 0)
        {
            return ptr;
        }
        else
        {
            return detail::assume_aligned<Alignment>(ptr);
        }
    }
}
}  // namespace cntgs::detail