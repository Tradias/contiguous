#ifndef CNTGS_CONTIGUOUS_ITERATOR_H
#define CNTGS_CONTIGUOUS_ITERATOR_H

#include "cntgs/contiguous/detail/iteratorUtils.h"
#include "cntgs/contiguous/detail/typeUtils.h"

#include <iterator>

namespace cntgs
{
template <class Vector>
class ContiguousVectorIterator
{
  public:
    using value_type = typename Vector::value_type;
    using reference =
        detail::ConditionalT<std::is_const_v<Vector>, typename Vector::const_reference, typename Vector::reference>;
    using pointer = detail::ArrowProxy<reference>;
    using difference_type = typename Vector::difference_type;
    using iterator_category = std::random_access_iterator_tag;

    ContiguousVectorIterator() = default;

    constexpr ContiguousVectorIterator(Vector& vector, typename Vector::size_type index) noexcept
        : vector(std::addressof(vector)), i(index)
    {
    }

    explicit constexpr ContiguousVectorIterator(Vector& vector) noexcept : ContiguousVectorIterator(vector, {}) {}

    template <class TVector>
    /*implicit*/ constexpr ContiguousVectorIterator(const ContiguousVectorIterator<TVector>& other) noexcept
        : vector(other.vector), i(other.i)
    {
    }

    ContiguousVectorIterator(const ContiguousVectorIterator&) = default;
    ContiguousVectorIterator(ContiguousVectorIterator&&) = default;

    template <class TVector>
    constexpr ContiguousVectorIterator& operator=(const ContiguousVectorIterator<TVector>& other) noexcept
    {
        this->vector = other.vector;
        this->i = other.i;
        return *this;
    }

    ContiguousVectorIterator& operator=(const ContiguousVectorIterator&) = default;
    ContiguousVectorIterator& operator=(ContiguousVectorIterator&&) = default;

    [[nodiscard]] constexpr auto index() const noexcept { return this->i; }

    [[nodiscard]] constexpr reference operator*() const noexcept { return (*this->vector)[this->i]; }

    [[nodiscard]] constexpr reference operator*() noexcept { return (*this->vector)[this->i]; }

    [[nodiscard]] constexpr pointer operator->() const noexcept { return {*(*this)}; }

    constexpr ContiguousVectorIterator& operator++() noexcept
    {
        ++this->i;
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
        --this->i;
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
        copy.i += diff;
        return copy;
    }

    [[nodiscard]] constexpr difference_type operator+(ContiguousVectorIterator it) const noexcept
    {
        return this->i + it.i;
    }

    constexpr ContiguousVectorIterator& operator+=(difference_type diff) noexcept
    {
        this->i += diff;
        return *this;
    }

    [[nodiscard]] constexpr ContiguousVectorIterator operator-(difference_type diff) const noexcept
    {
        auto copy{*this};
        copy.i -= diff;
        return copy;
    }

    [[nodiscard]] constexpr difference_type operator-(ContiguousVectorIterator it) const noexcept
    {
        return this->i - it.i;
    }

    constexpr ContiguousVectorIterator& operator-=(difference_type diff) noexcept
    {
        this->i -= diff;
        return *this;
    }

    [[nodiscard]] reference operator[](difference_type diff) const noexcept { return *(*this + diff); }

    [[nodiscard]] constexpr bool operator==(const ContiguousVectorIterator& other) const noexcept
    {
        return this->i == other.i && this->vector == other.vector;
    }

    [[nodiscard]] constexpr bool operator!=(const ContiguousVectorIterator& other) const noexcept
    {
        return !(*this == other);
    }

    [[nodiscard]] constexpr bool operator<(const ContiguousVectorIterator& other) const noexcept
    {
        return this->i < other.i && this->vector == other.vector;
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
    template <class TVector>
    friend class ContiguousVectorIterator;

    Vector* vector{};
    typename Vector::size_type i{};
};
}  // namespace cntgs

#endif  // CNTGS_CONTIGUOUS_ITERATOR_H
