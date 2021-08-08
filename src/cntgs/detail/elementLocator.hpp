// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_DETAIL_ELEMENTLOCATOR_HPP
#define CNTGS_DETAIL_ELEMENTLOCATOR_HPP

#include "cntgs/detail/array.hpp"
#include "cntgs/detail/elementTraits.hpp"
#include "cntgs/detail/memory.hpp"
#include "cntgs/detail/parameterListTraits.hpp"
#include "cntgs/detail/typeUtils.hpp"
#include "cntgs/detail/utility.hpp"

#include <algorithm>
#include <array>
#include <type_traits>

namespace cntgs::detail
{
template <class Locator>
auto move_elements_forward(std::size_t from, std::size_t to, std::byte* memory_begin, const Locator& locator) noexcept
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

    std::byte** last_element_address{};
    std::byte* last_element{};

    BaseElementLocator() = default;

    constexpr BaseElementLocator(std::byte** last_element_address, std::byte* last_element) noexcept
        : last_element_address(last_element_address), last_element(last_element)
    {
    }

  public:
    static constexpr auto reserved_bytes(std::size_t element_count) noexcept
    {
        return element_count * RESERVED_BYTES_PER_ELEMENT;
    }

    bool empty(std::byte* memory_begin) const noexcept
    {
        return this->last_element_address == reinterpret_cast<std::byte**>(memory_begin);
    }

    std::size_t size(std::byte* memory_begin) const noexcept
    {
        return this->last_element_address - reinterpret_cast<std::byte**>(memory_begin);
    }

    static auto element_address(std::size_t index, std::byte* memory_begin) noexcept
    {
        const auto element_addresses_begin = reinterpret_cast<std::byte**>(memory_begin);
        return element_addresses_begin[index];
    }

    constexpr auto data_end() const noexcept { return last_element; }

    void resize(std::size_t new_size, std::byte* memory_begin) noexcept
    {
        this->last_element = BaseElementLocator::element_address(new_size, memory_begin);
        this->last_element_address = reinterpret_cast<std::byte**>(memory_begin) + new_size;
    }

    void move_elements_forward(std::size_t from, std::size_t to, std::byte* memory_begin) const noexcept
    {
        const auto diff = detail::move_elements_forward(from, to, memory_begin, *this);
        const auto first_element_address = reinterpret_cast<std::byte**>(memory_begin);
        std::transform(first_element_address + from, this->last_element_address, first_element_address + to,
                       [&](auto address)
                       {
                           return address - diff;
                       });
    }
};

template <bool IsAllFixedSize, class... Types>
class ElementLocator : public BaseElementLocator
{
  private:
    using ElementTraits = detail::ElementTraitsT<Types...>;
    using FixedSizesArray = typename detail::ParameterListTraits<Types...>::FixedSizesArray;

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
        this->trivially_copy_into(old_locator, old_max_element_count, old_memory_begin, new_max_element_count,
                                  new_memory_begin);
    }

    template <class... Args>
    void emplace_back(const FixedSizesArray& fixed_sizes, Args&&... args)
    {
        *this->last_element_address = last_element;
        ++this->last_element_address;
        last_element = ElementTraits::emplace_at(last_element, fixed_sizes, std::forward<Args>(args)...);
    }

    template <class... Args>
    static void emplace_at(std::size_t index, std::byte* memory_begin, const FixedSizesArray& fixed_sizes,
                           Args&&... args)
    {
        const auto element_addresses_begin = reinterpret_cast<std::byte**>(memory_begin);
        element_addresses_begin[index + 1] =
            ElementTraits::emplace_at_aliased(element_addresses_begin[index], fixed_sizes, std::forward<Args>(args)...);
    }

    void trivially_copy_into(std::size_t old_max_element_count, std::byte* old_memory_begin,
                             std::size_t new_max_element_count, std::byte* new_memory_begin) noexcept
    {
        this->trivially_copy_into(*this, old_max_element_count, old_memory_begin, new_max_element_count,
                                  new_memory_begin);
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
                             std::byte* old_memory_begin, std::size_t new_max_element_count,
                             std::byte* new_memory_begin) noexcept
    {
        const auto new_start = ElementLocator::calculate_element_start(new_max_element_count, new_memory_begin);
        const auto old_start = ElementLocator::calculate_element_start(old_max_element_count, old_memory_begin);
        const auto size_diff = std::distance(new_memory_begin, new_start) - std::distance(old_memory_begin, old_start);
        const auto new_last_element_address = ElementLocator::copy_element_addresses(
            new_memory_begin, old_memory_begin, old_locator.last_element_address, size_diff);
        const auto old_used_memory_size = std::distance(old_start, old_locator.last_element);
        std::memcpy(new_start, old_start, old_used_memory_size);
        this->last_element_address = new_last_element_address;
        this->last_element = new_memory_begin + std::distance(old_memory_begin, old_locator.last_element) + size_diff;
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
    std::size_t element_count{};
    std::size_t stride{};
    std::byte* start{};

    BaseAllFixedSizeElementLocator() = default;

    constexpr BaseAllFixedSizeElementLocator(std::size_t element_count, std::size_t stride, std::byte* start) noexcept
        : element_count(element_count), stride(stride), start(start)
    {
    }

  public:
    static constexpr auto reserved_bytes(std::size_t) noexcept { return std::size_t{}; }

    constexpr bool empty(const std::byte*) const noexcept { return this->element_count == std::size_t{}; }

    constexpr std::size_t size(const std::byte*) const noexcept { return this->element_count; }

    constexpr auto element_address(std::size_t index, const std::byte*) const noexcept
    {
        return this->start + this->stride * index;
    }

    constexpr auto data_end() const noexcept { return this->start + this->stride * this->element_count; }

    constexpr void resize(std::size_t new_size, const std::byte*) noexcept { this->element_count = new_size; }

    void move_elements_forward(std::size_t from, std::size_t to, const std::byte*) const noexcept
    {
        detail::move_elements_forward(from, to, {}, *this);
    }
};

