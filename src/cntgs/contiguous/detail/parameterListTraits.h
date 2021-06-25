#pragma once

#include "cntgs/contiguous/detail/fixedSizeGetter.h"
#include "cntgs/contiguous/detail/math.h"
#include "cntgs/contiguous/detail/parameterTraits.h"
#include "cntgs/contiguous/detail/parameterType.h"
#include "cntgs/contiguous/detail/utility.h"

#include <array>
#include <cstddef>
#include <tuple>
#include <type_traits>

namespace cntgs::detail
{
#ifdef CNTGS_MAX_FIXED_SIZE_VECTOR_PARAMETER
static constexpr auto MAX_FIXED_SIZE_VECTOR_PARAMETER = CNTGS_MAX_FIXED_SIZE_VECTOR_PARAMETER;
#else
static constexpr auto MAX_FIXED_SIZE_VECTOR_PARAMETER = 15;
#endif

template <class... Types>
struct ParameterListTraits
{
    template <class T>
    using FixedSizeGetter = detail::FixedSizeGetter<T, detail::TypeList<Types...>>;

    template <std::size_t I>
    using ParameterTraitsAt = detail::ParameterTraits<std::tuple_element_t<I, std::tuple<Types...>>>;

    static constexpr auto CONTIGUOUS_COUNT =
        (std::size_t{} + ... + (detail::ParameterTraits<Types>::TYPE != detail::ParameterType::PLAIN));
    static constexpr auto CONTIGUOUS_FIXED_SIZE_COUNT =
        (std::size_t{} + ... + (detail::ParameterTraits<Types>::TYPE == detail::ParameterType::FIXED_SIZE));
    static constexpr auto MAX_ALIGNMENT = detail::MAX_SIZE_T_OF<detail::ParameterTraits<Types>::ALIGNMENT...>;

    static constexpr auto IS_NOTHROW_DESTRUCTIBLE =
        (std::is_nothrow_destructible_v<typename detail::ParameterTraits<Types>::ValueType> && ...);
    static constexpr auto IS_NOTHROW_COPY_ASSIGNABLE =
        (std::is_nothrow_copy_assignable_v<typename detail::ParameterTraits<Types>::ValueType> && ...);
    static constexpr auto IS_NOTHROW_MOVE_ASSIGNABLE =
        (std::is_nothrow_move_assignable_v<typename detail::ParameterTraits<Types>::ValueType> && ...);
    static constexpr auto IS_NOTHROW_SWAPPABLE =
        (std::is_nothrow_swappable_v<typename detail::ParameterTraits<Types>::ValueType> && ...);

    static constexpr auto IS_TRIVIALLY_DESTRUCTIBLE =
        (std::is_trivially_destructible_v<typename detail::ParameterTraits<Types>::ValueType> && ...);
    static constexpr auto IS_TRIVIALLY_COPY_CONSTRUCTIBLE =
        (std::is_trivially_copy_constructible_v<typename detail::ParameterTraits<Types>::ValueType> && ...);
    static constexpr auto IS_TRIVIALLY_MOVE_CONSTRUCTIBLE =
        (std::is_trivially_move_constructible_v<typename detail::ParameterTraits<Types>::ValueType> && ...);
    static constexpr auto IS_TRIVIALLY_COPY_ASSIGNABLE =
        (std::is_trivially_copy_assignable_v<typename detail::ParameterTraits<Types>::ValueType> && ...);
    static constexpr auto IS_TRIVIALLY_MOVE_ASSIGNABLE =
        (std::is_trivially_move_assignable_v<typename detail::ParameterTraits<Types>::ValueType> && ...);

