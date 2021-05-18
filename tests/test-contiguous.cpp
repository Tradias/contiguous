#include "cntgs/contiguous.h"

#include <doctest/doctest.h>

#include <array>
#include <list>
#include <string>
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
    CHECK_EQ(sizeof(uint32_t) + 2 * sizeof(float*), cntgs::detail::ContiguousVectorTraits<TwoVarying>::SIZE_IN_MEMORY);
    CHECK_EQ(2, cntgs::detail::ContiguousVectorTraits<TwoVarying>::CONTIGUOUS_COUNT);
    CHECK_EQ(2, cntgs::detail::ContiguousVectorTraits<TwoFixed>::CONTIGUOUS_FIXED_SIZE_COUNT);
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

TEST_CASE("ContiguousTest: two fixed size: get_fixed_size<I>()")
{
    TwoFixed vector{2, {}, {10, 20}};
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

TEST_CASE("ContiguousTest: one fixed one varying size: correct memory_consumption()")
{
    const auto varying_byte_count = 6 * sizeof(float);
    OneFixedOneVarying vector{2, varying_byte_count, {1}};
    const auto expected =
        2 * (1 * sizeof(uint16_t) + sizeof(float*) + sizeof(std::byte*) + sizeof(uint32_t)) + varying_byte_count;
    CHECK_EQ(expected, vector.memory_consumption());
}

TEST_CASE("ContiguousTest: two fixed size: correct memory_consumption()")
{
    TwoFixed vector{2, {}, {1, 2}};
    const auto expected = 2 * (1 * sizeof(float) + sizeof(uint32_t) + 2 * sizeof(float));
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

TEST_CASE("ContiguousTest: one varying size: emplace_back with lists and subscript operator")
{
    std::list expected_firsts{1.f, 2.f};
    OneVarying vector{1, expected_firsts.size() * sizeof(float)};
    vector.emplace_back(10u, expected_firsts);
    auto&& [id, firsts] = vector[0];
    CHECK_EQ(10u, id);
    CHECK(std::equal(firsts.begin(), firsts.end(), expected_firsts.begin(), expected_firsts.end()));
}

TEST_CASE("ContiguousTest: one varying size: subscript operator returns a reference")
{
    OneVarying vector{1, 0};
    vector.emplace_back(10, std::initializer_list<float>{});
    SUBCASE("mutable")
    {
        auto&& [id1, e1] = vector[0];
        id1 = 20u;
        auto&& [id2, e2] = vector[0];
        CHECK_EQ(20u, id2);
    }
    SUBCASE("const")
    {
        auto&& [id, span] = std::as_const(vector)[0];
        CHECK(std::is_same_v<const uint32_t&, decltype(id)>);
        CHECK(std::is_same_v<cntgs::Span<const float>, decltype(span)>);
    }
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
    TwoFixed vector{2, {}, {expected_firsts.size(), expected_seconds.size()}};
    vector.emplace_back(expected_firsts, 10u, expected_seconds);
    vector.emplace_back(expected_firsts, 10u, expected_seconds);
    for (size_t i = 0; i < vector.size(); ++i)
    {
        auto&& [firsts, id, seconds] = vector[i];
        CHECK_EQ(10u, id);
        CHECK(std::equal(firsts.begin(), firsts.end(), expected_firsts.begin(), expected_firsts.end()));
        CHECK(std::equal(seconds.begin(), seconds.end(), expected_seconds.begin(), expected_seconds.end()));
    }
}

TEST_CASE("ContiguousTest: one fixed size: emplace_back with iterator and subscript operator")
{
    std::list expected_elements{1.f, 2.f};
    OneFixed vector{1, {}, {expected_elements.size()}};
    vector.emplace_back(10u, expected_elements.begin());
    auto&& [id, elements] = vector[0];
    CHECK_EQ(10u, id);
    CHECK(std::equal(elements.begin(), elements.end(), expected_elements.begin(), expected_elements.end()));
}

TEST_CASE("ContiguousTest: std::string emplace_back with iterator and subscript operator")
{
    cntgs::ContiguousVector<cntgs::FixedSize<std::string>, std::string> vector{1, {}, {1}};
    vector.emplace_back(std::vector{std::string("a very long test string")},
                        std::string("another very long test string"));
    auto&& [fixed, string] = vector[0];
    REQUIRE_EQ(1, fixed.size());
    CHECK_EQ("a very long test string", fixed[0]);
    CHECK_EQ("another very long test string", string);
}

template <class T>
void check_iterator(T&& vector)
{
    auto begin = vector.begin();
    using IterTraits = std::iterator_traits<decltype(begin)>;
    CHECK(std::is_same_v<std::random_access_iterator_tag, IterTraits::iterator_category>);
    std::for_each(vector.begin()++, ++vector.begin(), [&](auto&& elem) {
        auto&& [uinteger, floats] = vector[0];
        CHECK_EQ(uinteger, std::get<0>(elem));
        CHECK(std::equal(floats.begin(), floats.end(), std::get<1>(elem).begin(), std::get<1>(elem).end()));
    });
    std::for_each(vector.begin() + 1, vector.end(), [&](auto&& elem) {
        auto&& [uinteger, floats] = vector[1];
        CHECK_EQ(uinteger, std::get<0>(elem));
        CHECK(std::equal(floats.begin(), floats.end(), std::get<1>(elem).begin(), std::get<1>(elem).end()));
    });
}

TEST_CASE("ContiguousTest: one fixed size: begin() end()")
{
    OneFixed vector{2, {}, {2}};
    vector.emplace_back(10u, std::array{1.f, 2.f});
    vector.emplace_back(20u, std::array{-1.f, -2.f});
    SUBCASE("mutable")
    {
        auto begin = vector.begin();
        CHECK(std::is_same_v<OneFixed::reference, decltype(*begin)>);
        CHECK(std::is_same_v<OneFixed::iterator, decltype(begin)>);
        check_iterator(vector);
    }
    SUBCASE("const")
    {
        auto begin = std::as_const(vector).begin();
        CHECK(std::is_same_v<OneFixed::const_reference, decltype(*begin)>);
        CHECK(std::is_same_v<OneFixed::const_iterator, decltype(begin)>);
        check_iterator(std::as_const(vector));
    }
}
}  // namespace test_contiguous
