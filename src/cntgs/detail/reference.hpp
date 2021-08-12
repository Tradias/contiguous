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
template <std::size_t I, bool IsConst, class... Parameter>
struct tuple_element<I, ::cntgs::BasicContiguousReference<IsConst, Parameter...>>
    : std::tuple_element<I, ::cntgs::detail::ToTupleOfContiguousReferences<IsConst, Parameter...>>
{
};

template <bool IsConst, class... Parameter>
struct tuple_size<::cntgs::BasicContiguousReference<IsConst, Parameter...>>
    : std::integral_constant<std::size_t, sizeof...(Parameter)>
{
};
}  // namespace std

namespace cntgs
{
template <std::size_t I, bool IsConst, class... Parameter>
[[nodiscard]] constexpr std::tuple_element_t<I, cntgs::BasicContiguousReference<IsConst, Parameter...>> get(
    const cntgs::BasicContiguousReference<IsConst, Parameter...>& reference) noexcept;
}  // namespace cntgs

#endif  // CNTGS_DETAIL_REFERENCE_HPP
