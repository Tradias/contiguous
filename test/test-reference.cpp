// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "utils/check.hpp"
#include "utils/fixture.hpp"
#include "utils/functional.hpp"
#include "utils/noexcept.hpp"
#include "utils/range.hpp"
#include "utils/typedefs.hpp"

#include <cntgs/contiguous.hpp>
#include <doctest/doctest.h>

#include <memory>

namespace test_contiguous
{
TEST_SUITE_BEGIN(CNTGS_TEST_CPP_VERSION);

using namespace cntgs;
using namespace cntgs::test;

template <bool IsNoThrow>
using NoThrowReference = typename cntgs::ContiguousVector<cntgs::FixedSize<test::Thrower<IsNoThrow>>>::reference;

TEST_CASE("ContiguousReference: ContiguousReference is conditionally nothrow")
{
    test::check_conditionally_nothrow_comparison<NoThrowReference>();
    test::check_always_nothrow_move_construct<NoThrowReference>();
    CHECK(std::is_nothrow_copy_constructible_v<NoThrowReference<true>>);
    CHECK(std::is_nothrow_copy_constructible_v<NoThrowReference<false>>);
    test::check_conditionally_nothrow_move_assign<NoThrowReference>();
    test::check_conditionally_nothrow_copy_assign<NoThrowReference>();
}

TEST_CASE("ContiguousReference: Contiguous(Const)Reference converting constructors")
{
    auto vector = test::one_varying_vector_for_elements();
    using Vector = decltype(vector);
    SUBCASE("bind mutable reference to const reference")
    {
        Vector::reference ref{vector[1]};
        CHECK_EQ(vector[1], Vector::const_reference{ref});
        CHECK_EQ(vector[1], Vector::const_reference{std::move(ref)});
    }
    SUBCASE("bind const value_type to const reference")
    {
        const Vector::value_type value{vector[1]};
        CHECK_EQ(vector[1], Vector::const_reference{value});
        CHECK_EQ(vector[1], Vector::const_reference{std::move(value)});
    }
    Vector::value_type value{vector[1]};
    SUBCASE("bind mutable value_type to reference")
    {
        CHECK_EQ(vector[1], Vector::reference{value});
        CHECK_EQ(vector[1], Vector::reference{std::move(value)});
    }
    SUBCASE("bind mutable value_type to const reference")
    {
        CHECK_EQ(vector[1], Vector::const_reference{value});
        CHECK_EQ(vector[1], Vector::const_reference{std::move(value)});
    }
}

TEST_CASE("ContiguousReference: OneFixed::const_reference can be used to copy elements")
{
    auto floats2 = FLOATS1;
    floats2.front() = 100.f;
    test::OneFixed vector{2, {FLOATS1.size()}};
    vector.emplace_back(10u, FLOATS1);
    vector.emplace_back(20u, floats2);
    test::OneFixed::reference first = vector[0];
    test::OneFixed::const_reference second = std::as_const(vector)[1];
    first = second;
    test::check_equal_using_get(vector[0], 20u, floats2);
    test::check_equal_using_get(vector[1], 20u, floats2);
}

TEST_CASE("ContiguousReference: OneFixed::reference can be used to move elements")
{
    using Vector = cntgs::ContiguousVector<int, std::unique_ptr<int>, cntgs::FixedSize<std::unique_ptr<int>>>;
    Vector vector{2, {1}};
    vector.emplace_back(1, std::make_unique<int>(10), test::array_one_unique_ptr(20));
    vector.emplace_back(2, std::make_unique<int>(30), test::array_one_unique_ptr(40));
    Vector::reference first = vector[0];
    Vector::reference second = vector[1];
    first = std::move(second);
    test::check_equal_using_get(vector[0], 2, 30, test::array_one_unique_ptr(40));
    test::check_equal_using_get(vector[1], 2, nullptr, std::array{std::unique_ptr<int>{}});
}

TEST_CASE("ContiguousReference: swap and iter_swap with ContiguousVectorIterator of FixedSize std::unique_ptr")
{
    cntgs::ContiguousVector<std::unique_ptr<int>, cntgs::FixedSize<std::unique_ptr<int>>> vector{2, {1}};
    std::array v{std::make_unique<int>(20)};
    vector.emplace_back(std::make_unique<int>(10), std::make_move_iterator(v.begin()));
    vector.emplace_back(std::make_unique<int>(30), std::array{std::make_unique<int>(40)});
    SUBCASE("std::iter_swap") { std::iter_swap(vector.begin(), ++vector.begin()); }
    SUBCASE("cntgs::swap")
    {
        using std::swap;
        swap(vector[0], vector[1]);
        swap(vector[1], vector[1]);
    }
    {
        auto&& [a, b] = vector[0];
        CHECK_EQ(30, *a);
        CHECK_EQ(40, *b.front());
    }
    {
        auto&& [a, b] = vector[1];
        CHECK_EQ(10, *a);
        CHECK_EQ(20, *b.front());
    }
}

TEST_CASE("ContiguousReference: FixedSize swap partially trivial")
{
    using Vector = cntgs::ContiguousVector<int, cntgs::FixedSize<float>, cntgs::FixedSize<std::unique_ptr<int>>>;
    Vector vector{2, {FLOATS1.size(), 1}};
    vector.emplace_back(10, FLOATS1, test::array_one_unique_ptr(20));
    vector.emplace_back(30, FLOATS1_ALT, test::array_one_unique_ptr(40));
    using std::swap;
    swap(vector[0], vector[1]);
    swap(vector[1], vector[1]);
    test::check_equal_using_get(vector.front(), 30, FLOATS1_ALT, test::array_one_unique_ptr(40));
    test::check_equal_using_get(vector.back(), 10, FLOATS1, test::array_one_unique_ptr(20));
}

TEST_CASE("ContiguousReference: OneVarying::(const_)reference comparison operators")
{
    auto vector = one_varying_vector_for_elements();
    SUBCASE("equal") { check_equality(vector, test::Identity{}, test::Identity{}); }
    SUBCASE("not equal") { check_inequality(vector, test::Identity{}, test::Identity{}); }
    SUBCASE("less") { check_less(vector, test::Identity{}, test::Identity{}); }
    SUBCASE("less equal") { check_less_equal(vector, test::Identity{}, test::Identity{}); }
    SUBCASE("greater") { check_greater(vector, test::Identity{}, test::Identity{}); }
    SUBCASE("greater equal") { check_greater_equal(vector, test::Identity{}, test::Identity{}); }
}

TEST_SUITE_END();
}  // namespace test_contiguous
