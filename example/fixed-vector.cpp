// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <cntgs/contiguous.hpp>

#include <cassert>
#include <vector>

int main()
{
    // begin-snippet: fixed-vector-definition
    using Vector = cntgs::ContiguousVector<cntgs::FixedSize<float>,     //
                                           cntgs::FixedSize<uint32_t>,  //
                                           float>;
    // end-snippet

    const auto initial_capacity = 1;
    const auto first_object_count = 3;
    const auto second_object_count = 5;

    // begin-snippet: fixed-vector-construction
    cntgs::ContiguousVector<cntgs::FixedSize<float>,     //
                            cntgs::FixedSize<uint32_t>,  //
                            float>
        vector{initial_capacity, {first_object_count, second_object_count}};
    // end-snippet

    assert(initial_capacity == vector.capacity());
    assert(0 == vector.size());

    // begin-snippet: fixed-vector-emplace_back
    std::vector first{1.f, 2.f, 3.f};
    std::vector second{10u, 20u, 30u, 40u, 50u};
    vector.emplace_back(first, second.begin(), 0.f);
    // end-snippet

    auto&& [firsts, seconds, the_uint] = vector[0];

    assert(8 == std::addressof(the_uint) - firsts.data());

    assert(first_object_count == vector.get_fixed_size<0>());
    assert(second_object_count == vector.get_fixed_size<1>());

    return 0;
}