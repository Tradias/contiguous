// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_CNTGS_ITERATOR_HPP
#define CNTGS_CNTGS_ITERATOR_HPP

#include "cntgs/detail/elementLocator.hpp"
#include "cntgs/detail/forward.hpp"
#include "cntgs/detail/iterator.hpp"
#include "cntgs/detail/typeTraits.hpp"

#include <iterator>

namespace cntgs
{
template <bool IsConst, class Options, class... Parameter>
class ContiguousVectorIterator
{
  private:
    using Vector = cntgs::BasicContiguousVector<Options, Parameter...>;
    using ElementLocatorAndFixedSizes = detail::ElementLocatorAndFixedSizes<Parameter...>;
    using SizeType = typename Vector::size_type;
    using MemoryPointer = typename std::allocator_traits<typename Vector::allocator_type>::pointer;

  public:
    using value_type = typename Vector::value_type;
    using reference = detail::ConditionalT<IsConst, typename Vector::const_reference, typename Vector::reference>;
    using pointer = detail::ArrowProxy<reference>;
    using difference_type = typename Vector::difference_type;
    using iterator_category = std::random_access_iterator_tag;

    ContiguousVectorIterator() = default;

    constexpr ContiguousVectorIterator(const Vector& vector, SizeType index) noexcept
        : i_(index), memory_(vector.memory_.get()), locator_(vector.locator_)
    {
    }

    constexpr explicit ContiguousVectorIterator(const Vector& vector) noexcept
        : ContiguousVectorIterator(vector, SizeType{})
    {
    }

    template <bool OtherIsConst>
    /*implicit*/ constexpr ContiguousVectorIterator(
        const ContiguousVectorIterator<OtherIsConst, Options, Parameter...>& other) noexcept
        : i_(other.i_), memory_(other.memory_), locator_(other.locator_)
    {
    }

    ContiguousVectorIterator(const ContiguousVectorIterator&) = default;
    ContiguousVectorIterator(ContiguousVectorIterator&&) = default;

    template <bool OtherIsConst>
    constexpr ContiguousVectorIterator& operator=(
        const ContiguousVectorIterator<OtherIsConst, Options, Parameter...>& other) noexcept
    {
        i_ = other.i_;
        memory_ = other.memory_;
        locator_ = other.locator_;
        return *this;
    }

    ContiguousVectorIterator& operator=(const ContiguousVectorIterator&) = default;
    ContiguousVectorIterator& operator=(ContiguousVectorIterator&&) = default;

    [[nodiscard]] constexpr auto index() const noexcept { return i_; }

    [[nodiscard]] constexpr auto data() const noexcept -> detail::ConditionalT<IsConst, const std::byte*, std::byte*>
    {
        return locator_->element_address(i_, memory_);
    }

    [[nodiscard]] constexpr reference operator*() noexcept
    {
        return reference{locator_->element_address(i_, memory_), locator_.fixed_sizes()};
    }

    [[nodiscard]] constexpr reference operator*() const noexcept
    {
        return reference{locator_->element_address(i_, memory_), locator_.fixed_sizes()};
    }

    [[nodiscard]] constexpr pointer operator->() const noexcept { return {*(*this)}; }

    constexpr ContiguousVectorIterator& operator++() noexcept
    {
        ++i_;
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
        --i_;
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
        copy.i_ += diff;
        return copy;
    }

    [[nodiscard]] constexpr difference_type operator+(ContiguousVectorIterator it) const noexcept
    {
        return i_ + it.i_;
    }

    constexpr ContiguousVectorIterator& operator+=(difference_type diff) noexcept
    {
        i_ += diff;
        return *this;
    }

    [[nodiscard]] constexpr ContiguousVectorIterator operator-(difference_type diff) const noexcept
    {
        auto copy{*this};
        copy.i_ -= diff;
        return copy;
    }

    [[nodiscard]] constexpr difference_type operator-(ContiguousVectorIterator it) const noexcept
    {
        return i_ - it.i_;
    }

    constexpr ContiguousVectorIterator& operator-=(difference_type diff) noexcept
    {
        i_ -= diff;
        return *this;
    }

    [[nodiscard]] reference operator[](difference_type diff) const noexcept { return *(*this + diff); }

    [[nodiscard]] constexpr bool operator==(const ContiguousVectorIterator& other) const noexcept
    {
        return i_ == other.i_ && memory_ == other.memory_;
    }

    [[nodiscard]] constexpr bool operator!=(const ContiguousVectorIterator& other) const noexcept
    {
        return !(*this == other);
    }

    [[nodiscard]] constexpr bool operator<(const ContiguousVectorIterator& other) const noexcept
    {
        return i_ < other.i_ && memory_ == other.memory_;
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
    friend cntgs::ContiguousVectorIterator<!IsConst, Options, Parameter...>;

    SizeType i_{};
    MemoryPointer memory_;
    ElementLocatorAndFixedSizes locator_;
};
}  // namespace cntgs

#endif  // CNTGS_CNTGS_ITERATOR_HPP