    static constexpr bool IS_MIXED =
        CONTIGUOUS_FIXED_SIZE_COUNT != 0 && CONTIGUOUS_FIXED_SIZE_COUNT != CONTIGUOUS_COUNT;
    static constexpr bool IS_ALL_FIXED_SIZE =
        CONTIGUOUS_FIXED_SIZE_COUNT != 0 && CONTIGUOUS_FIXED_SIZE_COUNT == CONTIGUOUS_COUNT;
    static constexpr bool IS_ALL_VARYING_SIZE = CONTIGUOUS_FIXED_SIZE_COUNT == 0 && CONTIGUOUS_COUNT != 0;
    static constexpr bool IS_NONE_SPECIAL = CONTIGUOUS_COUNT == 0;

    using FixedSizes = std::array<std::size_t, CONTIGUOUS_FIXED_SIZE_COUNT>;

    static_assert(detail::MAX_FIXED_SIZE_VECTOR_PARAMETER > CONTIGUOUS_FIXED_SIZE_COUNT,
                  "Maximum number of FixedSize vector parameter exceeded. Define CNTGS_MAX_FIXED_SIZE_VECTOR_PARAMETER "
                  "to a higher limit.");

    static constexpr auto make_index_sequence() noexcept { return std::make_index_sequence<sizeof...(Types)>{}; }

    static constexpr auto SKIP_ASSIGNMENT = std::numeric_limits<std::size_t>::max();
    static constexpr auto REQUIRES_INDIVIDUAL_ASSIGNMENT = SKIP_ASSIGNMENT - 1;

    template <template <class> class Predicate, std::size_t... I>
    static constexpr auto calculate_consecutive_indices(std::index_sequence<I...>) noexcept
    {
        std::array<std::size_t, sizeof...(Types)> consecutive_indices{((void)I, SKIP_ASSIGNMENT)...};
        std::size_t index = 0;
        (
            [&] {
                if constexpr (Predicate<typename detail::ParameterTraits<Types>::ValueType>{})
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

    static constexpr auto CONSECUTIVE_TRIVIALLY_COPY_ASSIGNABLE_INDICES =
        calculate_consecutive_indices<std::is_trivially_copy_assignable>(make_index_sequence());
    static constexpr auto CONSECUTIVE_TRIVIALLY_MOVE_ASSIGNABLE_INDICES =
        calculate_consecutive_indices<std::is_trivially_move_assignable>(make_index_sequence());
    static constexpr std::array CONSECUTIVE_TRIVIALLY_ASSIGNABLE_INDICES{CONSECUTIVE_TRIVIALLY_COPY_ASSIGNABLE_INDICES,
                                                                         CONSECUTIVE_TRIVIALLY_MOVE_ASSIGNABLE_INDICES};

    template <bool UseMove, std::size_t I, class SourceTuple, class TargetTuple>
    static void assign(const SourceTuple& source, const TargetTuple& target)
    {
        static constexpr auto INDEX = std::get<I>(std::get<UseMove>(CONSECUTIVE_TRIVIALLY_ASSIGNABLE_INDICES));
        if constexpr (INDEX < REQUIRES_INDIVIDUAL_ASSIGNMENT)
        {
            const auto target_start = ParameterTraitsAt<I>::start_address(std::get<I>(target));
            const auto source_start = ParameterTraitsAt<I>::start_address(std::get<I>(source));
            const auto source_end = ParameterTraitsAt<INDEX>::end_address(std::get<INDEX>(source));
            std::memcpy(target_start, source_start, source_end - source_start);
        }
        else if constexpr (INDEX == REQUIRES_INDIVIDUAL_ASSIGNMENT)
        {
            if constexpr (UseMove)
            {
                ParameterTraitsAt<I>::move(std::get<I>(source), std::get<I>(target));
            }
            else
            {
                ParameterTraitsAt<I>::copy(std::get<I>(source), std::get<I>(target));
            }
        }
    }

    template <bool UseMove, class SourceTuple, class TargetTuple, std::size_t... I>
    static void assign(const SourceTuple& source, const TargetTuple& target, std::index_sequence<I...>)
    {
        (assign<UseMove, I>(source, target), ...);
    }
};
}  // namespace cntgs::detail
