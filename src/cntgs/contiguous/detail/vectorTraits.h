#pragma once

#include "cntgs/contiguous/detail/forward.h"
#include "cntgs/contiguous/detail/memory.h"
#include "cntgs/contiguous/detail/tuple.h"

#include <tuple>

namespace cntgs::detail
{
template <class... Types>
struct ContiguousVectorTraits
{
    using ReferenceReturnType = cntgs::ContiguousTuple<detail::ContiguousTupleQualifier::REFERENCE, Types...>;
    using ConstReferenceReturnType =
        cntgs::ContiguousTuple<detail::ContiguousTupleQualifier::CONST_REFERENCE, Types...>;
    using PointerReturnType = detail::ToContiguousTupleOfPointerReturnType<std::tuple<Types...>>;
};
}  // namespace cntgs::detail