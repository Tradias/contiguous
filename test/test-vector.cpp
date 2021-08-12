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
#include <list>
#include <memory_resource>
#include <string>
#include <vector>
#include <version>

#ifdef __cpp_lib_ranges
#include <ranges>
#endif

#ifdef __cpp_lib_span
#include <span>
#endif

namespace test_vector
{
TEST_SUITE_BEGIN(CNTGS_TEST_CPP_VERSION);

using namespace cntgs;
using namespace cntgs::test;

template <class T>
auto check_size1_and_capacity2(T& v)
{
    CHECK_EQ(1, v.size());
    CHECK_EQ(2, v.capacity());
    CHECK_FALSE(v.empty());
}

TEST_CASE("ContiguousVector: Plain size() and capacity()")
{
    Plain v{2};
    v.emplace_back(10u, 1.5f);
    check_size1_and_capacity2(v);
}

TEST_CASE("ContiguousVector: OneVarying size() and capacity()")
{
    OneVarying v{2, FLOATS1.size() * sizeof(float)};
    v.emplace_back(10u, FLOATS1);
    check_size1_and_capacity2(v);
}

TEST_CASE("ContiguousVector: TwoVarying size() and capacity()")
{
    TwoVarying v{2, FLOATS1.size() * sizeof(float) + FLOATS2.size() * sizeof(float)};
    v.emplace_back(10u, FLOATS1, FLOATS2);
    check_size1_and_capacity2(v);
}

TEST_CASE("ContiguousVector: OneFixed size() and capacity()")
{
    OneFixed v{2, {FLOATS1.size()}};
    v.emplace_back(10u, FLOATS1);
    check_size1_and_capacity2(v);
}

TEST_CASE("ContiguousVector: TwoFixed size() and capacity()")
{
    TwoFixed v{2, {FLOATS1.size(), FLOATS2.size()}};
    v.emplace_back(FLOATS1, 10u, FLOATS2);
    check_size1_and_capacity2(v);
}

TEST_CASE("ContiguousVector: OneFixedOneVarying size() and capacity()")
{
    OneFixedOneVarying v{2, FLOATS2.size() * sizeof(float), {FLOATS1.size()}};
    v.emplace_back(FLOATS1, 10u, FLOATS2);
    check_size1_and_capacity2(v);
}

TEST_CASE("ContiguousVector: OneVarying empty()")
{
    OneVarying vector{0, {}};
    CHECK(vector.empty());
}

TEST_CASE("ContiguousVector: TwoFixed get_fixed_size<I>()")
{
    TwoFixed vector{2, {10, 20}};
    CHECK_EQ(10, vector.get_fixed_size<0>());
    CHECK_EQ(20, vector.get_fixed_size<1>());
}

TEST_CASE("ContiguousVector: one fixed one varying size: correct memory_consumption()")
{
    using Vector = cntgs::ContiguousVector<cntgs::FixedSize<uint16_t>, uint32_t, cntgs::VaryingSize<float>>;
    const auto varying_byte_count = 6 * sizeof(float);
    Vector vector{2, varying_byte_count, {3}};
    const auto expected =
        2 * (3 * sizeof(uint16_t) + sizeof(std::size_t) + sizeof(std::byte*) + sizeof(uint32_t)) + varying_byte_count;
    CHECK_EQ(expected, vector.memory_consumption());
}

TEST_CASE("ContiguousVector: TwoFixed correct memory_consumption()")
{
    TwoFixed vector{2, {1, 2}};
    const auto expected = 2 * (1 * sizeof(float) + sizeof(uint32_t) + 2 * sizeof(float));
    CHECK_EQ(expected, vector.memory_consumption());
}

TEST_CASE("ContiguousVector: TwoVarying emplace_back with arrays")
{
    TwoVarying vector{1, FLOATS1.size() * sizeof(float) + FLOATS2.size() * sizeof(float)};
    vector.emplace_back(10u, FLOATS1, FLOATS2);
    test::check_equal_using_get(vector[0], 10u, FLOATS1, FLOATS2);
}

TEST_CASE("ContiguousVector: OneVarying emplace_back with lists")
{
    OneVarying vector{1, FLOATS_LIST.size() * sizeof(float)};
    vector.emplace_back(10u, FLOATS_LIST);
    test::check_equal_using_get(vector[0], 10u, FLOATS_LIST);
}

TEST_CASE("ContiguousVector: TwoFixed emplace_back with lists")
{
    TwoFixed vector{2, {FLOATS_LIST.size(), FLOATS1.size()}};
    vector.emplace_back(FLOATS_LIST, 10u, FLOATS1);
    vector.emplace_back(FLOATS_LIST, 10u, FLOATS1);
    for (auto&& element : vector)
    {
        test::check_equal_using_get(element, FLOATS_LIST, 10u, FLOATS1);
    }
}

template <class Vector>
void check_pop_back(Vector&& vector)
{
    vector.emplace_back(array_one_unique_ptr(10), std::make_unique<int>(20));
    vector.emplace_back(array_one_unique_ptr(30), std::make_unique<int>(40));
    vector.pop_back();
    CHECK_EQ(1, vector.size());
    test::check_equal_using_get(vector.back(), array_one_unique_ptr(10), 20);
    vector.emplace_back(array_one_unique_ptr(50), std::make_unique<int>(60));
    CHECK_EQ(2, vector.size());
    test::check_equal_using_get(vector.back(), array_one_unique_ptr(50), 60);
}

TEST_CASE("ContiguousVector: pop_back")
{
    check_pop_back(OneFixedUniquePtr{2, {1}});
    check_pop_back(OneVaryingUniquePtr{2, 2 * (2 * sizeof(std::unique_ptr<int>))});
}

TEST_CASE("ContiguousVector: OneVarying subscript operator returns a reference")
{
    OneVarying vector{1, sizeof(float)};
    vector.emplace_back(10, std::initializer_list<float>{1});
    SUBCASE("mutable")
    {
        auto&& [i1, e1] = vector[0];
        i1 = 20u;
        auto&& [i2, e2] = vector[0];
        CHECK_EQ(20u, i2);
    }
    SUBCASE("const")
    {
        auto&& [i, span] = std::as_const(vector)[0];
        CHECK(std::is_same_v<const uint32_t&, decltype(i)>);
        CHECK(std::is_same_v<cntgs::Span<const float>, decltype(span)>);
    }
}

TEST_CASE("ContiguousVector: OneVarying emplace_back c-style array")
{
    float carray[]{0.1f, 0.2f};
    OneVarying vector{1, std::size(carray) * sizeof(float)};
    vector.emplace_back(10, carray);
    test::check_equal_using_get(vector[0], 10u, carray);
}

#ifdef __cpp_lib_ranges
TEST_CASE("ContiguousVector: OneVarying emplace_back std::views::iota")
{
    auto iota = std::views::iota(0, 10) | std::views::transform(
                                              [](auto i)
                                              {
                                                  return float(i);
                                              });
    OneVarying vector{1, std::ranges::size(iota) * sizeof(float)};
    vector.emplace_back(10, iota);
    test::check_equal_using_get(vector[0], 10u, iota);
}

TEST_CASE("ContiguousVector: OneFixed emplace_back std::views::iota")
{
    auto bound_iota = std::views::iota(0, 10) | std::views::transform(
                                                    [](auto i)
                                                    {
                                                        return float(i);
                                                    });
    auto unbound_iota = std::views::iota(0) | std::views::transform(
                                                  [](auto i)
                                                  {
                                                      return float(i);
                                                  });
    OneFixed vector{2, {std::ranges::size(bound_iota)}};
    vector.emplace_back(10, bound_iota);
    vector.emplace_back(10, unbound_iota.begin());
    for (auto&& i : {0, 1})
    {
        test::check_equal_using_get(vector[i], 10u, bound_iota);
    }
}
#endif

TEST_CASE("ContiguousVector: OneFixed emplace_back with iterator")
{
    std::vector expected_elements{1.f, 2.f};
    OneFixed vector{1, {expected_elements.size()}};
    SUBCASE("begin() iterator") { vector.emplace_back(10u, expected_elements.begin()); }
    SUBCASE("data() iterator") { vector.emplace_back(10u, expected_elements.data()); }
    test::check_equal_using_get(vector[0], 10u, expected_elements);
}

TEST_CASE("ContiguousVector: std::string emplace_back with iterator")
{
    cntgs::ContiguousVector<cntgs::FixedSize<std::string>, std::string, const std::string*> vector{1, {1}};
    std::vector v{STRING1};
    SUBCASE("emplace_back range") { vector.emplace_back(v, STRING2, &STRING2); }
    SUBCASE("emplace_back iterator") { vector.emplace_back(v.begin(), STRING2, &STRING2); }
    auto&& [fixed, string, string_ptr] = vector[0];
    CHECK_EQ(1, fixed.size());
    CHECK_EQ(STRING1, fixed[0]);
    CHECK_EQ(STRING2, string);
    CHECK_EQ(STRING2, *string_ptr);
}

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

template <bool IsNoexcept>
using NoexceptVector = cntgs::ContiguousVector<cntgs::FixedSize<Noexcept<IsNoexcept>>>;

TEST_CASE("ContiguousVector: ContiguousVector is conditionally nothrow")
{
    check_conditionally_nothrow_comparison<NoexceptVector>();
    check_always_nothrow_move_construct<NoexceptVector>();
    CHECK(std::is_nothrow_move_assignable_v<NoexceptVector<true>>);
    CHECK(std::is_nothrow_move_assignable_v<NoexceptVector<false>>);
    CHECK_FALSE(std::is_nothrow_move_assignable_v<test::pmr::ContiguousVector<float>>);
}

TEST_CASE("ContiguousVector: OneFixedUniquePtr erase(Iterator)")
{
    OneFixedUniquePtr vector{2, {1}};
    vector.emplace_back(array_one_unique_ptr(10), std::make_unique<int>(20));
    vector.emplace_back(array_one_unique_ptr(30), std::make_unique<int>(40));
    vector.erase(vector.begin());
    CHECK_EQ(1, vector.size());
    test::check_equal_using_get(vector.front(), array_one_unique_ptr(30), 40);
}

TEST_CASE("ContiguousVector: OneVaryingUniquePtr erase(Iterator)")
{
    OneVaryingUniquePtr vector{3, 3 * (2 * sizeof(std::unique_ptr<int>))};
    vector.emplace_back(array_one_unique_ptr(10), std::make_unique<int>(20));
    vector.emplace_back(array_two_unique_ptr(30, 40), std::make_unique<int>(50));
    vector.emplace_back(array_two_unique_ptr(60, 70), std::make_unique<int>(80));
    vector.erase(vector.begin());
    CHECK_EQ(2, vector.size());
    test::check_equal_using_get(vector.front(), array_two_unique_ptr(30, 40), 50);
    test::check_equal_using_get(vector.back(), array_two_unique_ptr(60, 70), 80);
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
    TwoVarying vector{4, 84};
    vector.emplace_back(10u, FLOATS1, FLOATS2);
    vector.emplace_back(20u, FLOATS2_ALT, FLOATS1);
    vector.emplace_back(30u, FLOATS1, FLOATS2_ALT);
    vector.emplace_back(40u, FLOATS2, FLOATS2_ALT);
    auto it = vector.erase(++vector.begin());
    CHECK_EQ(++vector.begin(), it);
    TwoVarying vector2{3, 64};
    vector2.emplace_back(10u, FLOATS1, FLOATS2);
    vector2.emplace_back(30u, FLOATS1, FLOATS2_ALT);
    vector2.emplace_back(40u, FLOATS2, FLOATS2_ALT);
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
        test::check_equal_using_get(vector.front(), array_one_unique_ptr(50), 60);
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
        test::check_equal_using_get(vector.front(), FLOATS2_ALT, 30u, FLOATS2);
    }
    SUBCASE("erase all")
    {
        vector.erase(vector.begin(), vector.end());
        CHECK_EQ(0, vector.size());
        vector.emplace_back(FLOATS2_ALT, 30u, FLOATS2);
        test::check_equal_using_get(vector.front(), FLOATS2_ALT, 30u, FLOATS2);
    }
    SUBCASE("erase none")
    {
        vector.erase(vector.begin(), vector.begin());
        CHECK_EQ(3, vector.size());
        test::check_equal_using_get(vector.front(), FLOATS2, 10u, FLOATS2);
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
        test::check_equal_using_get(vector.front(), 30u, FLOATS1, FLOATS2_ALT);
    }
    SUBCASE("erase all")
    {
        vector.erase(vector.begin(), vector.end());
        CHECK_EQ(0, vector.size());
        vector.emplace_back(30u, FLOATS2_ALT, FLOATS2);
        test::check_equal_using_get(vector.front(), 30u, FLOATS2_ALT, FLOATS2);
    }
    SUBCASE("erase none")
    {
        vector.erase(vector.begin(), vector.begin());
        CHECK_EQ(3, vector.size());
        test::check_equal_using_get(vector.front(), 10u, FLOATS1, FLOATS2);
    }
}

