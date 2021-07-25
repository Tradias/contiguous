#ifndef CNTGS_DETAIL_VECTORTRAITS_H
#define CNTGS_DETAIL_VECTORTRAITS_H

#include "cntgs/contiguous/detail/forward.h"
#include "cntgs/contiguous/detail/memory.h"
#include "cntgs/contiguous/detail/tuple.h"

#include <tuple>

namespace cntgs::detail
{
template <class... Types>
struct ContiguousVectorTraits
{
    using ReferenceType = cntgs::ContiguousReference<cntgs::ContiguousReferenceQualifier::MUTABLE, Types...>;
    using ConstReferenceType = cntgs::ContiguousReference<cntgs::ContiguousReferenceQualifier::CONST, Types...>;
    using PointerType = detail::ToTupleOfContiguousPointer<std::tuple<Types...>>;
};
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_VECTORTRAITS_H
