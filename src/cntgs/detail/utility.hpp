// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_DETAIL_UTILITY_HPP
#define CNTGS_DETAIL_UTILITY_HPP

#include "cntgs/span.hpp"

#include <utility>

namespace cntgs::detail
{
template <class T>
struct MoveDefaultingValue
{
    T value;

    constexpr explicit MoveDefaultingValue(T value) noexcept : value(value) {}

    ~MoveDefaultingValue() = default;

    MoveDefaultingValue(const MoveDefaultingValue&) = default;

    constexpr MoveDefaultingValue(MoveDefaultingValue&& other) noexcept : value(other.value) { other.value = T{}; }

    MoveDefaultingValue& operator=(const MoveDefaultingValue& other) = default;

    constexpr MoveDefaultingValue& operator=(MoveDefaultingValue&& other) noexcept
    {
        this->value = other.value;
        other.value = T{};
        return *this;
    }
};

template <class T, bool = (std::is_empty_v<T> && !std::is_final_v<T>)>
class EmptyBaseOptimization
{
  private:
    T value;

  public:
    EmptyBaseOptimization() = default;

    constexpr explicit EmptyBaseOptimization(const T& value) noexcept(std::is_nothrow_copy_constructible_v<T>)
        : value{value}
    {
    }

    constexpr explicit EmptyBaseOptimization(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>)
        : value{std::move(value)}
    {
    }

    constexpr auto& get() noexcept { return value; }

    constexpr const auto& get() const noexcept { return value; }
};

template <class T>
class EmptyBaseOptimization<T, true> : private T
{
  public:
    EmptyBaseOptimization() = default;

    constexpr explicit EmptyBaseOptimization(const T& value) noexcept(std::is_nothrow_copy_constructible_v<T>)
        : T{value}
    {
    }

    constexpr explicit EmptyBaseOptimization(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>)
        : T{std::move(value)}
    {
    }

    constexpr T& get() noexcept { return *this; }

    constexpr const T& get() const noexcept { return *this; }
};

template <class T>
constexpr auto as_const(cntgs::Span<T> value) noexcept
{
    return cntgs::Span<std::add_const_t<T>>{value};
}

template <class T>
constexpr decltype(auto) as_const(T& value) noexcept
{
    return std::as_const(value);
}

template <class T>
constexpr decltype(auto) as_ref(T* value) noexcept
{
    return (*value);
}

template <class T>
constexpr decltype(auto) as_ref(T& value) noexcept
{
    return (value);
}

template <class T>
constexpr decltype(auto) as_const_ref(T&& value) noexcept
{
    return detail::as_const(detail::as_ref(std::forward<T>(value)));
}
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_UTILITY_HPP
