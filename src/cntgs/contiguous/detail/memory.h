#pragma once

#include "cntgs/contiguous/detail/attributes.h"
#include "cntgs/contiguous/detail/iterator.h"
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
using TypeErasedDeleter = std::aligned_storage_t<32>;

template <std::size_t N>
struct alignas(N) AlignedByte
{
    std::byte byte;
};

template <class T, class Deleter = std::default_delete<T>>
class MaybeOwnedPtr
{
  private:
    using StorageType = std::unique_ptr<T, Deleter>;

  public:
    using pointer = typename StorageType::pointer;
    using element_type = typename StorageType::element_type;
    using deleter_type = typename StorageType::deleter_type;

    StorageType ptr;
    bool owned{};

    MaybeOwnedPtr() = default;

    explicit MaybeOwnedPtr(StorageType&& ptr) noexcept : ptr(std::move(ptr)), owned(true) {}

    MaybeOwnedPtr(pointer data, Deleter deleter, bool owned = false) noexcept
        : ptr(data, std::move(deleter)), owned(owned)
    {
    }

    MaybeOwnedPtr(const MaybeOwnedPtr& other) = delete;

    MaybeOwnedPtr(MaybeOwnedPtr&& other) = default;

    MaybeOwnedPtr& operator=(const MaybeOwnedPtr& other) = delete;

    MaybeOwnedPtr& operator=(MaybeOwnedPtr&& other) noexcept
    {
        if (this != &other)
        {
            this->release_ptr_if_not_owned();
            ptr = std::move(other.ptr);
            owned = other.owned;
        }
        return *this;
    }

    ~MaybeOwnedPtr() noexcept { this->release_ptr_if_not_owned(); }

    [[nodiscard]] decltype(auto) get() const noexcept { return this->ptr.get(); }

    [[nodiscard]] constexpr bool is_owned() const noexcept { return this->owned; }

    explicit operator bool() const noexcept { return bool(this->ptr); }

    [[nodiscard]] pointer release() noexcept { return this->ptr.release(); }

    [[nodiscard]] decltype(auto) get_deleter() noexcept { return this->ptr.get_deleter(); }

    [[nodiscard]] decltype(auto) get_deleter() const noexcept { return this->ptr.get_deleter(); }

  private:
    void release_ptr_if_not_owned() noexcept
    {
        if (!owned)
        {
            (void)this->ptr.release();
        }
    }
};

template <class T, class Allocator>
constexpr void destroy_deallocate(T* ptr, Allocator allocator, std::size_t size = 1) noexcept
{
    using Traits = typename std::allocator_traits<Allocator>::template rebind_traits<std::remove_extent_t<T>>;
    typename Traits::allocator_type rebound_allocator{std::move(allocator)};
    Traits::destroy(rebound_allocator, ptr);
    Traits::deallocate(rebound_allocator, ptr, size);
}

template <class Allocator, bool IsSized>
class AllocatorDeleter : private detail::EmptyBaseOptimizationT<Allocator>
{
  private:
    using Base = detail::EmptyBaseOptimizationT<Allocator>;

  public:
    using allocator_type = Allocator;

    std::size_t size;

    constexpr AllocatorDeleter() noexcept(std::is_nothrow_default_constructible_v<Allocator>)
        : Base{Allocator{}}, size()
    {
    }

    constexpr AllocatorDeleter(std::size_t size, Allocator allocator) noexcept : Base{std::move(allocator)}, size(size)
    {
    }

    template <class T>
    constexpr void operator()(T* ptr) noexcept
    {
        detail::destroy_deallocate(ptr, std::move(Base::get()), this->size);
    }

    constexpr allocator_type get_allocator() const noexcept { return Base::get(); }
};

template <class Allocator>
class AllocatorDeleter<Allocator, false> : private detail::EmptyBaseOptimizationT<Allocator>
{
  private:
    using Base = detail::EmptyBaseOptimizationT<Allocator>;

  public:
    using allocator_type = Allocator;

    constexpr AllocatorDeleter() noexcept(std::is_nothrow_default_constructible_v<Allocator>) : Base{Allocator{}} {}

    constexpr AllocatorDeleter(std::size_t size, Allocator allocator) noexcept : Base{std::move(allocator)} {}

    template <class T>
    constexpr void operator()(T* ptr) noexcept
    {
        detail::destroy_deallocate(ptr, std::move(Base::get()));
    }

    constexpr allocator_type get_allocator() const noexcept { return Base::get(); }
};

template <bool IsSized, class Allocator>
constexpr auto make_allocator_deleter(std::size_t size, Allocator allocator) noexcept
{
    if constexpr (IsSized)
    {
        return detail::AllocatorDeleter<Allocator, IsSized>{size, allocator};
    }
    else
    {
        return detail::AllocatorDeleter<Allocator, IsSized>{allocator};
    }
}

