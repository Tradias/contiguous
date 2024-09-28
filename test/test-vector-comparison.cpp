// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "utils/doctest.hpp"
#include "utils/typedefs.hpp"

#include <cntgs/contiguous.hpp>

#include <array>
#include <string>
#include <vector>

namespace test_vector_comparison
{
using namespace cntgs;
using namespace test;

template <class Vector1, class Vector2>
void check_equality(Vector1& lhs, Vector2& rhs)
{
    CHECK_EQ(lhs, rhs);
    CHECK_FALSE((lhs < rhs));
    CHECK_LE(lhs, rhs);
    CHECK_FALSE((rhs > lhs));
    CHECK_GE(lhs, rhs);
}

template <class Vector1, class Vector2>
void check_less(Vector1& lhs, Vector2& rhs)
{
    CHECK_NE(lhs, rhs);
    CHECK_LT(lhs, rhs);
    CHECK_LE(lhs, rhs);
    CHECK_GT(rhs, lhs);
    CHECK_GE(rhs, lhs);
}

TEST_CASE("ContiguousVector: ContiguousVector of std::string comparison operators")
{
    using Vector = cntgs::ContiguousVector<cntgs::FixedSize<std::string>, std::string>;
    Vector lhs{2, {1}};
    lhs.emplace_back(std::vector{"a"}, "a");
    SUBCASE("equal")
    {
        Vector rhs{1, {1}};
        rhs.emplace_back(std::vector{"a"}, "a");
        check_equality(lhs, rhs);
    }
    SUBCASE("empty vectors are equal")
    {
        Vector rhs{1, {1}};
        CHECK_NE(lhs, rhs);
        Vector empty{1, {1}};
        CHECK_EQ(empty, rhs);
    }
    SUBCASE("less")
    {
        Vector rhs{1, {1}};
        rhs.emplace_back(std::vector{"b"}, "b");
        check_less(lhs, rhs);
    }
    SUBCASE("empty vectors are less")
    {
        Vector empty{1, {1}};
        CHECK_LT(empty, lhs);
        CHECK_LE(empty, lhs);
    }
    SUBCASE("greater with greater size")
    {
        lhs.emplace_back(std::vector{"a"}, "a");
        Vector rhs{1, {1}};
        rhs.emplace_back(std::vector{"b"}, "a");
        CHECK_NE(lhs, rhs);
        CHECK_FALSE((lhs < rhs));
        CHECK_FALSE((lhs <= rhs));
        CHECK_GT(lhs, rhs);
        CHECK_GE(lhs, rhs);
    }
}

TEST_CASE("ContiguousVector: ContiguousVector of VaryingSize unsigned char comparison operators")
{
    using Vector = cntgs::ContiguousVector<cntgs::VaryingSize<UInt8>>;
    Vector lhs{2, 5};
    lhs.emplace_back(std::array{UInt8{0}, UInt8{1}, UInt8{2}});
    SUBCASE("equal")
    {
        Vector rhs{1, 3};
        rhs.emplace_back(std::array{UInt8{0}, UInt8{1}, UInt8{2}});
        check_equality(lhs, rhs);
    }
    SUBCASE("not equal size")
    {
        Vector rhs{1, 2};
        rhs.emplace_back(std::array{UInt8{0}, UInt8{1}});
        CHECK_NE(lhs, rhs);
    }
    SUBCASE("less")
    {
        Vector rhs{1, 4};
        rhs.emplace_back(std::array{UInt8{0}, UInt8{1}, UInt8{2}, UInt8{3}});
        check_less(lhs, rhs);
    }
    SUBCASE("not equal across varying size boundaries")
    {
        lhs.emplace_back(std::array{UInt8{3}, UInt8{4}});
        Vector rhs{1, 5};
        rhs.emplace_back(std::array{UInt8{0}, UInt8{1}, UInt8{2}, UInt8{3}, UInt8{4}});
        CHECK_NE(lhs, rhs);
    }
}

TEST_CASE("ContiguousVector: ContiguousVector of FixedSize unsigned char comparison operators")
{
    using Vector = cntgs::ContiguousVector<cntgs::FixedSize<UInt8>>;
    Vector empty{0, {3}};
    Vector lhs{2, {3}};
    lhs.emplace_back(std::array{UInt8{0}, UInt8{1}, UInt8{2}});
    SUBCASE("equal")
    {
        Vector rhs{1, {3}};
        rhs.emplace_back(std::array{UInt8{0}, UInt8{1}, UInt8{2}});
        check_equality(lhs, rhs);
    }
    SUBCASE("not equal size")
    {
        Vector rhs{1, {2}};
        rhs.emplace_back(std::array{UInt8{0}, UInt8{1}});
        CHECK_NE(lhs, rhs);
    }
    SUBCASE("less")
    {
        Vector rhs{1, {3}};
        rhs.emplace_back(std::array{UInt8{0}, UInt8{1}, UInt8{3}});
        check_less(lhs, rhs);
    }
    SUBCASE("not equal across fixed size boundaries")
    {
        Vector rhs{2, {3}};
        rhs.emplace_back(std::array{UInt8{0}, UInt8{1}, UInt8{2}});
        rhs.emplace_back(std::array{UInt8{0}, UInt8{1}, UInt8{2}});
        CHECK_NE(lhs, rhs);
    }
    SUBCASE("empty range is not equal to non-empty range")
    {
        Vector rhs{0, {3}};
        CHECK_NE(lhs, rhs);
        CHECK_NE(rhs, lhs);
    }
    SUBCASE("empty range is less than non-empty range")
    {
        Vector rhs{0, {3}};
        CHECK_FALSE((lhs < rhs));
        CHECK_LT(rhs, lhs);
    }
    SUBCASE("empty ranges are equal and not less")
    {
        Vector rhs{0, {3}};
        CHECK_EQ(empty, rhs);
        CHECK_EQ(rhs, empty);
        CHECK_FALSE((empty < rhs));
        CHECK_FALSE((rhs < empty));
    }
}
}  // namespace test_vector_comparison