TEST_CASE("ContiguousVector: std::string OneFixed emplace_back->reserve->emplace_back")
{
    cntgs::ContiguousVector<cntgs::FixedSize<std::string>, std::string, int> vector{1, {1}};
    vector.emplace_back(std::array{STRING1}, STRING1, 42);
    vector.reserve(2);
    vector.emplace_back(std::array{STRING2}, STRING2, 84);
    test::check_equal_using_get(vector[0], std::array{STRING1}, STRING1, 42);
    test::check_equal_using_get(vector[1], std::array{STRING2}, STRING2, 84);
}

TEST_CASE("ContiguousVector: trivial OneFixed emplace_back->reserve->emplace_back")
{
    cntgs::ContiguousVector<cntgs::FixedSize<float>, int> vector{1, {FLOATS1.size()}};
    vector.emplace_back(FLOATS1, 42);
    vector.reserve(2);
    vector.emplace_back(FLOATS1, 84);
    test::check_equal_using_get(vector[0], FLOATS1, 42);
    test::check_equal_using_get(vector[1], FLOATS1, 84);
}

TEST_CASE("ContiguousVector: trivial VaryingSize emplace_back->reserve->emplace_back")
{
    cntgs::ContiguousVector<cntgs::VaryingSize<float>, int> vector{1, FLOATS1.size() * sizeof(float)};
    vector.emplace_back(FLOATS1, 42);
    vector.reserve(2, FLOATS1.size() * sizeof(float) + FLOATS2.size() * sizeof(float));
    vector.emplace_back(FLOATS2, 84);
    test::check_equal_using_get(vector[0], FLOATS1, 42);
    test::check_equal_using_get(vector[1], FLOATS2, 84);
}

