#include "cntgs/contiguous.hpp"
#include "utils/functional.hpp"
#include "utils/range.hpp"

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
TEST_SUITE_BEGIN(CNTGS_TEST_CPP_VERSION);

using namespace cntgs;

using UInt8 = unsigned char;

using Plain = cntgs::ContiguousVector<uint32_t, float>;
using OneVarying = cntgs::ContiguousVector<uint32_t, cntgs::VaryingSize<float>>;
using TwoVarying = cntgs::ContiguousVector<uint32_t, cntgs::VaryingSize<float>, cntgs::VaryingSize<float>>;
using OneFixed = cntgs::ContiguousVector<uint32_t, cntgs::FixedSize<float>>;
using TwoFixed = cntgs::ContiguousVector<cntgs::FixedSize<float>, uint32_t, cntgs::FixedSize<float>>;
using OneFixedOneVarying = cntgs::ContiguousVector<cntgs::FixedSize<float>, uint32_t, cntgs::VaryingSize<float>>;
using OneFixedUniquePtr = cntgs::ContiguousVector<cntgs::FixedSize<std::unique_ptr<int>>, std::unique_ptr<int>>;
using OneVaryingUniquePtr = cntgs::ContiguousVector<cntgs::VaryingSize<std::unique_ptr<int>>, std::unique_ptr<int>>;

template <class Vector = OneVarying>
struct ToValueType
{
    template <class T>
    auto operator()(T&& t)
    {
        return typename Vector::value_type{std::forward<T>(t)};
    }
};

struct TestMemoryResource
{
    using allocator_type = std::pmr::polymorphic_allocator<std::byte>;

    std::array<std::byte, 256> buffer{};
    std::pmr::monotonic_buffer_resource resource{buffer.data(), buffer.size()};

    auto get_allocator() noexcept { return std::pmr::polymorphic_allocator<std::byte>{&resource}; }

    auto check_was_used(const std::pmr::polymorphic_allocator<std::byte>& allocator)
    {
        CHECK_EQ(&resource, allocator.resource());
        CHECK(std::any_of(buffer.begin(), buffer.end(),
                          [](auto&& byte)
                          {
                              return byte != std::byte{};
                          }));
    }

    auto check_was_not_used(const std::pmr::polymorphic_allocator<std::byte>& allocator)
    {
        CHECK_EQ(&resource, allocator.resource());
        CHECK(std::all_of(buffer.begin(), buffer.end(),
                          [](auto&& byte)
                          {
                              return byte == std::byte{};
                          }));
    }
};

template <bool IsNoThrow>
struct Thrower
{
    Thrower() noexcept(IsNoThrow);
    Thrower(const Thrower&) noexcept(IsNoThrow);
    Thrower(Thrower&&) noexcept(IsNoThrow);
    Thrower& operator=(const Thrower&) noexcept(IsNoThrow);
    Thrower& operator=(Thrower&&) noexcept(IsNoThrow);
    bool operator==(const Thrower&) const noexcept(IsNoThrow);
    bool operator<(const Thrower&) const noexcept(IsNoThrow);
};

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

