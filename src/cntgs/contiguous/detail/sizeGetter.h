#pragma once

#include "cntgs/contiguous/detail/parameterTraits.h"
#include "cntgs/contiguous/detail/parameterType.h"
#include "cntgs/contiguous/detail/typeUtils.h"

#include <array>
#include <cstddef>
#include <tuple>

namespace cntgs::detail
{
template <class... Types>
struct FixedSizeGetter
{
    template <std::size_t... I>
    static constexpr auto calculate_fixed_size_indices(std::index_sequence<I...>) noexcept
    {
        std::array<std::size_t, sizeof...(Types)> fixed_size_indices{};
        [[maybe_unused]] std::size_t index = 0;
        (
            [&]
            {
                if constexpr (detail::ParameterTraits<Types>::TYPE == detail::ParameterType::FIXED_SIZE)
                {
                    std::get<I>(fixed_size_indices) = index;
                    ++index;
                }
            }(),
            ...);
        return fixed_size_indices;
    }

    static constexpr auto FIXED_SIZE_INDICES =
        calculate_fixed_size_indices(std::make_index_sequence<sizeof...(Types)>{});

    template <class Type, std::size_t I, std::size_t N>
    static constexpr auto get([[maybe_unused]] const std::array<std::size_t, N>& fixed_sizes) noexcept
    {
        if constexpr (detail::ParameterType::FIXED_SIZE == detail::ParameterTraits<Type>::TYPE)
        {
            return std::get<std::get<I>(FIXED_SIZE_INDICES)>(fixed_sizes);
        }
        else
        {
            return std::size_t{};
        }
    }
};

struct ContiguousTupleSizeGetter
{
    template <class Type, std::size_t I, class... U>
    static constexpr auto get([[maybe_unused]] const std::tuple<U...>& tuple) noexcept
    {
        if constexpr (detail::ParameterType::FIXED_SIZE == detail::ParameterTraits<Type>::TYPE)
        {
            return std::get<I>(tuple).size();
        }
        else
        {
            return std::size_t{};
        }
    }
};
}  // namespace cntgs::detail