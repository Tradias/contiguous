// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_DETAIL_VECTORTRAITS_HPP
#define CNTGS_DETAIL_VECTORTRAITS_HPP

#include "cntgs/detail/forward.hpp"
#include "cntgs/detail/tuple.hpp"

#include <tuple>

namespace cntgs::detail
{
template <class... Parameter>
struct ContiguousVectorTraits
{
    using ReferenceType = cntgs::BasicContiguousReference<false, Parameter...>;
    using ConstReferenceType = cntgs::BasicContiguousReference<true, Parameter...>;
    using PointerType = detail::ToTupleOfContiguousPointer<Parameter...>;
};
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_VECTORTRAITS_HPP
