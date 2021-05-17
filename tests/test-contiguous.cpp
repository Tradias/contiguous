#include "cntgs/contiguous.h"

#include <doctest/doctest.h>

#include <array>
#include <list>
#include <vector>

namespace test_contiguous
{
using OneVarying = cntgs::ContiguousVector<uint32_t, cntgs::VaryingSize<float>>;
using TwoVarying = cntgs::ContiguousVector<uint32_t, cntgs::VaryingSize<float>, cntgs::VaryingSize<float>>;
using OneFixed = cntgs::ContiguousVector<uint32_t, cntgs::FixedSize<float>>;
using TwoFixed = cntgs::ContiguousVector<cntgs::FixedSize<float>, uint32_t, cntgs::FixedSize<float>>;
using OneFixedOneVarying = cntgs::ContiguousVector<cntgs::FixedSize<uint16_t>, uint32_t, cntgs::VaryingSize<float>>;

TEST_CASE("ContiguousTest: traits")
{
    CHECK_EQ(sizeof(uint32_t) + 2 * sizeof(float*), TwoVarying::SIZE_IN_MEMORY);
    CHECK_EQ(2, TwoVarying::CONTIGUOUS_COUNT);
    CHECK_EQ(2, TwoFixed::CONTIGUOUS_FIXED_SIZE_COUNT);
}

TEST_CASE("ContiguousTest: one varying size: equality")
{
    // std::array firsts{1.f, 2.f};
    // OneVarying first{1, firsts.size() * sizeof(float)};
    // first.emplace_back(10u, firsts);
    // std::array seconds{1.f, 2.f};
    // OneVarying second{1, seconds.size() * sizeof(float)};
    // second.emplace_back(10u, seconds);
    // CHECK_EQ(first, second);
    // second.emplace_back(20u, seconds);
    // CHECK_NE(first, second);
}

TEST_CASE("ContiguousTest: two varying size: size() and capacity()")
{
    std::array elements{1.f, 2.f};
    OneVarying vector{2, elements.size() * sizeof(float)};
    vector.emplace_back(10u, elements);
    CHECK_EQ(1, vector.size());
    CHECK_EQ(2, vector.capacity());
}

TEST_CASE("ContiguousTest: two varying size: empty()")
{
    OneVarying vector{0, {}};
    CHECK(vector.empty());
}

TEST_CASE("ContiguousTest: two varying size: get_fixed_size<I>()")
{
    TwoFixed vector{2, 0, {10, 20}};
    CHECK_EQ(10, vector.get_fixed_size<0>());
    CHECK_EQ(20, vector.get_fixed_size<1>());
}

TEST_CASE("ContiguousTest: one varying size: reference can be converted to value_type")
{
    std::array elements{1.f, 2.f};
    OneVarying vector{2, elements.size() * sizeof(float)};
    vector.emplace_back(10u, elements);
    OneVarying::value_type value = vector[0];
}

TEST_CASE("ContiguousTest: one varying size: correct memory_consumption()")
{
    const auto varying_byte_count = 6 * sizeof(float);
    OneFixedOneVarying vector{2, varying_byte_count, {1}};
    const auto expected =
        2 * (1 * sizeof(uint16_t) + sizeof(float*) + sizeof(std::byte*) + sizeof(uint32_t)) + varying_byte_count;
    vector.emplace_back(std::array{uint16_t{1}}, 10u, std::array{1.f, 2.f, 3.f});
    vector.emplace_back(std::array{uint16_t{1}}, 10u, std::array{1.f, 2.f, 3.f});
    CHECK_EQ(expected, vector.memory_consumption());
}

TEST_CASE("ContiguousTest: two varying size: emplace_back with arrays and subscript operator")
{
    std::array expected_firsts{1.f, 2.f};
    std::array expected_seconds{-1.f, -2.f, -3.f, -4.f, -5.f};
    TwoVarying vector{1, expected_firsts.size() * sizeof(float) + expected_seconds.size() * sizeof(float)};
    vector.emplace_back(10u, expected_firsts, expected_seconds);
    auto&& [id, firsts, seconds] = vector[0];
    CHECK_EQ(10u, id);
    CHECK(std::equal(firsts.begin(), firsts.end(), expected_firsts.begin(), expected_firsts.end()));
    CHECK(std::equal(seconds.begin(), seconds.end(), expected_seconds.begin(), expected_seconds.end()));
}

TEST_CASE("ContiguousTest: two varying size: emplace_back with lists and subscript operator")
{
    std::list expected_firsts{1.f, 2.f};
    OneVarying vector{1, expected_firsts.size() * sizeof(float)};
    vector.emplace_back(10u, expected_firsts);
    auto&& [id, firsts] = vector[0];
    CHECK_EQ(10u, id);
    CHECK(std::equal(firsts.begin(), firsts.end(), expected_firsts.begin(), expected_firsts.end()));
}

TEST_CASE("ContiguousTest: one varying size: subscript operator returns a modifyable reference")
{
    OneVarying vector{1, 0};
    vector.emplace_back(10, std::initializer_list<float>{});
    auto&& [id1, e1] = vector[0];
    id1 = 20u;
    auto&& [id2, e2] = vector[0];
    CHECK_EQ(20u, id2);
}

TEST_CASE("ContiguousTest: one varying size: emplace_back c-style array")
{
    float carray[]{0.1f, 0.2f};
    OneVarying vector{1, std::size(carray) * sizeof(float)};
    vector.emplace_back(10, carray);
    auto&& [id, array] = vector[0];
    CHECK_EQ(10u, id);
    CHECK(std::equal(std::begin(array), std::end(array), std::begin(carray), std::end(carray)));
}

TEST_CASE("ContiguousTest: two fixed size: emplace_back with lists and subscript operator")
{
    std::list expected_firsts{1.f, 2.f};
    std::vector expected_seconds{-1.f, -2.f};
    TwoFixed vector{1, 0, {expected_firsts.size(), expected_seconds.size()}};
    vector.emplace_back(expected_firsts, 10u, expected_seconds);
    auto&& [firsts, id, seconds] = vector[0];
    CHECK_EQ(10u, id);
    CHECK(std::equal(firsts.begin(), firsts.end(), expected_firsts.begin(), expected_firsts.end()));
    CHECK(std::equal(seconds.begin(), seconds.end(), expected_seconds.begin(), expected_seconds.end()));
}

TEST_CASE("ContiguousTest: one fixed size: emplace_back with iterator and subscript operator")
{
    std::list expected_firsts{1.f, 2.f};
    OneFixed vector{1, 0, {expected_firsts.size()}};
    vector.emplace_back(10u, expected_firsts.begin());
    auto&& [id, firsts] = vector[0];
    CHECK_EQ(10u, id);
    CHECK(std::equal(firsts.begin(), firsts.end(), expected_firsts.begin(), expected_firsts.end()));
}
}  // namespace test_contiguous
