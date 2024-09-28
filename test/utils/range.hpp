// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_UTILS_RANGE_HPP
#define CNTGS_UTILS_RANGE_HPP

#include <algorithm>
#include <type_traits>

namespace test
{
template <class, class = void>
inline constexpr bool IS_RANGE = false;

template <class T>
inline constexpr bool
    IS_RANGE<T, std::void_t<decltype(std::begin(std::declval<T&>())), decltype(std::end(std::declval<T&>()))>> = true;

template <class Lhs, class Rhs, class Comp = std::equal_to<>>
auto range_equal(Lhs&& lhs, Rhs&& rhs, Comp&& comp = {})
{
    return std::equal(std::begin(lhs), std::end(lhs), std::begin(rhs), std::end(rhs), std::forward<Comp>(comp));
}
}  // namespace test

#endif  // CNTGS_UTILS_RANGE_HPP
