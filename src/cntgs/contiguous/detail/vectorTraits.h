#pragma once

#include "cntgs/contiguous/detail/forward.h"
#include "cntgs/contiguous/detail/memory.h"
#include "cntgs/contiguous/detail/tuple.h"

#include <tuple>

namespace cntgs::detail
{
template <class T>
struct VectorTraits
{
};

template <class... Types>
struct VectorTraits<cntgs::ContiguousVector<Types...>>
{
    using ValueReturnType = cntgs::ContiguousElement<Types...>;
    using ReferenceReturnType = cntgs::ContiguousTuple<detail::ContiguousTupleQualifier::REFERENCE, Types...>;
    using ConstReferenceReturnType =
        cntgs::ContiguousTuple<detail::ContiguousTupleQualifier::CONST_REFERENCE, Types...>;
    using PointerReturnType = detail::ToContiguousTupleOfPointerReturnType<std::tuple<Types...>>;
    using StorageType = detail::MaybeOwnedPtr<std::byte[]>;
};

template <class... Types>
using ContiguousVectorTraits = detail::VectorTraits<cntgs::ContiguousVector<Types...>>;
}  // namespace cntgs::detail