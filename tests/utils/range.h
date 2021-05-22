#pragma once

#include "cntgs/contiguous/detail/range.h"

#include <algorithm>
#include <type_traits>

namespace cntgs::test
{
template <class Lhs, class Rhs>
auto range_equal(Lhs&& lhs, Rhs&& rhs)
{
    return std::equal(std::begin(lhs), std::end(lhs), std::begin(rhs), std::end(rhs));
}
}  // namespace cntgs::test