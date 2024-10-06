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

namespace test_vector_erase
{
using namespace cntgs;
using namespace test;

TEST_CASE("ContiguousVector: OneFixedUniquePtr erase(Iterator)")
{
    OneFixedUniquePtr vector{2, {1}};
    vector.emplace_back(array_one_unique_ptr(10), std::make_unique<int>(20));
    vector.emplace_back(array_one_unique_ptr(30), std::make_unique<int>(40));
    vector.erase(vector.begin());
    CHECK_EQ(1, vector.size());
    check_equal_using_get(vector.front(), array_one_unique_ptr(30), 40);
}

TEST_CASE("ContiguousVector: OneVaryingUniquePtr erase(Iterator)")
{
    OneVaryingUniquePtr vector{3, 3 * (2 * sizeof(std::unique_ptr<int>))};
    vector.emplace_back(array_one_unique_ptr(10), std::make_unique<int>(20));
    vector.emplace_back(array_two_unique_ptr(30, 40), std::make_unique<int>(50));
    vector.emplace_back(array_two_unique_ptr(60, 70), std::make_unique<int>(80));
    vector.erase(vector.begin());
    CHECK_EQ(2, vector.size());
    check_equal_using_get(vector.front(), array_two_unique_ptr(30, 40), 50);
    check_equal_using_get(vector.back(), array_two_unique_ptr(60, 70), 80);
}

TEST_CASE("ContiguousVector: TwoFixed erase(Iterator)")
{
    TwoFixed vector{4, {FLOATS2.size(), FLOATS2_ALT.size()}};
    vector.emplace_back(FLOATS2, 10u, FLOATS2);
    vector.emplace_back(FLOATS2_ALT, 20u, FLOATS2);
    vector.emplace_back(FLOATS2, 30u, FLOATS2_ALT);
    vector.emplace_back(FLOATS2_ALT, 40u, FLOATS2_ALT);
    auto it = vector.erase(++vector.begin());
    CHECK_EQ(++vector.begin(), it);
    TwoFixed vector2{3, {FLOATS2.size(), FLOATS2_ALT.size()}};
    vector2.emplace_back(FLOATS2, 10u, FLOATS2);
    vector2.emplace_back(FLOATS2, 30u, FLOATS2_ALT);
    vector2.emplace_back(FLOATS2_ALT, 40u, FLOATS2_ALT);
    CHECK_EQ(vector2, vector);
}

TEST_CASE("ContiguousVector: TwoVarying erase(Iterator)")
{
    CheckedSoft<TwoVarying> vector{4, 21 * sizeof(float)};
    vector.emplace_back(10u, FLOATS1, FLOATS2);
    vector.emplace_back(20u, FLOATS2_ALT, FLOATS1);
    vector.emplace_back(30u, FLOATS1, FLOATS2_ALT);
    vector.emplace_back(40u, FLOATS2, FLOATS2_ALT);
    CHECK_EQ(224, vector.memory_consumption());
    auto it = vector.erase(++vector.begin());
    CHECK_EQ(++vector.begin(), it);
    CheckedSoft<TwoVarying> vector2{3, 16 * sizeof(float)};
    vector2.emplace_back(10u, FLOATS1, FLOATS2);
    vector2.emplace_back(30u, FLOATS1, FLOATS2_ALT);
    vector2.emplace_back(40u, FLOATS2, FLOATS2_ALT);
    CHECK_EQ(168, vector2.memory_consumption());
    CHECK_EQ(vector2, vector);
}

TEST_CASE("ContiguousVector: OneFixedUniquePtr erase(Iterator, Iterator)")
{
    OneFixedUniquePtr vector{3, {1}};
    vector.emplace_back(array_one_unique_ptr(10), std::make_unique<int>(20));
    vector.emplace_back(array_one_unique_ptr(30), std::make_unique<int>(40));
    vector.emplace_back(array_one_unique_ptr(50), std::make_unique<int>(60));
    SUBCASE("erase first two")
    {
        auto it = vector.erase(vector.begin(), std::next(vector.begin(), 2));
        CHECK_EQ(vector.begin(), it);
        CHECK_EQ(1, vector.size());
        check_equal_using_get(vector.front(), array_one_unique_ptr(50), 60);
    }
    SUBCASE("erase all")
    {
        auto it = vector.erase(vector.begin(), vector.end());
        CHECK_EQ(vector.end(), it);
        CHECK_EQ(0, vector.size());
    }
    SUBCASE("erase none")
    {
        auto it = vector.erase(vector.begin(), vector.begin());
        CHECK_EQ(vector.begin(), it);
        CHECK_EQ(3, vector.size());
    }
}

TEST_CASE("ContiguousVector: TwoFixed erase(Iterator, Iterator)")
{
    TwoFixed vector{3, {FLOATS2.size(), FLOATS2_ALT.size()}};
    vector.emplace_back(FLOATS2, 10u, FLOATS2);
    vector.emplace_back(FLOATS2, 20u, FLOATS2_ALT);
    vector.emplace_back(FLOATS2_ALT, 30u, FLOATS2);
    SUBCASE("erase first two")
    {
        auto it = vector.erase(vector.begin(), std::next(vector.begin(), 2));
        CHECK_EQ(vector.begin(), it);
        CHECK_EQ(1, vector.size());
        check_equal_using_get(vector.front(), FLOATS2_ALT, 30u, FLOATS2);
    }
    SUBCASE("erase all")
    {
        vector.erase(vector.begin(), vector.end());
        CHECK_EQ(0, vector.size());
        vector.emplace_back(FLOATS2_ALT, 30u, FLOATS2);
        check_equal_using_get(vector.front(), FLOATS2_ALT, 30u, FLOATS2);
    }
    SUBCASE("erase none")
    {
        vector.erase(vector.begin(), vector.begin());
        CHECK_EQ(3, vector.size());
        check_equal_using_get(vector.front(), FLOATS2, 10u, FLOATS2);
    }
}

TEST_CASE("ContiguousVector: TwoVarying erase(Iterator, Iterator)")
{
    TwoVarying vector{4, 60};
    vector.emplace_back(10u, FLOATS1, FLOATS2);
    vector.emplace_back(20u, FLOATS2_ALT, FLOATS1);
    vector.emplace_back(30u, FLOATS1, FLOATS2_ALT);
    SUBCASE("erase first two")
    {
        auto it = vector.erase(vector.begin(), std::next(vector.begin(), 2));
        CHECK_EQ(vector.begin(), it);
        CHECK_EQ(1, vector.size());
        check_equal_using_get(vector.front(), 30u, FLOATS1, FLOATS2_ALT);
    }
    SUBCASE("erase all")
    {
        vector.erase(vector.begin(), vector.end());
        CHECK_EQ(0, vector.size());
        vector.emplace_back(30u, FLOATS2_ALT, FLOATS2);
        check_equal_using_get(vector.front(), 30u, FLOATS2_ALT, FLOATS2);
    }
    SUBCASE("erase none")
    {
        vector.erase(vector.begin(), vector.begin());
        CHECK_EQ(3, vector.size());
        check_equal_using_get(vector.front(), 10u, FLOATS1, FLOATS2);
    }
}
}  // namespace test_vector_erase
