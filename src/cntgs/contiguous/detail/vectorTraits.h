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
    using ReferenceType = cntgs::ContiguousTuple<detail::ContiguousTupleQualifier::REFERENCE, Types...>;
    using ConstReferenceType = cntgs::ContiguousTuple<detail::ContiguousTupleQualifier::CONST_REFERENCE, Types...>;
    using PointerType = detail::ToTupleOfContiguousPointer<std::tuple<Types...>>;
};
}  // namespace cntgs::detail