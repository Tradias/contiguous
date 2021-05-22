#pragma once

#include "cntgs/contiguous/detail/forward.h"
#include "cntgs/contiguous/detail/parameterTraits.h"

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
using ToContiguousValue = typename detail::ParameterTraits<T>::ValueReturnType;

template <class T>
using ToContiguousTupleOfValueReturnType = typename TransformTuple<ToContiguousValue, T>::Type;

template <class T>
using ToContiguousReference = typename detail::ParameterTraits<T>::ReferenceReturnType;

template <class T>
using ToContiguousTupleOfReferenceReturnType = typename TransformTuple<ToContiguousReference, T>::Type;

template <class T>
using ToContiguousConstReference = typename detail::ParameterTraits<T>::ConstReferenceReturnType;

template <class T>
using ToContiguousTupleOfConstReferenceReturnType = typename TransformTuple<ToContiguousConstReference, T>::Type;

template <class T>
using ToContiguousPointer = typename detail::ParameterTraits<T>::PointerReturnType;

template <class T>
using ToContiguousTupleOfPointerReturnType = typename TransformTuple<ToContiguousPointer, T>::Type;
}  // namespace cntgs::detail