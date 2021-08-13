// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_DETAIL_ALLOCATOR_HPP
#define CNTGS_DETAIL_ALLOCATOR_HPP

#include "cntgs/detail/memory.hpp"
#include "cntgs/detail/utility.hpp"

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
    struct Impl : detail::EmptyBaseOptimization<Allocator>
    {
        using Base = detail::EmptyBaseOptimization<Allocator>;

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

    static constexpr auto allocate_if_not_zero(std::size_t size, Allocator allocator)
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
        : impl(AllocatorAwarePointer::allocate_if_not_zero(size, allocator), size, allocator)
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

    constexpr AllocatorAwarePointer(AllocatorAwarePointer&& other) noexcept
        : impl(other.release(), other.size(), other.get_allocator())
    {
    }

#if __cpp_constexpr_dynamic_alloc
    constexpr
#endif
        ~AllocatorAwarePointer() noexcept
    {
        this->deallocate();
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
            if (this->size() < other.size() || !this->get())
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
            this->propagate_on_container_move_assignment(other);
            this->deallocate();
            this->get() = other.release();
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

    constexpr explicit operator bool() const noexcept { return this->get() != nullptr; }

    constexpr auto release() noexcept { return std::exchange(this->impl.ptr, nullptr); }

    constexpr void reset(AllocatorAwarePointer&& other) noexcept
    {
        this->deallocate();
        this->get() = other.release();
        this->size() = other.size();
    }

    constexpr void propagate_on_container_copy_assignment(const AllocatorAwarePointer& other) noexcept
    {
        if constexpr (AllocatorTraits::propagate_on_container_copy_assignment::value)
        {
            this->get_allocator() = other.get_allocator();
        }
    }

    constexpr void propagate_on_container_move_assignment(AllocatorAwarePointer& other) noexcept
    {
        if constexpr (AllocatorTraits::propagate_on_container_move_assignment::value)
        {
            this->get_allocator() = std::move(other.get_allocator());
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

template <class T>
auto type_erase_allocator(T&& allocator) noexcept
{
    detail::TypeErasedAllocator result;
    detail::construct_at(reinterpret_cast<detail::RemoveCvrefT<T>*>(&result), std::forward<T>(allocator));
    return result;
}
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_ALLOCATOR_HPP
