// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_DETAIL_TUPLE_HPP
#define CNTGS_DETAIL_TUPLE_HPP

#include "cntgs/detail/parameterTraits.hpp"
#include "cntgs/detail/typeUtils.hpp"

#include <tuple>

namespace cntgs::detail
{
template <class... Types>
using ToTupleOfContiguousPointer = std::tuple<typename detail::ParameterTraits<Types>::PointerType...>;

template <bool IsConst, class... Types>
using ToTupleOfContiguousReferences =
    detail::ConditionalT<IsConst, std::tuple<typename detail::ParameterTraits<Types>::ConstReferenceType...>,
                         std::tuple<typename detail::ParameterTraits<Types>::ReferenceType...>>;
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_TUPLE_HPP
