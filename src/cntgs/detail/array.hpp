// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_DETAIL_ARRAY_HPP
#define CNTGS_DETAIL_ARRAY_HPP

#include <array>
#include <cstddef>
#include <utility>

namespace cntgs::detail
{
template <class T, std::size_t N>
struct Array
{
    std::array<T, N> array_;
};

template <class T>
struct Array<T, 0>
{
    Array() = default;

    constexpr explicit Array(const std::array<T, 0>&) noexcept {}
};

template <std::size_t I, class T, std::size_t N>
constexpr decltype(auto) get(const detail::Array<T, N>& array) noexcept
{
    return std::get<I>(array.array_);
}

template <std::size_t, class T>
constexpr auto get(const detail::Array<T, 0>&) noexcept
{
    return T{};
}

template <std::size_t N, class T, std::size_t K, std::size_t... I>
constexpr auto convert_array_to_size(const detail::Array<T, K>& array, std::index_sequence<I...>)
{
    return detail::Array<T, N>{detail::get<I>(array)...};
}

template <std::size_t N, class T, std::size_t K>
constexpr auto convert_array_to_size(const detail::Array<T, K>& array)
{
    return detail::convert_array_to_size<N>(array, std::make_index_sequence<(std::min)(N, K)>{});
}
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_ARRAY_HPP
