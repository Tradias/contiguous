#pragma once

#include "cntgs/contiguous/detail/math.h"

#include <array>
#include <cstddef>
#include <utility>

namespace cntgs::detail
{
template <std::size_t N, class T, std::size_t K, std::size_t... I>
auto convert_array_to_size(std::array<T, K>& array, std::index_sequence<I...>)
{
    return std::array<T, N>{std::move(std::get<I>(array))...};
}

template <std::size_t N, class T, std::size_t K>
auto convert_array_to_size(std::array<T, K>& array)
{
    return detail::convert_array_to_size<N>(array, std::make_index_sequence<detail::min_size_t_of<N, K>()>{});
}
}  // namespace cntgs::detail