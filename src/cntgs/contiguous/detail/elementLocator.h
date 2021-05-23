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
class BaseElementLocator
{
  protected:
    using Traits = detail::ContiguousVectorTraits<Types...>;
    using SizeType = typename Traits::SizeType;

    template <class T>
    using FixedSizeGetter = typename Traits::template FixedSizeGetter<T>;

    static constexpr auto TYPE_COUNT = sizeof...(Types);

    template <std::size_t N, std::size_t... I>
    static constexpr auto calculate_element_size(const std::array<SizeType, N>& fixed_sizes,
                                                 std::index_sequence<I...>) noexcept
    {
        SizeType result{};
        ((result += detail::ParameterTraits<Types>::MEMORY_OVERHEAD +
                    alignment_offset<detail::ParameterTraits<Types>::ALIGNMENT>(result) +
                    aligned_size_in_memory<Types>(FixedSizeGetter<Types>::template get<I>(fixed_sizes))),
         ...);
        return result;
    }

    template <std::size_t I, class Type, class NeedsAlignmentSelector, std::size_t N, class... T>
    static constexpr auto load_element(std::byte* address, const std::array<SizeType, N>& fixed_sizes) noexcept
    {
        return detail::ParameterTraits<Type>::from_address<NeedsAlignmentSelector::VALUE<I>>(
            address, FixedSizeGetter<Type>::template get<I>(fixed_sizes));
    }

    template <class NeedsAlignmentSelector, std::size_t N, class... T, std::size_t... I>
    static constexpr auto tuple_of_pointers_at_impl(std::tuple<T...>& result, std::byte* address,
                                                    const std::array<SizeType, N>& fixed_sizes,
                                                    std::index_sequence<I...>) noexcept
    {
        ((std::tie(std::get<I>(result), address) =
              load_element<I, Types, NeedsAlignmentSelector>(address, fixed_sizes)),
         ...);
        return result;
    }

    template <class NeedsAlignmentSelector, std::size_t N>
    static constexpr auto tuple_of_pointers_at_impl(std::byte* address,
                                                    const std::array<SizeType, N>& fixed_sizes) noexcept
    {
        typename Traits::PointerReturnType result;
        return tuple_of_pointers_at_impl<NeedsAlignmentSelector>(result, address, fixed_sizes,
                                                                 std::make_index_sequence<TYPE_COUNT>{});
    }

  public:
    template <std::size_t N>
    static constexpr auto calculate_element_size(const std::array<SizeType, N>& fixed_sizes) noexcept
    {
        return calculate_element_size(fixed_sizes, std::make_index_sequence<TYPE_COUNT>{});
    }
};

struct DefaultAlignmentSelector
{
    template <std::size_t>
    static constexpr auto VALUE = true;
};

template <class... Types>
class ElementLocator : public detail::BaseElementLocator<Types...>
{
  private:
    using Base = detail::BaseElementLocator<Types...>;
    using Traits = typename Base::Traits;
    using SizeType = typename Base::SizeType;

    template <class T>
    using FixedSizeGetter = typename Base::template FixedSizeGetter<T>;

    static constexpr auto TYPE_COUNT = Base::TYPE_COUNT;

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

    static constexpr auto reserved_bytes(SizeType element_count) noexcept { return element_count * sizeof(std::byte*); }

    bool empty(std::byte* memory_begin) const noexcept
    {
        return this->last_element_address == reinterpret_cast<std::byte**>(memory_begin);
    }

    SizeType size(std::byte* memory_begin) const noexcept
    {
        return this->last_element_address - reinterpret_cast<std::byte**>(memory_begin);
    }

    template <std::size_t N, std::size_t... I, class... Args>
    auto emplace_back(const std::array<SizeType, N>& fixed_sizes, std::index_sequence<I...>, Args&&... args)
    {
        *this->last_element_address = last_element;
        ++this->last_element_address;
        ((last_element = detail::ParameterTraits<Types>::store_contiguously(
              std::forward<Args>(args), last_element, FixedSizeGetter<Types>::template get<I>(fixed_sizes))),
         ...);
    }

    template <std::size_t N, class... Args>
    auto emplace_back(const std::array<SizeType, N>& fixed_sizes, Args&&... args)
    {
        return emplace_back(fixed_sizes, std::make_index_sequence<TYPE_COUNT>{}, std::forward<Args>(args)...);
    }

    template <std::size_t N>
    auto tuple_of_pointers_at(SizeType i, std::byte* memory_begin,
                              const std::array<SizeType, N>& fixed_sizes) const noexcept
    {
        return this->tuple_of_pointers_at_impl<detail::DefaultAlignmentSelector>(this->at(memory_begin, i),
                                                                                 fixed_sizes);
    }

    auto at(std::byte* memory_begin, SizeType index) const noexcept
    {
        const auto element_address = reinterpret_cast<std::byte**>(memory_begin);
        return element_address[index];
    }
};

struct AllFixedSizeAlignmentSelector
{
    template <std::size_t I>
    static constexpr auto VALUE = I != 0;
};

template <class... Types>
class AllFixedSizeElementLocator : public detail::BaseElementLocator<Types...>
{
  private:
    using Base = detail::BaseElementLocator<Types...>;
    using Traits = typename Base::Traits;
    using SizeType = typename Base::SizeType;

    template <class T>
    using FixedSizeGetter = typename Base::template FixedSizeGetter<T>;

    static constexpr auto TYPE_COUNT = Base::TYPE_COUNT;

    SizeType element_count{};
    SizeType stride{};
    std::byte* start{};

  public:
    AllFixedSizeElementLocator() = default;

    template <std::size_t N>
    AllFixedSizeElementLocator(SizeType, std::byte* memory_begin, const std::array<SizeType, N>& fixed_sizes) noexcept
        : stride(Base::calculate_element_size(fixed_sizes)),
          start(reinterpret_cast<std::byte*>(detail::align<Traits::MAX_ALIGNMENT>(memory_begin)))
    {
    }

    static constexpr auto reserved_bytes(SizeType) noexcept { return SizeType{}; }

    constexpr bool empty(std::byte*) const noexcept { return this->element_count == SizeType{}; }

    constexpr SizeType size(std::byte*) const noexcept { return this->element_count; }

    template <std::size_t N, std::size_t... I, class... Args>
    auto emplace_back(const std::array<SizeType, N>& fixed_sizes, std::index_sequence<I...>, Args&&... args)
    {
        auto last_element = this->at({}, element_count);
        ((last_element = detail::ParameterTraits<Types>::store_contiguously(
              std::forward<Args>(args), last_element, FixedSizeGetter<Types>::template get<I>(fixed_sizes))),
         ...);
        ++this->element_count;
    }

    template <std::size_t N, class... Args>
    auto emplace_back(const std::array<SizeType, N>& fixed_sizes, Args&&... args)
    {
        return emplace_back(fixed_sizes, std::make_index_sequence<TYPE_COUNT>{}, std::forward<Args>(args)...);
    }

    template <std::size_t N>
    constexpr auto tuple_of_pointers_at(SizeType i, std::byte*,
                                        const std::array<SizeType, N>& fixed_sizes) const noexcept
    {
        return this->tuple_of_pointers_at_impl<detail::AllFixedSizeAlignmentSelector>(this->at(i), fixed_sizes);
    }

    constexpr auto at(std::byte*, SizeType index) const noexcept { return start + this->stride * index; }

    constexpr auto at(SizeType index) const noexcept { return start + this->stride * index; }
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