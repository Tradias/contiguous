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
    using element_type = T;
    using value_type = std::remove_cv_t<T>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using iterator = T*;
    using reverse_iterator = std::reverse_iterator<iterator>;

    iterator first;
    iterator last;

    Span() = default;

    template <class U>
    explicit Span(Span<U> other) noexcept : first(other.first), last(other.last)
    {
    }

    constexpr Span(iterator first, iterator last) noexcept : first(first), last(last) {}

    constexpr Span(iterator first, size_type size) noexcept : first(first), last(first + size) {}

    constexpr iterator begin() const noexcept { return first; }

    constexpr iterator end() const noexcept { return last; }

    constexpr size_type size() const noexcept { return last - first; }

    constexpr pointer data() const noexcept { return first; }

    constexpr reference operator[](size_type i) const { return first[i]; }

    constexpr reference front() const { return *first; }

    constexpr reference back() const { return *(last - 1); }
};
#endif
}  // namespace cntgs