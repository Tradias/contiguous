// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_DETAIL_ELEMENTTRAITS_HPP
#define CNTGS_DETAIL_ELEMENTTRAITS_HPP

#include "cntgs/detail/algorithm.hpp"
#include "cntgs/detail/attributes.hpp"
#include "cntgs/detail/forward.hpp"
#include "cntgs/detail/memory.hpp"
#include "cntgs/detail/parameterListTraits.hpp"
#include "cntgs/detail/parameterTraits.hpp"
#include "cntgs/detail/reference.hpp"
#include "cntgs/detail/sizeGetter.hpp"
#include "cntgs/detail/typeTraits.hpp"
#include "cntgs/detail/vectorTraits.hpp"

#include <array>
#include <cstddef>
#include <limits>
#include <type_traits>

namespace cntgs::detail
{
template <bool UseMove, class Type, class Source, class Target>
constexpr void construct_one_if_non_trivial([[maybe_unused]] Source&& source, [[maybe_unused]] const Target& target)
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

struct ElementSize
{
    std::size_t size;
    std::size_t stride;
};

template <class, class...>
class ElementTraits;

template <std::size_t... I, class... Parameter>
class ElementTraits<std::index_sequence<I...>, Parameter...>
{
  private:
    using ListTraits = detail::ParameterListTraits<Parameter...>;

  public:
    template <std::size_t K>
    using ParameterTraitsAt = typename ListTraits::template ParameterTraitsAt<K>;

  private:
    using FixedSizesArray = typename ListTraits::FixedSizesArray;
    using ContiguousPointer = typename detail::ContiguousVectorTraits<Parameter...>::PointerType;
    using ContiguousReference = typename detail::ContiguousVectorTraits<Parameter...>::ReferenceType;
    using FixedSizeGetter = detail::FixedSizeGetter<Parameter...>;

    static constexpr std::size_t LARGEST_LEADING_ALIGNMENT_UNTIL_VARYING_SIZE =
        ListTraits::LARGEST_LEADING_ALIGNMENT_UNTIL_VARYING_SIZE;
    static constexpr std::size_t SKIP = std::numeric_limits<std::size_t>::max();
    static constexpr std::size_t MANUAL = SKIP - 1;

    template <template <class> class Predicate>
    static constexpr auto calculate_consecutive_indices() noexcept
    {
        std::array<std::size_t, sizeof...(Parameter)> consecutive_indices{((void)I, SKIP)...};
        [[maybe_unused]] std::size_t index{};
        (
            [&]
            {
                if constexpr (Predicate<typename detail::ParameterTraits<Parameter>::ValueType>::value)
                {
                    consecutive_indices[index] = I;
                }
                else
                {
                    index = I + 1;
                    consecutive_indices[I] = MANUAL;
                }
            }(),
            ...);
        return consecutive_indices;
    }

    static constexpr std::array CONSECUTIVE_TRIVIALLY_ASSIGNABLE_INDICES{
        calculate_consecutive_indices<std::is_trivially_copy_assignable>(),
        calculate_consecutive_indices<std::is_trivially_move_assignable>()};

    static constexpr auto CONSECUTIVE_TRIVIALLY_SWAPPABLE_INDICES{
        calculate_consecutive_indices<detail::IsTriviallySwappable>()};

    static constexpr auto CONSECUTIVE_EQUALITY_MEMCMPABLE_INDICES{
        calculate_consecutive_indices<detail::EqualityMemcmpCompatible>()};

    static constexpr auto CONSECUTIVE_LEXICOGRAPHICAL_MEMCMPABLE_INDICES{
        calculate_consecutive_indices<detail::LexicographicalMemcmpCompatible>()};

    template <std::size_t K, std::size_t L, bool IsLhsConst, bool IsRhsConst>
    static constexpr auto get_data_begin_and_end(
        const cntgs::BasicContiguousReference<IsLhsConst, Parameter...>& lhs,
        const cntgs::BasicContiguousReference<IsRhsConst, Parameter...>& rhs) noexcept
    {
        return std::tuple{ParameterTraitsAt<K>::data_begin(cntgs::get<K>(lhs)),
                          ParameterTraitsAt<L>::data_end(cntgs::get<L>(lhs)),
                          ParameterTraitsAt<K>::data_begin(cntgs::get<K>(rhs))};
    }

    static constexpr ElementSize calculate_element_size(const FixedSizesArray& fixed_sizes) noexcept
    {
        std::size_t size{};
        std::size_t offset{};
        std::size_t padding{};
        (
            [&]
            {
                const auto [next_offset, next_size, next_padding] =
                    detail::ParameterTraits<Parameter>::template aligned_size_in_memory<
                        ListTraits::template previous_alignment<I>(), ListTraits::template next_alignment<I>()>(
                        offset, FixedSizeGetter::template get<Parameter, I>(fixed_sizes));
                size += next_size;
                offset = next_offset;
                if constexpr (I == sizeof...(Parameter) - 1)
                {
                    padding = next_padding;
                }
            }(),
            ...);
        return {size, size + padding};
    }

