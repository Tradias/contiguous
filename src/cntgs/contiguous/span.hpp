#ifndef CNTGS_CONTIGUOUS_SPAN_HPP
#define CNTGS_CONTIGUOUS_SPAN_HPP

#include <cstddef>
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

    iterator first;
    iterator last;

    Span() = default;

    template <class U>
    explicit constexpr Span(const Span<U>& other) noexcept : first(other.first), last(other.last)
    {
    }

    Span(const Span& other) = default;

    constexpr Span(iterator first, iterator last) noexcept : first(first), last(last) {}

    constexpr Span(iterator first, size_type size) noexcept(noexcept(first + size)) : first(first), last(first + size)
    {
    }

    [[nodiscard]] constexpr iterator begin() const noexcept { return this->first; }

    [[nodiscard]] constexpr iterator end() const noexcept { return this->last; }

    [[nodiscard]] constexpr reverse_iterator rbegin() const noexcept { return reverse_iterator{this->end()}; }

    [[nodiscard]] constexpr reverse_iterator rend() const noexcept { return reverse_iterator{this->begin()}; }

    [[nodiscard]] constexpr bool empty() const noexcept { return this->first == this->last; }

    [[nodiscard]] constexpr size_type size() const noexcept { return this->last - this->first; }

    [[nodiscard]] constexpr pointer data() const noexcept { return this->first; }

    [[nodiscard]] constexpr reference operator[](size_type i) const noexcept { return this->first[i]; }

    [[nodiscard]] constexpr reference front() const noexcept { return this->first[0]; }

    [[nodiscard]] constexpr reference back() const noexcept { return *(this->last - 1); }

#ifdef __cpp_lib_span
    constexpr operator std::span<T>() const noexcept { return std::span<T>{first, last}; }
#endif
};
}  // namespace cntgs

#endif  // CNTGS_CONTIGUOUS_SPAN_HPP
