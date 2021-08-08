// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_DETAIL_VECTOR_HPP
#define CNTGS_DETAIL_VECTOR_HPP

#include "cntgs/detail/forward.hpp"

namespace cntgs::detail
{
template <class Vector>
struct ContiguousVectorAccess
{
    using ListTraits = typename Vector::ListTraits;

    template <class... Args>
    static auto construct(Args&&... args)
    {
        return Vector{std::forward<Args>(args)...};
    }
};
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_VECTOR_HPP
