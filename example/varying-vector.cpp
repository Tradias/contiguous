// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <cntgs/contiguous.hpp>

#include <array>
#include <cassert>

int main()
{
    // begin-snippet: varying-vector
    const auto initial_capacity = 2;
    const auto varying_object_count = 5;

    using Vector = cntgs::ContiguousVector<cntgs::VaryingSize<int32_t>,  //
                                           float>;
    Vector vector{initial_capacity, varying_object_count * sizeof(int32_t)};

    assert(initial_capacity == vector.capacity());

    vector.emplace_back(std::array{1, 2}, 10.f);
    vector.emplace_back(std::array{3, 4, 5}, 20.f);

    assert(2 == vector.size());

    auto&& [varying_int, the_float] = vector[0];

    assert((std::is_same_v<cntgs::Span<int32_t>, decltype(varying_int)>));
    assert((std::is_same_v<float&, decltype(the_float)>));
    // end-snippet

    return 0;
}