#pragma once

#include "cntgs/contiguous/detail/parameterParser.h"

namespace cntgs
{
template <class... Types>
using ContiguousVector = typename detail::ParameterParserT<Types...>::VectorType;
}  // namespace cntgs