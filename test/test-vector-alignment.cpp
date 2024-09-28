// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "utils/check.hpp"
#include "utils/doctest.hpp"
#include "utils/fixture.hpp"
#include "utils/typedefs.hpp"

#include <cntgs/contiguous.hpp>

#include <array>

namespace test_vector_alignment
{
using namespace cntgs;
using namespace test;

TEST_CASE("ContiguousVector: PlainAligned size() and capacity()")
{
    PlainAligned v{2};
    v.emplace_back('a', 10u);
    check_size1_and_capacity2(v);
}

TEST_CASE("ContiguousVector: OneVaryingAligned size() and capacity()")
{
    OneVaryingAligned v{2, FLOATS1.size() * sizeof(float)};
    v.emplace_back(FLOATS1, 10u);
    check_size1_and_capacity2(v);
}

TEST_CASE("ContiguousVector: TwoVaryingAligned size() and capacity()")
{
    TwoVaryingAligned v{2, FLOATS1.size() * sizeof(float) + FLOATS2.size() * sizeof(float)};
    v.emplace_back(10u, FLOATS1, FLOATS2);
    check_size1_and_capacity2(v);
}

TEST_CASE("ContiguousVector: OneFixedAligned size(), capacity() and memory_consumption()")
{
    OneFixedAligned v{2, {FLOATS1.size()}};
    v.emplace_back(10u, FLOATS1);
    check_size1_and_capacity2(v);
    CHECK_EQ(2 * (32 + FLOATS1.size() * sizeof(float)), v.memory_consumption());
}

TEST_CASE("ContiguousVector: TwoFixedAligned size() and capacity()")
{
    TwoFixedAligned v{2, {FLOATS1.size(), FLOATS2.size()}};
    v.emplace_back(FLOATS1, 10u, FLOATS2);
    check_size1_and_capacity2(v);
    const auto size = (FLOATS1.size() * sizeof(float) + 8 + sizeof(uint32_t) + FLOATS2.size() * sizeof(float));
    CHECK_EQ(2 * (size + size % 8) + 7, v.memory_consumption());
}

TEST_CASE("ContiguousVector: OneFixedOneVaryingAligned size() and capacity()")
{
    OneFixedOneVaryingAligned v{2, FLOATS2.size() * sizeof(float), {FLOATS1.size()}};
    v.emplace_back(FLOATS1, 10u, FLOATS2);
    check_size1_and_capacity2(v);
}

TEST_CASE("ContiguousVector: PlainAligned emplace_back() and subscript operator")
{
    PlainAligned vector{5};
    for (uint32_t i = 0; i < 5; ++i)
    {
        vector.emplace_back('a', i);
    }
    for (uint32_t i = 0; i < 5; ++i)
    {
        check_equal_using_get(vector[i], 'a', i);
        auto&& [a, b] = vector[i];
        check_alignment(&b, 8);
    }
}

TEST_CASE("ContiguousVector: OneVaryingAligned emplace_back() and subscript operator")
{
    OneVaryingAligned vector{5, 5 * FLOATS1.size() * sizeof(float)};
    for (uint32_t i = 0; i < 5; ++i)
    {
        vector.emplace_back(FLOATS1, i);
    }
    for (uint32_t i = 0; i < 5; ++i)
    {
        check_equal_using_get(vector[i], FLOATS1, i);
        auto&& [a, b] = vector[i];
        check_alignment(a, 16);
    }
}

TEST_CASE("ContiguousVector: TwoVaryingAligned emplace_back() and subscript operator")
{
    TwoVaryingAligned vector{5, 5 * (FLOATS1.size() * sizeof(float) + FLOATS2.size() * sizeof(float))};
    for (uint32_t i = 0; i < 5; ++i)
    {
        vector.emplace_back(i, FLOATS1, FLOATS2);
    }
    for (uint32_t i = 0; i < 5; ++i)
    {
        check_equal_using_get(vector[i], i, FLOATS1, FLOATS2);
        auto&& [a, b, c] = vector[i];
        check_alignment(b, 8);
        check_alignment(c, 16);
    }
}

TEST_CASE("ContiguousVector: OneFixedAligned emplace_back() and subscript operator")
{
    OneFixedAligned vector{5, {FLOATS1.size()}};
    for (uint32_t i = 0; i < 5; ++i)
    {
        vector.emplace_back(i, FLOATS1);
    }
    for (uint32_t i = 0; i < 5; ++i)
    {
        check_equal_using_get(vector[i], i, FLOATS1);
        auto&& [a, b] = vector[i];
        check_alignment(b, 32);
    }
}

TEST_CASE("ContiguousVector: TwoFixedAligned emplace_back() and subscript operator")
{
    TwoFixedAligned vector{5, {FLOATS1.size(), FLOATS2.size()}};
    for (uint32_t i = 0; i < 5; ++i)
    {
        vector.emplace_back(FLOATS1, i, FLOATS2);
    }
    for (uint32_t i = 0; i < 5; ++i)
    {
        check_equal_using_get(vector[i], FLOATS1, i, FLOATS2);
        auto&& [a, b, c] = vector[i];
        check_alignment(a, 8);
        check_alignment(&b, 16);
    }
}

TEST_CASE("ContiguousVector: TwoFixedAlignedAlt emplace_back() and subscript operator")
{
    std::array uint2{50u, 100u, 150u};
    TwoFixedAlignedAlt vector{5, {FLOATS1.size(), uint2.size()}};
    for (uint32_t i = 0; i < 5; ++i)
    {
        vector.emplace_back(FLOATS1, uint2, i);
    }
    for (uint32_t i = 0; i < 5; ++i)
    {
        check_equal_using_get(vector[i], FLOATS1, uint2, i);
        auto&& [a, b, c] = vector[i];
        check_alignment(a, 32);
    }
}

TEST_CASE("ContiguousVector: OneFixedOneVaryingAligned emplace_back() and subscript operator")
{
    OneFixedOneVaryingAligned vector{5, 5 * FLOATS2.size() * sizeof(float), {FLOATS1.size()}};
    for (uint32_t i = 0; i < 5; ++i)
    {
        vector.emplace_back(FLOATS1, i, FLOATS2);
    }
    for (uint32_t i = 0; i < 5; ++i)
    {
        check_equal_using_get(vector[i], FLOATS1, i, FLOATS2);
        auto&& [a, b, c] = vector[i];
        check_alignment(a, 16);
        check_alignment(c, 8);
    }
}
}  // namespace test_vector_alignment
