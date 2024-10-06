// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "utils/check.hpp"
#include "utils/doctest.hpp"
#include "utils/fixture.hpp"
#include "utils/testMemoryResource.hpp"

#include <cntgs/contiguous.hpp>

namespace test_vector_special_member
{
using namespace cntgs;
using namespace test;

TEST_CASE("ContiguousVector: OneVaryingUniquePtr move assign empty vector")
{
    auto expected = varying_vector_of_unique_ptrs();
    decltype(expected) vector{};
    vector = std::move(expected);
    CHECK_EQ(10, *cntgs::get<0>(vector[0]).front());
}

TEST_CASE("ContiguousVector: OneVaryingUniquePtr with polymorphic_allocator move assignment")
{
    auto check_expected = [](auto&& vector)
    {
        check_equal_using_get(vector[0], array_two_unique_ptr(10, 20), 30);
        check_equal_using_get(vector[1], array_one_unique_ptr(40), 50);
    };
    TestPmrMemoryResource resource;
    auto vector = varying_vector_of_unique_ptrs(resource.get_allocator());
    SUBCASE("move into smaller vector equal allocator")
    {
        decltype(vector) vector2{0, 0, resource.get_allocator()};
        vector2 = std::move(vector);
        resource.check_was_used(vector2.get_allocator());
        check_expected(vector2);
    }
    SUBCASE("move into larger vector equal allocator")
    {
        decltype(vector) vector2{3, 10, resource.get_allocator()};
        vector2 = std::move(vector);
        resource.check_was_used(vector2.get_allocator());
        check_expected(vector2);
    }
    TestPmrMemoryResource resource2;
#ifdef NDEBUG
    SUBCASE("move into smaller vector")
    {
        decltype(vector) vector2{0, 0, resource2.get_allocator()};
        vector2 = std::move(vector);
        resource2.check_was_not_used(vector2.get_allocator());
        check_expected(vector2);
    }
#endif
    SUBCASE("move into larger vector")
    {
        decltype(vector) vector2{3, 10, resource2.get_allocator()};
        vector2 = std::move(vector);
        resource2.check_was_used(vector2.get_allocator());
        check_expected(vector2);
    }
}

TEST_CASE("ContiguousVector: OneFixedUniquePtr with polymorphic_allocator move assignment")
{
    TestPmrMemoryResource resource;
    auto vector = fixed_vector_of_unique_ptrs(resource.get_allocator());
    TestPmrMemoryResource resource2;
    SUBCASE("move into smaller vector")
    {
        decltype(vector) vector2{0, {}, resource2.get_allocator()};
        vector2 = std::move(vector);
        resource2.check_was_used(vector2.get_allocator());
        check_equal_using_get(vector2[0], array_one_unique_ptr(10), 20);
    }
    SUBCASE("move into larger vector")
    {
        decltype(vector) vector2{3, {1}, resource2.get_allocator()};
        vector2 = std::move(vector);
        resource2.check_was_used(vector2.get_allocator());
        check_equal_using_get(vector2[0], array_one_unique_ptr(10), 20);
    }
}

TEST_CASE("ContiguousVector: OneVaryingString copy construct")
{
    auto vector = varying_vector_of_strings();
    decltype(vector) vector2{vector};
    CHECK_EQ(vector, vector2);
}

TEST_CASE("ContiguousVector: OneVaryingString copy assign empty vector")
{
    auto expected = varying_vector_of_strings();
    decltype(expected) vector{};
    vector = expected;
    CHECK_EQ(expected, vector);
}

TEST_CASE("ContiguousVector: OneVaryingString with std::allocator copy assignment")
{
    auto vector = varying_vector_of_strings();
    SUBCASE("copy into smaller vector")
    {
        decltype(vector) vector2{0, 0};
        vector2 = vector;
        CHECK_EQ(vector, vector2);
    }
    SUBCASE("copy into larger vector")
    {
        decltype(vector) vector2{3, 10};
        vector2 = vector;
        CHECK_EQ(vector, vector2);
    }
}

TEST_CASE("ContiguousVector: OneVaryingString with polymorphic_allocator copy assignment")
{
    TestPmrMemoryResource resource;
    auto vector = varying_vector_of_strings(resource.get_allocator());
    TestPmrMemoryResource resource2;
    SUBCASE("copy into smaller vector")
    {
        decltype(vector) vector2{0, 0, resource2.get_allocator()};
        vector2 = vector;
        resource2.check_was_used(vector2.get_allocator());
        CHECK_EQ(vector, vector2);
    }
    SUBCASE("copy into larger vector")
    {
        decltype(vector) vector2{3, 10, resource2.get_allocator()};
        vector2 = vector;
        CHECK_EQ(&resource2.resource, vector2.get_allocator().resource());
        CHECK_EQ(vector, vector2);
    }
}

TEST_CASE("ContiguousVector: OneFixedString copy construct")
{
    auto vector = fixed_vector_of_strings();
    decltype(vector) vector2{vector};
    CHECK_EQ(vector, vector2);
    CHECK_EQ(2, vector2.get_fixed_size<0>());
}

TEST_CASE("ContiguousVector: OneFixedString copy assign empty vector")
{
    auto expected = fixed_vector_of_strings();
    decltype(expected) vector{};
    vector = expected;
    CHECK_EQ(expected, vector);
}

TEST_CASE("ContiguousVector: OneFixedString with std::allocator copy assignment")
{
    auto vector = fixed_vector_of_strings();
    SUBCASE("copy into smaller vector")
    {
        decltype(vector) vector2{0, {}};
        vector2 = vector;
        CHECK_EQ(vector, vector2);
    }
    SUBCASE("copy into larger vector")
    {
        decltype(vector) vector2{3, {3}};
        vector2 = vector;
        CHECK_EQ(vector, vector2);
        CHECK_EQ(2, vector2.get_fixed_size<0>());
    }
}

TEST_CASE("ContiguousVector: OneFixedString with polymorphic_allocator copy assignment")
{
    TestPmrMemoryResource resource;
    auto vector = fixed_vector_of_strings(resource.get_allocator());
    TestPmrMemoryResource resource2;
    SUBCASE("copy into smaller vector")
    {
        decltype(vector) vector2{0, {}, resource2.get_allocator()};
        vector2 = vector;
        resource2.check_was_used(vector2.get_allocator());
        CHECK_EQ(vector, vector2);
    }
    SUBCASE("copy into larger vector")
    {
        decltype(vector) vector2{3, {3}, resource2.get_allocator()};
        vector2 = vector;
        CHECK_EQ(&resource2.resource, vector2.get_allocator().resource());
        CHECK_EQ(vector, vector2);
        CHECK_EQ(2, vector2.get_fixed_size<0>());
    }
}
}  // namespace test_vector_special_member
