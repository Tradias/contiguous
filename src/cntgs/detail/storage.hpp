// Copyright (c) 2024 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_DETAIL_STORAGE_HPP
#define CNTGS_DETAIL_STORAGE_HPP

#include "cntgs/detail/allocator.hpp"
#include "cntgs/detail/memory.hpp"

namespace cntgs::detail
{
template <bool HasOffset, class Allocator>
class Storage : public AllocatorAwarePointer<Allocator>
{
  private:
    using Base = AllocatorAwarePointer<Allocator>;

    static constexpr auto ALIGNMENT = alignof(typename std::allocator_traits<Allocator>::value_type);

  public:
    using Base::Base;

    ~Storage() noexcept { restore_ptr(); }

    Storage(const Storage& other) = default;

    Storage(Storage&& other) = default;

    Storage& operator=(const Storage& other)
    {
        const auto offset = restore_ptr();
        static_cast<Base&>(*this) = other;
        add_offset(offset);
        return *this;
    }

    Storage& operator=(Storage&& other) noexcept
    {
        const auto offset = restore_ptr();
        static_cast<Base&>(*this) = static_cast<Base&&>(other);
        add_offset(offset);
        return *this;
    }

    std::byte* get() const noexcept { return reinterpret_cast<std::byte*>(Base::get()); }

    void reset(Storage&& other) noexcept
    {
        const auto offset = restore_ptr();
        Base::reset(static_cast<Storage&&>(other));
        add_offset(offset);
    }

    void add_offset(std::ptrdiff_t offset)
    {
        Base::get() = reinterpret_cast<typename Base::pointer>(reinterpret_cast<std::byte*>(Base::get()) + offset);
    }

    std::size_t get_offset() const noexcept { return get() - allocation_begin(); }

    std::byte* allocation_begin() const noexcept { return detail::align_down(get(), ALIGNMENT); }

  private:
    std::ptrdiff_t restore_ptr() noexcept
    {
        const auto current = Base::get();
        Base::get() = detail::align_down(current, ALIGNMENT);
        return current - Base::get();
    }
};

template <class Allocator>
class Storage<false, Allocator> : public AllocatorAwarePointer<Allocator>
{
  private:
    using Base = AllocatorAwarePointer<Allocator>;

  public:
    using Base::Base;

    std::byte* get() const noexcept { return reinterpret_cast<std::byte*>(Base::get()); }

    void add_offset(std::ptrdiff_t) {}

    std::size_t get_offset() const noexcept { return {}; }

    std::byte* allocation_begin() const noexcept { return get(); }
};
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_STORAGE_HPP
