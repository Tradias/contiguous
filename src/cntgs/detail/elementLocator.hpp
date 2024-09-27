// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_DETAIL_ELEMENTLOCATOR_HPP
#define CNTGS_DETAIL_ELEMENTLOCATOR_HPP

#include "cntgs/detail/elementTraits.hpp"
#include "cntgs/detail/memory.hpp"
#include "cntgs/detail/parameterListTraits.hpp"
#include "cntgs/detail/typeTraits.hpp"
#include "cntgs/detail/utility.hpp"

#include <algorithm>
#include <type_traits>

namespace cntgs::detail
{
template <class Locator>
auto move_elements(std::size_t from, std::size_t to, std::byte* memory_begin, const Locator& locator) noexcept
{
    const auto target = locator.element_address(to, memory_begin);
    const auto source = locator.element_address(from, memory_begin);
    const auto count = static_cast<std::size_t>(locator.data_end() - source);
    std::memmove(target, source, count);
    return source - target;
}

class BaseElementLocator
{
  protected:
    static constexpr auto RESERVED_BYTES_PER_ELEMENT = sizeof(std::byte*);

    std::byte** last_element_address_{};
    std::byte* last_element_{};

    BaseElementLocator() = default;

    constexpr BaseElementLocator(std::byte** last_element_address, std::byte* last_element) noexcept
        : last_element_address_(last_element_address), last_element_(last_element)
    {
    }

  public:
    static constexpr auto reserved_bytes(std::size_t element_count) noexcept
    {
        return element_count * RESERVED_BYTES_PER_ELEMENT;
    }

    bool empty(std::byte* memory_begin) const noexcept
    {
        return last_element_address_ == reinterpret_cast<std::byte**>(memory_begin);
    }

    std::size_t size(std::byte* memory_begin) const noexcept
    {
        return last_element_address_ - reinterpret_cast<std::byte**>(memory_begin);
    }

    static auto element_address(std::size_t index, std::byte* memory_begin) noexcept
    {
        const auto element_addresses_begin = reinterpret_cast<std::byte**>(memory_begin);
        return element_addresses_begin[index];
    }

    constexpr auto data_end() const noexcept { return last_element_; }

    void resize(std::size_t new_size, std::byte* memory_begin) noexcept
    {
        last_element_ = BaseElementLocator::element_address(new_size, memory_begin);
        last_element_address_ = reinterpret_cast<std::byte**>(memory_begin) + new_size;
    }

    void move_elements_forward(std::size_t from, std::size_t to, std::byte* memory_begin) const noexcept
    {
        const auto diff = detail::move_elements(from, to, memory_begin, *this);
        const auto element_addresses_begin = reinterpret_cast<std::byte**>(memory_begin);
        std::transform(element_addresses_begin + from, last_element_address_, element_addresses_begin + to,
                       [&](auto address)
                       {
                           return address - diff;
                       });
    }

    void make_room_for_last_element_at(std::size_t from, std::size_t size_of_element,
                                       std::byte* memory_begin) const noexcept
    {
        const auto source = element_address(from, memory_begin);
        const auto target = source + size_of_element;
        const auto count = static_cast<std::size_t>(data_end() - source);
        std::memmove(target, source, count);
        *last_element_address_ = last_element_;
        const auto element_addresses_begin = reinterpret_cast<std::byte**>(memory_begin);
        std::transform(element_addresses_begin + from, last_element_address_, element_addresses_begin + from + 1,
                       [&](auto address)
                       {
                           return address + size_of_element;
                       });
    }
};

template <bool IsAllFixedSize, class... Parameter>
class ElementLocator : public BaseElementLocator
{
  private:
    using ElementTraits = detail::ElementTraitsT<Parameter...>;
    using FixedSizesArray = typename detail::ParameterListTraits<Parameter...>::FixedSizesArray;

  public:
    ElementLocator() = default;

    ElementLocator(std::size_t max_element_count, std::byte* memory_begin, const FixedSizesArray&) noexcept
        : BaseElementLocator(reinterpret_cast<std::byte**>(memory_begin),
                             ElementLocator::calculate_element_start(max_element_count, memory_begin))
    {
    }

    ElementLocator(ElementLocator& old_locator, std::size_t old_max_element_count, std::byte* old_memory_begin,
                   std::size_t new_max_element_count, std::byte* new_memory_begin) noexcept
    {
        trivially_copy_into(old_locator, old_max_element_count, old_memory_begin, new_max_element_count,
                            new_memory_begin);
    }

    template <class... Args>
    auto emplace_back(const FixedSizesArray& fixed_sizes, Args&&... args)
    {
        const auto new_last_element =
            ElementTraits::emplace_at(last_element_, fixed_sizes, std::forward<Args>(args)...);
        *last_element_address_ = last_element_;
        ++last_element_address_;
        last_element_ = new_last_element;
        return last_element_;
    }

