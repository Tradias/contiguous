// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <cntgs/contiguous.hpp>

#include <array>
#include <cassert>

void fill_vector(cntgs::ContiguousVector<cntgs::FixedSize<float>, cntgs::VaryingSize<uint32_t>>& vector)
{
    vector.emplace_back(std::array{1.f, 2.f}, std::array{1u});
}

int main()
{
    // begin-snippet: type-erased-vector
    using Vector = cntgs::ContiguousVector<cntgs::FixedSize<float>,  //
                                           cntgs::VaryingSize<uint32_t>>;
    Vector vector{1, 2 * sizeof(uint32_t), {1}};
    fill_vector(vector);

    cntgs::TypeErasedVector type_erased_vector = cntgs::type_erase(std::move(vector));

    auto restored = cntgs::restore<Vector>(std::move(type_erased_vector));
    // end-snippet

    assert(1u == cntgs::get<0>(restored[0]).front());

    return 0;
}