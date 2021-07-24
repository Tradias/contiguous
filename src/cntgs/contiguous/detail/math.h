#ifndef CNTGS_DETAIL_MATH_H
#define CNTGS_DETAIL_MATH_H

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
inline constexpr auto MAX_SIZE_T_OF = detail::max_size_t_of<I...>();
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_MATH_H
