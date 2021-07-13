#include "cntgs/contiguous.h"
#include "utils/functional.h"
#include "utils/memory.h"
#include "utils/range.h"

#include <doctest/doctest.h>

#include <array>
#include <list>
#include <memory_resource>
#include <optional>
#include <string>
#include <vector>
#include <version>

#ifdef __cpp_lib_ranges
#include <ranges>
#endif

namespace test_contiguous
{
using namespace cntgs;

using Plain = cntgs::ContiguousVector<uint32_t, float>;
using OneVarying = cntgs::ContiguousVector<uint32_t, cntgs::VaryingSize<float>>;
using TwoVarying = cntgs::ContiguousVector<uint32_t, cntgs::VaryingSize<float>, cntgs::VaryingSize<float>>;
using OneFixed = cntgs::ContiguousVector<uint32_t, cntgs::FixedSize<float>>;
using TwoFixed = cntgs::ContiguousVector<cntgs::FixedSize<float>, uint32_t, cntgs::FixedSize<float>>;
using OneFixedOneVarying = cntgs::ContiguousVector<cntgs::FixedSize<float>, uint32_t, cntgs::VaryingSize<float>>;
using OneFixedUniquePtr = cntgs::ContiguousVector<cntgs::FixedSize<std::unique_ptr<int>>, std::unique_ptr<int>>;
using OneVaryingUniquePtr = cntgs::ContiguousVector<cntgs::VaryingSize<std::unique_ptr<int>>, std::unique_ptr<int>>;

static constexpr std::array FLOATS1{1.f, 2.f};
static constexpr std::array FLOATS1_ALT{11.f, 22.f};
static constexpr std::array FLOATS2{-3.f, -4.f, -5.f};
static constexpr std::array FLOATS2_ALT{-33.f, -44.f, -55.f};
const std::list FLOATS_LIST{1.f, 2.f};
const std::string STRING1{"a very long test string"};
const std::string STRING2{"another very long test string"};

auto floats1(float one = 1.f, float two = 2.f) { return std::array{one, two}; }

auto floats1(float one = 1.f, float two = 2.f, float three = 3.f) { return std::array{one, two, three}; }

auto array_one_unique_ptr(int v1 = 10) { return std::array{std::make_unique<int>(v1)}; }

auto array_two_unique_ptr(int v1 = 30, int v2 = 40)
{
    return std::array{std::make_unique<int>(v1), std::make_unique<int>(v2)};
}

template <class T>
auto check_size1_and_capacity2(T& v)
{
    CHECK_EQ(1, v.size());
    CHECK_EQ(2, v.capacity());
    CHECK_FALSE(v.empty());
}

TEST_CASE("ContiguousTest: Plain size() and capacity()")
{
    Plain v{2};
    v.emplace_back(10u, 1.5f);
    check_size1_and_capacity2(v);
}

TEST_CASE("ContiguousTest: OneVarying size() and capacity()")
{
    OneVarying v{2, FLOATS1.size() * sizeof(float)};
    v.emplace_back(10u, FLOATS1);
    check_size1_and_capacity2(v);
}

TEST_CASE("ContiguousTest: TwoVarying size() and capacity()")
{
    TwoVarying v{2, FLOATS1.size() * sizeof(float) + FLOATS2.size() * sizeof(float)};
    v.emplace_back(10u, FLOATS1, FLOATS2);
    check_size1_and_capacity2(v);
}

TEST_CASE("ContiguousTest: OneFixed size() and capacity()")
{
    OneFixed v{2, {FLOATS1.size()}};
    v.emplace_back(10u, FLOATS1);
    check_size1_and_capacity2(v);
}

TEST_CASE("ContiguousTest: TwoFixed size() and capacity()")
{
    TwoFixed v{2, {FLOATS1.size(), FLOATS2.size()}};
    v.emplace_back(FLOATS1, 10u, FLOATS2);
    check_size1_and_capacity2(v);
}

TEST_CASE("ContiguousTest: OneFixedOneVarying size() and capacity()")
{
    OneFixedOneVarying v{2, FLOATS2.size() * sizeof(float), {FLOATS1.size()}};
    v.emplace_back(FLOATS1, 10u, FLOATS2);
    check_size1_and_capacity2(v);
}

TEST_CASE("ContiguousTest: OneVarying empty()")
{
    OneVarying vector{0, {}};
    CHECK(vector.empty());
}

TEST_CASE("ContiguousTest: TwoFixed get_fixed_size<I>()")
{
    TwoFixed vector{2, {10, 20}};
    CHECK_EQ(10, vector.get_fixed_size<0>());
    CHECK_EQ(20, vector.get_fixed_size<1>());
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

TEST_CASE("ContiguousTest: OneVarying mutating value_type does not mutate underlying ContiguousVector")
{
    OneVarying vector{1, FLOATS1.size() * sizeof(float)};
    vector.emplace_back(10u, FLOATS1);
    mutate_first_and_check(vector);
}

TEST_CASE("ContiguousTest: OneFixed mutating value_type does not mutate underlying ContiguousVector")
{
    OneFixed vector{1, {FLOATS1.size()}};
    vector.emplace_back(10u, FLOATS1);
    auto value = mutate_first_and_check(vector);
    cntgs::get<1>(value).front() = 12.f;
    vector[0] = std::move(value);
    CHECK_EQ(12.f, cntgs::get<1>(vector[0]).front());
}

TEST_CASE("ContiguousTest: value_type can be copy constructed")
{
    OneFixed vector{1, {FLOATS1.size()}};
    vector.emplace_back(10u, FLOATS1);
    OneFixed::value_type value1{vector[0]};
    OneFixed::value_type value2{value1};
    CHECK_EQ(10u, cntgs::get<0>(value2));
    CHECK(test::range_equal(FLOATS1, cntgs::get<1>(value2)));
}

TEST_CASE("ContiguousTest: value_type can be copy assigned")
{
    using Vector = cntgs::ContiguousVector<std::string, cntgs::FixedSize<std::string>>;
    Vector vector{2, {1}};
    vector.emplace_back(STRING1, std::array{STRING2});
    vector.emplace_back(STRING2, std::array{STRING1});
    Vector::value_type value1{vector[0]};
    Vector::value_type value2{vector[1]};
    value2 = value1;
    value2 = value2;
    CHECK_EQ(STRING1, cntgs::get<0>(value2));
    CHECK(test::range_equal(std::array{STRING2}, cntgs::get<1>(value2)));
}

template <class Allocator = std::allocator<void>>
auto fixed_vector_of_unique_ptrs(Allocator allocator = {})
{
    cntgs::BasicContiguousVector<Allocator, cntgs::FixedSize<std::unique_ptr<int>>, std::unique_ptr<int>> vector{
        2, {1}, allocator};
    vector.emplace_back(array_one_unique_ptr(10), std::make_unique<int>(20));
    vector.emplace_back(array_one_unique_ptr(30), std::make_unique<int>(40));
    return vector;
}

TEST_CASE("ContiguousTest: value_type can be move constructed")
{
    auto vector = fixed_vector_of_unique_ptrs();
    OneFixedUniquePtr::value_type value1{vector[0]};
    OneFixedUniquePtr::value_type value2{std::move(value1)};
    CHECK_EQ(10, *cntgs::get<0>(value2).front());
    CHECK_EQ(20, *cntgs::get<1>(value2));
}

TEST_CASE("ContiguousTest: value_type can be move assigned")
{
    auto vector = fixed_vector_of_unique_ptrs();
    OneFixedUniquePtr::value_type value1{vector[0]};
    OneFixedUniquePtr::value_type value2{vector[1]};
    value1 = std::move(value2);
    CHECK_EQ(30, *cntgs::get<0>(value1).front());
    CHECK_EQ(40, *cntgs::get<1>(value1));
}

TEST_CASE("ContiguousTest: one fixed one varying size: correct memory_consumption()")
{
    using Vector = cntgs::ContiguousVector<cntgs::FixedSize<uint16_t>, uint32_t, cntgs::VaryingSize<float>>;
    const auto varying_byte_count = 6 * sizeof(float);
    Vector vector{2, varying_byte_count, {3}};
    const auto expected =
        2 * (3 * sizeof(uint16_t) + sizeof(std::size_t) + sizeof(std::byte*) + sizeof(uint32_t)) + varying_byte_count;
    CHECK_EQ(expected, vector.memory_consumption());
}

TEST_CASE("ContiguousTest: TwoFixed correct memory_consumption()")
{
    TwoFixed vector{2, {1, 2}};
    const auto expected = 2 * (1 * sizeof(float) + sizeof(uint32_t) + 2 * sizeof(float));
    CHECK_EQ(expected, vector.memory_consumption());
}

TEST_CASE("ContiguousTest: TwoVarying emplace_back with arrays")
{
    TwoVarying vector{1, FLOATS1.size() * sizeof(float) + FLOATS2.size() * sizeof(float)};
    vector.emplace_back(10u, FLOATS1, FLOATS2);
    auto&& [a, b, c] = vector[0];
    CHECK_EQ(10u, a);
    CHECK(test::range_equal(FLOATS1, b));
    CHECK(test::range_equal(FLOATS2, c));
}

TEST_CASE("ContiguousTest: OneVarying emplace_back with lists")
{
    OneVarying vector{1, FLOATS_LIST.size() * sizeof(float)};
    vector.emplace_back(10u, FLOATS_LIST);
    auto&& [a, b] = vector[0];
    CHECK_EQ(10u, a);
    CHECK(test::range_equal(FLOATS_LIST, b));
}

TEST_CASE("ContiguousTest: TwoFixed emplace_back with lists")
{
    TwoFixed vector{2, {FLOATS_LIST.size(), FLOATS1.size()}};
    vector.emplace_back(FLOATS_LIST, 10u, FLOATS1);
    vector.emplace_back(FLOATS_LIST, 10u, FLOATS1);
    for (auto&& element : vector)
    {
        auto&& [a, b, c] = element;
        CHECK(test::range_equal(FLOATS_LIST, a));
        CHECK_EQ(10u, b);
        CHECK(test::range_equal(FLOATS1, c));
    }
}

template <class Vector>
void check_pop_back(Vector&& vector)
{
    vector.emplace_back(array_one_unique_ptr(10), std::make_unique<int>(20));
    vector.emplace_back(array_one_unique_ptr(30), std::make_unique<int>(40));
    vector.pop_back();
    CHECK_EQ(1, vector.size());
    {
        auto&& [a, b] = vector.back();
        CHECK(test::range_equal(array_one_unique_ptr(10), a, test::DereferenceEqual{}));
        CHECK_EQ(20, *b);
    }
    vector.emplace_back(array_one_unique_ptr(50), std::make_unique<int>(60));
    CHECK_EQ(2, vector.size());
    {
        auto&& [a, b] = vector.back();
        CHECK(test::range_equal(array_one_unique_ptr(50), a, test::DereferenceEqual{}));
        CHECK_EQ(60, *b);
    }
}

TEST_CASE("ContiguousTest: pop_back")
{
    check_pop_back(OneFixedUniquePtr{2, {1}});
    check_pop_back(OneVaryingUniquePtr{2, 2 * (2 * sizeof(std::unique_ptr<int>))});
}

TEST_CASE("ContiguousTest: OneVarying subscript operator returns a reference")
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

TEST_CASE("ContiguousTest: OneVarying emplace_back c-style array")
{
    float carray[]{0.1f, 0.2f};
    OneVarying vector{1, std::size(carray) * sizeof(float)};
    vector.emplace_back(10, carray);
    auto&& [i, array] = vector[0];
    CHECK_EQ(10u, i);
    CHECK(test::range_equal(carray, array));
}

#ifdef __cpp_lib_ranges
TEST_CASE("ContiguousTest: OneVarying emplace_back std::views::iota")
{
    auto iota = std::views::iota(0, 10) | std::views::transform(
                                              [](auto i)
                                              {
                                                  return float(i);
                                              });
    OneVarying vector{1, std::ranges::size(iota) * sizeof(float)};
    vector.emplace_back(10, iota);
    auto&& [i, array] = vector[0];
    CHECK_EQ(10u, i);
    CHECK(test::range_equal(iota, array));
}

TEST_CASE("ContiguousTest: OneFixed emplace_back std::views::iota")
{
    auto iota = std::views::iota(0, 10) | std::views::transform(
                                              [](auto i)
                                              {
                                                  return float(i);
                                              });
    OneFixed vector{1, {std::ranges::size(iota)}};
    vector.emplace_back(10, iota);
    auto&& [i, array] = vector[0];
    CHECK_EQ(10u, i);
    CHECK(test::range_equal(iota, array));
}
#endif

TEST_CASE("ContiguousTest: OneFixed emplace_back with iterator")
{
    std::vector expected_elements{1.f, 2.f};
    OneFixed vector{1, {expected_elements.size()}};
    SUBCASE("begin() iterator") { vector.emplace_back(10u, expected_elements.begin()); }
    SUBCASE("data() iterator") { vector.emplace_back(10u, expected_elements.data()); }
    auto&& [i, elements] = vector[0];
    CHECK_EQ(10u, i);
    CHECK(test::range_equal(expected_elements, elements));
}

TEST_CASE("ContiguousTest: std::string emplace_back with iterator")
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

TEST_CASE("ContiguousTest: ContiguousVector::value_type is conditionally nothrow")
{
    CHECK(std::is_nothrow_destructible_v<cntgs::ContiguousVector<cntgs::FixedSize<float>, float>::value_type>);
    CHECK(std::is_nothrow_destructible_v<cntgs::ContiguousVector<cntgs::VaryingSize<float>>::value_type>);
}

template <class T>
void check_iterator(T& vector)
{
    auto begin = vector.begin();
    using IterTraits = std::iterator_traits<decltype(begin)>;
    CHECK(std::is_same_v<std::random_access_iterator_tag, typename IterTraits::iterator_category>);
    std::for_each((vector.begin()++)--, --(++(++vector.begin())),
                  [&](auto&& elem)
                  {
                      auto&& [uinteger, floats] = vector[0];
                      CHECK_EQ(uinteger, cntgs::get<0>(elem));
                      CHECK(test::range_equal(cntgs::get<1>(elem), floats));
                  });
    std::for_each(((vector.begin() + 2) - 1), vector.end(),
                  [&](auto&& elem)
                  {
                      auto&& [uinteger, floats] = vector[1];
                      CHECK_EQ(uinteger, cntgs::get<0>(elem));
                      CHECK(test::range_equal(cntgs::get<1>(elem), floats));
                  });
}

TEST_CASE("ContiguousTest: OneFixed begin() end()")
{
    OneFixed vector{2, {2}};
    vector.emplace_back(10u, FLOATS1);
    vector.emplace_back(20u, FLOATS1);
    SUBCASE("mutable")
    {
        [[maybe_unused]] auto begin = vector.begin();
        CHECK(std::is_same_v<OneFixed::reference, decltype(*begin)>);
        CHECK(std::is_same_v<OneFixed::iterator, decltype(begin)>);
        check_iterator(vector);
    }
    SUBCASE("const")
    {
        [[maybe_unused]] auto begin = std::as_const(vector).begin();
        CHECK(std::is_same_v<OneFixed::const_reference, decltype(*begin)>);
        CHECK(std::is_same_v<OneFixed::const_iterator, decltype(begin)>);
        check_iterator(std::as_const(vector));
    }
}

TEST_CASE("ContiguousTest: swap and iter_swap with ContiguousVectorIterator of FixedSize std::unique_ptr")
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

TEST_CASE("ContiguousTest: FixedSize swap partially trivial")
{
    using Vector = cntgs::ContiguousVector<int, cntgs::FixedSize<float>, cntgs::FixedSize<std::unique_ptr<int>>>;
    Vector vector{2, {FLOATS1.size(), 1}};
    vector.emplace_back(10, FLOATS1, array_one_unique_ptr(20));
    vector.emplace_back(30, FLOATS1_ALT, array_one_unique_ptr(40));
    using std::swap;
    swap(vector[0], vector[1]);
    swap(vector[1], vector[1]);
    {
        auto&& [a, b, c] = vector.front();
        CHECK_EQ(30, a);
        CHECK(test::range_equal(FLOATS1_ALT, b));
        CHECK(test::range_equal(array_one_unique_ptr(40), c, test::DereferenceEqual{}));
    }
    {
        auto&& [a, b, c] = vector.back();
        CHECK_EQ(10, a);
        CHECK(test::range_equal(FLOATS1, b));
        CHECK(test::range_equal(array_one_unique_ptr(20), c, test::DereferenceEqual{}));
    }
}

TEST_CASE("ContiguousTest: OneFixed::reference equality comparison")
{
    OneVarying vector{4, 4 * (FLOATS1.size() * 2 + FLOATS1_ALT.size() + 3) * sizeof(float)};
    vector.emplace_back(10u, FLOATS1);
    vector.emplace_back(20u, FLOATS1_ALT);
    vector.emplace_back(10u, FLOATS1);
    vector.emplace_back(15u, floats1(10.f, 20.f, 30.f));
    SUBCASE("equal")
    {
        CHECK_EQ(vector[0], vector[2]);
        CHECK_EQ(std::as_const(vector)[0], vector[2]);
        CHECK_EQ(vector[0], std::as_const(vector)[2]);
        CHECK_EQ(std::as_const(vector)[0], std::as_const(vector)[2]);
    }
    SUBCASE("not equal")
    {
        CHECK_NE(vector[0], vector[1]);
        CHECK_NE(std::as_const(vector)[0], vector[1]);
        CHECK_NE(vector[0], std::as_const(vector)[1]);
        CHECK_NE(std::as_const(vector)[0], std::as_const(vector)[1]);
    }
    SUBCASE("less")
    {
        CHECK_LT(vector[2], vector[1]);
        CHECK_LT(std::as_const(vector)[0], vector[1]);
        CHECK_LT(vector[2], std::as_const(vector)[3]);
        CHECK_FALSE(std::as_const(vector)[0] < std::as_const(vector)[2]);
    }
    SUBCASE("less equal")
    {
        CHECK_LE(vector[0], vector[1]);
        CHECK_LE(std::as_const(vector)[0], vector[1]);
        CHECK_LE(vector[2], std::as_const(vector)[3]);
        CHECK_FALSE(std::as_const(vector)[1] <= std::as_const(vector)[2]);
    }
    SUBCASE("greater")
    {
        CHECK_GT(vector[1], vector[0]);
        CHECK_GT(std::as_const(vector)[1], vector[0]);
        CHECK_GT(vector[3], std::as_const(vector)[2]);
        CHECK_FALSE(std::as_const(vector)[2] >= std::as_const(vector)[1]);
    }
    SUBCASE("greater equal")
    {
        CHECK_GE(vector[1], vector[0]);
        CHECK_GE(std::as_const(vector)[0], vector[2]);
        CHECK_FALSE(vector[0] >= std::as_const(vector)[1]);
        CHECK_GE(std::as_const(vector)[3], std::as_const(vector)[2]);
    }
}

TEST_CASE("ContiguousTest: OneFixed::const_reference can be used to copy elements")
{
    auto floats2 = FLOATS1;
    floats2.front() = 100.f;
    OneFixed vector{2, {FLOATS1.size()}};
    vector.emplace_back(10u, FLOATS1);
    vector.emplace_back(20u, floats2);
    OneFixed::reference first = vector[0];
    OneFixed::const_reference second = std::as_const(vector)[1];
    first = second;
    auto&& [a, b] = vector[0];
    auto&& [c, d] = vector[1];
    CHECK_EQ(20u, a);
    CHECK(test::range_equal(floats2, b));
    CHECK_EQ(20u, c);
    CHECK(test::range_equal(floats2, d));
}

TEST_CASE("ContiguousTest: OneFixed::reference can be used to move elements")
{
    using Vector = cntgs::ContiguousVector<int, std::unique_ptr<int>, cntgs::FixedSize<std::unique_ptr<int>>>;
    Vector vector{2, {1}};
    vector.emplace_back(1, std::make_unique<int>(10), array_one_unique_ptr(20));
    vector.emplace_back(2, std::make_unique<int>(30), array_one_unique_ptr(40));
    Vector::reference first = vector[0];
    Vector::reference second = vector[1];
    first = std::move(second);
    auto&& [a, b, c] = vector[0];
    auto&& [d, e, f] = vector[1];
    CHECK_EQ(2, a);
    CHECK_EQ(30, *b);
    CHECK_EQ(40, *c.front());
    CHECK_EQ(2, d);
    CHECK_EQ(nullptr, e);
    CHECK_EQ(nullptr, f.front());
}

TEST_CASE("ContiguousTest: std::rotate with ContiguousVectorIterator of FixedSize std::unique_ptr")
{
    cntgs::ContiguousVector<std::unique_ptr<int>, cntgs::FixedSize<std::unique_ptr<int>>> vector{2, {1}};
    vector.emplace_back(std::make_unique<int>(10), array_one_unique_ptr(20));
    vector.emplace_back(std::make_unique<int>(30), array_one_unique_ptr(40));
    std::rotate(vector.begin(), ++vector.begin(), vector.end());
    auto&& [a, b] = vector[0];
    CHECK_EQ(30, *a);
    CHECK_EQ(40, *b.front());
}

TEST_CASE("ContiguousTest: OneFixedUniquePtr erase(Iterator)")
{
    OneFixedUniquePtr vector{2, {1}};
    vector.emplace_back(array_one_unique_ptr(10), std::make_unique<int>(20));
    vector.emplace_back(array_one_unique_ptr(30), std::make_unique<int>(40));
    vector.erase(vector.begin());
    CHECK_EQ(1, vector.size());
    auto&& [a, b] = vector.front();
    CHECK(test::range_equal(array_one_unique_ptr(30), a, test::DereferenceEqual{}));
    CHECK_EQ(40, *b);
}

TEST_CASE("ContiguousTest: OneVaryingUniquePtr erase(Iterator)")
{
    OneVaryingUniquePtr vector{3, 3 * (2 * sizeof(std::unique_ptr<int>))};
    vector.emplace_back(array_one_unique_ptr(10), std::make_unique<int>(20));
    vector.emplace_back(array_two_unique_ptr(30, 40), std::make_unique<int>(50));
    vector.emplace_back(array_two_unique_ptr(60, 70), std::make_unique<int>(80));
    vector.erase(vector.begin());
    CHECK_EQ(2, vector.size());
    auto&& [a, b] = vector.front();
    CHECK(test::range_equal(array_two_unique_ptr(30, 40), a, test::DereferenceEqual{}));
    CHECK_EQ(50, *b);
    auto&& [c, e] = vector.back();
    CHECK(test::range_equal(array_two_unique_ptr(60, 70), c, test::DereferenceEqual{}));
    CHECK_EQ(80, *e);
}

TEST_CASE("ContiguousTest: TwoFixed erase(Iterator)")
{
    TwoFixed vector{2, {FLOATS2.size(), FLOATS2_ALT.size()}};
    vector.emplace_back(FLOATS2, 10u, FLOATS2);
    vector.emplace_back(FLOATS2_ALT, 20u, FLOATS2);
    auto it = vector.erase(++vector.begin());
    CHECK_EQ(vector.end(), it);
    CHECK_EQ(1, vector.size());
    auto&& [a, b, c] = vector.front();
    CHECK(test::range_equal(FLOATS2, a));
    CHECK_EQ(10u, b);
    CHECK(test::range_equal(FLOATS2, c));
}

TEST_CASE("ContiguousTest: OneFixedUniquePtr erase(Iterator, Iterator)")
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
        auto&& [a, b] = vector.front();
        CHECK(test::range_equal(array_one_unique_ptr(50), a, test::DereferenceEqual{}));
        CHECK_EQ(60, *b);
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

TEST_CASE("ContiguousTest: TwoFixed erase(Iterator, Iterator)")
{
    TwoFixed vector{3, {FLOATS2.size(), FLOATS2_ALT.size()}};
    vector.emplace_back(FLOATS2, 10u, FLOATS2);
    vector.emplace_back(FLOATS2, 20u, FLOATS2_ALT);
    vector.emplace_back(FLOATS2_ALT, 30u, FLOATS2);
    SUBCASE("erase last two")
    {
        vector.erase(++vector.begin(), vector.end());
        CHECK_EQ(1, vector.size());
        auto&& [a, b, c] = vector.front();
        CHECK(test::range_equal(FLOATS2, a));
        CHECK_EQ(10u, b);
        CHECK(test::range_equal(FLOATS2, c));
    }
    SUBCASE("erase all")
    {
        vector.erase(vector.begin(), vector.end());
        CHECK_EQ(0, vector.size());
        vector.emplace_back(FLOATS2_ALT, 30u, FLOATS2);
        auto&& [a, b, c] = vector.front();
        CHECK(test::range_equal(FLOATS2_ALT, a));
        CHECK_EQ(30u, b);
        CHECK(test::range_equal(FLOATS2, c));
    }
    SUBCASE("erase none")
    {
        vector.erase(vector.begin(), vector.begin());
        CHECK_EQ(3, vector.size());
        auto&& [a, b, c] = vector.front();
        CHECK(test::range_equal(FLOATS2, a));
        CHECK_EQ(10u, b);
        CHECK(test::range_equal(FLOATS2, c));
    }
}

TEST_CASE("ContiguousTest: OneFixed construct with unique_ptr and span")
{
    std::optional<OneFixed> vector;
    const auto memory_size = 2 * (sizeof(uint32_t) + 2 * sizeof(float));
    test::AllocationGuard<std::byte> ptr{memory_size};
    SUBCASE("unique_ptr") { vector.emplace(memory_size, ptr.release(), 2, std::array{FLOATS1.size()}); }
    SUBCASE("span") { vector.emplace(cntgs::Span<std::byte>{ptr.ptr, memory_size}, 2, std::array{FLOATS1.size()}); }
    CHECK(vector);
    vector->emplace_back(10u, FLOATS1);
    auto&& [i, e] = (*vector)[0];
    CHECK_EQ(10u, i);
    CHECK(test::range_equal(FLOATS1, e));
}

TEST_CASE("ContiguousTest: Plain construct with unique_ptr and span")
{
    std::optional<Plain> vector;
    const auto memory_size = 2 * (sizeof(uint32_t) + sizeof(float));
    test::AllocationGuard<std::byte> ptr{memory_size};
    SUBCASE("unique_ptr") { vector.emplace(memory_size, ptr.release(), 2); }
    SUBCASE("span") { vector.emplace(cntgs::Span<std::byte>{ptr.ptr, memory_size}, 2); }
    CHECK(vector);
    vector->emplace_back(10u, 5.f);
    auto&& [i, f] = (*vector)[0];
    CHECK_EQ(10u, i);
    CHECK_EQ(5.f, f);
}

TEST_CASE("ContiguousTest: type_erase OneFixed and restore")
{
    OneFixed vector{2, {FLOATS2.size()}};
    vector.emplace_back(10u, FLOATS2);
    auto erased = cntgs::type_erase(std::move(vector));
    OneFixed restored;
    SUBCASE("by lvalue reference") { restored = OneFixed{erased}; }
    SUBCASE("by move") { restored = OneFixed{std::move(erased)}; }
    auto&& [i, e] = restored[0];
    CHECK_EQ(10u, i);
    CHECK(test::range_equal(FLOATS2, e));
}

TEST_CASE("ContiguousTest: std::string TypeErasedVector")
{
    cntgs::ContiguousVector<std::string> vector{2};
    vector.emplace_back(STRING1);
    vector.emplace_back(STRING2);
    auto erased = cntgs::type_erase(std::move(vector));
    auto move_constructed_erased{std::move(erased)};
    cntgs::ContiguousVector<std::string> restored;
    SUBCASE("by move") { restored = cntgs::ContiguousVector<std::string>{std::move(move_constructed_erased)}; }
    SUBCASE("by lvalue reference")
    {
        for (size_t i = 0; i < 2; i++)
        {
            restored = cntgs::ContiguousVector<std::string>{move_constructed_erased};
        }
    }
    auto&& [string_one] = restored[0];
    CHECK_EQ(STRING1, string_one);
    auto&& [string_two] = restored[1];
    CHECK_EQ(STRING2, string_two);
}

TEST_CASE("ContiguousTest: std::any OneFixed emplace_back->reserve->emplace_back")
{
    cntgs::ContiguousVector<cntgs::FixedSize<std::string>, std::string, int> vector{1, {1}};
    vector.emplace_back(std::array{STRING1}, STRING1, 42);
    vector.reserve(2);
    vector.emplace_back(std::array{STRING2}, STRING2, 84);
    auto&& [a, b, c] = vector[0];
    CHECK_EQ(STRING1, a.front());
    CHECK_EQ(STRING1, b);
    CHECK_EQ(42, c);
    auto&& [d, e, f] = vector[1];
    CHECK_EQ(STRING2, d.front());
    CHECK_EQ(STRING2, e);
    CHECK_EQ(84, f);
}

TEST_CASE("ContiguousTest: trivial OneFixed emplace_back->reserve->emplace_back")
{
    cntgs::ContiguousVector<cntgs::FixedSize<float>, int> vector{1, {FLOATS1.size()}};
    vector.emplace_back(FLOATS1, 42);
    vector.reserve(2);
    vector.emplace_back(FLOATS1, 84);
    auto&& [a, b] = vector[0];
    CHECK(test::range_equal(FLOATS1, a));
    CHECK_EQ(42, b);
    auto&& [c, d] = vector[1];
    CHECK(test::range_equal(FLOATS1, c));
    CHECK_EQ(84, d);
}

TEST_CASE("ContiguousTest: trivial VaryingSize emplace_back->reserve->emplace_back")
{
    cntgs::ContiguousVector<cntgs::VaryingSize<float>, int> vector{1, FLOATS1.size() * sizeof(float)};
    vector.emplace_back(FLOATS1, 42);
    vector.reserve(2, FLOATS1.size() * sizeof(float) + FLOATS2.size() * sizeof(float));
    vector.emplace_back(FLOATS2, 84);
    auto&& [a, b] = vector[0];
    CHECK(test::range_equal(FLOATS1, a));
    CHECK_EQ(42, b);
    auto&& [c, d] = vector[1];
    CHECK(test::range_equal(FLOATS2, c));
    CHECK_EQ(84, d);
}

TEST_CASE("ContiguousTest: std::unique_ptr VaryingSize reserve and shrink")
{
    cntgs::ContiguousVector<cntgs::VaryingSize<std::unique_ptr<int>>, std::unique_ptr<int>> vector{0, 0};
    vector.reserve(1, 3 * sizeof(std::unique_ptr<int>));
    vector.emplace_back(array_one_unique_ptr(), std::make_unique<int>(20));
    auto&& [a, b] = vector[0];
    CHECK_EQ(10, *a.front());
    CHECK_EQ(20, *b);
    vector.reserve(3, 8 * sizeof(std::unique_ptr<int>));
    vector.emplace_back(array_two_unique_ptr(), std::make_unique<int>(50));
    vector.emplace_back(array_one_unique_ptr(), std::make_unique<int>(20));
    auto&& [c, d] = vector[1];
    CHECK_EQ(30, *c.front());
    CHECK_EQ(40, *c.back());
    CHECK_EQ(50, *d);
    auto&& [e, f] = vector[2];
    CHECK_EQ(10, *e.front());
    CHECK_EQ(20, *f);
}

TEST_CASE("ContiguousTest: trivial OneFixed reserve with polymorphic_allocator")
{
    cntgs::BasicContiguousVector<std::pmr::polymorphic_allocator<std::byte>, cntgs::FixedSize<float>, int> vector{0,
                                                                                                                  {10}};
    vector.reserve(2);
    CHECK_EQ(2, vector.capacity());
}

#ifdef __cpp_lib_span
TEST_CASE("ContiguousTest: cntgs::Span can be implicitly converted to std::span")
{
    std::array<int, 1> ints{42};
    cntgs::Span<int> span{ints.data(), ints.size()};
    std::span<int> actual = span;
    CHECK_EQ(1, actual.size());
    CHECK_EQ(42, actual.front());
}
#endif

using PlainAligned = cntgs::ContiguousVector<char, cntgs::AlignAs<uint32_t, 8>>;
using OneVaryingAligned = cntgs::ContiguousVector<cntgs::VaryingSize<cntgs::AlignAs<float, 16>>, uint32_t>;
using TwoVaryingAligned = cntgs::ContiguousVector<uint32_t, cntgs::VaryingSize<cntgs::AlignAs<float, 8>>,
                                                  cntgs::VaryingSize<cntgs::AlignAs<float, 8>>>;
using OneFixedAligned = cntgs::ContiguousVector<uint32_t, cntgs::FixedSize<cntgs::AlignAs<float, 32>>>;
using TwoFixedAligned = cntgs::ContiguousVector<cntgs::FixedSize<cntgs::AlignAs<float, 8>>,
                                                cntgs::AlignAs<uint32_t, 16>, cntgs::FixedSize<float>>;
using TwoFixedAlignedAlt =
    cntgs::ContiguousVector<cntgs::FixedSize<cntgs::AlignAs<float, 32>>, cntgs::FixedSize<uint32_t>, uint32_t>;
using OneFixedOneVaryingAligned = cntgs::ContiguousVector<cntgs::FixedSize<cntgs::AlignAs<float, 16>>, uint32_t,
                                                          cntgs::VaryingSize<cntgs::AlignAs<float, 8>>>;

TEST_CASE("ContiguousTest: PlainAligned size() and capacity()")
{
    PlainAligned v{2};
    v.emplace_back('a', 10u);
    check_size1_and_capacity2(v);
}

TEST_CASE("ContiguousTest: OneVaryingAligned size() and capacity()")
{
    OneVaryingAligned v{2, FLOATS1.size() * sizeof(float)};
    v.emplace_back(FLOATS1, 10u);
    check_size1_and_capacity2(v);
}

TEST_CASE("ContiguousTest: TwoVaryingAligned size() and capacity()")
{
    TwoVaryingAligned v{2, FLOATS1.size() * sizeof(float) + FLOATS2.size() * sizeof(float)};
    v.emplace_back(10u, FLOATS1, FLOATS2);
    check_size1_and_capacity2(v);
}

TEST_CASE("ContiguousTest: OneFixedAligned size(), capacity() and memory_consumption()")
{
    OneFixedAligned v{2, {FLOATS1.size()}};
    v.emplace_back(10u, FLOATS1);
    check_size1_and_capacity2(v);
    CHECK_EQ(2 * (32 + FLOATS1.size() * sizeof(float)), v.memory_consumption());
}

TEST_CASE("ContiguousTest: TwoFixedAligned size() and capacity()")
{
    TwoFixedAligned v{2, {FLOATS1.size(), FLOATS2.size()}};
    v.emplace_back(FLOATS1, 10u, FLOATS2);
    check_size1_and_capacity2(v);
    const auto size = (FLOATS1.size() * sizeof(float) + 8 + sizeof(uint32_t) + FLOATS2.size() * sizeof(float));
    CHECK_EQ(2 * (size + size % 8) + 7, v.memory_consumption());
}

TEST_CASE("ContiguousTest: OneFixedOneVaryingAligned size() and capacity()")
{
    OneFixedOneVaryingAligned v{2, FLOATS2.size() * sizeof(float), {FLOATS1.size()}};
    v.emplace_back(FLOATS1, 10u, FLOATS2);
    check_size1_and_capacity2(v);
}

template <std::size_t Alignment, class T>
void check_alignment(T* t)
{
    auto* ptr = static_cast<void*>(t);
    auto size = std::numeric_limits<size_t>::max();
    std::align(Alignment, sizeof(uint32_t), ptr, size);
    CHECK_EQ(t, ptr);
}

template <std::size_t Alignment, class T>
void check_alignment(T& t)
{
    check_alignment<Alignment>(std::data(t));
}

TEST_CASE("ContiguousTest: PlainAligned emplace_back() and subscript operator")
{
    PlainAligned vector{5};
    for (uint32_t i = 0; i < 5; ++i)
    {
        vector.emplace_back('a', i);
    }
    for (uint32_t i = 0; i < 5; ++i)
    {
        auto&& [a, b] = vector[i];
        CHECK_EQ('a', a);
        CHECK_EQ(i, b);
        check_alignment<8>(&b);
    }
}

TEST_CASE("ContiguousTest: OneVaryingAligned emplace_back() and subscript operator")
{
    OneVaryingAligned vector{5, 5 * FLOATS1.size() * sizeof(float)};
    for (uint32_t i = 0; i < 5; ++i)
    {
        vector.emplace_back(FLOATS1, i);
    }
    for (uint32_t i = 0; i < 5; ++i)
    {
        auto&& [a, b] = vector[i];
        CHECK(test::range_equal(FLOATS1, a));
        CHECK_EQ(i, b);
        check_alignment<16>(a);
    }
}

TEST_CASE("ContiguousTest: TwoVaryingAligned emplace_back() and subscript operator")
{
    TwoVaryingAligned vector{5, 5 * (FLOATS1.size() * sizeof(float) + FLOATS2.size() * sizeof(float))};
    for (uint32_t i = 0; i < 5; ++i)
    {
        vector.emplace_back(i, FLOATS1, FLOATS2);
    }
    for (uint32_t i = 0; i < 5; ++i)
    {
        auto&& [a, b, c] = vector[i];
        CHECK_EQ(i, a);
        CHECK(test::range_equal(FLOATS1, b));
        CHECK(test::range_equal(FLOATS2, c));
        check_alignment<8>(b);
        check_alignment<8>(c);
    }
}

TEST_CASE("ContiguousTest: OneFixedAligned emplace_back() and subscript operator")
{
    OneFixedAligned vector{5, {FLOATS1.size()}};
    for (uint32_t i = 0; i < 5; ++i)
    {
        vector.emplace_back(i, FLOATS1);
    }
    for (uint32_t i = 0; i < 5; ++i)
    {
        auto&& [a, b] = vector[i];
        CHECK_EQ(i, a);
        CHECK(test::range_equal(FLOATS1, b));
        check_alignment<32>(b);
    }
}

TEST_CASE("ContiguousTest: TwoFixedAligned emplace_back() and subscript operator")
{
    TwoFixedAligned vector{5, {FLOATS1.size(), FLOATS2.size()}};
    for (uint32_t i = 0; i < 5; ++i)
    {
        vector.emplace_back(FLOATS1, i, FLOATS2);
    }
    for (uint32_t i = 0; i < 5; ++i)
    {
        auto&& [a, b, c] = vector[i];
        CHECK(test::range_equal(FLOATS1, a));
        CHECK_EQ(i, b);
        CHECK(test::range_equal(FLOATS2, c));
        check_alignment<8>(a);
        check_alignment<16>(&b);
    }
}

TEST_CASE("ContiguousTest: TwoFixedAlignedAlt emplace_back() and subscript operator")
{
    std::array uint2{50u, 100u, 150u};
    TwoFixedAlignedAlt vector{5, {FLOATS1.size(), uint2.size()}};
    for (uint32_t i = 0; i < 5; ++i)
    {
        vector.emplace_back(FLOATS1, uint2, i);
    }
    for (uint32_t i = 0; i < 5; ++i)
    {
        auto&& [a, b, c] = vector[i];
        CHECK(test::range_equal(FLOATS1, a));
        CHECK(test::range_equal(uint2, b));
        CHECK_EQ(i, c);
        check_alignment<32>(a);
    }
}

TEST_CASE("ContiguousTest: OneFixedOneVaryingAligned emplace_back() and subscript operator")
{
    OneFixedOneVaryingAligned vector{5, 5 * FLOATS2.size() * sizeof(float), {FLOATS1.size()}};
    for (uint32_t i = 0; i < 5; ++i)
    {
        vector.emplace_back(FLOATS1, i, FLOATS2);
    }
    for (uint32_t i = 0; i < 5; ++i)
    {
        auto&& [a, b, c] = vector[i];
        CHECK(test::range_equal(FLOATS1, a));
        CHECK_EQ(i, b);
        CHECK(test::range_equal(FLOATS2, c));
        check_alignment<16>(a);
        check_alignment<8>(c);
    }
}

template <class Allocator, std::size_t N>
auto check_memory_resource_was_used(const Allocator& allocator, const std::array<std::byte, N>& buffer,
                                    const std::pmr::monotonic_buffer_resource& resource)
{
    CHECK_EQ(&resource, allocator.resource());
    CHECK(std::any_of(buffer.begin(), buffer.end(),
                      [](auto&& byte)
                      {
                          return byte != std::byte{};
                      }));
}

TEST_CASE("ContiguousTest: OneFixedUniquePtr with polymorphic_allocator")
{
    using Alloc = std::pmr::polymorphic_allocator<int>;
    std::array<std::byte, 256> buffer{};
    std::pmr::monotonic_buffer_resource resource{buffer.data(), buffer.size()};
    SUBCASE("vector")
    {
        auto vector = fixed_vector_of_unique_ptrs<Alloc>(&resource);
        check_memory_resource_was_used(vector.get_allocator(), buffer, resource);
    }
    SUBCASE("value_type")
    {
        auto vector = fixed_vector_of_unique_ptrs();
        using ValueType = typename decltype(fixed_vector_of_unique_ptrs<Alloc>(&resource))::value_type;
        ValueType value{std::move(vector[0]), &resource};
        SUBCASE("move construct from tuple and allocator")
        {
            check_memory_resource_was_used(value.get_allocator(), buffer, resource);
        }
        SUBCASE("move construct from element and allocator")
        {
            ValueType value2{std::move(value), &resource};
            check_memory_resource_was_used(value2.get_allocator(), buffer, resource);
            ValueType value3{std::move(value2), Alloc{}};
            CHECK_EQ(Alloc{}, value3.get_allocator());
        }
    }
}
}  // namespace test_contiguous