    template <class ParameterT, bool IgnoreAliasing, std::size_t K, class Args>
    static std::byte* store_one(std::byte* address, std::size_t fixed_size, Args&& args) noexcept
    {
        static constexpr auto PREVIOUS_ALIGNMENT = ListTraits::template previous_alignment<K>();
        return detail::ParameterTraits<ParameterT>::template store<PREVIOUS_ALIGNMENT, IgnoreAliasing>(
            std::forward<Args>(args), address, fixed_size);
    }

    template <bool IgnoreAliasing, class... Args>
    static std::byte* emplace_at(std::byte* address, const FixedSizesArray& fixed_sizes, Args&&... args)
    {
        ((address = store_one<Parameter, IgnoreAliasing, I>(
              address, FixedSizeGetter::template get<Parameter, I>(fixed_sizes), std::forward<Args>(args))),
         ...);
        return address;
    }

    template <class ParameterT, std::size_t K, class FixedSizeGetterType, class FixedSizesType>
    static auto load_one(std::byte* CNTGS_RESTRICT address, const FixedSizesType& fixed_sizes) noexcept
    {
        static constexpr auto PREVIOUS_ALIGNMENT = ListTraits::template previous_alignment<K>();
        static constexpr auto IS_SIZE_PROVIDED = FixedSizeGetterType::template CAN_PROVIDE_SIZE<ParameterT>;
        return detail::ParameterTraits<ParameterT>::template load<PREVIOUS_ALIGNMENT, IS_SIZE_PROVIDED>(
            address, FixedSizeGetterType::template get<ParameterT, K>(fixed_sizes));
    }

  public:
    using StorageElementType = detail::Aligned<LARGEST_LEADING_ALIGNMENT_UNTIL_VARYING_SIZE>;

    template <class StorageType, class Allocator>
    static constexpr StorageType allocate_memory(std::size_t size_in_bytes, const Allocator& allocator)
    {
        const auto remainder = size_in_bytes % LARGEST_LEADING_ALIGNMENT_UNTIL_VARYING_SIZE;
        auto count = size_in_bytes / LARGEST_LEADING_ALIGNMENT_UNTIL_VARYING_SIZE;
        count += remainder == 0 ? 0 : 1;
        return StorageType(count, allocator);
    }

    static constexpr std::byte* align_for_first_parameter(std::byte* address) noexcept
    {
        return detail::align_if<(ListTraits::template trailing_alignment<(sizeof...(I) - 1)>()) <
                                    LARGEST_LEADING_ALIGNMENT_UNTIL_VARYING_SIZE,
                                LARGEST_LEADING_ALIGNMENT_UNTIL_VARYING_SIZE>(address);
    }

    template <class... Args>
    CNTGS_RESTRICT_RETURN static std::byte* emplace_at(std::byte* CNTGS_RESTRICT address,
                                                       const FixedSizesArray& fixed_sizes, Args&&... args)
    {
        return emplace_at<true>(address, fixed_sizes, std::forward<Args>(args)...);
    }

    template <class... Args>
    static std::byte* emplace_at_aliased(std::byte* address, const FixedSizesArray& fixed_sizes, Args&&... args)
    {
        return emplace_at<false>(address, fixed_sizes, std::forward<Args>(args)...);
    }

    template <class FixedSizeGetterType = ElementTraits::FixedSizeGetter,
              class FixedSizesType = ElementTraits::FixedSizesArray>
    static ContiguousPointer load_element_at(std::byte* CNTGS_RESTRICT address,
                                             const FixedSizesType& fixed_sizes) noexcept
    {
        ContiguousPointer result;
        ((std::tie(std::get<I>(result), address) = load_one<Parameter, I, FixedSizeGetterType>(address, fixed_sizes)),
         ...);
        return result;
    }

    static constexpr std::size_t calculate_element_stride(const FixedSizesArray& fixed_sizes) noexcept
    {
        return calculate_element_size(fixed_sizes).stride;
    }

    static constexpr std::size_t calculate_needed_memory_size(std::size_t max_element_count,
                                                              std::size_t varying_size_bytes,
                                                              const FixedSizesArray& fixed_sizes) noexcept
    {
        const auto [element_size, element_stride] = calculate_element_size(fixed_sizes);
        const auto padding = max_element_count == 0 ? 0 : (element_stride - element_size);
        return varying_size_bytes + element_stride * max_element_count - padding;
    }

    template <bool UseMove, bool IsConst>
    static constexpr void construct_if_non_trivial(const cntgs::BasicContiguousReference<IsConst, Parameter...>& source,
                                                   const ContiguousPointer& target)
    {
        (detail::construct_one_if_non_trivial<UseMove, Parameter>(cntgs::get<I>(source), std::get<I>(target)), ...);
    }

