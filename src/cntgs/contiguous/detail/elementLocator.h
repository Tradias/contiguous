#pragma once

#include "cntgs/contiguous/detail/forward.h"
#include "cntgs/contiguous/detail/math.h"
#include "cntgs/contiguous/detail/parameterTraits.h"
#include "cntgs/contiguous/detail/vectorTraits.h"

#include <array>
#include <type_traits>

namespace cntgs::detail
{
template <class... Types>
class ElementLocator
{
  private:
    using Traits = detail::ContiguousVectorTraitsT<Types...>;
    using SizeType = typename Traits::SizeType;

    std::byte** last_element_address{};

  public:
    template <std::size_t N>
    ElementLocator(std::byte* memory_begin, const std::array<std::size_t, N>&) noexcept
        : last_element_address(reinterpret_cast<std::byte**>(memory_begin))
    {
    }

    static constexpr auto reserved_bytes(typename Traits::SizeType element_count) noexcept
    {
        return element_count * sizeof(std::byte*);
    }

    bool empty(std::byte* memory_begin) const noexcept
    {
        return this->last_element_address == reinterpret_cast<std::byte**>(memory_begin);
    }

    SizeType size(std::byte* memory_begin) const noexcept
    {
        return this->last_element_address - reinterpret_cast<std::byte**>(memory_begin);
    }

    void add_element(std::byte* element_address) noexcept
    {
        *this->last_element_address = element_address;
        ++this->last_element_address;
    }

    auto at(std::byte* memory_begin, SizeType index) const noexcept
    {
        const auto start = reinterpret_cast<std::byte**>(memory_begin);
        return start[index];
    }
};

template <class... Types>
class AllFixedSizeElementLocator
{
  private:
    using Traits = detail::ContiguousVectorTraitsT<Types...>;
    using SizeType = typename Traits::SizeType;

    SizeType element_count{};
    SizeType stride{};

  public:
    template <std::size_t N>
    AllFixedSizeElementLocator(std::byte*, const std::array<SizeType, N>& fixed_sizes) noexcept
        : stride(calculate_stride(fixed_sizes, std::make_index_sequence<sizeof...(Types)>{}))
    {
    }

    static constexpr auto reserved_bytes(SizeType) noexcept { return SizeType{}; }

    constexpr bool empty(std::byte*) const noexcept { return this->element_count == SizeType{}; }

    constexpr SizeType size(std::byte*) const noexcept { return this->element_count; }

    constexpr void add_element(std::byte*) noexcept { ++element_count; }

    constexpr auto at(std::byte* memory_begin, SizeType index) const noexcept
    {
        return memory_begin + this->stride * index;
    }

  private:
    template <std::size_t N, std::size_t... I>
    static constexpr auto calculate_stride(const std::array<SizeType, N>& fixed_sizes,
                                           std::index_sequence<I...>) noexcept
    {
        return ((Traits::template FixedSizeGetter<Types>::template get<I>(fixed_sizes) *
                     detail::ParameterTraits<Types>::VALUE_BYTES +
                 detail::ParameterTraits<Types>::SIZE_IN_MEMORY) +
                ...);
    }
};

template <class... Types>
using ElementLocatorT =
    std::conditional_t<detail::ContiguousVectorTraitsT<Types...>::CONTIGUOUS_FIXED_SIZE_COUNT ==
                           detail::ContiguousVectorTraitsT<Types...>::CONTIGUOUS_COUNT,
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