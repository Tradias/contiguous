#pragma once

#include "cntgs/contiguous/detail/attributes.h"
#include "cntgs/contiguous/detail/fixedSizeGetter.h"
#include "cntgs/contiguous/detail/forward.h"
#include "cntgs/contiguous/detail/memory.h"
#include "cntgs/contiguous/detail/parameterListTraits.h"
#include "cntgs/contiguous/detail/parameterTraits.h"
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

template <class... Types>
using AlignmentSelectorT = std::conditional_t<(detail::ParameterListTraits<Types...>::CONTIGUOUS_FIXED_SIZE_COUNT ==
                                               detail::ParameterListTraits<Types...>::CONTIGUOUS_COUNT),
                                              detail::IgnoreFirstAlignmentSelector, detail::DefaultAlignmentSelector>;

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
class ElementTraits;

template <std::size_t... I, class... Types>
class ElementTraits<std::index_sequence<I...>, Types...>
{
  private:
    using ListTraits = detail::ParameterListTraits<Types...>;
    using FixedSizes = typename ListTraits::FixedSizes;
    using PointerReturnType = typename detail::ContiguousVectorTraits<Types...>::PointerReturnType;
    using ReferenceReturnType = typename detail::ContiguousVectorTraits<Types...>::ReferenceReturnType;
    using AlignmentSelector = detail::AlignmentSelectorT<Types...>;

    template <class T>
    using FixedSizeGetter = detail::FixedSizeGetter<T, detail::TypeList<Types...>>;

    static constexpr auto SKIP_ASSIGNMENT = std::numeric_limits<std::size_t>::max();
    static constexpr auto REQUIRES_INDIVIDUAL_ASSIGNMENT = SKIP_ASSIGNMENT - 1;

    template <template <class> class Predicate>
    static constexpr auto calculate_consecutive_indices() noexcept
    {
        std::array<std::size_t, sizeof...(Types)> consecutive_indices{((void)I, SKIP_ASSIGNMENT)...};
        [[maybe_unused]] std::size_t index = 0;
        (
            [&] {
                if constexpr (Predicate<typename detail::ParameterTraits<Types>::ValueType>::value)
                {
                    consecutive_indices[index] = I;
                }
                else
                {
                    index = I + 1;
                    consecutive_indices[I] = REQUIRES_INDIVIDUAL_ASSIGNMENT;
                }
            }(),
            ...);
        return consecutive_indices;
    }

    static constexpr std::array CONSECUTIVE_TRIVIALLY_ASSIGNABLE_INDICES{
        calculate_consecutive_indices<std::is_trivially_copy_assignable>(),
        calculate_consecutive_indices<std::is_trivially_move_assignable>()};

    template <class NeedsAlignmentSelector, bool IgnoreAliasing, class... Args>
    CNTGS_RESTRICT_RETURN static std::byte* emplace_at(std::byte* CNTGS_RESTRICT address, const FixedSizes& fixed_sizes,
                                                       Args&&... args)
    {
        ((address =
              detail::ParameterTraits<Types>::template store<NeedsAlignmentSelector::template VALUE<I>, IgnoreAliasing>(
                  std::forward<Args>(args), address, FixedSizeGetter<Types>::template get<I>(fixed_sizes))),
         ...);
        return address;
    }

  public:
    template <std::size_t K>
    using ParameterTraitsAt = typename ListTraits::template ParameterTraitsAt<K>;

    template <class NeedsAlignmentSelector, class... Args>
    CNTGS_RESTRICT_RETURN static std::byte* emplace_at(std::byte* CNTGS_RESTRICT address, const FixedSizes& fixed_sizes,
                                                       Args&&... args)
    {
        return emplace_at<NeedsAlignmentSelector, true>(address, fixed_sizes, std::forward<Args>(args)...);
    }

    template <class NeedsAlignmentSelector, class... Args>
    CNTGS_RESTRICT_RETURN static std::byte* emplace_at_aliased(std::byte* CNTGS_RESTRICT address,
                                                               const FixedSizes& fixed_sizes, Args&&... args)
    {
        return emplace_at<NeedsAlignmentSelector, false>(address, fixed_sizes, std::forward<Args>(args)...);
    }

    template <class NeedsAlignmentSelector = AlignmentSelector,
              template <class> class FixedSizeGetterType = FixedSizeGetter, class FixedSizesType = FixedSizes>
    static auto load_element_at(std::byte* address, const FixedSizesType& fixed_sizes) noexcept
    {
        PointerReturnType result;
        ((std::tie(std::get<I>(result), address) =
              detail::ParameterTraits<Types>::template load<NeedsAlignmentSelector::template VALUE<I>>(
                  address, FixedSizeGetterType<Types>::template get<I>(fixed_sizes))),
         ...);
        return result;
    }

    static constexpr auto calculate_element_size(const FixedSizes& fixed_sizes) noexcept
    {
        std::size_t result{};
        if constexpr (ListTraits::IS_ALL_FIXED_SIZE)
        {
            ((result += detail::ParameterTraits<Types>::guaranteed_size_in_memory(
                            FixedSizeGetter<Types>::template get<I>(fixed_sizes)) +
                        alignment_offset<detail::ParameterTraits<Types>::ALIGNMENT>(result)),
             ...);
        }
        else
        {
            ((result += detail::ParameterTraits<Types>::aligned_size_in_memory(
                            FixedSizeGetter<Types>::template get<I>(fixed_sizes)) +
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

    template <bool UseMove, std::size_t K, class SourceTuple, class TargetTuple>
    static void assign(const SourceTuple& source, const TargetTuple& target)
    {
        static constexpr auto INDEX = std::get<K>(std::get<UseMove>(CONSECUTIVE_TRIVIALLY_ASSIGNABLE_INDICES));
        if constexpr (INDEX == REQUIRES_INDIVIDUAL_ASSIGNMENT)
        {
            if constexpr (UseMove)
            {
                ParameterTraitsAt<K>::move(std::get<K>(source), std::get<K>(target));
            }
            else
            {
                ParameterTraitsAt<K>::copy(std::get<K>(source), std::get<K>(target));
            }
        }
        else if constexpr (INDEX != SKIP_ASSIGNMENT)
        {
            const auto target_start = ParameterTraitsAt<K>::start_address(std::get<K>(target));
            const auto source_start = ParameterTraitsAt<K>::start_address(std::get<K>(source));
            const auto source_end = ParameterTraitsAt<INDEX>::end_address(std::get<INDEX>(source));
            std::memcpy(target_start, source_start, source_end - source_start);
        }
    }

    template <bool UseMove, class SourceTuple, class TargetTuple>
    static void assign(const SourceTuple& source, const TargetTuple& target)
    {
        (assign<UseMove, I>(source, target), ...);
    }

    static void destruct(const ReferenceReturnType& element) noexcept
    {
        (detail::ParameterTraits<Types>::destroy(std::get<I>(element.tuple)), ...);
    }
};

template <class... Types>
using ElementTraitsT = detail::ElementTraits<std::make_index_sequence<sizeof...(Types)>, Types...>;
}  // namespace cntgs::detail
