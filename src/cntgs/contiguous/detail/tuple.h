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
using ToContiguousValue =
    std::conditional_t<detail::IS_CONTIGUOUS<T>, typename detail::ParameterTraits<T>::ReturnType, T>;

template <class T>
using ToContiguousTupleOfValueReturnType = typename TransformTuple<ToContiguousValue, T>::Type;

template <class T>
using ToContiguousReference =
    std::conditional_t<detail::IS_CONTIGUOUS<T>, typename detail::ParameterTraits<T>::ReturnType,
                       std::add_lvalue_reference_t<T>>;

template <class T>
using ToContiguousConstReference =
    std::conditional_t<detail::IS_CONTIGUOUS<T>, typename detail::ParameterTraits<T>::ConstReturnType,
                       std::add_lvalue_reference_t<std::add_const_t<T>>>;

template <class T>
using ToContiguousTupleOfReferenceReturnType = typename TransformTuple<ToContiguousReference, T>::Type;

template <class T>
using ToContiguousTupleOfConstReferenceReturnType = typename TransformTuple<ToContiguousConstReference, T>::Type;

template <class T>
using ToContiguousPointer = std::conditional_t<detail::IS_CONTIGUOUS<T>,
                                               typename detail::ParameterTraits<T>::ReturnType, std::add_pointer_t<T>>;

template <class T>
using ToContiguousTupleOfPointerReturnType = typename TransformTuple<ToContiguousPointer, T>::Type;
}  // namespace cntgs::detail