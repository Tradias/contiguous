#pragma once

#include <cstddef>

#ifdef __cpp_lib_span
#include <span>
#endif

namespace cntgs
{
#ifdef __cpp_lib_span
template <class T>
using Span = std::span<T>;
#else
template <class T>
struct Span
{
    using value_type = T;
    using iterator_type = T*;
    using difference_type = std::ptrdiff_t;
    using size_type = std::size_t;

    iterator_type first;
    iterator_type last;

    Span() = default;

    template <class U>
    explicit Span(Span<U> other) noexcept : first(other.first), last(other.last)
    {
    }

    constexpr Span(iterator_type first, iterator_type last) noexcept : first(first), last(last) {}

    constexpr Span(iterator_type first, size_type size) noexcept : first(first), last(first + size) {}

    constexpr auto begin() const noexcept { return first; }

    constexpr auto end() const noexcept { return last; }

    constexpr size_type size() const noexcept { return last - first; }

    constexpr auto data() const noexcept { return first; }

    constexpr auto operator[](size_type i) const noexcept { return first[i]; }
};
#endif
}  // namespace cntgs