template <class... Types>
class ElementLocator<true, Types...> : public BaseAllFixedSizeElementLocator
{
  private:
    using ElementTraits = detail::ElementTraitsT<Types...>;
    using FixedSizesArray = typename detail::ParameterListTraits<Types...>::FixedSizesArray;

  public:
    ElementLocator() = default;

    constexpr ElementLocator(std::size_t, std::byte* memory_begin, const FixedSizesArray& fixed_sizes) noexcept
        : BaseAllFixedSizeElementLocator({}, ElementTraits::calculate_element_size(fixed_sizes),
                                         ElementLocator::calculate_element_start(memory_begin))
    {
    }

    ElementLocator(ElementLocator& old_locator, std::size_t, const std::byte*, std::size_t,
                   std::byte* new_memory_begin) noexcept
        : BaseAllFixedSizeElementLocator(old_locator.element_count, old_locator.stride, {})
    {
        this->trivially_copy_into(old_locator, new_memory_begin);
    }

    template <class... Args>
    void emplace_back(const FixedSizesArray& fixed_sizes, Args&&... args)
    {
        auto last_element = this->element_address(this->element_count, {});
        ++this->element_count;
        ElementTraits::emplace_at(last_element, fixed_sizes, std::forward<Args>(args)...);
    }

    template <class... Args>
    void emplace_at(std::size_t index, const std::byte*, const FixedSizesArray& fixed_sizes, Args&&... args)
    {
        ElementTraits::emplace_at_aliased(this->element_address(index, {}), fixed_sizes, std::forward<Args>(args)...);
    }

    void trivially_copy_into(std::size_t, const std::byte*, std::size_t, std::byte* new_memory_begin) noexcept
    {
        this->trivially_copy_into(*this, new_memory_begin);
    }

    constexpr auto calculate_new_memory_size(std::size_t max_element_count, std::size_t varying_size_bytes,
                                             const FixedSizesArray&) noexcept
    {
        constexpr auto ALIGNMENT_OVERHEAD = ElementTraits::template ParameterTraitsAt<0>::ALIGNMENT - 1;
        return varying_size_bytes + this->stride * max_element_count +
               ElementLocator::reserved_bytes(max_element_count) + ALIGNMENT_OVERHEAD;
    }

  private:
    void trivially_copy_into(const ElementLocator& old_locator, std::byte* new_memory_begin) noexcept
    {
        const auto new_start = ElementLocator::calculate_element_start(new_memory_begin);
        std::memcpy(new_start, old_locator.start, old_locator.element_count * old_locator.stride);
        this->start = new_start;
    }

    static constexpr auto calculate_element_start(std::byte* memory_begin) noexcept
    {
        return detail::align<ElementTraits::template ParameterTraitsAt<0>::ALIGNMENT>(memory_begin);
    }
};

template <class... Types>
using ElementLocatorT = detail::ElementLocator<detail::ParameterListTraits<Types...>::IS_FIXED_SIZE_OR_PLAIN, Types...>;

template <class... Types>
class ElementLocatorAndFixedSizes
    : private detail::EmptyBaseOptimizationT<typename detail::ParameterListTraits<Types...>::FixedSizesArray>
{
  private:
    static constexpr auto HAS_FIXED_SIZES = detail::ParameterListTraits<Types...>::CONTIGUOUS_FIXED_SIZE_COUNT > 0;

    using FixedSizesArray = typename detail::ParameterListTraits<Types...>::FixedSizesArray;
    using Locator = detail::ElementLocatorT<Types...>;
    using Base = detail::EmptyBaseOptimizationT<FixedSizesArray>;

  public:
    Locator locator;

    ElementLocatorAndFixedSizes() = default;

    constexpr ElementLocatorAndFixedSizes(const Locator& locator, const FixedSizesArray& fixed_sizes) noexcept
        : Base{fixed_sizes}, locator(locator)
    {
    }

    constexpr ElementLocatorAndFixedSizes(std::size_t max_element_count, std::byte* memory,
                                          const FixedSizesArray& fixed_sizes) noexcept
        : Base{fixed_sizes}, locator(max_element_count, memory, fixed_sizes)
    {
    }

    constexpr auto operator->() noexcept { return std::addressof(this->locator); }

    constexpr auto operator->() const noexcept { return std::addressof(this->locator); }

    constexpr auto& operator*() noexcept { return this->locator; }

    constexpr const auto& operator*() const noexcept { return this->locator; }

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