    template <class... Args>
    static auto emplace_at(std::size_t index, std::byte* memory_begin, const FixedSizesArray& fixed_sizes,
                           Args&&... args)
    {
        const auto element_addresses_begin = reinterpret_cast<std::byte**>(memory_begin);
        element_addresses_begin[index + 1] =
            ElementTraits::emplace_at_aliased(element_addresses_begin[index], fixed_sizes, std::forward<Args>(args)...);
        return element_addresses_begin[index + 1];
    }

    template <class... Args>
    static auto emplace_at(std::byte* address, const std::byte*, const FixedSizesArray& fixed_sizes, Args&&... args)
    {
        ElementTraits::emplace_at_aliased(address, fixed_sizes, std::forward<Args>(args)...);
    }

    void trivially_copy_into(std::size_t old_max_element_count, std::byte* CNTGS_RESTRICT old_memory_begin,
                             std::size_t new_max_element_count, std::byte* CNTGS_RESTRICT new_memory_begin) noexcept
    {
        trivially_copy_into(*this, old_max_element_count, old_memory_begin, new_max_element_count, new_memory_begin);
    }

    static constexpr auto calculate_new_memory_size(std::size_t max_element_count, std::size_t varying_size_bytes,
                                                    const FixedSizesArray& fixed_sizes) noexcept
    {
        constexpr auto ALIGNMENT_OVERHEAD = ElementTraits::template ParameterTraitsAt<0>::ALIGNMENT - 1;
        return varying_size_bytes + ElementTraits::calculate_element_size(fixed_sizes) * max_element_count +
               ElementLocator::reserved_bytes(max_element_count) + ALIGNMENT_OVERHEAD;
    }

  private:
    void trivially_copy_into(ElementLocator& old_locator, std::size_t old_max_element_count,
                             std::byte* CNTGS_RESTRICT old_memory_begin, std::size_t new_max_element_count,
                             std::byte* CNTGS_RESTRICT new_memory_begin) noexcept
    {
        const auto new_start = ElementLocator::calculate_element_start(new_max_element_count, new_memory_begin);
        const auto old_start = ElementLocator::calculate_element_start(old_max_element_count, old_memory_begin);
        const auto size_diff = std::distance(new_memory_begin, new_start) - std::distance(old_memory_begin, old_start);
        const auto new_last_element_address = ElementLocator::copy_element_addresses(
            new_memory_begin, old_memory_begin, old_locator.last_element_address_, size_diff);
        const auto old_used_memory_size = std::distance(old_start, old_locator.last_element_);
        std::memcpy(new_start, old_start, old_used_memory_size);
        last_element_address_ = new_last_element_address;
        last_element_ = new_memory_begin + std::distance(old_memory_begin, old_locator.last_element_) + size_diff;
    }

    static auto copy_element_addresses(std::byte* new_memory_begin, std::byte* old_memory_begin,
                                       std::byte** old_last_element_address, std::ptrdiff_t size_diff) noexcept
    {
        auto new_last_element_address = reinterpret_cast<std::byte**>(new_memory_begin);
        std::for_each(reinterpret_cast<std::byte**>(old_memory_begin), old_last_element_address,
                      [&](std::byte* element)
                      {
                          *new_last_element_address =
                              new_memory_begin + std::distance(old_memory_begin, element) + size_diff;
                          ++new_last_element_address;
                      });
        return new_last_element_address;
    }

    static constexpr auto calculate_element_start(std::size_t max_element_count, std::byte* memory_begin) noexcept
    {
        return detail::align<ElementTraits::template ParameterTraitsAt<0>::ALIGNMENT>(
            memory_begin + BaseElementLocator::reserved_bytes(max_element_count));
    }
};

class BaseAllFixedSizeElementLocator
{
  protected:
    std::size_t element_count_{};
    std::size_t stride_{};
    std::byte* start_{};

    BaseAllFixedSizeElementLocator() = default;

    constexpr BaseAllFixedSizeElementLocator(std::size_t element_count, std::size_t stride, std::byte* start) noexcept
        : element_count_(element_count), stride_(stride), start_(start)
    {
    }

  public:
    static constexpr auto reserved_bytes(std::size_t) noexcept { return std::size_t{}; }

    constexpr bool empty(const std::byte*) const noexcept { return element_count_ == std::size_t{}; }

    constexpr std::size_t size(const std::byte*) const noexcept { return element_count_; }

    constexpr auto element_address(std::size_t index, const std::byte*) const noexcept
    {
        return start_ + stride_ * index;
    }

    constexpr auto data_end() const noexcept { return start_ + stride_ * element_count_; }

    constexpr void resize(std::size_t new_size, const std::byte*) noexcept { element_count_ = new_size; }

    void move_elements_forward(std::size_t from, std::size_t to, const std::byte*) const noexcept
    {
        detail::move_elements(from, to, {}, *this);
    }

    void make_room_for_last_element_at(std::size_t from, std::size_t size_of_element, const std::byte*) const noexcept
    {
        const auto source = element_address(from, {});
        const auto target = source + size_of_element;
        const auto count = static_cast<std::size_t>(data_end() - source);
        std::memmove(target, source, count);
    }
};

template <class... Parameter>
class ElementLocator<true, Parameter...> : public BaseAllFixedSizeElementLocator
{
  private:
    using ElementTraits = detail::ElementTraitsT<Parameter...>;
    using FixedSizesArray = typename detail::ParameterListTraits<Parameter...>::FixedSizesArray;

