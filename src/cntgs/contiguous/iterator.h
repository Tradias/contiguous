#pragma once

#include "cntgs/contiguous/detail/iterator.h"

#include <iterator>

namespace cntgs
{
template <class Vector>
class ContiguousVectorIterator
{
  public:
    using value_type = typename Vector::value_type;
    using reference =
        std::conditional_t<std::is_const_v<Vector>, typename Vector::const_reference, typename Vector::reference>;
    using pointer = reference;
    using difference_type = typename Vector::difference_type;
    using iterator_category = std::random_access_iterator_tag;

    ContiguousVectorIterator() = default;

    constexpr ContiguousVectorIterator(Vector& vector, typename Vector::size_type index) noexcept
        : vector(std::addressof(vector)), index(index)
    {
    }

    explicit constexpr ContiguousVectorIterator(Vector& vector) noexcept : ContiguousVectorIterator(vector, {}) {}

    template <class TVector>
    constexpr ContiguousVectorIterator(const ContiguousVectorIterator<TVector>& other) noexcept
        : vector(other.vector), index(index)
    {
    }

    ContiguousVectorIterator(const ContiguousVectorIterator&) = default;
    ContiguousVectorIterator(ContiguousVectorIterator&&) = default;

    template <class TVector>
    constexpr ContiguousVectorIterator& operator=(const ContiguousVectorIterator<TVector>& other) noexcept
    {
        this->vector = other.vector;
        this->index = other.index;
    }

    ContiguousVectorIterator& operator=(const ContiguousVectorIterator&) = default;
    ContiguousVectorIterator& operator=(ContiguousVectorIterator&&) = default;

    [[nodiscard]] constexpr reference operator*() const noexcept { return (*vector)[index]; }

    [[nodiscard]] constexpr reference operator*() noexcept { return (*vector)[index]; }

    [[nodiscard]] constexpr detail::ArrowProxy<reference> operator->() const noexcept { return {*(*this)}; }

    constexpr ContiguousVectorIterator& operator++() noexcept
    {
        ++index;
        return *this;
    }

    constexpr ContiguousVectorIterator operator++(int) noexcept
    {
        auto copy{*this};
        ++(*this);
        return copy;
    }

    constexpr ContiguousVectorIterator& operator--() noexcept
    {
        --index;
        return *this;
    }

    constexpr ContiguousVectorIterator operator--(int) noexcept
    {
        auto copy{*this};
        --(*this);
        return copy;
    }

    [[nodiscard]] constexpr ContiguousVectorIterator operator+(difference_type diff) const noexcept
    {
        auto copy{*this};
        copy.index += diff;
        return copy;
    }

    [[nodiscard]] constexpr difference_type operator+(ContiguousVectorIterator it) const noexcept
    {
        return this->index + it.index;
    }

    constexpr ContiguousVectorIterator& operator+=(difference_type diff) noexcept
    {
        index += diff;
        return *this;
    }

    [[nodiscard]] constexpr ContiguousVectorIterator operator-(difference_type diff) const noexcept
    {
        auto copy{*this};
        copy.index -= diff;
        return copy;
    }

    [[nodiscard]] constexpr difference_type operator-(ContiguousVectorIterator it) const noexcept
    {
        return this->index - it.index;
    }

    constexpr ContiguousVectorIterator& operator-=(difference_type diff) noexcept
    {
        this->index -= diff;
        return *this;
    }

    [[nodiscard]] reference operator[](difference_type diff) const noexcept { return *(*this + diff); }

    [[nodiscard]] constexpr bool operator==(const ContiguousVectorIterator& other) const noexcept
    {
        return this->index == other.index && this->vector == other.vector;
    }

    [[nodiscard]] constexpr bool operator!=(const ContiguousVectorIterator& other) const noexcept
    {
        return !(*this == other);
    }

    [[nodiscard]] constexpr bool operator<(const ContiguousVectorIterator& other) const noexcept
    {
        return this->index < other.index && this->vector == other.vector;
    }

    [[nodiscard]] constexpr bool operator>(const ContiguousVectorIterator& other) const noexcept
    {
        return other < *this;
    }

    [[nodiscard]] constexpr bool operator<=(const ContiguousVectorIterator& other) const noexcept
    {
        return !(*this > other);
    }

    [[nodiscard]] constexpr bool operator>=(const ContiguousVectorIterator& other) const noexcept
    {
        return !(*this < other);
    }

  private:
    Vector* vector{};
    typename Vector::size_type index{};
};
}  // namespace cntgs