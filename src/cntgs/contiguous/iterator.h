#pragma once

#include "cntgs/contiguous/detail/iterator.h"
#include "cntgs/contiguous/detail/tuple.h"

#include <iterator>

namespace cntgs
{
template <class... Types>
class ContiguousVector;

template <class Vector>
class ContiguousVectorIterator
{
    static constexpr auto IS_CONST = std::is_const_v<Vector>;

  public:
    using value_type = typename Vector::value_type;
    using reference = std::conditional_t<IS_CONST, typename Vector::const_reference, typename Vector::reference>;
    using pointer = detail::ToContiguousTupleOfPointerReturnTypes<typename Vector::Tuple>;
    using difference_type = typename Vector::difference_type;
    using iterator_category = std::random_access_iterator_tag;

    ContiguousVectorIterator() = default;

    constexpr ContiguousVectorIterator(Vector& vector, std::size_t index) noexcept
        : vector(std::addressof(vector)), index(index)
    {
    }

    constexpr ContiguousVectorIterator(Vector& vector) noexcept : ContiguousVectorIterator(vector, {}) {}

    [[nodiscard]] auto operator*() const noexcept { return (*vector)[index]; }

    [[nodiscard]] auto operator*() noexcept { return (*vector)[index]; }

    [[nodiscard]] constexpr auto operator->() const noexcept { return detail::ArrowProxy<reference>{*(*this)}; }

    constexpr decltype(auto) operator++() noexcept
    {
        ++index;
        return *this;
    }

    constexpr auto operator++(int) noexcept
    {
        auto copy{*this};
        ++(*this);
        return copy;
    }

    constexpr decltype(auto) operator--() noexcept
    {
        --index;
        return *this;
    }

    constexpr auto operator--(int) noexcept
    {
        auto copy{*this};
        --(*this);
        return copy;
    }

    constexpr auto operator+(difference_type diff) const noexcept
    {
        auto copy{*this};
        copy.index += diff;
        return copy;
    }

    constexpr decltype(auto) operator+=(difference_type diff) noexcept
    {
        index += diff;
        return *this;
    }

    constexpr auto operator-(difference_type diff) const noexcept
    {
        auto copy{*this};
        copy.index -= diff;
        return copy;
    }

    constexpr decltype(auto) operator-=(difference_type diff) noexcept
    {
        index -= diff;
        return *this;
    }

    auto operator[](difference_type diff) const noexcept { return *(*this + diff); }

    constexpr bool operator==(const ContiguousVectorIterator& other) const noexcept
    {
        return index == other.index && vector == other.vector;
    }

    constexpr bool operator!=(const ContiguousVectorIterator& other) const noexcept { return !(*this == other); }

    constexpr bool operator<(const ContiguousVectorIterator& other) const noexcept
    {
        return index < other.index && vector == other.vector;
    }

    constexpr bool operator>(const ContiguousVectorIterator& other) const noexcept { return other < *this; }

    constexpr bool operator<=(const ContiguousVectorIterator& other) const noexcept { return !(*this > other); }

    constexpr bool operator>=(const ContiguousVectorIterator& other) const noexcept { return !(*this < other); }

  private:
    Vector* vector{};
    typename Vector::size_type index{};
};
}  // namespace cntgs