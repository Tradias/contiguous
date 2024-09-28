// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "utils/check.hpp"
#include "utils/doctest.hpp"
#include "utils/fixture.hpp"
#include "utils/testMemoryResource.hpp"
#include "utils/typedefs.hpp"

#include <cntgs/contiguous.hpp>

#include <array>
#include <list>
#include <string>
#include <vector>
#include <version>

#ifdef __cpp_lib_ranges
#include <ranges>
#endif

namespace test_vector_emplace
{
using namespace cntgs;
using namespace test;

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

TEST_CASE("ContiguousVector: TwoVarying emplace_back with arrays")
{
    TwoVarying vector{1, FLOATS1.size() * sizeof(float) + FLOATS2.size() * sizeof(float)};
    vector.emplace_back(10u, FLOATS1, FLOATS2);
    check_equal_using_get(vector[0], 10u, FLOATS1, FLOATS2);
}

TEST_CASE("ContiguousVector: OneVarying emplace_back with lists")
{
    OneVarying vector{1, FLOATS_LIST.size() * sizeof(float)};
    vector.emplace_back(10u, FLOATS_LIST);
    check_equal_using_get(vector[0], 10u, FLOATS_LIST);
}

TEST_CASE("ContiguousVector: TwoFixed emplace_back with lists")
{
    TwoFixed vector{2, {FLOATS_LIST.size(), FLOATS1.size()}};
    vector.emplace_back(FLOATS_LIST, 10u, FLOATS1);
    vector.emplace_back(FLOATS_LIST, 10u, FLOATS1);
    for (auto&& element : vector)
    {
        check_equal_using_get(element, FLOATS_LIST, 10u, FLOATS1);
    }
}

template <class Vector>
void check_pop_back(Vector&& vector)
{
    vector.emplace_back(array_one_unique_ptr(10), std::make_unique<int>(20));
    vector.emplace_back(array_one_unique_ptr(30), std::make_unique<int>(40));
    vector.pop_back();
    CHECK_EQ(1, vector.size());
    check_equal_using_get(vector.back(), array_one_unique_ptr(10), 20);
    vector.emplace_back(array_one_unique_ptr(50), std::make_unique<int>(60));
    CHECK_EQ(2, vector.size());
    check_equal_using_get(vector.back(), array_one_unique_ptr(50), 60);
}

TEST_CASE("ContiguousVector: pop_back")
{
    check_pop_back(OneFixedUniquePtr{2, {1}});
    check_pop_back(OneVaryingUniquePtr{2, 2 * (2 * sizeof(std::unique_ptr<int>))});
}

TEST_CASE("ContiguousVector: OneVarying emplace_back c-style array")
{
    float carray[]{0.1f, 0.2f};
    OneVarying vector{1, std::size(carray) * sizeof(float)};
    vector.emplace_back(10, carray);
    check_equal_using_get(vector[0], 10u, carray);
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
    check_equal_using_get(vector[0], 10u, iota);
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
        check_equal_using_get(vector[i], 10u, bound_iota);
    }
}
#endif

