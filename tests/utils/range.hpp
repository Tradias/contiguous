#pragma once

#include <algorithm>
#include <type_traits>

namespace cntgs::test
{
template <class Lhs, class Rhs, class Comp = std::equal_to<>>
auto range_equal(Lhs&& lhs, Rhs&& rhs, Comp&& comp = {})
{
    return std::equal(std::begin(lhs), std::end(lhs), std::begin(rhs), std::end(rhs), std::forward<Comp>(comp));
}
}  // namespace cntgs::test