// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_DETAIL_RANGE_HPP
#define CNTGS_DETAIL_RANGE_HPP

#include <iterator>
#include <type_traits>
#include <utility>

namespace cntgs::detail
{
template <class, class = void>
inline constexpr bool HAS_DATA_AND_SIZE = false;

template <class T>
inline constexpr bool HAS_DATA_AND_SIZE<
    T, std::void_t<decltype(std::data(std::declval<T&>())), decltype(std::size(std::declval<T&>()))>> = true;

template <class, class = void>
inline constexpr bool IS_RANGE = false;

template <class T>
inline constexpr bool
    IS_RANGE<T, std::void_t<decltype(std::begin(std::declval<T&>())), decltype(std::end(std::declval<T&>()))>> = true;
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_RANGE_HPP
