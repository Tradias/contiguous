#ifndef CNTGS_DETAIL_MEMORY_H
#define CNTGS_DETAIL_MEMORY_H

#include "cntgs/contiguous/detail/attributes.h"
#include "cntgs/contiguous/detail/iteratorUtils.h"
#include "cntgs/contiguous/detail/range.h"
#include "cntgs/contiguous/detail/typeUtils.h"
#include "cntgs/contiguous/detail/utility.h"
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
using Byte = std::underlying_type_t<std::byte>;
using TypeErasedAllocator = std::aligned_storage_t<32>;

template <std::size_t N>
struct alignas(N) AlignedByte
{
    std::byte byte;
};

template <class Allocator>
class AllocatorAwarePointer
{
  private:
    using AllocatorTraits = std::allocator_traits<Allocator>;
    using Pointer = typename AllocatorTraits::pointer;

  public:
    using allocator_type = Allocator;
    using pointer = Pointer;
    using value_type = typename AllocatorTraits::value_type;

  private:
    struct Impl : detail::EmptyBaseOptimizationT<Allocator>
    {
        using Base = detail::EmptyBaseOptimizationT<Allocator>;

        Pointer ptr{};
        std::size_t size{};

        Impl() = default;

        constexpr Impl(Pointer ptr, std::size_t size, const Allocator& allocator) noexcept
            : Base{allocator}, ptr(ptr), size(size)
        {
        }
    };

    Impl impl;

  public:
    AllocatorAwarePointer() = default;

    constexpr AllocatorAwarePointer(std::size_t size, Allocator allocator)
        : impl(AllocatorTraits::allocate(allocator, size), size, allocator)
    {
    }

    constexpr AllocatorAwarePointer(pointer ptr, std::size_t size, const Allocator& allocator) noexcept
        : impl(ptr, size, allocator)
    {
    }

    constexpr AllocatorAwarePointer(const AllocatorAwarePointer& other)
        : AllocatorAwarePointer(other.size(),
                                AllocatorTraits::select_on_container_copy_construction(other.get_allocator()))
    {
    }

    constexpr AllocatorAwarePointer(const AllocatorAwarePointer& other, Allocator allocator)
        : impl(AllocatorTraits::allocate(allocator, other.size()), other.size(), allocator)
    {
    }

    constexpr AllocatorAwarePointer(AllocatorAwarePointer&& other) noexcept
        : impl(std::exchange(other.get(), nullptr), other.size(), other.get_allocator())
    {
    }

    constexpr AllocatorAwarePointer(AllocatorAwarePointer&& other, const Allocator& allocator) noexcept
        : impl(std::exchange(other.get(), nullptr), other.size(), allocator)
    {
    }

    ~AllocatorAwarePointer() noexcept { this->deallocate(); }

    constexpr void propagate_on_container_copy_assignment(const AllocatorAwarePointer& other) noexcept
    {
        if constexpr (AllocatorTraits::propagate_on_container_copy_assignment::value)
        {
            this->get_allocator() = other.get_allocator();
        }
    }

    constexpr AllocatorAwarePointer& operator=(const AllocatorAwarePointer& other)
    {
        if (this != std::addressof(other))
        {
            if constexpr (AllocatorTraits::propagate_on_container_copy_assignment::value &&
                          !AllocatorTraits::is_always_equal::value)
            {
                if (this->get_allocator() != other.get_allocator())
                {
                    this->deallocate();
                    this->propagate_on_container_copy_assignment(other);
                    this->size() = other.size();
                    this->get() = this->allocate();
                    return *this;
                }
            }
            this->propagate_on_container_copy_assignment(other);
            if (this->size() < other.size())
            {
                this->deallocate();
                this->size() = other.size();
                this->get() = this->allocate();
            }
        }
        return *this;
    }

