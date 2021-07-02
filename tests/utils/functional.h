#pragma once

namespace cntgs::test
{
struct DereferenceEqual
{
    template <class Dereferencable1, class Dereferencable2>
    bool operator()(const Dereferencable1& lhs, const Dereferencable2& rhs)
    {
        return *lhs == *rhs;
    }
};
}  // namespace cntgs::test