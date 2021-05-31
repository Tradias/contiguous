#pragma once

#include "cntgs/contiguous/span.h"

#include <utility>

namespace cntgs::detail
{
template <class T>
constexpr cntgs::Span<T> dereference(const cntgs::Span<T>& memory) noexcept
{
    return memory;
}

template <class T>
constexpr cntgs::Span<T> dereference(cntgs::Span<T>& memory) noexcept
{
    return memory;
}

template <class T>
constexpr cntgs::Span<T> dereference(cntgs::Span<T>&& memory) noexcept
{
    return memory;
}

template <class T>
constexpr decltype(auto) dereference(T&& memory) noexcept
{
    return *memory;
}

template <class T>
constexpr cntgs::Span<std::add_const_t<T>> as_const(const cntgs::Span<T>& memory) noexcept
{
    return {memory.data(), memory.size()};
}

template <class T>
constexpr cntgs::Span<std::add_const_t<T>> as_const(cntgs::Span<T>& memory) noexcept
{
    return {memory.data(), memory.size()};
}

template <class T>
constexpr cntgs::Span<std::add_const_t<T>> as_const(cntgs::Span<T>&& memory) noexcept
{
    return {memory.data(), memory.size()};
}

template <class T>
constexpr decltype(auto) as_const(T&& memory) noexcept
{
    return std::as_const(memory);
}
}  // namespace cntgs::detail