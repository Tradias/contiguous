#pragma once

#include "cntgs/contiguous/detail/elementLocator.h"
#include "cntgs/contiguous/detail/forward.h"
#include "cntgs/contiguous/detail/parameterListTraits.h"
#include "cntgs/contiguous/detail/typeUtils.h"
#include "cntgs/contiguous/detail/vectorTraits.h"

namespace cntgs::detail
{
template <class... Types>
struct ParameterParser;

template <class... Types, class... Option>
struct ParameterParser<detail::TypeList<Types...>, cntgs::Options<Option...>>
{
    using VectorType = cntgs::BasicContiguousVector<cntgs::Options<Option...>, Types...>;
    using ListTraits = detail::ParameterListTraits<Types...>;
    using VectorTraits = detail::ContiguousVectorTraits<Types...>;
    using ElementLocator = detail::ElementLocatorT<Types...>;
};

template <class... Types>
struct ParameterParser<detail::TypeList<Types...>> : ParameterParser<detail::TypeList<Types...>, cntgs::Options<>>
{
};

template <class... T, class U, class... Types>
struct ParameterParser<detail::TypeList<T...>, U, Types...> : ParameterParser<detail::TypeList<T..., U>, Types...>
{
};

template <class... Types>
using ParameterParserT = detail::ParameterParser<detail::TypeList<>, Types...>;
}  // namespace cntgs::detail