#ifndef CNTGS_DETAIL_VECTORTRAITS_HPP
#define CNTGS_DETAIL_VECTORTRAITS_HPP

#include "cntgs/contiguous/detail/forward.hpp"
#include "cntgs/contiguous/detail/memory.hpp"
#include "cntgs/contiguous/detail/tuple.hpp"

#include <tuple>

namespace cntgs::detail
{
template <class... Types>
struct ContiguousVectorTraits
{
    using ReferenceType = cntgs::BasicContiguousReference<false, Types...>;
    using ConstReferenceType = cntgs::BasicContiguousReference<true, Types...>;
    using PointerType = detail::ToTupleOfContiguousPointer<std::tuple<Types...>>;
};
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_VECTORTRAITS_HPP