  public:
    ElementLocator() = default;

    constexpr ElementLocator(std::size_t, std::byte* memory_begin, const FixedSizesArray& fixed_sizes) noexcept
        : BaseAllFixedSizeElementLocator({}, ElementTraits::calculate_element_size(fixed_sizes),
                                         ElementLocator::calculate_element_start(memory_begin))
    {
    }

    ElementLocator(ElementLocator& old_locator, std::size_t, const std::byte*, std::size_t,
                   std::byte* new_memory_begin) noexcept
        : BaseAllFixedSizeElementLocator(old_locator.element_count_, old_locator.stride_, {})
    {
        trivially_copy_into(old_locator, new_memory_begin);
    }

    template <class... Args>
    auto emplace_back(const FixedSizesArray& fixed_sizes, Args&&... args)
    {
        const auto last_element = element_address(element_count_, {});
        auto end = ElementTraits::emplace_at(last_element, fixed_sizes, std::forward<Args>(args)...);
        ++element_count_;
        return end;
    }

    template <class... Args>
    auto emplace_at(std::size_t index, const std::byte*, const FixedSizesArray& fixed_sizes, Args&&... args)
    {
        return ElementTraits::emplace_at_aliased(element_address(index, {}), fixed_sizes, std::forward<Args>(args)...);
    }

    void trivially_copy_into(std::size_t, const std::byte*, std::size_t, std::byte* new_memory_begin) noexcept
    {
        trivially_copy_into(*this, new_memory_begin);
    }

    constexpr auto calculate_new_memory_size(std::size_t max_element_count, std::size_t varying_size_bytes,
                                             const FixedSizesArray&) noexcept
    {
        constexpr auto ALIGNMENT_OVERHEAD = ElementTraits::template ParameterTraitsAt<0>::ALIGNMENT - 1;
        return varying_size_bytes + stride_ * max_element_count + ElementLocator::reserved_bytes(max_element_count) +
               ALIGNMENT_OVERHEAD;
    }

  private:
    void trivially_copy_into(const ElementLocator& old_locator, std::byte* new_memory_begin) noexcept
    {
        const auto new_start = ElementLocator::calculate_element_start(new_memory_begin);
        std::memcpy(new_start, old_locator.start_, old_locator.element_count_ * old_locator.stride_);
        start_ = new_start;
    }

    static constexpr auto calculate_element_start(std::byte* memory_begin) noexcept
    {
        return detail::align<ElementTraits::template ParameterTraitsAt<0>::ALIGNMENT>(memory_begin);
    }
};

template <class... Parameter>
using ElementLocatorT =
    detail::ElementLocator<detail::ParameterListTraits<Parameter...>::IS_FIXED_SIZE_OR_PLAIN, Parameter...>;

template <class... Parameter>
class ElementLocatorAndFixedSizes
    : private detail::EmptyBaseOptimization<typename detail::ParameterListTraits<Parameter...>::FixedSizesArray>
{
  private:
    static constexpr auto HAS_FIXED_SIZES = detail::ParameterListTraits<Parameter...>::CONTIGUOUS_FIXED_SIZE_COUNT > 0;

    using FixedSizesArray = typename detail::ParameterListTraits<Parameter...>::FixedSizesArray;
    using Locator = detail::ElementLocatorT<Parameter...>;
    using Base = detail::EmptyBaseOptimization<FixedSizesArray>;

  public:
    Locator locator_;

    ElementLocatorAndFixedSizes() = default;

    constexpr ElementLocatorAndFixedSizes(const Locator& locator, const FixedSizesArray& fixed_sizes) noexcept
        : Base{fixed_sizes}, locator_(locator)
    {
    }

    constexpr ElementLocatorAndFixedSizes(std::size_t max_element_count, std::byte* memory,
                                          const FixedSizesArray& fixed_sizes) noexcept
        : Base{fixed_sizes}, locator_(max_element_count, memory, fixed_sizes)
    {
    }

    constexpr auto operator->() noexcept { return std::addressof(locator_); }

    constexpr auto operator->() const noexcept { return std::addressof(locator_); }

    constexpr auto& operator*() noexcept { return locator_; }

    constexpr const auto& operator*() const noexcept { return locator_; }

    constexpr auto& fixed_sizes() noexcept { return Base::get(); }

    constexpr const auto& fixed_sizes() const noexcept { return Base::get(); }
};

using TypeErasedElementLocator =
    std::aligned_storage_t<std::max(sizeof(detail::ElementLocator<false>), sizeof(detail::ElementLocator<true>)),
                           std::max(alignof(detail::ElementLocator<false>), alignof(detail::ElementLocator<true>))>;

template <class T>
auto type_erase_element_locator(T&& locator) noexcept
{
    detail::TypeErasedElementLocator result;
    detail::construct_at(reinterpret_cast<detail::RemoveCvrefT<T>*>(&result), std::forward<T>(locator));
    return result;
}
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_ELEMENTLOCATOR_HPP
