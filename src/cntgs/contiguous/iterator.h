#pragma once

#include "cntgs/contiguous/detail/forward.h"
#include "cntgs/contiguous/detail/iterator.h"
#include "cntgs/contiguous/detail/vectorTraits.h"

#include <iterator>

namespace cntgs
{
template <class Vector>
class ContiguousVectorIterator
{
    static constexpr auto IS_CONST = std::is_const_v<Vector>;

  public:
    using value_type = typename Vector::value_type;
    using reference = std::conditional_t<IS_CONST, typename Vector::const_reference, typename Vector::reference>;
    using pointer = typename detail::VectorTraits<std::remove_const_t<Vector>>::PointerReturnType;
    using difference_type = typename Vector::difference_type;
    using iterator_category = std::random_access_iterator_tag;

    ContiguousVectorIterator() = default;

    constexpr ContiguousVectorIterator(Vector& vector, typename Vector::size_type index) noexcept
        : vector(std::addressof(vector)), index(index)
    {
    }

    constexpr ContiguousVectorIterator(Vector& vector) noexcept : ContiguousVectorIterator(vector, {}) {}

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

    [[nodiscard]] constexpr ContiguousVectorIterator operator+(ContiguousVectorIterator diff) const noexcept
    {
        auto copy{*this};
        copy.index += diff.index;
        return copy;
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

    [[nodiscard]] constexpr ContiguousVectorIterator operator-(ContiguousVectorIterator it) const noexcept
    {
        auto copy{*this};
        copy.index -= it.index;
        return copy;
    }

    constexpr ContiguousVectorIterator& operator-=(difference_type diff) noexcept
    {
        index -= diff;
        return *this;
    }

    [[nodiscard]] reference operator[](difference_type diff) const noexcept { return *(*this + diff); }

    [[nodiscard]] constexpr bool operator==(const ContiguousVectorIterator& other) const noexcept
    {
        return index == other.index && vector == other.vector;
    }

    [[nodiscard]] constexpr bool operator!=(const ContiguousVectorIterator& other) const noexcept
    {
        return !(*this == other);
    }

    [[nodiscard]] constexpr bool operator<(const ContiguousVectorIterator& other) const noexcept
    {
        return index < other.index && vector == other.vector;
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