template <class Allocator = std::allocator<int>>
auto fixed_vector_of_unique_ptrs(Allocator allocator = {})
{
    cntgs::BasicContiguousVector<Allocator, cntgs::FixedSize<std::unique_ptr<int>>, std::unique_ptr<int>> vector{
        2, {1}, allocator};
    vector.emplace_back(array_one_unique_ptr(10), std::make_unique<int>(20));
    vector.emplace_back(array_one_unique_ptr(30), std::make_unique<int>(40));
    return vector;
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

TEST_CASE("ContiguousTest: ContiguousElement converting constructors")
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

TEST_CASE("ContiguousTest: ContiguousElement construct from move-only type")
{
    auto vector = fixed_vector_of_unique_ptrs();
    using ValueType = decltype(vector)::value_type;
    SUBCASE("move from reference")
    {
        ValueType value{vector[0]};
        CHECK_EQ(nullptr, cntgs::get<1>(vector[0]));
        CHECK_EQ(20, *cntgs::get<1>(value));
    }
    TestMemoryResource resource;
    using AllocValueType = typename decltype(fixed_vector_of_unique_ptrs(resource.get_allocator()))::value_type;
    SUBCASE("move from reference and allocator")
    {
        AllocValueType value{vector[0], resource.get_allocator()};
        resource.check_was_used(value.get_allocator());
        CHECK_EQ(20, *cntgs::get<1>(value));
    }
    SUBCASE("move from value_type and allocator")
    {
        AllocValueType value{ValueType{vector[0]}, resource.get_allocator()};
        resource.check_was_used(value.get_allocator());
        AllocValueType value2{std::move(value)};
        CHECK_EQ(resource.get_allocator(), value2.get_allocator());
        CHECK_EQ(20, *cntgs::get<1>(value2));
    }
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

TEST_CASE("ContiguousTest: value_type can be move assigned")
{
    auto vector = fixed_vector_of_unique_ptrs();
    OneFixedUniquePtr::value_type value1{vector[0]};
    OneFixedUniquePtr::value_type value2{vector[1]};
    value1 = std::move(value2);
    auto&& [a, b] = value1;
    CHECK(test::range_equal(array_one_unique_ptr(30), a, test::DereferenceEqual{}));
    CHECK_EQ(40, *b);
}

TEST_CASE("ContiguousTest: value_type can be swapped")
{
    auto vector = fixed_vector_of_unique_ptrs();
    OneFixedUniquePtr::value_type value1{vector[0]};
    OneFixedUniquePtr::value_type value2{vector[1]};
    using std::swap;
    swap(value1, value2);
    swap(value1, value1);
    auto&& [a, b] = value1;
    CHECK(test::range_equal(array_one_unique_ptr(30), a, test::DereferenceEqual{}));
    CHECK_EQ(40, *b);
    auto&& [c, d] = value2;
    CHECK(test::range_equal(array_one_unique_ptr(10), c, test::DereferenceEqual{}));
    CHECK_EQ(20, *d);
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
    auto&& [a, b] = vector[0];
    CHECK_EQ(10u, a);
    CHECK(test::range_equal(iota, b));
}

TEST_CASE("ContiguousTest: OneFixed emplace_back std::views::iota")
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
        auto&& [a, b] = vector[i];
        CHECK_EQ(10u, a);
        CHECK(test::range_equal(bound_iota, b));
    }
}
#endif

TEST_CASE("ContiguousTest: OneVarying sizeof(iterator)")
{
    struct Expected
    {
        std::size_t i{};
        std::byte* memory;
        std::byte** last_element_address{};
        std::byte* last_element{};
    };
    CHECK_EQ(sizeof(Expected), sizeof(OneVarying::iterator));
}

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

TEST_CASE("ContiguousTest: ContiguousVector of std::string comparison operators")
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

TEST_CASE("ContiguousTest: ContiguousVector of VaryingSize unsigned char comparison operators")
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

TEST_CASE("ContiguousTest: ContiguousVector of FixedSize unsigned char comparison operators")
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

template <template <bool> class T>
void check_conditionally_nothrow_comparison()
{
    CHECK(noexcept(std::declval<const T<true>&>() == std::declval<const T<true>&>()));
    CHECK(noexcept(std::declval<const T<true>&>() < std::declval<const T<true>&>()));
    CHECK_FALSE(noexcept(std::declval<const T<false>&>() == std::declval<const T<false>&>()));
    CHECK_FALSE(noexcept(std::declval<const T<false>&>() < std::declval<const T<false>&>()));
}

template <template <bool> class T>
void check_always_nothrow_move_construct()
{
    CHECK(std::is_nothrow_move_constructible_v<T<true>>);
    CHECK(std::is_nothrow_move_constructible_v<T<false>>);
}

template <template <bool> class T>
void check_conditionally_nothrow_move_assign()
{
    CHECK(std::is_nothrow_move_assignable_v<T<true>>);
    CHECK_FALSE(std::is_nothrow_move_assignable_v<T<false>>);
}

