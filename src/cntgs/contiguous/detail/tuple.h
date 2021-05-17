#pragma once

#include "cntgs/contiguous/detail/traits.h"

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
    std::conditional_t<detail::IS_CONTIGUOUS<T>, typename detail::ContiguousTraits<T>::ReturnType, T>;

template <class T>
using ToContiguousTupleOfValueReturnTypes = typename TransformTuple<ToContiguousValue, T>::Type;

template <class T>
using ToContiguousReference =
    std::conditional_t<detail::IS_CONTIGUOUS<T>, typename detail::ContiguousTraits<T>::ReturnType,
                       std::add_lvalue_reference_t<T>>;

template <class T>
using ToContiguousTupleOfReferenceReturnTypes = typename TransformTuple<ToContiguousReference, T>::Type;

template <class T>
using ToContiguousPointer = std::conditional_t<detail::IS_CONTIGUOUS<T>,
                                               typename detail::ContiguousTraits<T>::ReturnType, std::add_pointer_t<T>>;

template <class T>
using ToContiguousTupleOfPointerReturnTypes = typename TransformTuple<ToContiguousPointer, T>::Type;

template <class T, class U>
struct ContiguousTupleProperties
{
};

template <class T, std::size_t... I>
struct ContiguousTupleProperties<T, std::index_sequence<I...>>
{
    static constexpr auto SIZE_IN_MEMORY =
        (std::size_t(0) + ... + detail::ContiguousTraits<std::tuple_element_t<I, T>>::SIZE_IN_MEMORY);
    static constexpr auto CONTIGUOUS_COUNT =
        (std::size_t(0) + ... + detail::ContiguousTraits<std::tuple_element_t<I, T>>::IS_CONTIGUOUS);
    static constexpr auto CONTIGUOUS_FIXED_SIZE_COUNT =
        (std::size_t(0) + ... + detail::ContiguousTraits<std::tuple_element_t<I, T>>::IS_FIXED_SIZE);
};

template <class T>
using ContiguousTuplePropertiesT = detail::ContiguousTupleProperties<T, std::make_index_sequence<std::tuple_size_v<T>>>;
}  // namespace cntgs::detail