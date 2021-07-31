#ifndef CNTGS_DETAIL_ALLOCATOR_HPP
#define CNTGS_DETAIL_ALLOCATOR_HPP

#include "cntgs/contiguous/detail/memory.hpp"
#include "cntgs/contiguous/detail/utility.hpp"

#include <cstddef>
#include <memory>
#include <type_traits>

namespace cntgs::detail
{
using TypeErasedAllocator = std::aligned_storage_t<32>;

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

    constexpr auto allocate() { return AllocatorTraits::allocate(this->get_allocator(), this->size()); }

    constexpr void deallocate() noexcept
    {
        if (this->get())
        {
            AllocatorTraits::deallocate(this->get_allocator(), this->get(), this->size());
        }
    }

    constexpr auto allocate_if_not_zero(std::size_t size, Allocator allocator)
    {
#ifdef __cpp_lib_is_constant_evaluated
        if (std::is_constant_evaluated() && size == 0)
        {
            return Pointer{};
        }
#endif
        return AllocatorTraits::allocate(allocator, size);
    }

  public:
    AllocatorAwarePointer() = default;

    constexpr AllocatorAwarePointer(std::size_t size, const Allocator& allocator)
        : impl(this->allocate_if_not_zero(size, allocator), size, allocator)
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

    constexpr AllocatorAwarePointer(const AllocatorAwarePointer& other, const Allocator& allocator)
        : impl(this->allocate_if_not_zero(other.size(), allocator), other.size(), allocator)
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

#if __cpp_constexpr_dynamic_alloc
    constexpr
#endif
        ~AllocatorAwarePointer() noexcept
    {
        this->deallocate();
    }

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
};

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

#if __cpp_constexpr_dynamic_alloc
    constexpr
#endif
        ~MaybeOwnedAllocatorAwarePointer() noexcept
    {
        this->release_ptr_if_not_owned();
    }

    constexpr decltype(auto) get() const noexcept { return this->ptr.get(); }

    constexpr decltype(auto) size() const noexcept { return this->ptr.size(); }

    constexpr bool is_owned() const noexcept { return this->owned; }

    explicit constexpr operator bool() const noexcept { return bool(this->ptr); }

    constexpr decltype(auto) release() noexcept { return this->ptr.release(); }

    constexpr decltype(auto) get_allocator() const noexcept { return this->ptr.get_allocator(); }

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

template <class Allocator>
constexpr void swap(detail::AllocatorAwarePointer<Allocator>& lhs,
                    detail::AllocatorAwarePointer<Allocator>& rhs) noexcept
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
constexpr void swap(detail::MaybeOwnedAllocatorAwarePointer<Allocator>& lhs,
                    detail::MaybeOwnedAllocatorAwarePointer<Allocator>& rhs) noexcept
{
    detail::swap(lhs.ptr, rhs.ptr);
    std::swap(lhs.owned, rhs.owned);
}

template <class T>
auto type_erase_allocator(T&& allocator) noexcept
{
    detail::TypeErasedAllocator result;
    detail::construct_at(reinterpret_cast<detail::RemoveCvrefT<T>*>(&result), std::forward<T>(allocator));
    return result;
}
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_ALLOCATOR_HPP
