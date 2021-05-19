#pragma once

#include <cstddef>

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
constexpr auto min_size_t_of() noexcept
{
    std::size_t minimum{std::numeric_limits<std::size_t>::max()};
    ((minimum = I < minimum ? I : minimum), ...);
    return minimum;
}
}  // namespace cntgs::detail