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
struct DefaultAlignmentNeeds
{
    template <std::size_t>
    static constexpr auto VALUE = true;
};

struct IgnoreFirstAlignmentNeeds
{
    template <std::size_t I>
    static constexpr auto VALUE = I != 0;
};

template <std::size_t Alignment>
constexpr auto alignment_offset(std::size_t position) noexcept
{
    return detail::align<Alignment>(position) - position;
}

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

template <class, class...>
class ElementTraits;

template <std::size_t... I, class... Parameter>
class ElementTraits<std::index_sequence<I...>, Parameter...>
{
  private:
    using ListTraits = detail::ParameterListTraits<Parameter...>;
    using FixedSizesArray = typename ListTraits::FixedSizesArray;
    using ContiguousPointer = typename detail::ContiguousVectorTraits<Parameter...>::PointerType;
    using ContiguousReference = typename detail::ContiguousVectorTraits<Parameter...>::ReferenceType;
    using AlignmentNeeds = detail::ConditionalT<ListTraits::IS_FIXED_SIZE_OR_PLAIN, detail::IgnoreFirstAlignmentNeeds,
                                                detail::DefaultAlignmentNeeds>;
    using FixedSizeGetter = detail::FixedSizeGetter<Parameter...>;

    static constexpr auto SKIP = std::numeric_limits<std::size_t>::max();
    static constexpr auto MANUAL = SKIP - 1;

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
        calculate_consecutive_indices<detail::LexicographicalMemcmpCompatibleT>()};

    template <std::size_t K, std::size_t L, bool IsLhsConst, bool IsRhsConst>
    static constexpr auto get_data_begin_and_end(
        const cntgs::BasicContiguousReference<IsLhsConst, Parameter...>& lhs,
        const cntgs::BasicContiguousReference<IsRhsConst, Parameter...>& rhs) noexcept
    {
        return std::tuple{ParameterTraitsAt<K>::data_begin(cntgs::get<K>(lhs)),
                          ParameterTraitsAt<L>::data_end(cntgs::get<L>(lhs)),
                          ParameterTraitsAt<K>::data_begin(cntgs::get<K>(rhs))};
    }

    template <bool IgnoreAliasing, class... Args>
    static std::byte* emplace_at(std::byte* CNTGS_RESTRICT address, const FixedSizesArray& fixed_sizes, Args&&... args)
    {
        ((address =
              detail::ParameterTraits<Parameter>::template store<AlignmentNeeds::template VALUE<I>, IgnoreAliasing>(
                  std::forward<Args>(args), address, FixedSizeGetter::template get<Parameter, I>(fixed_sizes))),
         ...);
        return address;
    }

    template <class Type, std::size_t K, class AlignmentNeedsType, class FixedSizeGetterType, class FixedSizesType>
    static auto load_one(std::byte* CNTGS_RESTRICT address, const FixedSizesType& fixed_sizes) noexcept
    {
        static constexpr auto NEEDS_ALIGNMENT = AlignmentNeedsType::template VALUE<K>;
        static constexpr auto IS_SIZE_PROVIDED = FixedSizeGetterType::template CAN_PROVIDE_SIZE<Type>;
        return detail::ParameterTraits<Type>::template load<NEEDS_ALIGNMENT, IS_SIZE_PROVIDED>(
            address, FixedSizeGetterType::template get<Type, K>(fixed_sizes));
    }

  public:
    template <std::size_t K>
    using ParameterTraitsAt = typename ListTraits::template ParameterTraitsAt<K>;

    template <class... Args>
    CNTGS_RESTRICT_RETURN static std::byte* emplace_at(std::byte* CNTGS_RESTRICT address,
                                                       const FixedSizesArray& fixed_sizes, Args&&... args)
    {
        return ElementTraits::template emplace_at<true>(address, fixed_sizes, std::forward<Args>(args)...);
    }

    template <class... Args>
    static std::byte* emplace_at_aliased(std::byte* CNTGS_RESTRICT address, const FixedSizesArray& fixed_sizes,
                                         Args&&... args)
    {
        return ElementTraits::template emplace_at<false>(address, fixed_sizes, std::forward<Args>(args)...);
    }

    template <class AlignmentNeedsType = ElementTraits::AlignmentNeeds,
              class FixedSizeGetterType = ElementTraits::FixedSizeGetter,
              class FixedSizesType = ElementTraits::FixedSizesArray>
    static auto load_element_at(std::byte* CNTGS_RESTRICT address, const FixedSizesType& fixed_sizes) noexcept
    {
        ContiguousPointer result;
        ((std::tie(std::get<I>(result), address) =
              ElementTraits::template load_one<Parameter, I, AlignmentNeedsType, FixedSizeGetterType>(address,
                                                                                                      fixed_sizes)),
         ...);
        return result;
    }

    static constexpr auto calculate_element_size(const FixedSizesArray& fixed_sizes) noexcept
    {
        std::size_t result{};
        if constexpr (ListTraits::IS_FIXED_SIZE_OR_PLAIN)
        {
            ((result += detail::ParameterTraits<Parameter>::guaranteed_size_in_memory(
                            FixedSizeGetter::template get<Parameter, I>(fixed_sizes)) +
                        detail::alignment_offset<detail::ParameterTraits<Parameter>::ALIGNMENT>(result)),
             ...);
        }
        else
        {
            ((result += detail::ParameterTraits<Parameter>::aligned_size_in_memory(
                            FixedSizeGetter::template get<Parameter, I>(fixed_sizes)) +
                        detail::alignment_offset<detail::ParameterTraits<Parameter>::ALIGNMENT>(result)),
             ...);
        }
        return result + detail::alignment_offset<ParameterTraitsAt<0>::ALIGNMENT>(result);
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