template <template <bool> class T>
void check_conditionally_nothrow_copy_assign()
{
    CHECK(std::is_nothrow_copy_assignable_v<T<true>>);
    CHECK_FALSE(std::is_nothrow_copy_assignable_v<T<false>>);
}

template <bool IsNoThrow>
using NoThrowVector = cntgs::ContiguousVector<cntgs::FixedSize<Thrower<IsNoThrow>>>;

TEST_CASE("ContiguousTest: ContiguousVector is conditionally nothrow")
{
    check_conditionally_nothrow_comparison<NoThrowVector>();
    check_always_nothrow_move_construct<NoThrowVector>();
    CHECK(std::is_nothrow_move_assignable_v<NoThrowVector<true>>);
    CHECK(std::is_nothrow_move_assignable_v<NoThrowVector<false>>);
    CHECK_FALSE(std::is_nothrow_move_assignable_v<
                cntgs::BasicContiguousVector<std::pmr::polymorphic_allocator<std::byte>, float>>);
}

template <bool IsNoThrow>
using NoThrowElement = typename cntgs::ContiguousVector<cntgs::FixedSize<Thrower<IsNoThrow>>>::value_type;

TEST_CASE("ContiguousTest: ContiguousElement is conditionally nothrow")
{
    check_conditionally_nothrow_comparison<NoThrowElement>();
    check_always_nothrow_move_construct<NoThrowElement>();
    check_conditionally_nothrow_move_assign<NoThrowElement>();
    check_conditionally_nothrow_copy_assign<NoThrowElement>();
}

template <bool IsNoThrow>
using NoThrowReference = typename cntgs::ContiguousVector<cntgs::FixedSize<Thrower<IsNoThrow>>>::reference;

TEST_CASE("ContiguousTest: ContiguousReference is conditionally nothrow")
{
    check_conditionally_nothrow_comparison<NoThrowReference>();
    check_always_nothrow_move_construct<NoThrowReference>();
    CHECK(std::is_nothrow_copy_constructible_v<NoThrowReference<true>>);
    CHECK(std::is_nothrow_copy_constructible_v<NoThrowReference<false>>);
    check_conditionally_nothrow_move_assign<NoThrowReference>();
    check_conditionally_nothrow_copy_assign<NoThrowReference>();
}

