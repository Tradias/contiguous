// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <cntgs/contiguous.hpp>

#include <cassert>
#include <memory>
#include <vector>

int main()
{
    const auto initial_capacity = 1;
    const auto varying_object_count = 2;
    const auto fixed_object_count = 2;

    // begin-snippet: mixed-vector-construction
    cntgs::ContiguousVector<cntgs::FixedSize<std::unique_ptr<int32_t>>,  //
                            cntgs::VaryingSize<std::unique_ptr<uint32_t>>>
        vector{initial_capacity, varying_object_count * sizeof(std::unique_ptr<uint32_t>), {fixed_object_count}};
    // end-snippet

    assert(initial_capacity == vector.capacity());
    assert(0 == vector.size());

    // begin-snippet: mixed-vector-emplace_back
    std::vector first{std::make_unique<int32_t>(1u), std::make_unique<int32_t>(2u)};
    std::vector second{std::make_unique<int32_t>(10), std::make_unique<int32_t>(20)};
    vector.emplace_back(std::make_move_iterator(first.begin()), std::move(second));
    // end-snippet

    // begin-snippet: mixed-vector-get
    auto&& objects_of_first_parameter = cntgs::get<0>(vector.front());
    // end-snippet

    assert((std::is_same_v<cntgs::Span<std::unique_ptr<int32_t>>, decltype(objects_of_first_parameter)>));
    assert(1u == *objects_of_first_parameter.front());

    return 0;
}