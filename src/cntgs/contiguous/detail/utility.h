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

    constexpr MoveDefaultingValue(MoveDefaultingValue&& other) noexcept : value(other.value) { other.value = T{}; }

    constexpr MoveDefaultingValue& operator=(MoveDefaultingValue&& other) noexcept
    {
        this->value = other.value;
        other.value = T{};
        return *this;
    }
};

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