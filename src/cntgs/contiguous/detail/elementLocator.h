#pragma once

#include "cntgs/contiguous/detail/forward.h"
#include "cntgs/contiguous/detail/math.h"
#include "cntgs/contiguous/detail/parameterTraits.h"
#include "cntgs/contiguous/detail/vectorTraits.h"

#include <algorithm>
#include <array>
#include <type_traits>

namespace cntgs::detail
{
template <class T>
static constexpr auto aligned_size_in_memory(std::size_t fixed_size) noexcept
{
    using Trait = detail::ParameterTraits<T>;
    if constexpr (Trait::ALIGNMENT == 0)
    {
        return fixed_size * Trait::VALUE_BYTES + Trait::SIZE_IN_MEMORY;
    }
    else
    {
        return std::max(fixed_size * Trait::VALUE_BYTES + Trait::SIZE_IN_MEMORY, Trait::ALIGNMENT);
    }
}

template <std::size_t Alignment>
static constexpr auto alignment_offset([[maybe_unused]] std::size_t position) noexcept
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

template <class... Types>
class ElementLocator
{
  private:
    using Traits = detail::ContiguousVectorTraits<Types...>;
    using SizeType = typename Traits::SizeType;

    std::byte** last_element_address{};
    std::byte* start{};
    std::byte* last_element{};

  public:
    ElementLocator() = default;

    template <std::size_t N>
    ElementLocator(SizeType max_element_count, std::byte* memory_begin, const std::array<SizeType, N>&) noexcept
        : last_element_address(reinterpret_cast<std::byte**>(memory_begin)),
          start(reinterpret_cast<std::byte*>(
              detail::align<Traits::MAX_ALIGNMENT>(memory_begin + ElementLocator::reserved_bytes(max_element_count)))),
          last_element(start)
    {
    }

    template <std::size_t N, std::size_t... I>
    static constexpr auto calculate_stride(const std::array<SizeType, N>& fixed_sizes,
                                           std::index_sequence<I...>) noexcept
    {
        SizeType result{};
        ((result +=
          detail::ParameterTraits<Types>::MEMORY_OVERHEAD +
          alignment_offset<detail::ParameterTraits<Types>::ALIGNMENT>(result) +
          aligned_size_in_memory<Types>(Traits::template FixedSizeGetter<Types>::template get<I>(fixed_sizes))),
         ...);
        return result;
    }

    static constexpr auto reserved_bytes(SizeType element_count) noexcept { return element_count * sizeof(std::byte*); }

    bool empty(std::byte* memory_begin) const noexcept
    {
        return this->last_element_address == reinterpret_cast<std::byte**>(memory_begin);
    }

    SizeType size(std::byte* memory_begin) const noexcept
    {
        return this->last_element_address - reinterpret_cast<std::byte**>(memory_begin);
    }

    constexpr auto next_free_address() noexcept
    {
        auto previous_last_element = last_element;
        *this->last_element_address = last_element;
        ++this->last_element_address;
        return previous_last_element;
    }

    constexpr void append(std::byte* address) noexcept { last_element = address; }

    auto at(std::byte* memory_begin, SizeType index) const noexcept
    {
        const auto element_address = reinterpret_cast<std::byte**>(memory_begin);
        return element_address[index];
    }
};

template <class... Types>
class AllFixedSizeElementLocator
{
  private:
    using Traits = detail::ContiguousVectorTraits<Types...>;
    using SizeType = typename Traits::SizeType;

    SizeType element_count{};
    SizeType stride{};
    std::byte* start{};

  public:
    AllFixedSizeElementLocator() = default;

    template <std::size_t N>
    AllFixedSizeElementLocator(SizeType, std::byte* memory_begin, const std::array<SizeType, N>& fixed_sizes) noexcept
        : stride(calculate_stride(fixed_sizes, std::make_index_sequence<sizeof...(Types)>{})),
          start(reinterpret_cast<std::byte*>(detail::align<Traits::MAX_ALIGNMENT>(memory_begin)))
    {
    }

    template <std::size_t N, std::size_t... I>
    static constexpr auto calculate_stride(const std::array<SizeType, N>& fixed_sizes,
                                           std::index_sequence<I...>) noexcept
    {
        SizeType result{};
        ((result +=
          alignment_offset<detail::ParameterTraits<Types>::ALIGNMENT>(result) +
          aligned_size_in_memory<Types>(Traits::template FixedSizeGetter<Types>::template get<I>(fixed_sizes))),
         ...);
        return result;
    }

    static constexpr auto reserved_bytes(SizeType) noexcept { return SizeType{}; }

    constexpr bool empty(std::byte*) const noexcept { return this->element_count == SizeType{}; }

    constexpr SizeType size(std::byte*) const noexcept { return this->element_count; }

    constexpr auto next_free_address() noexcept { return this->at({}, element_count); }

    constexpr void append(std::byte*) noexcept { ++element_count; }

    constexpr auto at(std::byte*, SizeType index) const noexcept { return start + this->stride * index; }
};

template <class... Types>
using ElementLocatorT =
    std::conditional_t<detail::ContiguousVectorTraits<Types...>::CONTIGUOUS_FIXED_SIZE_COUNT ==
                           detail::ContiguousVectorTraits<Types...>::CONTIGUOUS_COUNT,
                       detail::AllFixedSizeElementLocator<Types...>, detail::ElementLocator<Types...>>;

static constexpr auto MAX_ELEMENT_LOCATOR_SIZE =
    detail::max_size_t_of<sizeof(detail::ElementLocator<>), sizeof(detail::AllFixedSizeElementLocator<>)>();

template <class T>
auto type_erase_element_locator(T&& locator)
{
    std::array<std::byte, detail::MAX_ELEMENT_LOCATOR_SIZE> result;
    new (result.data()) T(std::forward<T>(locator));
    return result;
}
}  // namespace cntgs::detail