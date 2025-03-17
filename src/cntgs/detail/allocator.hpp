// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_DETAIL_ALLOCATOR_HPP
#define CNTGS_DETAIL_ALLOCATOR_HPP

#include "cntgs/detail/utility.hpp"

#include <cstddef>
#include <memory>
#include <utility>

namespace cntgs::detail
{
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

        Pointer ptr_{};
        std::size_t size_{};

        Impl() = default;

        constexpr Impl(Pointer ptr, std::size_t size, const Allocator& allocator) noexcept
            : Base{allocator}, ptr_(ptr), size_(size)
        {
        }
    };

    Impl impl_;

    constexpr auto allocate() { return AllocatorTraits::allocate(get_allocator(), size()); }

    constexpr void deallocate() noexcept
    {
        if (get())
        {
            AllocatorTraits::deallocate(get_allocator(), get(), size());
        }
    }

  public:
    AllocatorAwarePointer() = default;

    constexpr AllocatorAwarePointer(std::size_t size, const Allocator& allocator) : impl_({}, size, allocator)
    {
        get() = allocate();
    }

    constexpr AllocatorAwarePointer(pointer ptr, std::size_t size, const Allocator& allocator) noexcept
        : impl_(ptr, size, allocator)
    {
    }

    constexpr AllocatorAwarePointer(const AllocatorAwarePointer& other)
        : AllocatorAwarePointer(other.size(),
                                AllocatorTraits::select_on_container_copy_construction(other.get_allocator()))
    {
    }

    constexpr AllocatorAwarePointer(AllocatorAwarePointer&& other) noexcept
        : impl_(other.release(), other.size(), other.get_allocator())
    {
    }

    ~AllocatorAwarePointer() noexcept { deallocate(); }

    constexpr AllocatorAwarePointer& operator=(const AllocatorAwarePointer& other)
    {
        if (this != std::addressof(other))
        {
            if constexpr (AllocatorTraits::propagate_on_container_copy_assignment::value &&
                          !AllocatorTraits::is_always_equal::value)
            {
                if (get_allocator() != other.get_allocator())
                {
                    deallocate();
                    propagate_on_container_copy_assignment(other);
                    size() = other.size();
                    get() = allocate();
                    return *this;
                }
            }
            const bool needs_resize = size() < other.size();
            if (needs_resize)
            {
                deallocate();
            }
            propagate_on_container_copy_assignment(other);
            if (needs_resize)
            {
                size() = other.size();
                get() = allocate();
            }
        }
        return *this;
    }

    constexpr AllocatorAwarePointer& operator=(AllocatorAwarePointer&& other) noexcept
    {
        if (this != std::addressof(other))
        {
            deallocate();
            propagate_on_container_move_assignment(other);
            get() = other.release();
            size() = other.size();
        }
        return *this;
    }

    constexpr decltype(auto) get_allocator() noexcept { return impl_.get(); }

    constexpr const auto& get_allocator() const noexcept { return impl_.get(); }

    constexpr auto& get() noexcept { return impl_.ptr_; }

    constexpr auto get() const noexcept { return impl_.ptr_; }

    constexpr auto& size() noexcept { return impl_.size_; }

    constexpr auto size() const noexcept { return impl_.size_; }

    constexpr explicit operator bool() const noexcept { return get() != nullptr; }

    constexpr auto release() noexcept { return std::exchange(impl_.ptr_, nullptr); }

    constexpr void reset(AllocatorAwarePointer&& other) noexcept
    {
        deallocate();
        get() = other.release();
        size() = other.size();
    }

    constexpr void propagate_on_container_copy_assignment(const AllocatorAwarePointer& other) noexcept
    {
        if constexpr (AllocatorTraits::propagate_on_container_copy_assignment::value)
        {
            get_allocator() = other.get_allocator();
        }
    }

    constexpr void propagate_on_container_move_assignment(AllocatorAwarePointer& other) noexcept
    {
        if constexpr (AllocatorTraits::propagate_on_container_move_assignment::value)
        {
            get_allocator() = std::move(other.get_allocator());
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
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_ALLOCATOR_HPP
