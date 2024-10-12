// Copyright (c) 2024 Dennis Hezel

#ifndef CNTGS_DETAIL_UNMANAGEDVECTOR_HPP
#define CNTGS_DETAIL_UNMANAGEDVECTOR_HPP

#include "cntgs/detail/memory.hpp"

#include <cstddef>
#include <cstring>

namespace cntgs::detail
{
template <class T>
class UnmanagedVector
{
  private:
    T* data_{};
    std::size_t size_{};

  public:
    UnmanagedVector() = default;

    UnmanagedVector(const UnmanagedVector& other) = delete;

    constexpr UnmanagedVector(UnmanagedVector&& other) noexcept
        : data_(std::exchange(other.data_, nullptr)), size_(std::exchange(other.size_, 0))
    {
    }

    UnmanagedVector& operator=(const UnmanagedVector& other) = delete;

    constexpr UnmanagedVector& operator=(UnmanagedVector&& other) noexcept
    {
        data_ = std::exchange(other.data_, nullptr);
        size_ = std::exchange(other.size_, 0);
        return *this;
    }

    [[nodiscard]] constexpr bool empty() const noexcept { return size_ == 0; }

    [[nodiscard]] constexpr std::size_t size() const noexcept { return size_; }

    [[nodiscard]] constexpr T* data() const noexcept { return data_; }

    [[nodiscard]] constexpr T* begin() const noexcept { return data_; }

    [[nodiscard]] constexpr T* end() const noexcept { return data_ + size_; }

    [[nodiscard]] constexpr T& operator[](std::size_t i) const noexcept { return data_[i]; }

    constexpr void resize_from_capacity(std::size_t new_size) noexcept { size_ = new_size; }

    constexpr void put_back(T&& value) noexcept
    {
        data_[size_] = std::move(value);
        ++size_;
    }

    template <class Allocator>
    constexpr void reserve(std::size_t capacity, const Allocator& allocator)
    {
        using Traits = RebindTraits<Allocator, T>;
        typename Traits::allocator_type alloc(allocator);
        auto* const new_mem = Traits::allocate(alloc, capacity);
        if (!empty())
        {
            std::memcpy(new_mem, data_, size_ * sizeof(T));
        }
        data_ = new_mem;
    }
};
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_UNMANAGEDVECTOR_HPP
