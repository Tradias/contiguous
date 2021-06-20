#pragma once

#include "cntgs/contiguous/detail/attributes.h"
#include "cntgs/contiguous/detail/forward.h"
#include "cntgs/contiguous/detail/math.h"
#include "cntgs/contiguous/detail/memory.h"
#include "cntgs/contiguous/detail/parameterListTraits.h"
#include "cntgs/contiguous/detail/parameterTraits.h"
#include "cntgs/contiguous/detail/tupleQualifier.h"
#include "cntgs/contiguous/detail/typeUtils.h"
#include "cntgs/contiguous/detail/vectorTraits.h"
#include "cntgs/contiguous/tuple.h"

#include <algorithm>
#include <array>
#include <type_traits>

namespace cntgs::detail
{
template <std::size_t Alignment>
constexpr auto alignment_offset([[maybe_unused]] std::size_t position) noexcept
{
    if constexpr (Alignment == 0)
    {
        return std::size_t{};
    }
    else
    {
        return detail::align(Alignment, position) - position;
    }
}

template <class Locator>
auto calculate_element_start(std::size_t max_element_count, std::byte* memory_begin) noexcept
{
    return static_cast<std::byte*>(detail::align<Locator::template ParameterTraitsAt<0>::ALIGNMENT>(
        memory_begin + Locator::reserved_bytes(max_element_count)));
}

template <bool UseMove, class Type, class Source, class Target>
constexpr void construct_one_if_non_trivial([[maybe_unused]] Source& source, [[maybe_unused]] const Target& target)
{
    using ValueType = typename detail::ParameterTraits<Type>::ValueType;
    if constexpr (UseMove && !std::is_trivially_move_constructible_v<ValueType>)
    {
        detail::ParameterTraits<Type>::uninitialized_move(source, target);
    }
    else if constexpr (!UseMove && !std::is_trivially_copy_constructible_v<ValueType>)
    {
        detail::ParameterTraits<Type>::uninitialized_copy(source, target);
    }
}

template <class, class...>
struct BaseElementLocator;

template <std::size_t... I, class... Types>
struct BaseElementLocator<std::index_sequence<I...>, Types...>
{
    using ListTraits = detail::ParameterListTraits<Types...>;
    using FixedSizes = typename ListTraits::FixedSizes;
    using PointerReturnType = typename detail::ContiguousVectorTraits<Types...>::PointerReturnType;

    template <class T>
    using FixedSizeGetter = typename ListTraits::template FixedSizeGetter<T>;

    template <std::size_t K>
    using ParameterTraitsAt = typename ListTraits::template ParameterTraitsAt<K>;

    template <class NeedsAlignmentSelector, class... Args>
    CNTGS_RESTRICT_RETURN static std::byte* emplace_back(std::byte* CNTGS_RESTRICT last_element,
                                                         const FixedSizes& fixed_sizes, Args&&... args)
    {
        ((last_element = detail::ParameterTraits<Types>::template store<NeedsAlignmentSelector::template VALUE<I>>(
              std::forward<Args>(args), last_element, FixedSizeGetter<Types>::template get<I>(fixed_sizes))),
         ...);
        return last_element;
    }

    template <class NeedsAlignmentSelector>
    static auto load_element_at(std::byte* address, const FixedSizes& fixed_sizes) noexcept
    {
        PointerReturnType result;
        ((std::tie(std::get<I>(result), address) =
              detail::ParameterTraits<Types>::template load<NeedsAlignmentSelector::template VALUE<I>>(
                  address, FixedSizeGetter<Types>::template get<I>(fixed_sizes))),
         ...);
        return result;
    }

    static constexpr auto calculate_element_size(const FixedSizes& fixed_sizes) noexcept
    {
        std::size_t result{};
        ((result +=
          detail::ParameterTraits<Types>::aligned_size_in_memory(FixedSizeGetter<Types>::template get<I>(fixed_sizes)) +
          alignment_offset<detail::ParameterTraits<Types>::ALIGNMENT>(result)),
         ...);
        return result + alignment_offset<ParameterTraitsAt<0>::ALIGNMENT>(result);
    }

    template <bool UseMove, detail::ContiguousTupleQualifier Qualifier>
    static constexpr void construct_if_non_trivial(const cntgs::ContiguousTuple<Qualifier, Types...>& source,
                                                   const PointerReturnType& target)
    {
        (detail::construct_one_if_non_trivial<UseMove, Types>(cntgs::get<I>(source), std::get<I>(target)), ...);
    }
};

template <class... Types>
using BaseElementLocatorT = detail::BaseElementLocator<std::make_index_sequence<sizeof...(Types)>, Types...>;

struct DefaultAlignmentSelector
{
    template <std::size_t>
    static constexpr auto VALUE = true;
};

template <class... Types>
class ElementLocator : public detail::BaseElementLocatorT<Types...>
{
  private:
    using Base = detail::BaseElementLocatorT<Types...>;
    using FixedSizes = typename Base::FixedSizes;

    static constexpr auto RESERVED_BYTES_PER_ELEMENT = sizeof(std::byte*);

    std::byte** last_element_address{};
    std::byte* last_element{};

  public:
    ElementLocator() = default;

    ElementLocator(std::size_t max_element_count, std::byte* memory_begin, const FixedSizes&) noexcept
        : last_element_address(reinterpret_cast<std::byte**>(memory_begin)),
          last_element(detail::calculate_element_start<ElementLocator>(max_element_count, memory_begin))
    {
    }

