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
#include <cstring>
#include <vector>

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

inline std::byte* get_mixed_element_address(std::size_t index, std::byte* memory_begin,
                                            const std::size_t element_addresses[]) noexcept
{
    return memory_begin + element_addresses[index];
}

class IteratorMixedElementLocator;

template <class Allocator>
class BaseElementLocator
{
  protected:
    friend detail::IteratorMixedElementLocator;

    using ElementAddresses =
        std::vector<std::size_t, typename std::allocator_traits<Allocator>::template rebind_alloc<std::size_t>>;

    ElementAddresses element_addresses_;
    std::byte* last_element_{};

    BaseElementLocator() = default;

    explicit BaseElementLocator(ElementAddresses element_addresses, std::size_t new_max_element_count)
        : element_addresses_(std::move(element_addresses))
    {
        element_addresses_.reserve(new_max_element_count);
    }

    explicit BaseElementLocator(std::byte* last_element, std::size_t max_element_count,
                                const Allocator& allocator) noexcept
        : element_addresses_(allocator), last_element_(last_element)
    {
        element_addresses_.reserve(max_element_count);
    }

    friend void swap(BaseElementLocator& lhs, BaseElementLocator& rhs) noexcept
    {
        std::swap(lhs.element_addresses_, rhs.element_addresses_);
        std::swap(lhs.last_element_, rhs.last_element_);
    }

  public:
    static constexpr auto reserved_bytes(std::size_t) noexcept { return std::size_t{}; }

    bool empty(const std::byte*) const noexcept { return element_addresses_.empty(); }

    std::size_t memory_size() const noexcept
    {
        return element_addresses_.size() * sizeof(typename ElementAddresses::value_type);
    }

    std::size_t size(const std::byte*) const noexcept { return element_addresses_.size(); }

    std::byte* element_address(std::size_t index, std::byte* memory_begin) const noexcept
    {
        return detail::get_mixed_element_address(index, memory_begin, element_addresses_.data());
    }

    constexpr auto data_end() const noexcept { return last_element_; }

    void resize(std::size_t new_size, std::byte* memory_begin) noexcept
    {
        last_element_ = element_address(new_size, memory_begin);
        element_addresses_.resize(new_size);
    }

    void move_elements_forward(std::size_t from, std::size_t to, std::byte* memory_begin) noexcept
    {
        const auto diff = detail::move_elements(from, to, memory_begin, *this);
        std::transform(element_addresses_.begin() + from, element_addresses_.end(), element_addresses_.begin() + to,
                       [&](auto address)
                       {
                           return address - diff;
                       });
    }

    void make_room_for_last_element_at(std::size_t index, std::size_t size_of_element, std::byte* memory_begin) noexcept
    {
        const auto source = element_address(index, memory_begin);
        const auto target = source + size_of_element;
        const auto count = static_cast<std::size_t>(data_end() - source);
        std::memmove(target, source, count);
        element_addresses_.emplace_back(data_end() - memory_begin);
        const auto begin = element_addresses_.begin() + index + 1;
        std::transform(begin, element_addresses_.end(), begin,
                       [&](auto address)
                       {
                           return address + size_of_element;
                       });
    }
};

template <class Allocator, class... Parameter>
class ElementLocator : public BaseElementLocator<Allocator>
{
  private:
    using Base = BaseElementLocator<Allocator>;
    using ElementTraits = detail::ElementTraitsT<Parameter...>;
    using FixedSizesArray = typename detail::ParameterListTraits<Parameter...>::FixedSizesArray;

  public:
    ElementLocator() = default;

    ElementLocator(std::size_t max_element_count, std::byte* memory_begin, const FixedSizesArray&,
                   const Allocator& allocator) noexcept
        : Base{memory_begin, max_element_count, allocator}
    {
    }

    template <class Locator>
    ElementLocator(Locator&& old_locator, std::byte* old_memory_begin, std::size_t new_max_element_count,
                   std::byte* new_memory_begin)
        : Base{std::forward<Locator>(old_locator).element_addresses_, new_max_element_count}
    {
        trivially_copy_into(old_locator.last_element_, old_memory_begin, new_memory_begin);
    }

    template <class... Args>
    auto emplace_back(std::byte* memory_begin, const FixedSizesArray& fixed_sizes, Args&&... args)
    {
        const auto last_element = ElementTraits::align_for_first_parameter(this->last_element_);
        const auto new_last_element = ElementTraits::emplace_at(last_element, fixed_sizes, std::forward<Args>(args)...);
        this->element_addresses_.emplace_back(last_element - memory_begin);
        this->last_element_ = new_last_element;
        return new_last_element;
    }