TEST_CASE("ContiguousVector: std::unique_ptr VaryingSize reserve and shrink")
{
    cntgs::ContiguousVector<cntgs::VaryingSize<std::unique_ptr<int>>, std::unique_ptr<int>> vector{0, 0};
    vector.reserve(1, 3 * sizeof(std::unique_ptr<int>));
    vector.emplace_back(array_one_unique_ptr(), std::make_unique<int>(20));
    test::check_equal_using_get(vector[0], array_one_unique_ptr(), 20);
    vector.reserve(3, 8 * sizeof(std::unique_ptr<int>));
    vector.emplace_back(array_two_unique_ptr(), std::make_unique<int>(50));
    vector.emplace_back(array_one_unique_ptr(), std::make_unique<int>(20));
    test::check_equal_using_get(vector[1], array_two_unique_ptr(), 50);
    test::check_equal_using_get(vector[2], array_one_unique_ptr(), 20);
}

TEST_CASE("ContiguousVector: trivial OneFixed reserve with polymorphic_allocator")
{
    TestMemoryResource resource;
    test::pmr::ContiguousVector<cntgs::FixedSize<float>, int> vector{0, {10}, resource.get_allocator()};
    vector.reserve(2);
    CHECK_EQ(2, vector.capacity());
    vector.emplace_back(std::array{1.f}, 10);
    resource.check_was_used(vector.get_allocator());
}

