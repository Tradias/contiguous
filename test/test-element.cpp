// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "utils/check.hpp"
#include "utils/fixture.hpp"
#include "utils/functional.hpp"
#include "utils/noexcept.hpp"
#include "utils/range.hpp"
#include "utils/testMemoryResource.hpp"
#include "utils/typedefs.hpp"

#include <cntgs/contiguous.hpp>
#include <doctest/doctest.h>

#include <array>
#include <memory_resource>
#include <string>

namespace test_element
{
TEST_SUITE_BEGIN(CNTGS_TEST_CPP_VERSION);

using namespace cntgs;
using namespace cntgs::test;

TEST_CASE("ContiguousElement: converting constructors")
{
    OneFixed vector{1, {FLOATS1.size()}};
    vector.emplace_back(10u, FLOATS1);
    SUBCASE("copy from reference")
    {
        const auto ref{vector[0]};
        OneFixed::value_type value{ref};
        CHECK_EQ(vector[0], value);
    }
    SUBCASE("move from reference")
    {
        OneFixed::value_type value{vector[0]};
        CHECK_EQ(vector[0], value);
    }
    SUBCASE("copy from value_type")
    {
        const OneFixed::value_type ref{vector[0]};
        OneFixed::value_type value{ref};
        CHECK_EQ(vector[0], value);
    }
    SUBCASE("move from value_type")
    {
        OneFixed::value_type value{OneFixed::value_type{vector[0]}};
        CHECK_EQ(vector[0], value);
    }
    using Vector2 = cntgs::BasicContiguousVector<std::allocator<int>, uint32_t, cntgs::FixedSize<float>>;
    Vector2 vector2{1, {FLOATS1.size()}};
    vector2.emplace_back(10u, FLOATS1);
    SUBCASE("copy from value_type")
    {
        const Vector2::value_type ref{vector2[0]};
        OneFixed::value_type value{ref};
        CHECK_EQ(vector[0], value);
    }
    SUBCASE("move from value_type")
    {
        OneFixed::value_type value{Vector2::value_type{vector2[0]}};
        CHECK_EQ(vector[0], value);
    }
    SUBCASE("copy from value_type and equal allocator")
    {
        const Vector2::value_type const_value{vector[0]};
        OneFixed::value_type value{const_value, OneFixed::allocator_type{}};
        CHECK_EQ(vector[0], value);
    }
    SUBCASE("move from value_type and equal allocator")
    {
        OneFixed::value_type value{Vector2::value_type{vector[0]}, OneFixed::allocator_type{}};
        CHECK_EQ(vector[0], value);
    }
    TestMemoryResource resource;
    using ValueType =
        cntgs::BasicContiguousVector<TestMemoryResource::allocator_type, uint32_t, cntgs::FixedSize<float>>::value_type;
    SUBCASE("copy from reference and allocator")
    {
        const auto ref{vector[0]};
        ValueType value{ref, resource.get_allocator()};
        CHECK_EQ(vector[0], value);
        resource.check_was_used(value.get_allocator());
    }
    SUBCASE("move from reference and allocator")
    {
        ValueType value{vector[0], resource.get_allocator()};
        CHECK_EQ(vector[0], value);
        resource.check_was_used(value.get_allocator());
    }
    SUBCASE("copy from value_type and non-equal allocator")
    {
        const ValueType const_value{vector[0]};
        ValueType value{const_value, resource.get_allocator()};
        CHECK_EQ(vector[0], value);
        resource.check_was_used(value.get_allocator());
    }
    SUBCASE("move from value_type and equal allocator")
    {
        ValueType value{ValueType{vector[0], resource.get_allocator()}, resource.get_allocator()};
        CHECK_EQ(vector[0], value);
        resource.check_was_used(value.get_allocator());
    }
    SUBCASE("move from value_type and non-equal allocator")
    {
        ValueType value{ValueType{vector[0]}, resource.get_allocator()};
        CHECK_EQ(vector[0], value);
        resource.check_was_used(value.get_allocator());
    }
    SUBCASE("copy from value_type and uncomparable allocator")
    {
        const OneFixed::value_type const_value{vector[0]};
        ValueType value{const_value, resource.get_allocator()};
        CHECK_EQ(vector[0], value);
        resource.check_was_used(value.get_allocator());
    }
    SUBCASE("move from value_type and uncomparable allocator")
    {
        ValueType value{OneFixed::value_type{vector[0]}, resource.get_allocator()};
        CHECK_EQ(vector[0], value);
        resource.check_was_used(value.get_allocator());
    }
}

TEST_CASE("ContiguousElement: construct from move-only type")
{
    auto vector = fixed_vector_of_unique_ptrs();
    using ValueType = decltype(vector)::value_type;
    SUBCASE("move from reference")
    {
        ValueType value{vector[0]};
        test::check_equal_using_get(vector[0], array_one_unique_ptr(nullptr), nullptr);
        test::check_equal_using_get(value, array_one_unique_ptr(10), std::make_unique<int>(20));
    }
    TestMemoryResource resource;
    using AllocValueType = typename decltype(fixed_vector_of_unique_ptrs(resource.get_allocator()))::value_type;
    SUBCASE("move from reference and allocator")
    {
        AllocValueType value{vector[0], resource.get_allocator()};
        resource.check_was_used(value.get_allocator());
        test::check_equal_using_get(value, array_one_unique_ptr(10), std::make_unique<int>(20));
    }
    SUBCASE("move from value_type and allocator")
    {
        AllocValueType value{ValueType{vector[0]}, resource.get_allocator()};
        resource.check_was_used(value.get_allocator());
        test::check_equal_using_get(value, array_one_unique_ptr(10), std::make_unique<int>(20));
        AllocValueType value2{std::move(value)};
        CHECK_EQ(resource.get_allocator(), value2.get_allocator());
        test::check_equal_using_get(value2, array_one_unique_ptr(10), std::make_unique<int>(20));
    }
}

TEST_CASE("ContiguousElement: FixedSize can be copy assigned - std::allocator")
{
    auto vector = fixed_vector_of_strings();
    using ValueType = decltype(vector)::value_type;
    using ReferenceType = decltype(vector)::reference;
    ValueType value1{vector[0]};
    ValueType value2{vector[1]};
    SUBCASE("value_type to value_type")
    {
        value2 = value1;
        value2 = value2;
    }
    SUBCASE("reference to value_type") { value2 = ReferenceType{value1}; }
    test::check_equal_using_get(value2, std::array{STRING1, STRING2}, STRING1);
}

TEST_CASE("ContiguousElement: VaryingSize can be copy assigned - std::allocator")
{
    auto vector = varying_vector_of_strings();
    using ValueType = decltype(vector)::value_type;
    ValueType value1{vector[0]};
    ValueType value2{vector[1]};
    SUBCASE("larger into smaller")
    {
        value2 = value1;
        test::check_equal_using_get(value2, std::array{STRING1, STRING2}, STRING1);
    }
    SUBCASE("smaller into larger")
    {
        value1 = value2;
        test::check_equal_using_get(value1, std::array{STRING1}, STRING2);
    }
}

TEST_CASE("ContiguousElement: FixedSize can be copy assigned - std::polymorphic_allocator")
{
    TestMemoryResource resource;
    auto vector = fixed_vector_of_strings(resource.get_allocator());
    using ValueType = decltype(vector)::value_type;
    SUBCASE("equal allocator")
    {
        ValueType value1{vector[0], resource.get_allocator()};
        ValueType value2{vector[1], resource.get_allocator()};
        value2 = value1;
        test::check_equal_using_get(value2, std::array{STRING1, STRING2}, STRING1);
    }
    SUBCASE("non-equal allocator")
    {
        ValueType value1{vector[0], resource.get_allocator()};
        ValueType value2{vector[1]};
        value2 = value1;
        test::check_equal_using_get(value2, std::array{STRING1, STRING2}, STRING1);
    }
}

TEST_CASE("ContiguousElement: VaryingSize can be copy assigned - std::polymorphic_allocator")
{
    TestMemoryResource resource;
    auto vector = varying_vector_of_strings(resource.get_allocator());
    using ValueType = decltype(vector)::value_type;
    SUBCASE("equal allocator")
    {
        ValueType value1{vector[0], resource.get_allocator()};
        ValueType value2{vector[1], resource.get_allocator()};
        SUBCASE("larger into smaller")
        {
            value2 = value1;
            test::check_equal_using_get(value2, std::array{STRING1, STRING2}, STRING1);
        }
        SUBCASE("smaller into larger")
        {
            value1 = value2;
            test::check_equal_using_get(value1, std::array{STRING1}, STRING2);
        }
    }
    SUBCASE("non-equal allocator")
    {
        ValueType value1{vector[0], resource.get_allocator()};
        ValueType value2{vector[1]};
        SUBCASE("larger into smaller")
        {
            value2 = value1;
            test::check_equal_using_get(value2, std::array{STRING1, STRING2}, STRING1);
        }
        SUBCASE("smaller into larger")
        {
            value1 = value2;
            test::check_equal_using_get(value1, std::array{STRING1}, STRING2);
        }
    }
}

TEST_CASE("ContiguousElement: FixedSize can be move assigned - std::allocator")
{
    auto vector = fixed_vector_of_unique_ptrs();
    using ValueType = decltype(vector)::value_type;
    ValueType value1{vector[0]};
    ValueType value2{vector[1]};
    value1 = std::move(value2);
    test::check_equal_using_get(value1, array_one_unique_ptr(30), 40);
}

TEST_CASE("ContiguousElement: FixedSize can be move assigned - std::allocator")
{
    auto vector = varying_vector_of_unique_ptrs();
    using ValueType = decltype(vector)::value_type;
    ValueType value1{vector[0]};
    ValueType value2{vector[1]};
    SUBCASE("larger into smaller")
    {
        value2 = std::move(value1);
        test::check_equal_using_get(value2, array_two_unique_ptr(10, 20), 30);
    }
    SUBCASE("smaller into larger")
    {
        value1 = std::move(value2);
        test::check_equal_using_get(value1, array_one_unique_ptr(40), 50);
    }
}

TEST_CASE("ContiguousElement: FixedSize can be move assigned - std::polymorphic_allocator")
{
    TestMemoryResource resource;
    auto vector = varying_vector_of_unique_ptrs(resource.get_allocator());
    using ValueType = decltype(vector)::value_type;
    SUBCASE("equal allocator")
    {
        ValueType value1{vector[0], resource.get_allocator()};
        ValueType value2{vector[1], resource.get_allocator()};
        SUBCASE("larger into smaller")
        {
            value2 = std::move(value1);
            test::check_equal_using_get(value2, array_two_unique_ptr(10, 20), 30);
        }
        SUBCASE("smaller into larger")
        {
            value1 = std::move(value2);
            test::check_equal_using_get(value1, array_one_unique_ptr(40), 50);
        }
    }
    SUBCASE("non-equal allocator")
    {
        ValueType value1{vector[0], resource.get_allocator()};
        ValueType value2{vector[1]};
        SUBCASE("larger into smaller")
        {
            value2 = std::move(value1);
            test::check_equal_using_get(value2, array_two_unique_ptr(10, 20), 30);
        }
        SUBCASE("smaller into larger")
        {
            value1 = std::move(value2);
            test::check_equal_using_get(value1, array_one_unique_ptr(40), 50);
        }
    }
}

TEST_CASE("ContiguousElement: FixedSize can be move assigned - std::polymorphic_allocator")
{
    TestMemoryResource resource;
    auto vector = fixed_vector_of_unique_ptrs(resource.get_allocator());
    using ValueType = decltype(vector)::value_type;
    SUBCASE("equal allocator")
    {
        ValueType value1{vector[0], resource.get_allocator()};
        ValueType value2{vector[1], resource.get_allocator()};
        value1 = std::move(value2);
        test::check_equal_using_get(value1, array_one_unique_ptr(30), 40);
    }
    SUBCASE("non-equal allocator")
    {
        ValueType value1{vector[0], resource.get_allocator()};
        ValueType value2{vector[1]};
        value1 = std::move(value2);
        test::check_equal_using_get(value1, array_one_unique_ptr(30), 40);
    }
}

TEST_CASE("ContiguousElement: can be swapped")
{
    auto vector = fixed_vector_of_unique_ptrs();
    using ValueType = decltype(vector)::value_type;
    ValueType value1{vector[0]};
    ValueType value2{vector[1]};
    using std::swap;
    swap(value1, value2);
    swap(value1, value1);
    test::check_equal_using_get(value1, array_one_unique_ptr(30), 40);
    test::check_equal_using_get(value2, array_one_unique_ptr(10), 20);
}

template <class Value>
void check_const_and_non_const(Value& value)
{
    CHECK_EQ(10u, cntgs::get<0>(value));
    CHECK_EQ(10u, cntgs::get<0>(std::as_const(value)));
    CHECK(test::range_equal(FLOATS1, cntgs::get<1>(value)));
    CHECK(test::range_equal(FLOATS1, cntgs::get<1>(std::as_const(value))));
}

template <class Vector>
auto mutate_first_and_check(Vector& vector)
{
    using ValueType = typename std::decay_t<Vector>::value_type;
    ValueType value{vector[0]};
    check_const_and_non_const(value);
    cntgs::get<1>(value).front() = 10.f;
    CHECK_EQ(1.f, cntgs::get<1>(vector[0]).front());
    ValueType value2{std::move(vector[0])};
    check_const_and_non_const(value2);
    return value2;
}

TEST_CASE("ContiguousElement: OneVarying mutation does not mutate underlying ContiguousVector")
{
    OneVarying vector{1, FLOATS1.size() * sizeof(float)};
    vector.emplace_back(10u, FLOATS1);
    mutate_first_and_check(vector);
}

TEST_CASE("ContiguousElement: OneFixed mutation does not mutate underlying ContiguousVector")
{
    OneFixed vector{1, {FLOATS1.size()}};
    vector.emplace_back(10u, FLOATS1);
    auto value = mutate_first_and_check(vector);
    cntgs::get<1>(value).front() = 12.f;
    vector[0] = std::move(value);
    CHECK_EQ(12.f, cntgs::get<1>(vector[0]).front());
}

template <bool IsNoexcept>
using NoexceptElement = typename cntgs::ContiguousVector<cntgs::FixedSize<Noexcept<IsNoexcept>>>::value_type;

TEST_CASE("ContiguousElement: is conditionally nothrow")
{
    check_conditionally_nothrow_comparison<NoexceptElement>();
    check_always_nothrow_move_construct<NoexceptElement>();
    CHECK(std::is_nothrow_move_assignable_v<NoexceptElement<true>>);
}

using ToValueTypeVarying = test::ToValueType<OneVarying>;

TEST_CASE("ContiguousElement: OneVarying::value_type comparison operators")
{
    auto vector = one_varying_vector_for_elements();
    SUBCASE("equal") { check_equality(vector, ToValueTypeVarying{}, ToValueTypeVarying{}); }
    SUBCASE("not equal") { check_inequality(vector, ToValueTypeVarying{}, ToValueTypeVarying{}); }
    SUBCASE("less") { check_less(vector, ToValueTypeVarying{}, ToValueTypeVarying{}); }
    SUBCASE("less equal") { check_less_equal(vector, ToValueTypeVarying{}, ToValueTypeVarying{}); }
    SUBCASE("greater") { check_greater(vector, ToValueTypeVarying{}, ToValueTypeVarying{}); }
    SUBCASE("greater equal") { check_greater_equal(vector, ToValueTypeVarying{}, ToValueTypeVarying{}); }
}

TEST_CASE("ContiguousElement: OneVarying::(const_)reference to OneVarying::value_type comparison operators")
{
    auto vector = one_varying_vector_for_elements();
    SUBCASE("equal") { check_equality(vector, test::Identity{}, ToValueTypeVarying{}); }
    SUBCASE("not equal") { check_inequality(vector, test::Identity{}, ToValueTypeVarying{}); }
    SUBCASE("less") { check_less(vector, test::Identity{}, ToValueTypeVarying{}); }
    SUBCASE("less equal") { check_less_equal(vector, test::Identity{}, ToValueTypeVarying{}); }
    SUBCASE("greater") { check_greater(vector, test::Identity{}, ToValueTypeVarying{}); }
    SUBCASE("greater equal") { check_greater_equal(vector, test::Identity{}, ToValueTypeVarying{}); }
}

TEST_CASE("ContiguousElement: OneVarying::value_type to OneVarying::(const_)reference comparison operators")
{
    auto vector = one_varying_vector_for_elements();
    SUBCASE("equal") { check_equality(vector, ToValueTypeVarying{}, test::Identity{}); }
    SUBCASE("not equal") { check_inequality(vector, ToValueTypeVarying{}, test::Identity{}); }
    SUBCASE("less") { check_less(vector, ToValueTypeVarying{}, test::Identity{}); }
    SUBCASE("less equal") { check_less_equal(vector, ToValueTypeVarying{}, test::Identity{}); }
    SUBCASE("greater") { check_greater(vector, ToValueTypeVarying{}, test::Identity{}); }
    SUBCASE("greater equal") { check_greater_equal(vector, ToValueTypeVarying{}, test::Identity{}); }
}

TEST_CASE("ContiguousElement: value_type to (const_)reference for memcmp-compatible lexicographical comparisons")
{
    cntgs::ContiguousVector<UInt8, cntgs::FixedSize<std::byte>> vector{4, {2}};
    vector.emplace_back(UInt8{10}, std::array{std::byte{1}, std::byte{2}});
    vector.emplace_back(UInt8{20}, std::array{std::byte{11}, std::byte{22}});
    vector.emplace_back(UInt8{10}, std::array{std::byte{1}, std::byte{2}});
    vector.emplace_back(UInt8{15}, std::array{std::byte{10}, std::byte{20}});
    SUBCASE("equal") { check_equality(vector, test::Identity{}, test::Identity{}); }
    SUBCASE("not equal") { check_inequality(vector, test::Identity{}, test::Identity{}); }
    SUBCASE("less") { check_less(vector, test::Identity{}, test::Identity{}); }
    SUBCASE("less equal") { check_less_equal(vector, test::Identity{}, test::Identity{}); }
    SUBCASE("greater") { check_greater(vector, test::Identity{}, test::Identity{}); }
    SUBCASE("greater equal") { check_greater_equal(vector, test::Identity{}, test::Identity{}); }
}

TEST_SUITE_END();
}  // namespace test_element