    template <class... Args>
    auto emplace_at(std::size_t index, std::byte* memory_begin, const FixedSizesArray& fixed_sizes, Args&&... args)
    {
        const auto element_addresses_begin = ElementTraits::emplace_at_aliased(
            memory_begin + this->element_addresses_[index], fixed_sizes, std::forward<Args>(args)...);
        this->element_addresses_[index + 1] = element_addresses_begin - memory_begin;
        return element_addresses_begin;
    }

    template <class... Args>
    static auto emplace_at(std::byte* address, const std::byte*, const FixedSizesArray& fixed_sizes, Args&&... args)
    {
        ElementTraits::emplace_at_aliased(address, fixed_sizes, std::forward<Args>(args)...);
    }

    void trivially_copy_into(std::byte* CNTGS_RESTRICT old_memory_begin,
                             std::byte* CNTGS_RESTRICT new_memory_begin) noexcept
    {
        trivially_copy_into(this->last_element_, old_memory_begin, new_memory_begin);
    }

    static constexpr auto calculate_new_memory_size(std::size_t max_element_count, std::size_t varying_size_bytes,
                                                    const FixedSizesArray& fixed_sizes) noexcept
    {
        return varying_size_bytes + ElementTraits::calculate_element_size(fixed_sizes) * max_element_count;
    }

