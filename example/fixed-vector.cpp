// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <cntgs/contiguous.hpp>

#include <array>
#include <cassert>

int main()
{
    // begin-snippet: fixed-vector
    const auto first_object_count = 3;
    const auto second_object_count = 5;

    using Vector = cntgs::ContiguousVector<cntgs::FixedSize<float>,     //
                                           cntgs::FixedSize<uint32_t>,  //
                                           float>;
    Vector vector{1, {first_object_count, second_object_count}};

    std::array first{1.f, 2.f, 3.f};
    std::array second{10u, 20u, 30u, 40u, 50u};
    vector.emplace_back(first, second.begin(), 0.f);

    auto&& [firsts, seconds, the_uint] = vector[0];
    
    assert(8 == std::addressof(the_uint) - firsts.data());

    assert(first_object_count == vector.get_fixed_size<0>());
    assert(second_object_count == vector.get_fixed_size<1>());
    // end-snippet

    return 0;
}