    template <bool UseMove, std::size_t K, bool IsLhsConst>
    static void assign_one(const cntgs::BasicContiguousReference<IsLhsConst, Parameter...>& source,
                           const ContiguousReference& target)
    {
        static constexpr auto INDEX = std::get<K>(std::get<UseMove>(CONSECUTIVE_TRIVIALLY_ASSIGNABLE_INDICES));
        if constexpr (INDEX == MANUAL)
        {
            if constexpr (UseMove)
            {
                ParameterTraitsAt<K>::move(cntgs::get<K>(source), cntgs::get<K>(target));
            }
            else
            {
                ParameterTraitsAt<K>::copy(cntgs::get<K>(source), cntgs::get<K>(target));
            }
        }
        else if constexpr (INDEX != SKIP)
        {
            const auto [source_start, source_end, target_start] =
                ElementTraits::template get_data_begin_and_end<K, INDEX>(source, target);
            std::memmove(target_start, source_start, source_end - source_start);
        }
    }

    template <bool UseMove, bool IsLhsConst>
    static void assign(const cntgs::BasicContiguousReference<IsLhsConst, Parameter...>& source,
                       const ContiguousReference& target)
    {
        (ElementTraits::template assign_one<UseMove, I>(source, target), ...);
    }

    template <std::size_t K>
    static void swap_one(const ContiguousReference& lhs, const ContiguousReference& rhs)
    {
        static constexpr auto INDEX = std::get<K>(CONSECUTIVE_TRIVIALLY_SWAPPABLE_INDICES);
        if constexpr (INDEX == MANUAL)
        {
            ParameterTraitsAt<K>::swap(cntgs::get<K>(lhs), cntgs::get<K>(rhs));
        }
        else if constexpr (INDEX != SKIP)
        {
            const auto [lhs_start, lhs_end, rhs_start] =
                ElementTraits::template get_data_begin_and_end<K, INDEX>(lhs, rhs);
            detail::trivial_swap_ranges(lhs_start, lhs_end, rhs_start);
        }
    }

    static void swap(const ContiguousReference& lhs, const ContiguousReference& rhs)
    {
        (ElementTraits::template swap_one<I>(lhs, rhs), ...);
    }

    template <std::size_t K, bool IsLhsConst, bool IsRhsConst>
    static constexpr auto equal_one(const cntgs::BasicContiguousReference<IsLhsConst, Parameter...>& lhs,
                                    const cntgs::BasicContiguousReference<IsRhsConst, Parameter...>& rhs)
    {
        constexpr auto INDEX = std::get<K>(CONSECUTIVE_EQUALITY_MEMCMPABLE_INDICES);
        if constexpr (INDEX == MANUAL)
        {
            return ParameterTraitsAt<K>::equal(cntgs::get<K>(lhs), cntgs::get<K>(rhs));
        }
        else if constexpr (INDEX != SKIP)
        {
            const auto [lhs_start, lhs_end, rhs_start] =
                ElementTraits::template get_data_begin_and_end<K, INDEX>(lhs, rhs);
            const auto rhs_end = ParameterTraitsAt<INDEX>::data_end(cntgs::get<INDEX>(rhs));
            return detail::trivial_equal(lhs_start, lhs_end, rhs_start, rhs_end);
        }
        else
        {
            return true;
        }
    }

    template <bool IsLhsConst, bool IsRhsConst>
    static constexpr auto equal(const cntgs::BasicContiguousReference<IsLhsConst, Parameter...>& lhs,
                                const cntgs::BasicContiguousReference<IsRhsConst, Parameter...>& rhs)
    {
        return (ElementTraits::template equal_one<I>(lhs, rhs) && ...);
    }

    template <std::size_t K, bool IsLhsConst, bool IsRhsConst>
    static constexpr auto lexicographical_compare_one(
        const cntgs::BasicContiguousReference<IsLhsConst, Parameter...>& lhs,
        const cntgs::BasicContiguousReference<IsRhsConst, Parameter...>& rhs)
    {
        constexpr auto INDEX = std::get<K>(CONSECUTIVE_LEXICOGRAPHICAL_MEMCMPABLE_INDICES);
        if constexpr (INDEX == MANUAL)
        {
            return ParameterTraitsAt<K>::lexicographical_compare(cntgs::get<K>(lhs), cntgs::get<K>(rhs));
        }
        else if constexpr (INDEX != SKIP)
        {
            const auto [lhs_start, lhs_end, rhs_start] =
                ElementTraits::template get_data_begin_and_end<K, INDEX>(lhs, rhs);
            const auto rhs_end = ParameterTraitsAt<INDEX>::data_end(cntgs::get<INDEX>(rhs));
            return detail::trivial_lexicographical_compare(lhs_start, lhs_end, rhs_start, rhs_end);
        }
        else
        {
            return true;
        }
    }

    template <bool IsLhsConst, bool IsRhsConst>
    static constexpr auto lexicographical_compare(const cntgs::BasicContiguousReference<IsLhsConst, Parameter...>& lhs,
                                                  const cntgs::BasicContiguousReference<IsRhsConst, Parameter...>& rhs)
    {
        return (ElementTraits::template lexicographical_compare_one<I>(lhs, rhs) && ...);
    }

    static void destruct(const ContiguousReference& reference) noexcept
    {
        (detail::ParameterTraits<Parameter>::destroy(cntgs::get<I>(reference)), ...);
    }
};

template <class... Parameter>
using ElementTraitsT = detail::ElementTraits<std::make_index_sequence<sizeof...(Parameter)>, Parameter...>;
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_ELEMENTTRAITS_HPP
