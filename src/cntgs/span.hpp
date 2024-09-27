// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_CNTGS_SPAN_HPP
#define CNTGS_CNTGS_SPAN_HPP

#include <cstddef>
#include <iterator>
#include <version>

#ifdef __cpp_lib_span
#include <span>
#endif

namespace cntgs
{
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

    iterator first_;
    iterator last_;

    Span() = default;

    template <class U>
    constexpr explicit Span(const Span<U>& other) noexcept : first_(other.first_), last_(other.last_)
    {
    }

    Span(const Span& other) = default;

    Span(Span&& other) = default;

    Span& operator=(const Span& other) = default;

    Span& operator=(Span&& other) = default;

    constexpr Span(iterator first, iterator last) noexcept : first_(first), last_(last) {}

    constexpr Span(iterator first, size_type size) noexcept(noexcept(first + size)) : first_(first), last_(first + size)
    {
    }

    [[nodiscard]] constexpr iterator begin() const noexcept { return first_; }

    [[nodiscard]] constexpr iterator end() const noexcept { return last_; }

    [[nodiscard]] constexpr reverse_iterator rbegin() const noexcept { return reverse_iterator{end()}; }

    [[nodiscard]] constexpr reverse_iterator rend() const noexcept { return reverse_iterator{begin()}; }

    [[nodiscard]] constexpr bool empty() const noexcept { return first_ == last_; }

    [[nodiscard]] constexpr size_type size() const noexcept { return last_ - first_; }

    [[nodiscard]] constexpr pointer data() const noexcept { return first_; }

    [[nodiscard]] constexpr reference operator[](size_type i) const noexcept { return first_[i]; }

    [[nodiscard]] constexpr reference front() const noexcept { return first_[0]; }

    [[nodiscard]] constexpr reference back() const noexcept { return *(last_ - 1); }

#ifdef __cpp_lib_span
    constexpr operator std::span<T>() const noexcept { return std::span<T>{first_, last_}; }
#endif
};
}  // namespace cntgs

#endif  // CNTGS_CNTGS_SPAN_HPP