template <bool IsSized, class T, class Allocator>
auto make_allocator_unique_ptr(T* ptr, std::size_t size, Allocator allocator) noexcept
{
    return std::unique_ptr<T, detail::AllocatorDeleter<Allocator, IsSized>>{
        ptr, detail::make_allocator_deleter<IsSized>(size, std::move(allocator))};
}

template <class T, class Allocator, class... Args>
auto allocate_unique(Allocator allocator, Args&&... args)
{
    using Traits = typename std::allocator_traits<Allocator>::template rebind_traits<std::remove_extent_t<T>>;
    typename Traits::allocator_type alloc{allocator};
    const auto no_throw = [&]
    {
        auto* ptr = Traits::allocate(alloc, 1);
        Traits::construct(alloc, ptr, std::forward<Args>(args)...);
        return detail::make_allocator_unique_ptr<false>(ptr, 1, std::move(allocator));
    };
#ifdef __cpp_exceptions
    if constexpr (std::is_nothrow_constructible_v<T, Args&&...>)
    {
        return no_throw();
    }
    else
    {
        auto* ptr = Traits::allocate(alloc, 1);
        try
        {
            Traits::construct(alloc, ptr, std::forward<Args>(args)...);
        }
        catch (...)
        {
            Traits::deallocate(alloc, ptr, 1);
            throw;
        }
        return detail::make_allocator_unique_ptr<false>(ptr, 1, std::move(allocator));
    }
#else
    return no_throw();
#endif
}

template <class T, class Allocator>
auto allocate_unique_for_overwrite(std::size_t size, Allocator allocator)
{
    using Traits = typename std::allocator_traits<Allocator>::template rebind_traits<std::remove_extent_t<T>>;
    typename Traits::allocator_type alloc{allocator};
    auto* ptr = Traits::allocate(alloc, size);
    return detail::make_allocator_unique_ptr<true>(ptr, size, std::move(allocator));
}

template <class T, class Allocator>
[[nodiscard]] auto acquire_or_create_new(detail::MaybeOwnedPtr<T, detail::AllocatorDeleter<Allocator, true>>&& ptr,
                                         std::size_t size, Allocator allocator)
{
    if (ptr)
    {
        return std::move(ptr);
    }
    return detail::MaybeOwnedPtr<T, detail::AllocatorDeleter<Allocator, true>>{
        detail::allocate_unique_for_overwrite<T>(size, std::move(allocator))};
}

template <class T>
auto copy_using_memcpy(const T* CNTGS_RESTRICT source, std::byte* CNTGS_RESTRICT target, std::size_t size) noexcept
{
    std::memcpy(target, source, size * sizeof(T));
    return target + size * sizeof(T);
}

template <class TargetType, bool IgnoreAliasing, class Range>
auto uninitialized_range_construct(Range&& range, std::byte* CNTGS_RESTRICT address)
{
    using RangeValueType = typename std::iterator_traits<decltype(std::begin(range))>::value_type;
    if constexpr (IgnoreAliasing && detail::HasDataAndSize<std::decay_t<Range>>{} &&
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

template <class TargetType, bool IgnoreAliasing, class Range>
auto uninitialized_construct(Range&& range, std::byte* CNTGS_RESTRICT address, std::size_t)
    -> std::enable_if_t<detail::IsRange<Range>::value, std::byte*>
{
    return uninitialized_range_construct<TargetType, IgnoreAliasing>(std::forward<Range>(range), address);
}

template <class TargetType, bool IgnoreAliasing, class Iterator>
auto uninitialized_construct(const Iterator& iterator, std::byte* CNTGS_RESTRICT address, std::size_t size)
    -> std::enable_if_t<!detail::IsRange<Iterator>::value, std::byte*>
{
    using IteratorValueType = typename std::iterator_traits<Iterator>::value_type;
    if constexpr (IgnoreAliasing && std::is_pointer_v<Iterator> &&
                  detail::MEMCPY_COMPATIBLE<TargetType, IteratorValueType>)
    {
        return detail::copy_using_memcpy(iterator, address, size);
    }
    else if constexpr (IgnoreAliasing && detail::CONTIGUOUS_ITERATOR_V<Iterator> &&
                       detail::MEMCPY_COMPATIBLE<TargetType, IteratorValueType>)
    {
        return detail::copy_using_memcpy(iterator.operator->(), address, size);
    }
    else
    {
        const auto prev_address = reinterpret_cast<std::add_pointer_t<TargetType>>(address);
        return reinterpret_cast<std::byte*>(std::uninitialized_copy_n(iterator, size, prev_address));
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
auto type_erase_deleter(T&& deleter) noexcept
{
    detail::TypeErasedDeleter result;
    detail::construct_at(reinterpret_cast<detail::RemoveCvrefT<T>*>(&result), std::forward<T>(deleter));
    return result;
}
}  // namespace cntgs::detail