#if defined(__cpp_lib_constexpr_dynamic_alloc) && defined(__cpp_constinit)
TEST_CASE("ContiguousVector: OneFixed constinit")
{
    SUBCASE("empty vector")
    {
        static constinit OneFixed v{0, {2}};
        // TODO this causes a call to trivially_copy_into which calls memcpy with nullptr as second argument
        v.reserve(2);
        v.emplace_back(10u, FLOATS1);
        check_size1_and_capacity2(v);
    }
    SUBCASE("constexpr")
    {
        constexpr auto SIZE = []
        {
            OneFixed v2{0, {2}};
            return v2.size();
        }();
        CHECK_EQ(0, SIZE);
    }
}
#endif

#ifdef __cpp_lib_span
TEST_CASE("ContiguousVector: cntgs::Span can be implicitly converted to std::span")
{
    std::array<int, 1> ints{42};
    cntgs::Span<int> span{ints.data(), ints.size()};
    std::span<int> actual = span;
    CHECK_EQ(1, actual.size());
    CHECK_EQ(42, actual.front());
}
#endif

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
        test::check_equal_using_get(vector[i], 'a', i);
        auto&& [a, b] = vector[i];
        check_alignment<8>(&b);
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
        test::check_equal_using_get(vector[i], FLOATS1, i);
        auto&& [a, b] = vector[i];
        check_alignment<16>(a);
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
        test::check_equal_using_get(vector[i], i, FLOATS1, FLOATS2);
        auto&& [a, b, c] = vector[i];
        check_alignment<8>(b);
        check_alignment<8>(c);
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
        test::check_equal_using_get(vector[i], i, FLOATS1);
        auto&& [a, b] = vector[i];
        check_alignment<32>(b);
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
        test::check_equal_using_get(vector[i], FLOATS1, i, FLOATS2);
        auto&& [a, b, c] = vector[i];
        check_alignment<8>(a);
        check_alignment<16>(&b);
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
        test::check_equal_using_get(vector[i], FLOATS1, uint2, i);
        auto&& [a, b, c] = vector[i];
        check_alignment<32>(a);
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
        test::check_equal_using_get(vector[i], FLOATS1, i, FLOATS2);
        auto&& [a, b, c] = vector[i];
        check_alignment<16>(a);
        check_alignment<8>(c);
    }
}