  private:
    void trivially_copy_into(std::byte* old_last_element, std::byte* CNTGS_RESTRICT old_memory_begin,
                             std::byte* CNTGS_RESTRICT new_memory_begin) noexcept
    {
        const auto new_start = new_memory_begin;
        const auto old_start = old_memory_begin;
        const auto size_diff = std::distance(new_memory_begin, new_start) - std::distance(old_memory_begin, old_start);
        const auto old_used_memory_size = std::distance(old_start, old_last_element);
        std::memcpy(new_start, old_start, old_used_memory_size);
        this->last_element_ = new_memory_begin + std::distance(old_memory_begin, old_last_element) + size_diff;
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
    constexpr bool empty(const std::byte*) const noexcept { return element_count_ == std::size_t{}; }

    static constexpr std::size_t memory_size() noexcept { return {}; }

    constexpr std::size_t size(const std::byte*) const noexcept { return element_count_; }

    constexpr std::byte* element_address(std::size_t index, const std::byte*) const noexcept
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
class AllFixedSizeElementLocator : public BaseAllFixedSizeElementLocator
{
  private:
    using ElementTraits = detail::ElementTraitsT<Parameter...>;
    using FixedSizesArray = typename detail::ParameterListTraits<Parameter...>::FixedSizesArray;

  public:
    AllFixedSizeElementLocator() = default;

    template <class Allocator>
    constexpr AllFixedSizeElementLocator(std::size_t, std::byte* memory_begin, const FixedSizesArray& fixed_sizes,
                                         const Allocator&) noexcept
        : BaseAllFixedSizeElementLocator({}, ElementTraits::calculate_element_size(fixed_sizes),
                                         AllFixedSizeElementLocator::calculate_element_start(memory_begin))
    {
    }

    AllFixedSizeElementLocator(const AllFixedSizeElementLocator& old_locator, const std::byte*, std::size_t,
                               std::byte* new_memory_begin) noexcept
        : BaseAllFixedSizeElementLocator(old_locator.element_count_, old_locator.stride_, {})
    {
        trivially_copy_into(old_locator, new_memory_begin);
    }

    template <class... Args>
    auto emplace_back(const std::byte*, const FixedSizesArray& fixed_sizes, Args&&... args)
    {
        const auto last_element = element_address(element_count_, {});
        const auto end = ElementTraits::emplace_at(last_element, fixed_sizes, std::forward<Args>(args)...);
        ++element_count_;
        return end;
    }

    template <class... Args>
    auto emplace_at(std::size_t index, const std::byte*, const FixedSizesArray& fixed_sizes, Args&&... args)
    {
        return ElementTraits::emplace_at_aliased(element_address(index, {}), fixed_sizes, std::forward<Args>(args)...);
    }

    void trivially_copy_into(const std::byte*, std::byte* new_memory_begin) noexcept
    {
        trivially_copy_into(*this, new_memory_begin);
    }

    constexpr auto calculate_new_memory_size(std::size_t max_element_count, std::size_t varying_size_bytes,
                                             const FixedSizesArray&) noexcept
    {
        return varying_size_bytes + stride_ * max_element_count;
    }

  private:
    void trivially_copy_into(const AllFixedSizeElementLocator& old_locator, std::byte* new_memory_begin) noexcept
    {
        const auto new_start = AllFixedSizeElementLocator::calculate_element_start(new_memory_begin);
        std::memcpy(new_start, old_locator.start_, old_locator.element_count_ * old_locator.stride_);
        start_ = new_start;
    }

    static constexpr auto calculate_element_start(std::byte* memory_begin) noexcept
    {
        return detail::align<ElementTraits::template ParameterTraitsAt<0>::ALIGNMENT>(memory_begin);
    }
};

template <class Allocator, class... Parameter>
using ElementLocatorT =
    detail::ConditionalT<detail::ParameterListTraits<Parameter...>::IS_FIXED_SIZE_OR_PLAIN,
                         AllFixedSizeElementLocator<Parameter...>, ElementLocator<Allocator, Parameter...>>;

template <class Allocator, class... Parameter>
class ElementLocatorAndFixedSizes
    : private detail::EmptyBaseOptimization<typename detail::ParameterListTraits<Parameter...>::FixedSizesArray>
{
  private:
    using FixedSizesArray = typename detail::ParameterListTraits<Parameter...>::FixedSizesArray;
    using Locator = detail::ElementLocatorT<Allocator, Parameter...>;
    using Base = detail::EmptyBaseOptimization<FixedSizesArray>;

  public:
    Locator locator_;

    ElementLocatorAndFixedSizes() = default;

    constexpr ElementLocatorAndFixedSizes(const Locator& locator, const FixedSizesArray& fixed_sizes) noexcept
        : Base{fixed_sizes}, locator_(locator)
    {
    }

    constexpr ElementLocatorAndFixedSizes(std::size_t max_element_count, std::byte* memory,
                                          const FixedSizesArray& fixed_sizes, const Allocator& allocator) noexcept
        : Base{fixed_sizes}, locator_(max_element_count, memory, fixed_sizes, allocator)
    {
    }

    constexpr auto operator->() noexcept { return std::addressof(locator_); }

    constexpr auto operator->() const noexcept { return std::addressof(locator_); }

    constexpr auto& operator*() noexcept { return locator_; }

    constexpr const auto& operator*() const noexcept { return locator_; }

    constexpr auto& fixed_sizes() noexcept { return Base::get(); }

    constexpr const auto& fixed_sizes() const noexcept { return Base::get(); }
};

class IteratorMixedElementLocator
{
  private:
    std::size_t* element_addresses_;

  public:
    template <class Allocator>
    explicit IteratorMixedElementLocator(BaseElementLocator<Allocator>& locator)
        : element_addresses_(locator.element_addresses_.data())
    {
    }

    auto element_address(std::size_t index, std::byte* memory_begin) const noexcept
    {
        return detail::get_mixed_element_address(index, memory_begin, element_addresses_);
    }
};

using IteratorAllFixedSizeElementLocator = BaseAllFixedSizeElementLocator;

template <bool IsAllFixedSize, class FixedSizesArray>
class IteratorElementLocatorAndFixedSizes : private detail::EmptyBaseOptimization<FixedSizesArray>
{
  private:
    using Locator =
        detail::ConditionalT<IsAllFixedSize, IteratorAllFixedSizeElementLocator, IteratorMixedElementLocator>;
    using Base = detail::EmptyBaseOptimization<FixedSizesArray>;

  public:
    Locator locator_;

    IteratorElementLocatorAndFixedSizes() = default;

    template <class... Parameter>
    constexpr IteratorElementLocatorAndFixedSizes(ElementLocatorAndFixedSizes<Parameter...>& locator) noexcept
        : Base{locator.fixed_sizes()}, locator_(locator.locator_)
    {
    }

    constexpr auto operator->() noexcept { return std::addressof(locator_); }

    constexpr auto operator->() const noexcept { return std::addressof(locator_); }

    constexpr auto& operator*() noexcept { return locator_; }

    constexpr const auto& operator*() const noexcept { return locator_; }

    constexpr auto& fixed_sizes() noexcept { return Base::get(); }

    constexpr const auto& fixed_sizes() const noexcept { return Base::get(); }
};
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_ELEMENTLOCATOR_HPP