    constexpr AllocatorAwarePointer& operator=(AllocatorAwarePointer&& other) noexcept
    {
        if (this != std::addressof(other))
        {
            if constexpr (AllocatorTraits::propagate_on_container_move_assignment::value)
            {
                this->get_allocator() = std::move(other.get_allocator());
            }
            this->get() = std::exchange(other.get(), nullptr);
            this->size() = other.size();
        }
        return *this;
    }

    constexpr decltype(auto) get_allocator() noexcept { return this->impl.get(); }

    constexpr auto get_allocator() const noexcept { return this->impl.get(); }

    constexpr auto& get() noexcept { return this->impl.ptr; }

    constexpr auto get() const noexcept { return this->impl.ptr; }

    constexpr auto& size() noexcept { return this->impl.size; }

    constexpr auto size() const noexcept { return this->impl.size; }

    explicit constexpr operator bool() const noexcept { return bool(this->get()); }

    constexpr auto release() noexcept { return std::exchange(this->impl.ptr, nullptr); }

    constexpr auto allocate() { return AllocatorTraits::allocate(this->get_allocator(), this->size()); }

    constexpr void deallocate() noexcept
    {
        if (this->get())
        {
            AllocatorTraits::deallocate(this->get_allocator(), this->get(), this->size());
        }
    }
};

template <class Allocator>
void swap(detail::AllocatorAwarePointer<Allocator>& lhs, detail::AllocatorAwarePointer<Allocator>& rhs) noexcept
{
    using std::swap;
    if constexpr (std::allocator_traits<Allocator>::propagate_on_container_swap::value)
    {
        swap(lhs.get_allocator(), rhs.get_allocator());
    }
    swap(lhs.get(), rhs.get());
    swap(lhs.size(), rhs.size());
}

template <class Allocator>
class MaybeOwnedAllocatorAwarePointer
{
  private:
    using StorageType = detail::AllocatorAwarePointer<Allocator>;

  public:
    using allocator_type = typename StorageType::allocator_type;
    using pointer = typename StorageType::pointer;
    using value_type = typename StorageType::value_type;

    StorageType ptr{};
    bool owned{};

    MaybeOwnedAllocatorAwarePointer() = default;

    constexpr MaybeOwnedAllocatorAwarePointer(pointer ptr, std::size_t size, bool is_owned,
                                              const allocator_type& allocator) noexcept
        : ptr(ptr, size, allocator), owned(is_owned)
    {
    }

    constexpr MaybeOwnedAllocatorAwarePointer(std::size_t size, const allocator_type& allocator) noexcept
        : ptr(size, allocator), owned(true)
    {
    }

    MaybeOwnedAllocatorAwarePointer(const MaybeOwnedAllocatorAwarePointer& other) = default;

    MaybeOwnedAllocatorAwarePointer(MaybeOwnedAllocatorAwarePointer&& other) = default;

    MaybeOwnedAllocatorAwarePointer& operator=(const MaybeOwnedAllocatorAwarePointer& other) = default;

    constexpr MaybeOwnedAllocatorAwarePointer& operator=(MaybeOwnedAllocatorAwarePointer&& other) noexcept
    {
        if (this != std::addressof(other))
        {
            this->release_ptr_if_not_owned();
            this->ptr = std::move(other.ptr);
            this->owned = other.owned;
        }
        return *this;
    }

    ~MaybeOwnedAllocatorAwarePointer() noexcept { this->release_ptr_if_not_owned(); }

    constexpr auto get() const noexcept { return this->ptr.get(); }

    constexpr auto size() const noexcept { return this->ptr.size(); }

    constexpr bool is_owned() const noexcept { return this->owned; }

    explicit constexpr operator bool() const noexcept { return bool(this->ptr); }

    constexpr auto release() noexcept { return this->ptr.release(); }

    constexpr auto get_allocator() const noexcept { return this->ptr.get_allocator(); }

    constexpr auto& get_impl() noexcept { return this->ptr; }

  private:
    constexpr void release_ptr_if_not_owned() noexcept
    {
        if (!this->owned)
        {
            (void)this->ptr.release();
        }
    }
};

