#pragma once

#include "cntgs/contiguous/detail/algorithm.h"
#include "cntgs/contiguous/detail/attributes.h"
#include "cntgs/contiguous/detail/forward.h"
#include "cntgs/contiguous/detail/memory.h"
#include "cntgs/contiguous/detail/parameterListTraits.h"
#include "cntgs/contiguous/detail/parameterTraits.h"
#include "cntgs/contiguous/detail/sizeGetter.h"
#include "cntgs/contiguous/detail/typeUtils.h"
#include "cntgs/contiguous/detail/vectorTraits.h"

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
constexpr auto alignment_offset([[maybe_unused]] std::size_t position) noexcept
{
    if constexpr (Alignment == 1)
    {
        return std::size_t{};
    }
    else
    {
        return detail::align(Alignment, position) - position;
    }
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
class ElementTraits;

template <std::size_t... I, class... Types>
class ElementTraits<std::index_sequence<I...>, Types...>
{
  private:
    using Self = ElementTraits<std::index_sequence<I...>, Types...>;
    using ListTraits = detail::ParameterListTraits<Types...>;
    using FixedSizes = typename ListTraits::FixedSizes;
    using ContiguousPointer = typename detail::ContiguousVectorTraits<Types...>::PointerType;
    using ContiguousReference = typename detail::ContiguousVectorTraits<Types...>::ReferenceType;
    using AlignmentNeeds =
        detail::ConditionalT<(ListTraits::CONTIGUOUS_FIXED_SIZE_COUNT == ListTraits::CONTIGUOUS_COUNT),
                             detail::IgnoreFirstAlignmentNeeds, detail::DefaultAlignmentNeeds>;
    using FixedSizeGetter = detail::FixedSizeGetter<Types...>;

    static constexpr auto SKIP = std::numeric_limits<std::size_t>::max();
    static constexpr auto MANUAL = SKIP - 1;

    template <template <class> class Predicate>
    static constexpr auto calculate_consecutive_indices() noexcept
    {
        std::array<std::size_t, sizeof...(Types)> consecutive_indices{((void)I, SKIP)...};
        [[maybe_unused]] std::size_t index = 0;
        (
            [&]
            {
                if constexpr (Predicate<typename detail::ParameterTraits<Types>::ValueType>::value)
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

    template <std::size_t K, std::size_t L, detail::ContiguousTupleQualifier LhsQualifier,
              detail::ContiguousTupleQualifier RhsQualifier>
    static constexpr auto get_data_begin_and_end(const cntgs::ContiguousTuple<LhsQualifier, Types...>& lhs,
                                                 const cntgs::ContiguousTuple<RhsQualifier, Types...>& rhs) noexcept
    {
        return std::tuple{ParameterTraitsAt<K>::data_begin(std::get<K>(lhs.tuple)),
                          ParameterTraitsAt<L>::data_end(std::get<L>(lhs.tuple)),
                          ParameterTraitsAt<K>::data_begin(std::get<K>(rhs.tuple))};
    }

    template <bool IgnoreAliasing, class... Args>
    static std::byte* emplace_at(std::byte* CNTGS_RESTRICT address, const FixedSizes& fixed_sizes, Args&&... args)
    {
        ((address = detail::ParameterTraits<Types>::template store<AlignmentNeeds::template VALUE<I>, IgnoreAliasing>(
              std::forward<Args>(args), address, FixedSizeGetter::template get<Types, I>(fixed_sizes))),
         ...);
        return address;
    }

  public:
    template <std::size_t K>
    using ParameterTraitsAt = typename ListTraits::template ParameterTraitsAt<K>;

    template <class... Args>
    CNTGS_RESTRICT_RETURN static std::byte* emplace_at(std::byte* CNTGS_RESTRICT address, const FixedSizes& fixed_sizes,
                                                       Args&&... args)
    {
        return Self::template emplace_at<true>(address, fixed_sizes, std::forward<Args>(args)...);
    }

    template <class... Args>
    static std::byte* emplace_at_aliased(std::byte* CNTGS_RESTRICT address, const FixedSizes& fixed_sizes,
                                         Args&&... args)
    {
        return Self::template emplace_at<false>(address, fixed_sizes, std::forward<Args>(args)...);
    }

    template <class AlignmentNeedsType = AlignmentNeeds, class FixedSizeGetterType = FixedSizeGetter,
              class FixedSizesType = FixedSizes>
    static auto load_element_at(std::byte* CNTGS_RESTRICT address, const FixedSizesType& fixed_sizes) noexcept
    {
        ContiguousPointer result;
        ((std::tie(std::get<I>(result), address) =
              detail::ParameterTraits<Types>::template load<AlignmentNeedsType::template VALUE<I>>(
                  address, FixedSizeGetterType::template get<Types, I>(fixed_sizes))),
         ...);
        return result;
    }

    static constexpr auto calculate_element_size(const FixedSizes& fixed_sizes) noexcept
    {
        std::size_t result{};
        if constexpr (ListTraits::IS_FIXED_OR_PLAIN)
        {
            ((result += detail::ParameterTraits<Types>::guaranteed_size_in_memory(
                            FixedSizeGetter::template get<Types, I>(fixed_sizes)) +
                        detail::alignment_offset<detail::ParameterTraits<Types>::ALIGNMENT>(result)),
             ...);
        }
        else
        {
            ((result += detail::ParameterTraits<Types>::aligned_size_in_memory(
                            FixedSizeGetter::template get<Types, I>(fixed_sizes)) +
                        detail::alignment_offset<detail::ParameterTraits<Types>::ALIGNMENT>(result)),
             ...);
        }
        return result + detail::alignment_offset<ParameterTraitsAt<0>::ALIGNMENT>(result);
    }

    template <bool UseMove, detail::ContiguousTupleQualifier Qualifier>
    static constexpr void construct_if_non_trivial(const cntgs::ContiguousTuple<Qualifier, Types...>& source,
                                                   const ContiguousPointer& target)
    {
        (detail::construct_one_if_non_trivial<UseMove, Types>(std::get<I>(source.tuple), std::get<I>(target)), ...);
    }

    template <bool UseMove, std::size_t K, detail::ContiguousTupleQualifier LhsQualifier,
              detail::ContiguousTupleQualifier RhsQualifier>
    static void assign(const cntgs::ContiguousTuple<LhsQualifier, Types...>& source,
                       const cntgs::ContiguousTuple<RhsQualifier, Types...>& target)
    {
        static constexpr auto INDEX = std::get<K>(std::get<UseMove>(CONSECUTIVE_TRIVIALLY_ASSIGNABLE_INDICES));
        if constexpr (INDEX == MANUAL)
        {
            if constexpr (UseMove)
            {
                ParameterTraitsAt<K>::move(std::get<K>(source.tuple), std::get<K>(target.tuple));
            }
            else
            {
                ParameterTraitsAt<K>::copy(std::get<K>(source.tuple), std::get<K>(target.tuple));
            }
        }
        else if constexpr (INDEX != SKIP)
        {
            const auto [source_start, source_end, target_start] =
                Self::template get_data_begin_and_end<K, INDEX>(source, target);
            std::memmove(target_start, source_start, source_end - source_start);
        }
    }

    template <bool UseMove, detail::ContiguousTupleQualifier LhsQualifier,
              detail::ContiguousTupleQualifier RhsQualifier>
    static void assign(const cntgs::ContiguousTuple<LhsQualifier, Types...>& source,
                       const cntgs::ContiguousTuple<RhsQualifier, Types...>& target)
    {
        (Self::template assign<UseMove, I>(source, target), ...);
    }

    template <std::size_t K>
    static void swap(const ContiguousReference& lhs, const ContiguousReference& rhs)
    {
        static constexpr auto INDEX = std::get<K>(CONSECUTIVE_TRIVIALLY_SWAPPABLE_INDICES);
        if constexpr (INDEX == MANUAL)
        {
            ParameterTraitsAt<K>::swap(std::get<K>(lhs.tuple), std::get<K>(rhs.tuple));
        }
        else if constexpr (INDEX != SKIP)
        {
            const auto [lhs_start, lhs_end, rhs_start] = Self::template get_data_begin_and_end<K, INDEX>(lhs, rhs);
            detail::trivial_swap_ranges(lhs_start, lhs_end, rhs_start);
        }
    }

    static void swap(const ContiguousReference& lhs, const ContiguousReference& rhs)
    {
        (Self::template swap<I>(lhs, rhs), ...);
    }

    template <std::size_t K, detail::ContiguousTupleQualifier LhsQualifier,
              detail::ContiguousTupleQualifier RhsQualifier>
    static constexpr auto equal(const cntgs::ContiguousTuple<LhsQualifier, Types...>& lhs,
                                const cntgs::ContiguousTuple<RhsQualifier, Types...>& rhs)
    {
        constexpr auto INDEX = std::get<K>(CONSECUTIVE_EQUALITY_MEMCMPABLE_INDICES);
        if constexpr (INDEX == MANUAL)
        {
            return ParameterTraitsAt<K>::equal(std::get<K>(lhs.tuple), std::get<K>(rhs.tuple));
        }
        else if constexpr (INDEX != SKIP)
        {
            const auto [lhs_start, lhs_end, rhs_start] = Self::template get_data_begin_and_end<K, INDEX>(lhs, rhs);
            const auto rhs_end = ParameterTraitsAt<INDEX>::data_end(std::get<INDEX>(rhs.tuple));
            return detail::trivial_equal(lhs_start, lhs_end, rhs_start, rhs_end);
        }
        else
        {
            return true;
        }
    }

    template <detail::ContiguousTupleQualifier LhsQualifier, detail::ContiguousTupleQualifier RhsQualifier>
    static constexpr auto equal(const cntgs::ContiguousTuple<LhsQualifier, Types...>& lhs,
                                const cntgs::ContiguousTuple<RhsQualifier, Types...>& rhs)
    {
        return (Self::template equal<I>(lhs, rhs) && ...);
    }

    template <std::size_t K, detail::ContiguousTupleQualifier LhsQualifier,
              detail::ContiguousTupleQualifier RhsQualifier>
    static constexpr auto lexicographical_compare(const cntgs::ContiguousTuple<LhsQualifier, Types...>& lhs,
                                                  const cntgs::ContiguousTuple<RhsQualifier, Types...>& rhs)
    {
        constexpr auto INDEX = std::get<K>(CONSECUTIVE_LEXICOGRAPHICAL_MEMCMPABLE_INDICES);
        if constexpr (INDEX == MANUAL)
        {
            return ParameterTraitsAt<K>::lexicographical_compare(std::get<K>(lhs.tuple), std::get<K>(rhs.tuple));
        }
        else if constexpr (INDEX != SKIP)
        {
            const auto [lhs_start, lhs_end, rhs_start] = Self::template get_data_begin_and_end<K, INDEX>(lhs, rhs);
            const auto rhs_end = ParameterTraitsAt<INDEX>::data_end(std::get<INDEX>(rhs.tuple));
            return detail::trivial_lexicographical_compare(lhs_start, lhs_end, rhs_start, rhs_end);
        }
        else
        {
            return true;
        }
    }

    template <detail::ContiguousTupleQualifier LhsQualifier, detail::ContiguousTupleQualifier RhsQualifier>
    static constexpr auto lexicographical_compare(const cntgs::ContiguousTuple<LhsQualifier, Types...>& lhs,
                                                  const cntgs::ContiguousTuple<RhsQualifier, Types...>& rhs)
    {
        return (Self::template lexicographical_compare<I>(lhs, rhs) && ...);
    }

    static void destruct(const ContiguousReference& reference) noexcept
    {
        (detail::ParameterTraits<Types>::destroy(std::get<I>(reference.tuple)), ...);
    }
};

template <class... Types>
using ElementTraitsT = detail::ElementTraits<std::make_index_sequence<sizeof...(Types)>, Types...>;
}  // namespace cntgs::detail