    ElementLocator(std::size_t max_element_count, std::byte* memory_begin, ElementLocator& other,
                   std::size_t other_max_element_count, std::byte* other_memory_begin) noexcept
    {
        this->copy_from(max_element_count, memory_begin, other, other_max_element_count, other_memory_begin);
    }

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

    template <class... Args>
    void emplace_back(const FixedSizes& fixed_sizes, Args&&... args)
    {
        *this->last_element_address = last_element;
        ++this->last_element_address;
        last_element = Base::template emplace_back<detail::DefaultAlignmentSelector>(last_element, fixed_sizes,
                                                                                     std::forward<Args>(args)...);
    }

    auto load_element_at(std::size_t i, std::byte* memory_begin, const FixedSizes& fixed_sizes) const noexcept
    {
        return Base::template load_element_at<detail::DefaultAlignmentSelector>(this->at(memory_begin, i), fixed_sizes);
    }

    auto at(std::byte* memory_begin, std::size_t index) const noexcept
    {
        const auto element_addresses_begin = reinterpret_cast<std::byte**>(memory_begin);
        return element_addresses_begin[index];
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
        const auto new_start = detail::calculate_element_start<ElementLocator>(new_max_element_count, new_memory_begin);
        const auto old_start = detail::calculate_element_start<ElementLocator>(old_max_element_count, old_memory_begin);
        const auto size_diff = std::distance(new_memory_begin, new_start) - std::distance(old_memory_begin, old_start);
        auto new_last_element_address = reinterpret_cast<std::byte**>(new_memory_begin);
        std::for_each(
            reinterpret_cast<std::byte**>(old_memory_begin), old_locator.last_element_address, [&](std::byte* element) {
                *new_last_element_address = new_memory_begin + std::distance(old_memory_begin, element) + size_diff;
                ++new_last_element_address;
            });
        const auto old_used_memory_size = std::distance(old_start, old_locator.last_element);
        std::memcpy(new_start, old_start, old_used_memory_size);
        this->last_element_address = new_last_element_address;
        this->last_element = new_memory_begin + std::distance(old_memory_begin, old_locator.last_element) + size_diff;
    }
};

struct IgnoreFirstAlignmentSelector
{
    template <std::size_t I>
    static constexpr auto VALUE = I != 0;
};

template <class... Types>
class AllFixedSizeElementLocator : public detail::BaseElementLocatorT<Types...>
{
  private:
    using Base = detail::BaseElementLocatorT<Types...>;
    using FixedSizes = typename Base::FixedSizes;

    std::size_t element_count{};
    std::size_t stride{};
    std::byte* start{};

  public:
    AllFixedSizeElementLocator() = default;

    AllFixedSizeElementLocator(std::size_t, std::byte* memory_begin, const FixedSizes& fixed_sizes) noexcept
        : stride(Base::calculate_element_size(fixed_sizes)),
          start(detail::calculate_element_start<AllFixedSizeElementLocator>({}, memory_begin))
    {
    }

    AllFixedSizeElementLocator(std::size_t, std::byte* memory_begin, AllFixedSizeElementLocator& other, std::size_t,
                               std::byte*) noexcept
        : element_count(other.element_count), stride(other.stride)
    {
        this->copy_from(memory_begin, other);
    }

    static constexpr auto reserved_bytes(std::size_t) noexcept { return std::size_t{}; }

    constexpr bool empty(std::byte*) const noexcept { return this->element_count == std::size_t{}; }

    constexpr std::size_t size(std::byte*) const noexcept { return this->element_count; }

    template <class... Args>
    void emplace_back(const FixedSizes& fixed_sizes, Args&&... args)
    {
        auto last_element = this->at(element_count);
        ++this->element_count;
        Base::template emplace_back<detail::IgnoreFirstAlignmentSelector>(last_element, fixed_sizes,
                                                                          std::forward<Args>(args)...);
    }

    auto load_element_at(std::size_t i, std::byte*, const FixedSizes& fixed_sizes) const noexcept
    {
        return Base::template load_element_at<detail::IgnoreFirstAlignmentSelector>(this->at(i), fixed_sizes);
    }

    [[nodiscard]] constexpr auto at(std::size_t index) const noexcept { return start + this->stride * index; }

    void copy_from(std::size_t, std::byte* new_memory_begin, std::size_t, std::byte*) noexcept
    {
        this->copy_from(new_memory_begin, *this);
    }

  private:
    void copy_from(std::byte* new_memory_begin, AllFixedSizeElementLocator& old) noexcept
    {
        const auto new_start = detail::calculate_element_start<AllFixedSizeElementLocator>({}, new_memory_begin);
        std::memcpy(new_start, old.start, old.element_count * old.stride);
        this->start = new_start;
    }
};

template <class... Types>
using ElementLocatorT =
    std::conditional_t<detail::ParameterListTraits<Types...>::CONTIGUOUS_FIXED_SIZE_COUNT ==
                           detail::ParameterListTraits<Types...>::CONTIGUOUS_COUNT,
                       detail::AllFixedSizeElementLocator<Types...>, detail::ElementLocator<Types...>>;

using TypeErasedElementLocator = std::aligned_storage_t<
    detail::MAX_SIZE_T_OF<sizeof(detail::ElementLocator<>), sizeof(detail::AllFixedSizeElementLocator<>)>,
    detail::MAX_SIZE_T_OF<alignof(detail::ElementLocator<>), alignof(detail::AllFixedSizeElementLocator<>)>>;

template <class T>
auto type_erase_element_locator(T&& locator) noexcept
{
    TypeErasedElementLocator result;
    detail::construct_at(reinterpret_cast<detail::RemoveCvrefT<T>*>(&result), std::forward<T>(locator));
    return result;
}
}  // namespace cntgs::detail