TEST_CASE("ContiguousVector: OneFixed emplace_back with iterator")
{
    std::vector expected_elements{1.f, 2.f};
    OneFixed vector{1, {expected_elements.size()}};
    SUBCASE("begin() iterator") { vector.emplace_back(10u, expected_elements.begin()); }
    SUBCASE("data() iterator") { vector.emplace_back(10u, expected_elements.data()); }
    check_equal_using_get(vector[0], 10u, expected_elements);
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

template <class Vector, class T2, class U2, class T3, class U3, class T4, class U4>
void check_after_emplace(Vector& vector, std::pair<T2, U2> expected_first, std::pair<T3, U3> expected_second,
                         std::pair<T4, U4> expected_third)
{
    check_equal_using_get(vector[0], std::get<0>(expected_first), std::get<1>(expected_first));
    check_equal_using_get(vector[1], std::get<0>(expected_second), std::get<1>(expected_second));
    check_equal_using_get(vector[2], std::get<0>(expected_third), std::get<1>(expected_third));
}

template <class Vector, class T1, class U1, class T2, class U2, class T3, class U3, class T4, class U4>
void check_emplace_at_begin(Vector& vector, std::pair<T1, U1> element_to_emplace, std::pair<T2, U2> expected_first,
                            std::pair<T3, U3> expected_second, std::pair<T4, U4> expected_third)
{
    auto it = vector.emplace(vector.begin(), std::get<0>(element_to_emplace), std::get<1>(element_to_emplace));
    CHECK(vector.begin() == it);
    check_after_emplace(vector, std::move(expected_first), std::move(expected_second), std::move(expected_third));
}

template <class Vector, class T1, class U1, class T2, class U2, class T3, class U3, class T4, class U4>
void check_emplace_in_the_middle(Vector& vector, std::pair<T1, U1> element_to_emplace, std::pair<T2, U2> expected_first,
                                 std::pair<T3, U3> expected_second, std::pair<T4, U4> expected_third)
{
    auto it = vector.emplace(++vector.begin(), std::get<0>(element_to_emplace), std::get<1>(element_to_emplace));
    CHECK(++vector.begin() == it);
    check_after_emplace(vector, std::move(expected_first), std::move(expected_second), std::move(expected_third));
}

template <class Vector, class T1, class U1, class T2, class U2, class T3, class U3, class T4, class U4>
void check_emplace_at_end(Vector& vector, std::pair<T1, U1> element_to_emplace, std::pair<T2, U2> expected_first,
                          std::pair<T3, U3> expected_second, std::pair<T4, U4> expected_third)
{
    auto it = vector.emplace(vector.end(), std::get<0>(element_to_emplace), std::get<1>(element_to_emplace));
    CHECK(--vector.end() == it);
    check_after_emplace(vector, std::move(expected_first), std::move(expected_second), std::move(expected_third));
}

TEST_CASE("ContiguousVector: FixedSize std::string emplace")
{
    cntgs::ContiguousVector<cntgs::FixedSize<std::string>, std::string> vector{4, {1}};
    vector.emplace_back(std::vector{STRING1}, STRING1);
    vector.emplace_back(std::vector{STRING1}, STRING2);
    SUBCASE("at begin")
    {
        check_emplace_at_begin(vector, std::pair{std::vector{STRING2}, STRING2},
                               std::pair{std::vector{STRING2}, STRING2}, std::pair{std::vector{STRING1}, STRING1},
                               std::pair{std::vector{STRING1}, STRING2});
    }
    SUBCASE("in the middle")
    {
        check_emplace_in_the_middle(vector, std::pair{std::vector{STRING2}, STRING2},
                                    std::pair{std::vector{STRING1}, STRING1}, std::pair{std::vector{STRING2}, STRING2},
                                    std::pair{std::vector{STRING1}, STRING2});
    }
    SUBCASE("at end")
    {
        check_emplace_at_end(vector, std::pair{std::vector{STRING2}, STRING2}, std::pair{std::vector{STRING1}, STRING1},
                             std::pair{std::vector{STRING1}, STRING2}, std::pair{std::vector{STRING2}, STRING2});
    }
}

TEST_CASE("ContiguousVector: OneFixed emplace")
{
    OneFixed vector{4, {2}};
    vector.emplace_back(10u, FLOATS1);
    vector.emplace_back(20u, FLOATS1_ALT);
    SUBCASE("at begin")
    {
        check_emplace_at_begin(vector, std::pair{30u, FLOATS1_ALT}, std::pair{30u, FLOATS1_ALT},
                               std::pair{10u, FLOATS1}, std::pair{20u, FLOATS1_ALT});
    }
    SUBCASE("in the middle")
    {
        check_emplace_in_the_middle(vector, std::pair{30u, FLOATS1_ALT}, std::pair{10u, FLOATS1},
                                    std::pair{30u, FLOATS1_ALT}, std::pair{20u, FLOATS1_ALT});
    }
    SUBCASE("at end")
    {
        check_emplace_at_end(vector, std::pair{30u, FLOATS1_ALT}, std::pair{10u, FLOATS1}, std::pair{20u, FLOATS1_ALT},
                             std::pair{30u, FLOATS1_ALT});
    }
}

TEST_CASE("ContiguousVector: OneVarying emplace")
{
    OneVarying vector{4, 12 * sizeof(float)};
    vector.emplace_back(10u, FLOATS2);
    vector.emplace_back(20u, FLOATS1_ALT);
    SUBCASE("at begin")
    {
        check_emplace_at_begin(vector, std::pair{30u, FLOATS2_ALT}, std::pair{30u, FLOATS2_ALT},
                               std::pair{10u, FLOATS2}, std::pair{20u, FLOATS1_ALT});
    }
    SUBCASE("in the middle")
    {
        check_emplace_in_the_middle(vector, std::pair{30u, FLOATS2_ALT}, std::pair{10u, FLOATS2},
                                    std::pair{30u, FLOATS2_ALT}, std::pair{20u, FLOATS1_ALT});
    }
    SUBCASE("at end")
    {
        check_emplace_at_end(vector, std::pair{30u, FLOATS2_ALT}, std::pair{10u, FLOATS2}, std::pair{20u, FLOATS1_ALT},
                             std::pair{30u, FLOATS2_ALT});
    }
}

TEST_CASE("ContiguousVector: OneVarying std::string emplace" * doctest::skip())
{
    cntgs::ContiguousVector<cntgs::VaryingSize<std::string>, std::string> vector{4, 7 * sizeof(std::string)};
    vector.emplace_back(std::array{STRING1, STRING2}, STRING1);
    vector.emplace_back(std::array{STRING1}, STRING2);
    SUBCASE("at begin")
    {
        check_emplace_at_begin(
            vector, std::pair{std::array{STRING2, STRING2}, STRING2}, std::pair{std::array{STRING2, STRING2}, STRING2},
            std::pair{std::array{STRING1, STRING2}, STRING1}, std::pair{std::array{STRING1}, STRING2});
    }
    SUBCASE("in the middle")
    {
        check_emplace_in_the_middle(
            vector, std::pair{std::array{STRING1, STRING2}, STRING1}, std::pair{std::array{STRING2, STRING2}, STRING2},
            std::pair{std::array{STRING1, STRING2}, STRING1}, std::pair{std::array{STRING1}, STRING2});
    }
    SUBCASE("at end")
    {
        check_emplace_at_end(vector, std::pair{std::array{STRING1, STRING2}, STRING1},
                             std::pair{std::array{STRING1}, STRING2}, std::pair{std::array{STRING2, STRING2}, STRING2},
                             std::pair{std::array{STRING1, STRING2}, STRING1});
    }
}

TEST_CASE("ContiguousVector: std::string OneFixed emplace_back->reserve->emplace_back")
{
    cntgs::ContiguousVector<cntgs::FixedSize<std::string>, std::string, int> vector{1, {1}};
    vector.emplace_back(std::array{STRING1}, STRING1, 42);
    vector.reserve(2);
    vector.emplace_back(std::array{STRING2}, STRING2, 84);
    check_equal_using_get(vector[0], std::array{STRING1}, STRING1, 42);
    check_equal_using_get(vector[1], std::array{STRING2}, STRING2, 84);
}

TEST_CASE("ContiguousVector: trivial OneFixed emplace_back->reserve->emplace_back")
{
    cntgs::ContiguousVector<cntgs::FixedSize<float>, int> vector{1, {FLOATS1.size()}};
    vector.emplace_back(FLOATS1, 42);
    vector.reserve(2);
    vector.emplace_back(FLOATS1, 84);
    check_equal_using_get(vector[0], FLOATS1, 42);
    check_equal_using_get(vector[1], FLOATS1, 84);
}

TEST_CASE("ContiguousVector: trivial VaryingSize emplace_back->reserve->emplace_back")
{
    cntgs::ContiguousVector<cntgs::VaryingSize<float>, int> vector{1, FLOATS1.size() * sizeof(float)};
    vector.emplace_back(FLOATS1, 42);
    vector.reserve(2, FLOATS1.size() * sizeof(float) + FLOATS2.size() * sizeof(float));
    vector.emplace_back(FLOATS2, 84);
    check_equal_using_get(vector[0], FLOATS1, 42);
    check_equal_using_get(vector[1], FLOATS2, 84);
}

TEST_CASE("ContiguousVector: std::unique_ptr VaryingSize reserve and shrink")
{
    cntgs::ContiguousVector<cntgs::VaryingSize<std::unique_ptr<int>>, std::unique_ptr<int>> vector{0, 0};
    vector.reserve(1, 3 * sizeof(std::unique_ptr<int>));
    vector.emplace_back(array_one_unique_ptr(), std::make_unique<int>(20));
    check_equal_using_get(vector[0], array_one_unique_ptr(), 20);
    vector.reserve(3, 8 * sizeof(std::unique_ptr<int>));
    vector.emplace_back(array_two_unique_ptr(), std::make_unique<int>(50));
    vector.emplace_back(array_one_unique_ptr(), std::make_unique<int>(20));
    check_equal_using_get(vector[1], array_two_unique_ptr(), 50);
    check_equal_using_get(vector[2], array_one_unique_ptr(), 20);
}

TEST_CASE("ContiguousVector: trivial OneFixed reserve with polymorphic_allocator")
{
    TestPmrMemoryResource resource;
    pmr::ContiguousVector<cntgs::FixedSize<float>, int> vector{0, {10}, resource.get_allocator()};
    vector.reserve(2);
    CHECK_EQ(2, vector.capacity());
    vector.emplace_back(std::array{1.f}, 10);
    resource.check_was_used(vector.get_allocator());
}
}  // namespace test_vector_emplace
