#ifndef CNTGS_DETAIL_ARRAY_HPP
#define CNTGS_DETAIL_ARRAY_HPP

#include <algorithm>
#include <array>
#include <cstddef>
#include <utility>

namespace cntgs::detail
{
template <std::size_t N, class T, std::size_t K, std::size_t... I>
constexpr auto convert_array_to_size(const std::array<T, K>& array, std::index_sequence<I...>)
{
    return std::array<T, N>{std::get<I>(array)...};
}

template <std::size_t N, class T, std::size_t K>
constexpr auto convert_array_to_size(const std::array<T, K>& array)
{
    return detail::convert_array_to_size<N>(array, std::make_index_sequence<std::min(N, K)>{});
}
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_ARRAY_HPP
