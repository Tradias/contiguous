#include <cntgs/contiguous.hpp>

#include <array>

using Vector = cntgs::ContiguousVector<cntgs::FixedSize<float>, int>;

int main()
{
    Vector v{10, {2}};
    v.emplace_back(std::array{10.f, 20.f}, 5);
    auto&& [a, b] = v[0];
    return b != 5;
}