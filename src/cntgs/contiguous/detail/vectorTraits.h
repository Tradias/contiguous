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
    using ReferenceType = cntgs::ContiguousReference<detail::ContiguousReferenceQualifier::MUTABLE, Types...>;
    using ConstReferenceType = cntgs::ContiguousReference<detail::ContiguousReferenceQualifier::CONST, Types...>;
    using PointerType = detail::ToTupleOfContiguousPointer<std::tuple<Types...>>;
};
}  // namespace cntgs::detail