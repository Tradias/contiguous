#pragma once

#include "cntgs/contiguous/detail/attributes.h"
#include "cntgs/contiguous/detail/fixedSizeGetter.h"
#include "cntgs/contiguous/detail/forward.h"
#include "cntgs/contiguous/detail/memory.h"
#include "cntgs/contiguous/detail/parameterListTraits.h"
#include "cntgs/contiguous/detail/parameterTraits.h"
#include "cntgs/contiguous/detail/typeUtils.h"
#include "cntgs/contiguous/detail/vectorTraits.h"

#include <array>
#include <cstddef>
#include <limits>
#include <type_traits>

namespace cntgs::detail
{
struct DefaultAlignmentSelector
{
    template <std::size_t>
    static constexpr auto VALUE = true;
};

struct IgnoreFirstAlignmentSelector
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
    using ListTraits = detail::ParameterListTraits<Types...>;
    using FixedSizes = typename ListTraits::FixedSizes;
    using PointerReturnType = typename detail::ContiguousVectorTraits<Types...>::PointerReturnType;
    using ReferenceReturnType = typename detail::ContiguousVectorTraits<Types...>::ReferenceReturnType;
    using AlignmentSelector =
        detail::ConditionalT<(ListTraits::CONTIGUOUS_FIXED_SIZE_COUNT == ListTraits::CONTIGUOUS_COUNT),
                             detail::IgnoreFirstAlignmentSelector, detail::DefaultAlignmentSelector>;
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

    template <bool IgnoreAliasing, class... Args>
    static std::byte* emplace_at(std::byte* CNTGS_RESTRICT address, const FixedSizes& fixed_sizes, Args&&... args)
    {
        ((address =
              detail::ParameterTraits<Types>::template store<AlignmentSelector::template VALUE<I>, IgnoreAliasing>(
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
        return emplace_at<true>(address, fixed_sizes, std::forward<Args>(args)...);
    }

    template <class... Args>
    static std::byte* emplace_at_aliased(std::byte* CNTGS_RESTRICT address, const FixedSizes& fixed_sizes,
                                         Args&&... args)
    {
        return emplace_at<false>(address, fixed_sizes, std::forward<Args>(args)...);
    }

    template <class NeedsAlignmentSelector = AlignmentSelector, class FixedSizeGetterType = FixedSizeGetter,
              class FixedSizesType = FixedSizes>
    static auto load_element_at(std::byte* CNTGS_RESTRICT address, const FixedSizesType& fixed_sizes) noexcept
    {
        PointerReturnType result;
        ((std::tie(std::get<I>(result), address) =
              detail::ParameterTraits<Types>::template load<NeedsAlignmentSelector::template VALUE<I>>(
                  address, FixedSizeGetterType::template get<Types, I>(fixed_sizes))),
         ...);
        return result;
    }

    static constexpr auto calculate_element_size(const FixedSizes& fixed_sizes) noexcept
    {
        std::size_t result{};
        if constexpr (ListTraits::IS_ALL_FIXED_SIZE || ListTraits::IS_NONE_SPECIAL)
        {
            ((result += detail::ParameterTraits<Types>::guaranteed_size_in_memory(
                            FixedSizeGetter::template get<Types, I>(fixed_sizes)) +
                        alignment_offset<detail::ParameterTraits<Types>::ALIGNMENT>(result)),
             ...);
        }
        else
        {
            ((result += detail::ParameterTraits<Types>::aligned_size_in_memory(
                            FixedSizeGetter::template get<Types, I>(fixed_sizes)) +
                        alignment_offset<detail::ParameterTraits<Types>::ALIGNMENT>(result)),
             ...);
        }
        return result + alignment_offset<ParameterTraitsAt<0>::ALIGNMENT>(result);
    }

    template <bool UseMove, detail::ContiguousTupleQualifier Qualifier>
    static constexpr void construct_if_non_trivial(const cntgs::ContiguousTuple<Qualifier, Types...>& source,
                                                   const PointerReturnType& target)
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
            const auto target_start = ParameterTraitsAt<K>::start_address(std::get<K>(target.tuple));
            const auto source_start = ParameterTraitsAt<K>::start_address(std::get<K>(source.tuple));
            const auto source_end = ParameterTraitsAt<INDEX>::end_address(std::get<INDEX>(source.tuple));
            std::memmove(target_start, source_start, source_end - source_start);
        }
    }

    template <bool UseMove, detail::ContiguousTupleQualifier LhsQualifier,
              detail::ContiguousTupleQualifier RhsQualifier>
    static void assign(const cntgs::ContiguousTuple<LhsQualifier, Types...>& source,
                       const cntgs::ContiguousTuple<RhsQualifier, Types...>& target)
    {
        (assign<UseMove, I>(source, target), ...);
    }

    template <std::size_t K>
    static void swap(const ReferenceReturnType& lhs, const ReferenceReturnType& rhs)
    {
        static constexpr auto INDEX = std::get<K>(CONSECUTIVE_TRIVIALLY_SWAPPABLE_INDICES);
        if constexpr (INDEX == MANUAL)
        {
            ParameterTraitsAt<K>::swap(std::get<K>(lhs.tuple), std::get<K>(rhs.tuple));
        }
        else if constexpr (INDEX != SKIP)
        {
            const auto rhs_start = ParameterTraitsAt<K>::start_address(std::get<K>(rhs.tuple));
            const auto lhs_start = ParameterTraitsAt<K>::start_address(std::get<K>(lhs.tuple));
            const auto lhs_end = ParameterTraitsAt<INDEX>::end_address(std::get<INDEX>(lhs.tuple));
            // some compilers (e.g. MSVC) perform handrolled optimizations if the argument type
            // to swap_ranges are trivially_swappable which includes that is has
            // no ADL discovered swap function. std::byte has such function, but
            // raw pointers do not
            using UnderlyingType = std::underlying_type_t<std::byte>;
            std::swap_ranges(reinterpret_cast<UnderlyingType*>(lhs_start), reinterpret_cast<UnderlyingType*>(lhs_end),
                             reinterpret_cast<UnderlyingType*>(rhs_start));
        }
    }

    static void swap(const ReferenceReturnType& lhs, const ReferenceReturnType& rhs) { (swap<I>(lhs, rhs), ...); }

    static void destruct(const ReferenceReturnType& element) noexcept
    {
        (detail::ParameterTraits<Types>::destroy(std::get<I>(element.tuple)), ...);
    }
};

template <class... Types>
using ElementTraitsT = detail::ElementTraits<std::make_index_sequence<sizeof...(Types)>, Types...>;
}  // namespace cntgs::detail
