#pragma once

#include "cntgs/contiguous/detail/parameterTraits.h"
#include "cntgs/contiguous/detail/typeUtils.h"

#include <array>
#include <cstddef>

namespace cntgs::detail
{
template <class... T, std::size_t... I>
constexpr auto calculate_fixed_size_indices(detail::TypeList<T...>, std::index_sequence<I...>) noexcept
{
    std::array<std::size_t, sizeof...(T)> fixed_size_indices{};
    [[maybe_unused]] std::size_t index = 0;
    (
        [&] {
            if constexpr (detail::ParameterTraits<T>::IS_FIXED_SIZE)
            {
                std::get<I>(fixed_size_indices) = index;
                ++index;
            }
        }(),
        ...);
    return fixed_size_indices;
}
}  // namespace cntgs::detail