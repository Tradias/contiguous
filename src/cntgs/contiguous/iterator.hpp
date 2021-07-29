#ifndef CNTGS_CONTIGUOUS_ITERATOR_HPP
#define CNTGS_CONTIGUOUS_ITERATOR_HPP

#include "cntgs/contiguous/detail/elementLocator.hpp"
#include "cntgs/contiguous/detail/forward.hpp"
#include "cntgs/contiguous/detail/iteratorUtils.hpp"
#include "cntgs/contiguous/detail/typeUtils.hpp"

#include <iterator>

namespace cntgs
{
template <bool IsConst, class Allocator, class... Types>
class ContiguousVectorIterator
{
  private:
    using Vector = cntgs::BasicContiguousVector<Allocator, Types...>;
    using ListTraits = detail::ParameterListTraits<Types...>;
    using ElementLocator = detail::ElementLocatorT<Types...>;

  public:
    using value_type = typename Vector::value_type;
    using reference = detail::ConditionalT<IsConst, typename Vector::const_reference, typename Vector::reference>;
    using pointer = detail::ArrowProxy<reference>;
    using difference_type = typename Vector::difference_type;
    using iterator_category = std::random_access_iterator_tag;

    ContiguousVectorIterator() = default;

    constexpr ContiguousVectorIterator(const Vector& vector, typename Vector::size_type index) noexcept
        : i(index), memory(vector.memory.get()), fixed_sizes(vector.fixed_sizes), locator(vector.locator)
    {
    }

    explicit constexpr ContiguousVectorIterator(const Vector& vector) noexcept : ContiguousVectorIterator(vector, {}) {}

    template <bool OtherIsConst>
    /*implicit*/ constexpr ContiguousVectorIterator(
        const ContiguousVectorIterator<OtherIsConst, Allocator, Types...>& other) noexcept
        : i(other.i), memory(other.memory), fixed_sizes(other.fixed_sizes), locator(other.locator)
    {
    }

    ContiguousVectorIterator(const ContiguousVectorIterator&) = default;
    ContiguousVectorIterator(ContiguousVectorIterator&&) = default;

    template <bool OtherIsConst>
    constexpr ContiguousVectorIterator& operator=(
        const ContiguousVectorIterator<OtherIsConst, Allocator, Types...>& other) noexcept
    {
        this->i = other.i;
        this->memory = other.memory;
        this->fixed_sizes = other.fixed_sizes;
        this->locator = other.locator;
        return *this;
    }

    ContiguousVectorIterator& operator=(const ContiguousVectorIterator&) = default;
    ContiguousVectorIterator& operator=(ContiguousVectorIterator&&) = default;

    [[nodiscard]] constexpr auto index() const noexcept { return this->i; }

    [[nodiscard]] constexpr auto data() const noexcept -> detail::ConditionalT<IsConst, const std::byte*, std::byte*>
    {
        return this->locator.element_address(this->i, this->memory);
    }

    [[nodiscard]] constexpr reference operator*() const noexcept
    {
        return reference{this->locator.element_address(i, this->memory), this->fixed_sizes};
    }

    [[nodiscard]] constexpr reference operator*() noexcept
    {
        return reference{this->locator.element_address(i, this->memory), this->fixed_sizes};
    }

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
        return this->i == other.i && this->memory == other.memory;
    }

    [[nodiscard]] constexpr bool operator!=(const ContiguousVectorIterator& other) const noexcept
    {
        return !(*this == other);
    }

    [[nodiscard]] constexpr bool operator<(const ContiguousVectorIterator& other) const noexcept
    {
        return this->i < other.i && this->memory == other.memory;
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
    template <bool, class, class...>
    friend class ContiguousVectorIterator;

    typename Vector::size_type i{};
    std::byte* memory;
    typename ListTraits::FixedSizes fixed_sizes;
    ElementLocator locator;
};
}  // namespace cntgs

#endif  // CNTGS_CONTIGUOUS_ITERATOR_HPP