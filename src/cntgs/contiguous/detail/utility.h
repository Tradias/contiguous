#pragma once

#include "cntgs/contiguous/span.h"

#include <utility>

namespace cntgs::detail
{
template <class T>
struct MoveDefaultingValue
{
    T value;

    explicit constexpr MoveDefaultingValue(T value) noexcept : value(value) {}

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

template <class T, bool Inherit>
class EmptyBaseOptimization
{
  private:
    T value;

  public:
    EmptyBaseOptimization() = default;

    explicit constexpr EmptyBaseOptimization(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>)
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

    explicit constexpr EmptyBaseOptimization(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>)
        : T{std::move(value)}
    {
    }

    constexpr T& get() noexcept { return *this; }

    constexpr const T& get() const noexcept { return *this; }
};

template <class T>
using EmptyBaseOptimizationT = detail::EmptyBaseOptimization<T, (std::is_empty_v<T> && !std::is_final_v<T>)>;

template <class T>
constexpr const cntgs::Span<T>& dereference(const cntgs::Span<T>& value) noexcept
{
    return value;
}

template <class T>
constexpr cntgs::Span<T>& dereference(cntgs::Span<T>& value) noexcept
{
    return value;
}

template <class T>
constexpr decltype(auto) dereference(T& value) noexcept
{
    return *value;
}

template <class T>
constexpr auto as_const(const cntgs::Span<T>& value) noexcept
{
    return cntgs::Span<std::add_const_t<T>>{value};
}

template <class T>
constexpr auto as_const(cntgs::Span<T>& value) noexcept
{
    return cntgs::Span<std::add_const_t<T>>{value};
}

template <class T>
constexpr decltype(auto) as_const(T& value) noexcept
{
    return std::as_const(value);
}
}  // namespace cntgs::detail