TEST_CASE("ContiguousVector: OneFixedUniquePtr with polymorphic_allocator")
{
    TestMemoryResource resource;
    auto vector = fixed_vector_of_unique_ptrs(resource.get_allocator());
    resource.check_was_used(vector.get_allocator());
}

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
        test::check_equal_using_get(vector[0], array_two_unique_ptr(10, 20), 30);
        test::check_equal_using_get(vector[1], array_one_unique_ptr(40), 50);
    };
    TestMemoryResource resource;
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
    TestMemoryResource resource2;
    SUBCASE("move into smaller vector")
    {
        decltype(vector) vector2{0, 0, resource2.get_allocator()};
        vector2 = std::move(vector);
        resource2.check_was_used(vector2.get_allocator());
        check_expected(vector2);
    }
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
    TestMemoryResource resource;
    auto vector = fixed_vector_of_unique_ptrs(resource.get_allocator());
    TestMemoryResource resource2;
    SUBCASE("move into smaller vector")
    {
        decltype(vector) vector2{0, {}, resource2.get_allocator()};
        vector2 = std::move(vector);
        resource2.check_was_used(vector2.get_allocator());
        test::check_equal_using_get(vector2[0], array_one_unique_ptr(10), 20);
    }
    SUBCASE("move into larger vector")
    {
        decltype(vector) vector2{3, {1}, resource2.get_allocator()};
        vector2 = std::move(vector);
        resource2.check_was_used(vector2.get_allocator());
        test::check_equal_using_get(vector2[0], array_one_unique_ptr(10), 20);
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
    TestMemoryResource resource;
    auto vector = varying_vector_of_strings(resource.get_allocator());
    TestMemoryResource resource2;
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
    TestMemoryResource resource;
    auto vector = fixed_vector_of_strings(resource.get_allocator());
    TestMemoryResource resource2;
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

template <class Vector>
void check_varying_vector_unique_ptrs(const Vector& vector)
{
    CHECK_EQ(2, vector.size());
    test::check_equal_using_get(vector[0], array_two_unique_ptr(10, 20), 30);
    test::check_equal_using_get(vector[1], array_one_unique_ptr(40), 50);
}

TEST_CASE("ContiguousVector: OneVaryingUniquePtr swap empty vector")
{
    auto expected = varying_vector_of_unique_ptrs();
    decltype(expected) vector{};
    using std::swap;
    swap(expected, vector);
    check_varying_vector_unique_ptrs(vector);
}

TEST_CASE("ContiguousVector: OneVaryingUniquePtr swap same vector")
{
    auto vector = varying_vector_of_unique_ptrs();
    using std::swap;
    swap(vector, vector);
    check_varying_vector_unique_ptrs(vector);
}

TEST_CASE("ContiguousVector: OneVaryingUniquePtr swap")
{
    TestMemoryResource resource;
    auto vector = varying_vector_of_unique_ptrs(resource.get_allocator());
    TestMemoryResource resource2;
    using std::swap;
    SUBCASE("swap into smaller vector")
    {
        decltype(vector) vector2{0, 0, resource2.get_allocator()};
        swap(vector2, vector);
        resource2.check_was_not_used(vector2.get_allocator());
        check_varying_vector_unique_ptrs(vector2);
    }
    SUBCASE("swap into larger vector")
    {
        decltype(vector) vector2{3, 10, resource2.get_allocator()};
        swap(vector2, vector);
        resource2.check_was_not_used(vector2.get_allocator());
        check_varying_vector_unique_ptrs(vector2);
    }
}

template <class... T>
void check_clear_followed_by_emplace_back(cntgs::BasicContiguousVector<T...>& vector)
{
    const auto expected_capacity = vector.capacity();
    vector.clear();
    CHECK_EQ(expected_capacity, vector.capacity());
    CHECK(vector.empty());
    vector.emplace_back(std::array{STRING2, STRING2}, STRING2);
    test::check_equal_using_get(vector[0], std::array{STRING2, STRING2}, STRING2);
}

TEST_CASE("ContiguousVector: OneFixedString clear")
{
    auto vector = fixed_vector_of_strings();
    check_clear_followed_by_emplace_back(vector);
}

TEST_CASE("ContiguousVector: OneVaryingString clear")
{
    auto vector = varying_vector_of_strings();
    check_clear_followed_by_emplace_back(vector);
}

TEST_SUITE_END();
}  // namespace test_vector
