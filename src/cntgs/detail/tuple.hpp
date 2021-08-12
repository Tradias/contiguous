// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_DETAIL_TUPLE_HPP
#define CNTGS_DETAIL_TUPLE_HPP

#include "cntgs/detail/parameterTraits.hpp"
#include "cntgs/detail/typeTraits.hpp"

#include <tuple>

namespace cntgs::detail
{
template <class... Parameter>
using ToTupleOfContiguousPointer = std::tuple<typename detail::ParameterTraits<Parameter>::PointerType...>;

template <bool IsConst, class... Parameter>
using ToTupleOfContiguousReferences =
    detail::ConditionalT<IsConst, std::tuple<typename detail::ParameterTraits<Parameter>::ConstReferenceType...>,
                         std::tuple<typename detail::ParameterTraits<Parameter>::ReferenceType...>>;
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_TUPLE_HPP
