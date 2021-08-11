// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_DETAIL_REFERENCE_HPP
#define CNTGS_DETAIL_REFERENCE_HPP

#include "cntgs/detail/forward.hpp"
#include "cntgs/detail/parameterTraits.hpp"
#include "cntgs/detail/tuple.hpp"

namespace std
{
template <std::size_t I, bool IsConst, class... Types>
struct tuple_element<I, ::cntgs::BasicContiguousReference<IsConst, Types...>>
    : std::tuple_element<I, ::cntgs::detail::ToTupleOfContiguousReferences<IsConst, Types...>>
{
};

template <bool IsConst, class... Types>
struct tuple_size<::cntgs::BasicContiguousReference<IsConst, Types...>>
    : std::integral_constant<std::size_t, sizeof...(Types)>
{
};
}  // namespace std

namespace cntgs
{
template <std::size_t I, bool IsConst, class... Types>
[[nodiscard]] constexpr std::tuple_element_t<I, cntgs::BasicContiguousReference<IsConst, Types...>> get(
    const cntgs::BasicContiguousReference<IsConst, Types...>& reference) noexcept;
}  // namespace cntgs

#endif  // CNTGS_DETAIL_REFERENCE_HPP