template <class T>
void check_iterator(T& vector)
{
    auto begin = vector.begin();
    using IterTraits = std::iterator_traits<decltype(begin)>;
    CHECK(std::is_same_v<std::random_access_iterator_tag, typename IterTraits::iterator_category>);
    CHECK_EQ(0, begin.index());
    CHECK_EQ(vector.data_begin(), begin.data());
    CHECK_EQ(vector[0].data_begin(), begin.data());
    auto next = std::next(begin);
    CHECK_EQ(1, next.index());
    CHECK_EQ(vector.data_end(), vector.end().data());
    CHECK_EQ(vector[1].data_begin(), next.data());
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

auto make_varying_size_vector()
{
    OneVarying vector{4, 4 * (FLOATS1.size() * 2 + FLOATS1_ALT.size() + 3) * sizeof(float)};
    vector.emplace_back(10u, FLOATS1);
    vector.emplace_back(20u, FLOATS1_ALT);
    vector.emplace_back(10u, FLOATS1);
    vector.emplace_back(15u, floats1(10.f, 20.f, 30.f));
    return vector;
}

TEST_CASE("ContiguousTest: Contiguous(Const)Reference converting constructors")
{
    auto vector = make_varying_size_vector();
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

template <class Vector, class LhsTransformer, class RhsTransformer>
void check_equality(Vector& vector, LhsTransformer lhs_transformer, RhsTransformer rhs_transformer)
{
    CHECK_EQ(lhs_transformer(vector[0]), rhs_transformer(vector[2]));
    CHECK_EQ(lhs_transformer(std::as_const(vector)[0]), rhs_transformer(vector[2]));
    CHECK_EQ(lhs_transformer(vector[0]), rhs_transformer(std::as_const(vector)[2]));
    CHECK_EQ(lhs_transformer(std::as_const(vector)[0]), rhs_transformer(std::as_const(vector)[2]));
}

template <class Vector, class LhsTransformer, class RhsTransformer>
void check_inequality(Vector& vector, LhsTransformer lhs_transformer, RhsTransformer rhs_transformer)
{
    CHECK_NE(lhs_transformer(vector[0]), rhs_transformer(vector[1]));
    CHECK_NE(lhs_transformer(std::as_const(vector)[0]), rhs_transformer(vector[1]));
    CHECK_NE(lhs_transformer(vector[0]), rhs_transformer(std::as_const(vector)[1]));
    CHECK_NE(lhs_transformer(std::as_const(vector)[0]), rhs_transformer(std::as_const(vector)[1]));
}

template <class Vector, class LhsTransformer, class RhsTransformer>
void check_less(Vector& vector, LhsTransformer lhs_transformer, RhsTransformer rhs_transformer)
{
    CHECK_LT(lhs_transformer(vector[2]), rhs_transformer(vector[1]));
    CHECK_LT(lhs_transformer(std::as_const(vector)[0]), rhs_transformer(vector[1]));
    CHECK_LT(lhs_transformer(vector[2]), rhs_transformer(std::as_const(vector)[3]));
    CHECK_FALSE(lhs_transformer(std::as_const(vector)[0]) < rhs_transformer(std::as_const(vector)[2]));
}

template <class Vector, class LhsTransformer, class RhsTransformer>
void check_less_equal(Vector& vector, LhsTransformer lhs_transformer, RhsTransformer rhs_transformer)
{
    CHECK_LE(lhs_transformer(vector[0]), rhs_transformer(vector[1]));
    CHECK_LE(lhs_transformer(std::as_const(vector)[0]), rhs_transformer(vector[1]));
    CHECK_LE(lhs_transformer(vector[2]), rhs_transformer(std::as_const(vector)[3]));
    CHECK_FALSE(lhs_transformer(std::as_const(vector)[1]) <= rhs_transformer(std::as_const(vector)[2]));
}

template <class Vector, class LhsTransformer, class RhsTransformer>
void check_greater(Vector& vector, LhsTransformer lhs_transformer, RhsTransformer rhs_transformer)
{
    CHECK_GT(lhs_transformer(vector[1]), rhs_transformer(vector[0]));
    CHECK_GT(lhs_transformer(std::as_const(vector)[1]), rhs_transformer(vector[0]));
    CHECK_GT(lhs_transformer(vector[3]), rhs_transformer(std::as_const(vector)[2]));
    CHECK_FALSE(lhs_transformer(std::as_const(vector)[2]) >= rhs_transformer(std::as_const(vector)[1]));
}

template <class Vector, class LhsTransformer, class RhsTransformer>
void check_greater_equal(Vector& vector, LhsTransformer lhs_transformer, RhsTransformer rhs_transformer)
{
    CHECK_GE(lhs_transformer(vector[1]), rhs_transformer(vector[0]));
    CHECK_GE(lhs_transformer(std::as_const(vector)[0]), rhs_transformer(vector[2]));
    CHECK_FALSE(lhs_transformer(vector[0]) >= rhs_transformer(std::as_const(vector)[1]));
    CHECK_GE(lhs_transformer(std::as_const(vector)[3]), rhs_transformer(std::as_const(vector)[2]));
}

TEST_CASE("ContiguousTest: OneVarying::(const_)reference comparison operators")
{
    auto vector = make_varying_size_vector();
    SUBCASE("equal") { check_equality(vector, test::Identity{}, test::Identity{}); }
    SUBCASE("not equal") { check_inequality(vector, test::Identity{}, test::Identity{}); }
    SUBCASE("less") { check_less(vector, test::Identity{}, test::Identity{}); }
    SUBCASE("less equal") { check_less_equal(vector, test::Identity{}, test::Identity{}); }
    SUBCASE("greater") { check_greater(vector, test::Identity{}, test::Identity{}); }
    SUBCASE("greater equal") { check_greater_equal(vector, test::Identity{}, test::Identity{}); }
}

TEST_CASE("ContiguousTest: OneVarying::value_type comparison operators")
{
    auto vector = make_varying_size_vector();
    SUBCASE("equal") { check_equality(vector, ToValueType{}, ToValueType{}); }
    SUBCASE("not equal") { check_inequality(vector, ToValueType{}, ToValueType{}); }
    SUBCASE("less") { check_less(vector, ToValueType{}, ToValueType{}); }
    SUBCASE("less equal") { check_less_equal(vector, ToValueType{}, ToValueType{}); }
    SUBCASE("greater") { check_greater(vector, ToValueType{}, ToValueType{}); }
    SUBCASE("greater equal") { check_greater_equal(vector, ToValueType{}, ToValueType{}); }
}

TEST_CASE("ContiguousTest: OneVarying::(const_)reference to OneVarying::value_type comparison operators")
{
    auto vector = make_varying_size_vector();
    SUBCASE("equal") { check_equality(vector, test::Identity{}, ToValueType{}); }
    SUBCASE("not equal") { check_inequality(vector, test::Identity{}, ToValueType{}); }
    SUBCASE("less") { check_less(vector, test::Identity{}, ToValueType{}); }
    SUBCASE("less equal") { check_less_equal(vector, test::Identity{}, ToValueType{}); }
    SUBCASE("greater") { check_greater(vector, test::Identity{}, ToValueType{}); }
    SUBCASE("greater equal") { check_greater_equal(vector, test::Identity{}, ToValueType{}); }
}

TEST_CASE("ContiguousTest: OneVarying::value_type to OneVarying::(const_)reference comparison operators")
{
    auto vector = make_varying_size_vector();
    SUBCASE("equal") { check_equality(vector, ToValueType{}, test::Identity{}); }
    SUBCASE("not equal") { check_inequality(vector, ToValueType{}, test::Identity{}); }
    SUBCASE("less") { check_less(vector, ToValueType{}, test::Identity{}); }
    SUBCASE("less equal") { check_less_equal(vector, ToValueType{}, test::Identity{}); }
    SUBCASE("greater") { check_greater(vector, ToValueType{}, test::Identity{}); }
    SUBCASE("greater equal") { check_greater_equal(vector, ToValueType{}, test::Identity{}); }
}

TEST_CASE(
    "ContiguousTest: ContiguousVector::value_type to ContiguousVector::(const_)reference for memcmp-compatible "
    "lexicographical comparisons")
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

TEST_CASE("ContiguousTest: TwoVarying erase(Iterator)")
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
    SUBCASE("erase first two")
    {
        auto it = vector.erase(vector.begin(), std::next(vector.begin(), 2));
        CHECK_EQ(vector.begin(), it);
        CHECK_EQ(1, vector.size());
        auto&& [a, b, c] = vector.front();
        CHECK(test::range_equal(FLOATS2_ALT, a));
        CHECK_EQ(30u, b);
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

TEST_CASE("ContiguousTest: TwoVarying erase(Iterator, Iterator)")
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
        auto&& [a, b, c] = vector.front();
        CHECK_EQ(30u, a);
        CHECK(test::range_equal(FLOATS1, b));
        CHECK(test::range_equal(FLOATS2_ALT, c));
    }
    SUBCASE("erase all")
    {
        vector.erase(vector.begin(), vector.end());
        CHECK_EQ(0, vector.size());
        vector.emplace_back(30u, FLOATS2_ALT, FLOATS2);
        auto&& [a, b, c] = vector.front();
        CHECK_EQ(30u, a);
        CHECK(test::range_equal(FLOATS2_ALT, b));
        CHECK(test::range_equal(FLOATS2, c));
    }
    SUBCASE("erase none")
    {
        vector.erase(vector.begin(), vector.begin());
        CHECK_EQ(3, vector.size());
        auto&& [a, b, c] = vector.front();
        CHECK_EQ(10u, a);
        CHECK(test::range_equal(FLOATS1, b));
        CHECK(test::range_equal(FLOATS2, c));
    }
}

TEST_CASE("ContiguousTest: OneFixed construct with span")
{
    const auto memory_size = 2 * (sizeof(uint32_t) + 2 * sizeof(float));
    auto ptr = std::make_unique<std::byte[]>(memory_size);
    OneFixed vector{cntgs::Span<std::byte>{ptr.get(), memory_size}, 2, std::array{FLOATS1.size()}};
    vector.emplace_back(10u, FLOATS1);
    auto&& [i, e] = vector[0];
    CHECK_EQ(10u, i);
    CHECK(test::range_equal(FLOATS1, e));
}

TEST_CASE("ContiguousTest: Plain construct with span")
{
    const auto memory_size = 2 * (sizeof(uint32_t) + sizeof(float));
    auto ptr = std::make_unique<std::byte[]>(memory_size);
    Plain vector{cntgs::Span<std::byte>{ptr.get(), memory_size}, 2};
    vector.emplace_back(10u, 5.f);
    auto&& [i, f] = vector[0];
    CHECK_EQ(10u, i);
    CHECK_EQ(5.f, f);
}

TEST_CASE("ContiguousTest: type_erase OneFixed and restore")
{
    OneFixed vector{2, {FLOATS2.size()}};
    vector.emplace_back(10u, FLOATS2);
    auto erased = cntgs::type_erase(std::move(vector));
    OneFixed restored;
    SUBCASE("by lvalue reference") { restored = cntgs::restore<OneFixed>(erased); }
    SUBCASE("by move") { restored = cntgs::restore<OneFixed>(std::move(erased)); }
    auto&& [i, e] = restored[0];
    CHECK_EQ(10u, i);
    CHECK(test::range_equal(FLOATS2, e));
}

TEST_CASE("ContiguousTest: type_erase OneFixed, restore and copy assign")
{
    using Vector = cntgs::ContiguousVector<std::string>;
    Vector vector{2};
    vector.emplace_back(STRING1);
    auto vector_copy{vector};
    vector_copy.emplace_back(STRING2);
    auto erased = cntgs::type_erase(std::move(vector));
    auto restored = cntgs::restore<Vector>(erased);
    restored = vector_copy;
    auto&& [a] = restored.back();
    CHECK_EQ(STRING2, a);
}

TEST_CASE("ContiguousTest: type_erase empty OneFixed, restore and copy assign")
{
    using Vector = cntgs::ContiguousVector<std::string>;
    Vector vector{0};
    auto vector_copy{vector};
    auto erased = cntgs::type_erase(std::move(vector));
    auto restored = cntgs::restore<Vector>(erased);
    restored = vector_copy;
    restored.reserve(1);
    restored.emplace_back(STRING1);
    auto&& [a] = restored.back();
    CHECK_EQ(STRING1, a);
}

TEST_CASE("ContiguousTest: std::string TypeErasedVector")
{
    using Vector = cntgs::ContiguousVector<std::string>;
    Vector vector{2};
    vector.emplace_back(STRING1);
    vector.emplace_back(STRING2);
    auto erased = cntgs::type_erase(std::move(vector));
    auto move_constructed_erased{std::move(erased)};
    std::optional<const Vector> restored;
    SUBCASE("by lvalue reference") { restored.emplace(cntgs::restore<Vector>(move_constructed_erased)); }
    SUBCASE("by move") { restored.emplace(cntgs::restore<Vector>(std::move(move_constructed_erased))); }
    auto&& [string_one] = (*restored)[0];
    CHECK_EQ(STRING1, string_one);
    auto&& [string_two] = (*restored)[1];
    CHECK_EQ(STRING2, string_two);
}

TEST_CASE("ContiguousTest: std::string OneFixed emplace_back->reserve->emplace_back")
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
    TestMemoryResource resource;
    cntgs::BasicContiguousVector<std::pmr::polymorphic_allocator<std::byte>, cntgs::FixedSize<float>, int> vector{
        0, {10}, resource.get_allocator()};
    vector.reserve(2);
    CHECK_EQ(2, vector.capacity());
    vector.emplace_back(std::array{1.f}, 10);
    resource.check_was_used(vector.get_allocator());
}