template <class T>
auto copy_using_memcpy(const T* CNTGS_RESTRICT source, std::byte* CNTGS_RESTRICT target, std::size_t size) noexcept
{
    std::memcpy(target, source, size * sizeof(T));
    return target + size * sizeof(T);
}

template <bool IgnoreAliasing, class TargetType, class Range>
auto uninitialized_range_construct(Range&& CNTGS_RESTRICT range, TargetType* CNTGS_RESTRICT address)
{
    using RangeValueType = typename std::iterator_traits<decltype(std::begin(range))>::value_type;
    if constexpr (IgnoreAliasing && detail::HasDataAndSize<std::decay_t<Range>>{} &&
                  detail::MEMCPY_COMPATIBLE<TargetType, RangeValueType>)
    {
        return detail::copy_using_memcpy(std::data(range), reinterpret_cast<std::byte*>(address), std::size(range));
    }
    else
    {
        if constexpr (!std::is_lvalue_reference_v<Range>)
        {
            return reinterpret_cast<std::byte*>(std::uninitialized_move(std::begin(range), std::end(range), address));
        }
        else
        {
            return reinterpret_cast<std::byte*>(std::uninitialized_copy(std::begin(range), std::end(range), address));
        }
    }
}

template <bool IgnoreAliasing, class TargetType, class Range>
auto uninitialized_construct(Range&& CNTGS_RESTRICT range, TargetType* CNTGS_RESTRICT address, std::size_t)
    -> std::enable_if_t<detail::IsRange<Range>::value, std::byte*>
{
    return uninitialized_range_construct<IgnoreAliasing>(std::forward<Range>(range), address);
}

template <bool IgnoreAliasing, class TargetType, class Iterator>
auto uninitialized_construct(const Iterator& CNTGS_RESTRICT iterator, TargetType* CNTGS_RESTRICT address,
                             std::size_t size) -> std::enable_if_t<!detail::IsRange<Iterator>::value, std::byte*>
{
    using IteratorValueType = typename std::iterator_traits<Iterator>::value_type;
    if constexpr (IgnoreAliasing && std::is_pointer_v<Iterator> &&
                  detail::MEMCPY_COMPATIBLE<TargetType, IteratorValueType>)
    {
        return detail::copy_using_memcpy(iterator, reinterpret_cast<std::byte*>(address), size);
    }
    else if constexpr (IgnoreAliasing && detail::CONTIGUOUS_ITERATOR_V<Iterator> &&
                       detail::MEMCPY_COMPATIBLE<TargetType, IteratorValueType>)
    {
        return detail::copy_using_memcpy(iterator.operator->(), reinterpret_cast<std::byte*>(address), size);
    }
    else
    {
        return reinterpret_cast<std::byte*>(std::uninitialized_copy_n(iterator, size, address));
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
    return static_cast<T*>(__builtin_assume_aligned(ptr, Alignment));
}
#endif

[[nodiscard]] constexpr auto align(std::size_t alignment, std::uintptr_t position) noexcept
{
    return (position - 1u + alignment) & (alignment * std::numeric_limits<std::size_t>::max());
}

template <std::size_t Alignment>
[[nodiscard]] constexpr auto align(std::uintptr_t position) noexcept
{
    if constexpr (Alignment == 1)
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
    return detail::assume_aligned<Alignment>(reinterpret_cast<void*>(aligned));
}

template <bool NeedsAlignment, std::size_t Alignment>
[[nodiscard]] void* align_if(void* ptr) noexcept
{
    if constexpr (NeedsAlignment)
    {
        ptr = detail::align<Alignment>(ptr);
    }
    return detail::assume_aligned<Alignment>(ptr);
}

template <class T>
auto type_erase_allocator(T&& allocator) noexcept
{
    detail::TypeErasedAllocator result;
    detail::construct_at(reinterpret_cast<detail::RemoveCvrefT<T>*>(&result), std::forward<T>(allocator));
    return result;
}
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_MEMORY_H
