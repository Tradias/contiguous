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
static constexpr auto MAX_SIZE_T_OF = detail::max_size_t_of<I...>();
}  // namespace cntgs::detail