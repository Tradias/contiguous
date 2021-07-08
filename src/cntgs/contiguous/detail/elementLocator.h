#pragma once

#include "cntgs/contiguous/detail/elementTraits.h"
#include "cntgs/contiguous/detail/memory.h"
#include "cntgs/contiguous/detail/parameterListTraits.h"
#include "cntgs/contiguous/detail/typeUtils.h"

#include <algorithm>
#include <array>
#include <type_traits>

namespace cntgs::detail
{
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

    auto element_address(std::size_t index, std::byte* memory_begin) const noexcept
    {
        const auto element_addresses_begin = reinterpret_cast<std::byte**>(memory_begin);
        return element_addresses_begin[index];
    }

    void resize(std::size_t new_size, std::byte* memory_begin) noexcept
    {
        this->last_element = this->element_address(new_size, memory_begin);
        this->last_element_address = reinterpret_cast<std::byte**>(memory_begin) + new_size;
    }
};

template <class... Types>
class ElementLocator : public BaseElementLocator
{
  private:
    using Self = ElementLocator<Types...>;
    using ElementTraits = detail::ElementTraitsT<Types...>;
    using FixedSizes = typename detail::ParameterListTraits<Types...>::FixedSizes;

  public:
    ElementLocator() = default;

    ElementLocator(std::size_t max_element_count, std::byte* memory_begin, const FixedSizes&) noexcept
        : BaseElementLocator(reinterpret_cast<std::byte**>(memory_begin),
                             Self::calculate_element_start(max_element_count, memory_begin))
    {
    }

    ElementLocator(std::size_t max_element_count, std::byte* memory_begin, ElementLocator& other,
                   std::size_t other_max_element_count, std::byte* other_memory_begin) noexcept
    {
        this->copy_from(max_element_count, memory_begin, other, other_max_element_count, other_memory_begin);
    }

    template <class... Args>
    void emplace_back(const FixedSizes& fixed_sizes, Args&&... args)
    {
        *this->last_element_address = last_element;
        ++this->last_element_address;
        last_element = ElementTraits::emplace_at(last_element, fixed_sizes, std::forward<Args>(args)...);
    }

    template <class... Args>
    void emplace_at(std::size_t index, std::byte* memory_begin, const FixedSizes& fixed_sizes, Args&&... args)
    {
        const auto element_addresses_begin = reinterpret_cast<std::byte**>(memory_begin);
        element_addresses_begin[index + 1] =
            ElementTraits::emplace_at_aliased(element_addresses_begin[index], fixed_sizes, std::forward<Args>(args)...);
    }

    void copy_from(std::size_t new_max_element_count, std::byte* new_memory_begin, std::size_t old_max_element_count,
                   std::byte* old_memory_begin) noexcept
    {
        this->copy_from(new_max_element_count, new_memory_begin, *this, old_max_element_count, old_memory_begin);
    }

  private:
    void copy_from(std::size_t new_max_element_count, std::byte* new_memory_begin, ElementLocator& old_locator,
                   std::size_t old_max_element_count, std::byte* old_memory_begin) noexcept
    {
        const auto new_start = Self::calculate_element_start(new_max_element_count, new_memory_begin);
        const auto old_start = Self::calculate_element_start(old_max_element_count, old_memory_begin);
        const auto size_diff = std::distance(new_memory_begin, new_start) - std::distance(old_memory_begin, old_start);
        auto new_last_element_address = reinterpret_cast<std::byte**>(new_memory_begin);
        std::for_each(reinterpret_cast<std::byte**>(old_memory_begin), old_locator.last_element_address,
                      [&](std::byte* element)
                      {
                          *new_last_element_address =
                              new_memory_begin + std::distance(old_memory_begin, element) + size_diff;
                          ++new_last_element_address;
                      });
        const auto old_used_memory_size = std::distance(old_start, old_locator.last_element);
        std::memcpy(new_start, old_start, old_used_memory_size);
        this->last_element_address = new_last_element_address;
        this->last_element = new_memory_begin + std::distance(old_memory_begin, old_locator.last_element) + size_diff;
    }

    static constexpr auto calculate_element_start(std::size_t max_element_count, std::byte* memory_begin) noexcept
    {
        return static_cast<std::byte*>(detail::align<ElementTraits::template ParameterTraitsAt<0>::ALIGNMENT>(
            memory_begin + BaseElementLocator::reserved_bytes(max_element_count)));
    }
};

