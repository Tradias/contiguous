// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <cntgs/contiguous.hpp>

#include <cassert>
#include <memory_resource>

int main()
{
    // begin-snippet: pmr-vector-definition
    using Vector = cntgs::BasicContiguousVector<                                       //
        cntgs::Options<cntgs::Allocator<std::pmr::polymorphic_allocator<std::byte>>>,  //
        cntgs::FixedSize<uint32_t>, float>;
    // end-snippet

    const auto initial_capacity = 0;
    const auto fixed_object_count = 3;

    // begin-snippet: pmr-vector-construction
    std::pmr::monotonic_buffer_resource resource;
    Vector vector{initial_capacity, {fixed_object_count}, &resource};
    // end-snippet

    assert(&resource == vector.get_allocator().resource());

    return 0;
}