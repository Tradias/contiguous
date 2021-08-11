// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_DETAIL_TUPLE_HPP
#define CNTGS_DETAIL_TUPLE_HPP

#include "cntgs/detail/forward.hpp"
#include "cntgs/detail/parameterTraits.hpp"
#include "cntgs/detail/utility.hpp"

#include <tuple>
#include <utility>

namespace cntgs::detail
{
template <template <class> class U, class T>
struct TransformTuple
{
};

template <template <class> class Transformer, class... T>
struct TransformTuple<Transformer, std::tuple<T...>>
{
    using Type = std::tuple<Transformer<T>...>;
};

template <class T>
using ToContiguousReference = typename detail::ParameterTraits<T>::ReferenceType;

template <class T>
using ToTupleOfContiguousReference = typename detail::TransformTuple<ToContiguousReference, T>::Type;

template <class T>
using ToContiguousConstReference = typename detail::ParameterTraits<T>::ConstReferenceType;

template <class T>
using ToTupleOfContiguousConstReference = typename detail::TransformTuple<ToContiguousConstReference, T>::Type;

template <class T>
using ToContiguousPointer = typename detail::ParameterTraits<T>::PointerType;

template <class T>
using ToTupleOfContiguousPointer = typename detail::TransformTuple<ToContiguousPointer, T>::Type;
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_TUPLE_HPP