class BaseAllFixedSizeElementLocator
{
  protected:
    std::size_t element_count{};
    std::size_t stride{};
    std::byte* start{};

    BaseAllFixedSizeElementLocator() = default;

    BaseAllFixedSizeElementLocator(std::size_t element_count, std::size_t stride, std::byte* start)
        : element_count(element_count), stride(stride), start(start)
    {
    }

  public:
    static constexpr auto reserved_bytes(std::size_t) noexcept { return std::size_t{}; }

    constexpr bool empty(std::byte*) const noexcept { return this->element_count == std::size_t{}; }

    constexpr std::size_t size(std::byte*) const noexcept { return this->element_count; }

    constexpr auto element_address(std::size_t index, std::byte*) const noexcept
    {
        return start + this->stride * index;
    }

    constexpr void resize(std::size_t new_size, std::byte*) noexcept { this->element_count = new_size; }
};

template <class... Types>
class AllFixedSizeElementLocator : public BaseAllFixedSizeElementLocator
{
  private:
    using Self = detail::AllFixedSizeElementLocator<Types...>;
    using ElementTraits = detail::ElementTraitsT<Types...>;
    using FixedSizes = typename detail::ParameterListTraits<Types...>::FixedSizes;

  public:
    AllFixedSizeElementLocator() = default;

    AllFixedSizeElementLocator(std::size_t, std::byte* memory_begin, const FixedSizes& fixed_sizes) noexcept
        : BaseAllFixedSizeElementLocator({}, ElementTraits::calculate_element_size(fixed_sizes),
                                         Self::calculate_element_start(memory_begin))
    {
    }

    AllFixedSizeElementLocator(std::size_t, std::byte* memory_begin, AllFixedSizeElementLocator& other, std::size_t,
                               std::byte*) noexcept
        : BaseAllFixedSizeElementLocator(other.element_count, other.stride, {})
    {
        this->copy_from(memory_begin, other);
    }

    template <class... Args>
    void emplace_back(const FixedSizes& fixed_sizes, Args&&... args)
    {
        auto last_element = this->element_address(element_count, {});
        ++this->element_count;
        ElementTraits::emplace_at(last_element, fixed_sizes, std::forward<Args>(args)...);
    }

    template <class... Args>
    void emplace_at(std::size_t index, std::byte*, const FixedSizes& fixed_sizes, Args&&... args)
    {
        ElementTraits::emplace_at_aliased(this->element_address(index, {}), fixed_sizes, std::forward<Args>(args)...);
    }

    void copy_from(std::size_t, std::byte* new_memory_begin, std::size_t, std::byte*) noexcept
    {
        this->copy_from(new_memory_begin, *this);
    }

  private:
    void copy_from(std::byte* new_memory_begin, AllFixedSizeElementLocator& old) noexcept
    {
        const auto new_start = Self::calculate_element_start(new_memory_begin);
        std::memcpy(new_start, old.start, old.element_count * old.stride);
        this->start = new_start;
    }

    static constexpr auto calculate_element_start(std::byte* memory_begin) noexcept
    {
        return static_cast<std::byte*>(
            detail::align<ElementTraits::template ParameterTraitsAt<0>::ALIGNMENT>(memory_begin));
    }
};

template <class... Types>
using ElementLocatorT =
    std::conditional_t<detail::ParameterListTraits<Types...>::CONTIGUOUS_FIXED_SIZE_COUNT ==
                           detail::ParameterListTraits<Types...>::CONTIGUOUS_COUNT,
                       detail::AllFixedSizeElementLocator<Types...>, detail::ElementLocator<Types...>>;

using TypeErasedElementLocator =
    std::aligned_storage_t<std::max(sizeof(detail::ElementLocator<>), sizeof(detail::AllFixedSizeElementLocator<>)),
                           std::max(alignof(detail::ElementLocator<>), alignof(detail::AllFixedSizeElementLocator<>))>;

template <class T>
auto type_erase_element_locator(T&& locator) noexcept
{
    detail::TypeErasedElementLocator result;
    detail::construct_at(reinterpret_cast<detail::RemoveCvrefT<T>*>(&result), std::forward<T>(locator));
    return result;
}
}  // namespace cntgs::detail
