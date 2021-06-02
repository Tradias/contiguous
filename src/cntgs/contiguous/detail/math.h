#pragma once

#include <cstddef>
#include <limits>

namespace cntgs::detail
{
template <std::size_t... I>
constexpr auto max_size_t_of() noexcept
{
    std::size_t maximum{};
    ((maximum = I > maximum ? I : maximum), ...);
    return maximum;
}

template <std::size_t... I>
static constexpr auto MAX_SIZE_T_OF = detail::max_size_t_of<I...>();

template <std::size_t... I>
constexpr auto min_size_t_of() noexcept
{
    std::size_t minimum{std::numeric_limits<std::size_t>::max()};
    ((minimum = I < minimum ? I : minimum), ...);
    return minimum;
}

template <std::size_t... I>
static constexpr auto MIN_SIZE_T_OF = detail::min_size_t_of<I...>();
}  // namespace cntgs::detail