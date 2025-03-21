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
    std::size_t distance_to_first;
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
    using SizeGetter = detail::SizeGetter<Parameter...>;

    static constexpr auto LARGEST_ALIGNMENT_BETWEEN_VARYING_SIZES = []
    {
        std::size_t alignment{};
        std::array<std::size_t, sizeof...(Parameter)> result{};
        auto begin = result.data();
        auto it = begin;
        (
            [&]
            {
                alignment = (std::max)(alignment, detail::ParameterTraits<Parameter>::ALIGNMENT);
                ++it;
                if constexpr (detail::ParameterTraits<Parameter>::TYPE == detail::ParameterType::VARYING_SIZE)
                {
                    detail::fill(begin, it, alignment);
                    begin = it;
                    alignment = {};
                }
            }(),
            ...);
        detail::fill(begin, it, alignment);
        return result;
    }();

    static constexpr std::size_t STORAGE_ELEMENT_ALIGNMENT =
        (std::max)({std::get<I>(LARGEST_ALIGNMENT_BETWEEN_VARYING_SIZES)...});
    static constexpr std::size_t SKIP = std::numeric_limits<std::size_t>::max();
    static constexpr std::size_t MANUAL = SKIP - 1;

    static constexpr auto calculate_trailing_alignments() noexcept
    {
        std::size_t offset{};
        std::size_t alignment{STORAGE_ELEMENT_ALIGNMENT};
        return std::array{[&]
                          {
                              const auto [next_offset, next_align, next_trailing_alignment] =
                                  detail::ParameterTraits<Parameter>::trailing_alignment(offset, alignment);
                              offset = next_offset;
                              alignment = next_align;
                              return next_trailing_alignment;
                          }()...};
    }

    static constexpr auto TRAILING_ALIGNMENTS = calculate_trailing_alignments();

    static constexpr std::size_t INDEX_OF_PARAMETER_WITH_LARGEST_ALIGNMENT = []
    {
        std::size_t index{};
        std::size_t alignment{};
        const bool has_no_varying_size = (
            [&]
            {
                if (alignment < detail::ParameterTraits<Parameter>::ALIGNMENT)
                {
                    index = I;
                    alignment = detail::ParameterTraits<Parameter>::ALIGNMENT;
                }
                return detail::ParameterTraits<Parameter>::TYPE != detail::ParameterType::VARYING_SIZE;
            }() &&
            ...);
        if (has_no_varying_size)
        {
            return index;
        }
        return std::size_t{};
    }();

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

    template <std::size_t K>
    static constexpr std::size_t trailing_alignment() noexcept
    {
        return detail::trailing_alignment(ParameterTraitsAt<(K)>::VALUE_BYTES, ParameterTraitsAt<(K)>::VALUE_ALIGNMENT);
    }

    template <std::size_t K>
    static constexpr std::size_t next_alignment() noexcept
    {
        if constexpr (sizeof...(Parameter) - 1 == K)
        {
            return STORAGE_ELEMENT_ALIGNMENT;
        }
        else
        {
            return std::get<(K + 1)>(LARGEST_ALIGNMENT_BETWEEN_VARYING_SIZES);
        }
    }

    template <std::size_t K>
    static constexpr std::size_t previous_trailing_alignment() noexcept
    {
        if constexpr (K == 0)
        {
            return STORAGE_ELEMENT_ALIGNMENT;
        }
        else
        {
            return std::get<(K - 1)>(TRAILING_ALIGNMENTS);
        }
    }

    template <std::size_t K, std::size_t L, bool IsLhsConst, bool IsRhsConst>
    static constexpr auto get_data_begin_and_end(
        const cntgs::BasicContiguousReference<IsLhsConst, Parameter...>& lhs,
        const cntgs::BasicContiguousReference<IsRhsConst, Parameter...>& rhs) noexcept
    {
        return std::tuple{ParameterTraitsAt<K>::data_begin(cntgs::get<K>(lhs)),
                          ParameterTraitsAt<L>::data_end(cntgs::get<L>(lhs)),
                          ParameterTraitsAt<K>::data_begin(cntgs::get<K>(rhs))};
    }

    template <class ParameterT, bool IgnoreAliasing, std::size_t K, class Args>
    static std::byte* store_one(std::byte* address, std::size_t fixed_size, Args&& args) noexcept
    {
        return detail::ParameterTraits<ParameterT>::template store<previous_trailing_alignment<K>(), IgnoreAliasing>(
            std::forward<Args>(args), address, fixed_size);
    }

    template <bool IgnoreAliasing, class... Args>
    static std::byte* emplace_at(std::byte* address, const FixedSizesArray& fixed_sizes, Args&&... args)
    {
        ((address = store_one<Parameter, IgnoreAliasing, I>(
              address, SizeGetter::template get_fixed_size<I>(fixed_sizes), static_cast<Args&&>(args))),
         ...);
        return address;
    }

    template <class ParameterT, std::size_t K, class SizeGetterType, class FixedSizesType>
    static auto load_one(std::byte* CNTGS_RESTRICT address, const FixedSizesType& CNTGS_RESTRICT fixed_sizes,
                         const ContiguousPointer& CNTGS_RESTRICT result) noexcept
    {
        return detail::ParameterTraits<ParameterT>::template load<previous_trailing_alignment<K>()>(
            address, SizeGetterType::template get<ParameterT, K>(fixed_sizes, result));
    }

    static constexpr ElementSize calculate_element_size_all_fixed_size(const FixedSizesArray& fixed_sizes) noexcept
    {
        BackwardSizeInMemory backward{};
        ForwardSizeInMemory forward{};
        (
            [&]
            {
                if constexpr (I < INDEX_OF_PARAMETER_WITH_LARGEST_ALIGNMENT)
                {
                    const auto next_backward = detail::ParameterTraits<Parameter>::backward_size_in_memory(
                        backward.offset,
                        SizeGetter::template get_fixed_size<(INDEX_OF_PARAMETER_WITH_LARGEST_ALIGNMENT + I) %
                                                            sizeof...(Parameter)>(fixed_sizes));
                    backward = next_backward;
                }
                else
                {
                    const auto next_forward = detail::ParameterTraits<Parameter>::forward_size_in_memory(
                        forward.offset, SizeGetter::template get_fixed_size<I>(fixed_sizes));
                    forward = next_forward;
                }
            }(),
            ...);
        const auto size = backward.offset + forward.offset;
        const auto distance_to_first = detail::align(backward.offset, STORAGE_ELEMENT_ALIGNMENT) - backward.offset;
        const auto leftover = (distance_to_first + size) % STORAGE_ELEMENT_ALIGNMENT;
        const auto padding = leftover > distance_to_first ? (STORAGE_ELEMENT_ALIGNMENT - leftover) + distance_to_first
                                                          : distance_to_first - leftover;
        return {size, size + padding, distance_to_first};
    }

    template <std::size_t IndexOfFirst, std::size_t LargestAlignmentWithinFirst>
    static constexpr ElementSize calculate_element_size_impl(const FixedSizesArray& fixed_sizes) noexcept
    {
        std::size_t size{};
        std::size_t offset{};
        std::size_t padding{};
        std::size_t alignment{STORAGE_ELEMENT_ALIGNMENT};
        std::size_t distance_to_first{};
        (
            [&]
            {
                const auto [next_offset, next_size, next_padding, next_align] =
                    detail::ParameterTraits<Parameter>::template aligned_size_in_memory<
                        previous_trailing_alignment<I>(), next_alignment<I>()>(
                        offset, alignment,
                        SizeGetter::template get_fixed_size<(IndexOfFirst + I) % sizeof...(Parameter)>(fixed_sizes));
                if constexpr (0 != LargestAlignmentWithinFirst && IndexOfFirst - 1 == I)
                {
                    distance_to_first = next_offset + next_padding;
                    distance_to_first =
                        distance_to_first -
                        detail::align_down(distance_to_first, ParameterTraitsAt<IndexOfFirst>::ALIGNMENT);
                }
                size += next_size;
                offset = next_offset;
                alignment = next_align;
                if constexpr (I == sizeof...(Parameter) - 1)
                {
                    padding = next_padding;
                }
            }(),
            ...);
        const auto stride = size + padding;
        return {size, stride, distance_to_first};
    }

  public:
    using StorageElementType = detail::Aligned<STORAGE_ELEMENT_ALIGNMENT>;

    static constexpr bool FIRST_ELEMENT_HAS_OFFSET = 0 != INDEX_OF_PARAMETER_WITH_LARGEST_ALIGNMENT;

    template <class StorageType, class Allocator>
    static constexpr StorageType allocate_memory(std::size_t size_in_bytes, const Allocator& allocator)
    {
        const auto remainder = size_in_bytes % STORAGE_ELEMENT_ALIGNMENT;
        auto count = size_in_bytes / STORAGE_ELEMENT_ALIGNMENT;
        count += remainder == 0 ? 0 : 1;
        return StorageType(count, allocator);
    }

    static constexpr std::byte* align_for_first_parameter(std::byte* address) noexcept
    {
        return detail::align_if<(previous_trailing_alignment<sizeof...(Parameter)>()) < STORAGE_ELEMENT_ALIGNMENT,
                                STORAGE_ELEMENT_ALIGNMENT>(address);
    }

    template <class... Args>
    CNTGS_RESTRICT_RETURN static std::byte* emplace_at(std::byte* CNTGS_RESTRICT address,
                                                       const FixedSizesArray& fixed_sizes, Args&&... args)
    {
        return emplace_at<true>(address, fixed_sizes, static_cast<Args&&>(args)...);
    }

    template <class... Args>
    static std::byte* emplace_at_aliased(std::byte* address, const FixedSizesArray& fixed_sizes, Args&&... args)
    {
        return emplace_at<false>(address, fixed_sizes, static_cast<Args&&>(args)...);
    }

    template <class SizeGetterType = ElementTraits::SizeGetter, class FixedSizesType = ElementTraits::FixedSizesArray>
    static ContiguousPointer load_element_at(std::byte* CNTGS_RESTRICT address,
                                             const FixedSizesType& CNTGS_RESTRICT fixed_sizes) noexcept
    {
        ContiguousPointer result;
        ((std::tie(std::get<I>(result), address) =
              load_one<Parameter, I, SizeGetterType>(address, fixed_sizes, result)),
         ...);
        return result;
    }

    static constexpr ElementSize calculate_element_size(const FixedSizesArray& fixed_sizes) noexcept
    {
        if constexpr (FIRST_ELEMENT_HAS_OFFSET)
        {
            return calculate_element_size_all_fixed_size(fixed_sizes);
        }
        else
        {
            return calculate_element_size_impl<0, 0>(fixed_sizes);
        }
    }

    static constexpr std::size_t calculate_needed_memory_size(std::size_t max_element_count,
                                                              std::size_t varying_size_bytes, ElementSize size) noexcept
    {
        const auto [element_size, element_stride, distance_to_first] = size;
        const auto padding = max_element_count == 0 ? 0 : (element_stride - element_size);
        return distance_to_first + varying_size_bytes + element_stride * max_element_count - padding;
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