#if defined(__cpp_lib_constexpr_dynamic_alloc) && defined(__cpp_constinit)
TEST_CASE("ContiguousTest: OneFixed constinit")
{
    SUBCASE("empty vector")
    {
        static constinit OneFixed v{0, {2}};
        v.reserve(2);
        v.emplace_back(10u, FLOATS1);
        check_size1_and_capacity2(v);
    }
    SUBCASE("from mutable std::array")
    {
        static constinit std::array<std::byte, 12> MEM{};
        static constinit OneFixed v{cntgs::Span{MEM.data(), MEM.size()}, 2, {2}};
        v.emplace_back(10u, FLOATS1);
        check_size1_and_capacity2(v);
        auto&& [a, b] = v[0];
        CHECK_EQ(10u, a);
        CHECK(test::range_equal(FLOATS1, b));
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

TEST_CASE("ContiguousTest: OneFixedUniquePtr with polymorphic_allocator")
{
    TestMemoryResource resource;
    auto vector = fixed_vector_of_unique_ptrs(resource.get_allocator());
    resource.check_was_used(vector.get_allocator());
}

template <class Allocator = std::allocator<int>>
auto varying_vector_of_unique_ptrs(Allocator allocator = {})
{
    cntgs::BasicContiguousVector<Allocator, cntgs::VaryingSize<std::unique_ptr<int>>, std::unique_ptr<int>> vector{
        2, 2 * sizeof(std::unique_ptr<int>), allocator};
    vector.emplace_back(array_one_unique_ptr(10), std::make_unique<int>(20));
    vector.emplace_back(array_one_unique_ptr(30), std::make_unique<int>(40));
    return vector;
}

TEST_CASE("ContiguousTest: OneVaryingUniquePtr move assign empty vector")
{
    auto expected = varying_vector_of_unique_ptrs();
    decltype(expected) vector{};
    vector = std::move(expected);
    CHECK_EQ(10, *cntgs::get<0>(vector[0]).front());
}

TEST_CASE("ContiguousTest: OneVaryingUniquePtr with polymorphic_allocator move assignment")
{
    TestMemoryResource resource;
    auto vector = varying_vector_of_unique_ptrs(resource.get_allocator());
    TestMemoryResource resource2;
    SUBCASE("move into smaller vector")
    {
        decltype(vector) vector2{0, 0, resource2.get_allocator()};
        vector2 = std::move(vector);
        resource2.check_was_used(vector2.get_allocator());
        CHECK_EQ(10, *cntgs::get<0>(vector2[0]).front());
    }
    SUBCASE("move into larger vector")
    {
        decltype(vector) vector2{3, 10, resource2.get_allocator()};
        vector2 = std::move(vector);
        resource2.check_was_used(vector2.get_allocator());
        CHECK_EQ(10, *cntgs::get<0>(vector2[0]).front());
    }
}

TEST_CASE("ContiguousTest: OneFixedUniquePtr with polymorphic_allocator move assignment")
{
    TestMemoryResource resource;
    auto vector = fixed_vector_of_unique_ptrs(resource.get_allocator());
    TestMemoryResource resource2;
    SUBCASE("move into smaller vector")
    {
        decltype(vector) vector2{0, {}, resource2.get_allocator()};
        vector2 = std::move(vector);
        resource2.check_was_used(vector2.get_allocator());
        CHECK_EQ(10, *cntgs::get<0>(vector2[0]).front());
    }
    SUBCASE("move into larger vector")
    {
        decltype(vector) vector2{3, {1}, resource2.get_allocator()};
        vector2 = std::move(vector);
        resource2.check_was_used(vector2.get_allocator());
        CHECK_EQ(10, *cntgs::get<0>(vector2[0]).front());
    }
}

template <class Allocator = std::allocator<int>>
auto varying_vector_of_strings(Allocator allocator = {})
{
    cntgs::BasicContiguousVector<Allocator, cntgs::VaryingSize<std::string>, std::string> vector{
        2, 3 * sizeof(std::string), allocator};
    vector.emplace_back(std::array{STRING1, STRING2}, STRING1);
    vector.emplace_back(std::array{STRING1}, STRING2);
    return vector;
}

TEST_CASE("ContiguousTest: OneVaryingString copy construct")
{
    auto vector = varying_vector_of_strings();
    decltype(vector) vector2{vector};
    CHECK_EQ(vector, vector2);
}

TEST_CASE("ContiguousTest: OneVaryingString copy assign empty vector")
{
    auto expected = varying_vector_of_strings();
    decltype(expected) vector{};
    vector = expected;
    CHECK_EQ(expected, vector);
}

TEST_CASE("ContiguousTest: OneVaryingString with std::allocator copy assignment")
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

TEST_CASE("ContiguousTest: OneVaryingString with polymorphic_allocator copy assignment")
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

template <class Allocator = std::allocator<int>>
auto fixed_vector_of_strings(Allocator allocator = {})
{
    cntgs::BasicContiguousVector<Allocator, cntgs::FixedSize<std::string>, std::string> vector{2, {2}, allocator};
    vector.emplace_back(std::array{STRING1, STRING2}, STRING1);
    vector.emplace_back(std::array{STRING1, STRING2}, STRING2);
    return vector;
}

TEST_CASE("ContiguousTest: OneFixedString copy construct")
{
    auto vector = fixed_vector_of_strings();
    decltype(vector) vector2{vector};
    CHECK_EQ(vector, vector2);
    CHECK_EQ(2, vector2.get_fixed_size<0>());
}

TEST_CASE("ContiguousTest: OneFixedString copy assign empty vector")
{
    auto expected = fixed_vector_of_strings();
    decltype(expected) vector{};
    vector = expected;
    CHECK_EQ(expected, vector);
}

TEST_CASE("ContiguousTest: OneFixedString with std::allocator copy assignment")
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

TEST_CASE("ContiguousTest: OneFixedString with polymorphic_allocator copy assignment")
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

TEST_CASE("ContiguousTest: OneVaryingUniquePtr swap empty vector")
{
    auto expected = varying_vector_of_unique_ptrs();
    decltype(expected) vector{};
    using std::swap;
    swap(expected, vector);
    CHECK_EQ(10, *cntgs::get<0>(vector[0]).front());
}

TEST_CASE("ContiguousTest: OneVaryingUniquePtr swap same vector")
{
    auto vector = varying_vector_of_unique_ptrs();
    using std::swap;
    swap(vector, vector);
    CHECK_EQ(10, *cntgs::get<0>(vector[0]).front());
}

TEST_CASE("ContiguousTest: OneVaryingUniquePtr swap")
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
        CHECK_EQ(10, *cntgs::get<0>(vector2[0]).front());
    }
    SUBCASE("swap into larger vector")
    {
        decltype(vector) vector2{3, 10, resource2.get_allocator()};
        swap(vector2, vector);
        resource2.check_was_not_used(vector2.get_allocator());
        CHECK_EQ(10, *cntgs::get<0>(vector2[0]).front());
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
    auto&& [a, b] = vector[0];
    CHECK(test::range_equal(std::array{STRING2, STRING2}, a));
    CHECK_EQ(STRING2, b);
}

TEST_CASE("ContiguousTest: OneFixedString clear")
{
    auto vector = fixed_vector_of_strings();
    check_clear_followed_by_emplace_back(vector);
}

TEST_CASE("ContiguousTest: OneVaryingString clear")
{
    auto vector = varying_vector_of_strings();
    check_clear_followed_by_emplace_back(vector);
}

TEST_SUITE_END();
}  // namespace test_contiguous
