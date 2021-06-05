#pragma once

#include "cntgs/contiguous/detail/forward.h"
#include "cntgs/contiguous/detail/parameterTraits.h"
#include "cntgs/contiguous/detail/tupleQualifier.h"
#include "cntgs/contiguous/detail/utility.h"

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
using ToContiguousTupleOfValueReturnType = typename detail::TransformTuple<ToContiguousValue, T>::Type;

template <class T>
using ToContiguousReference = typename detail::ParameterTraits<T>::ReferenceReturnType;

template <class T>
using ToContiguousTupleOfReferenceReturnType = typename detail::TransformTuple<ToContiguousReference, T>::Type;

template <class T>
using ToContiguousConstReference = typename detail::ParameterTraits<T>::ConstReferenceReturnType;

template <class T>
using ToContiguousTupleOfConstReferenceReturnType =
    typename detail::TransformTuple<ToContiguousConstReference, T>::Type;

template <class T>
using ToContiguousPointer = typename detail::ParameterTraits<T>::PointerReturnType;

template <class T>
using ToContiguousTupleOfPointerReturnType = typename detail::TransformTuple<ToContiguousPointer, T>::Type;

template <class Result, class... T, std::size_t... I>
constexpr auto convert_tuple_to(const std::tuple<T...>& tuple_of_pointer, std::index_sequence<I...>) noexcept
{
    return Result{detail::dereference(std::get<I>(tuple_of_pointer))...};
}

template <class Result, class... T>
constexpr auto convert_tuple_to(const std::tuple<T...>& tuple_of_pointer) noexcept
{
    return convert_tuple_to<Result>(tuple_of_pointer, std::make_index_sequence<sizeof...(T)>{});
}

template <std::size_t I, class Tuple>
constexpr decltype(auto) extract(const Tuple& tuple) noexcept
{
    return std::get<I>(tuple);
}

template <std::size_t I, class Tuple>
constexpr decltype(auto) extract(Tuple& tuple) noexcept
{
    return std::move(std::get<I>(tuple));
}
}  // namespace cntgs::detail