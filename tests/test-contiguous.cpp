#include "cntgs/contiguous.h"
#include "utils/range.h"

#include <doctest/doctest.h>

#include <array>
#include <list>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace test_contiguous
{
using namespace cntgs;

using NoSpecialParameter = cntgs::ContiguousVector<uint32_t, float>;
using OneVarying = cntgs::ContiguousVector<uint32_t, cntgs::VaryingSize<float>>;
using TwoVarying = cntgs::ContiguousVector<uint32_t, cntgs::VaryingSize<float>, cntgs::VaryingSize<float>>;
using OneFixed = cntgs::ContiguousVector<uint32_t, cntgs::FixedSize<float>>;
using TwoFixed = cntgs::ContiguousVector<cntgs::FixedSize<float>, uint32_t, cntgs::FixedSize<float>>;
using OneFixedOneVarying = cntgs::ContiguousVector<cntgs::FixedSize<float>, uint32_t, cntgs::VaryingSize<float>>;

TEST_CASE("ContiguousTest: no alignment: size() and capacity()")
{
    std::array firsts{1.f, 2.f};
    std::array seconds{1.f, 2.f};
    std::optional<std::variant<NoSpecialParameter, OneVarying, TwoVarying, OneFixed, TwoFixed, OneFixedOneVarying>>
        vector;
    SUBCASE("no special parameter")
    {
        NoSpecialParameter v{2};
        v.emplace_back(10u, 1.5f);
        vector.emplace(std::move(v));
    }
    SUBCASE("one varying")
    {
        OneVarying v{2, firsts.size() * sizeof(float)};
        v.emplace_back(10u, firsts);
        vector.emplace(std::move(v));
    }
    SUBCASE("two varying")
    {
        TwoVarying v{2, firsts.size() * sizeof(float) + seconds.size() * sizeof(float)};
        v.emplace_back(10u, firsts, seconds);
        vector.emplace(std::move(v));
    }
    SUBCASE("one fixed")
    {
        OneFixed v{2, {firsts.size()}};
        v.emplace_back(10u, firsts);
        vector.emplace(std::move(v));
    }
    SUBCASE("two fixed")
    {
        TwoFixed v{2, {firsts.size(), seconds.size()}};
        v.emplace_back(firsts, 10u, seconds);
        vector.emplace(std::move(v));
    }
    SUBCASE("one fixed one varying")
    {
        OneFixedOneVarying v{2, seconds.size() * sizeof(float), {firsts.size()}};
        v.emplace_back(firsts, 10u, seconds);
        vector.emplace(std::move(v));
    }
    CHECK(vector);
    std::visit(
        [&](auto&& v) {
            CHECK_EQ(1, v.size());
            CHECK_EQ(2, v.capacity());
            CHECK_FALSE(v.empty());
        },
        *vector);
}

TEST_CASE("ContiguousTest: one varying size: empty()")
{
    OneVarying vector{0, {}};
    CHECK(vector.empty());
}

TEST_CASE("ContiguousTest: two fixed size: get_fixed_size<I>()")
{
    TwoFixed vector{2, {10, 20}};
    CHECK_EQ(10, vector.get_fixed_size<0>());
    CHECK_EQ(20, vector.get_fixed_size<1>());
}

TEST_CASE("ContiguousTest: one varying size: reference can be converted to value_type")
{
    std::array elements{1.f, 2.f};
    OneVarying vector{2, elements.size() * sizeof(float)};
    vector.emplace_back(10u, elements);
    OneVarying::value_type value = vector[0];
    CHECK_EQ(10u, cntgs::get<0>(value));
}

TEST_CASE("ContiguousTest: one fixed one varying size: correct memory_consumption()")
{
    using Vector = cntgs::ContiguousVector<cntgs::FixedSize<uint16_t>, uint32_t, cntgs::VaryingSize<float>>;
    const auto varying_byte_count = 6 * sizeof(float);
    Vector vector{2, varying_byte_count, {1}};
    const auto expected =
        2 * (1 * sizeof(uint16_t) + sizeof(float*) + sizeof(std::byte*) + sizeof(uint32_t)) + varying_byte_count;
    CHECK_EQ(expected, vector.memory_consumption());
}

TEST_CASE("ContiguousTest: two fixed size: correct memory_consumption()")
{
    TwoFixed vector{2, {1, 2}};
    const auto expected = 2 * (1 * sizeof(float) + sizeof(uint32_t) + 2 * sizeof(float));
    CHECK_EQ(expected, vector.memory_consumption());
}

TEST_CASE("ContiguousTest: two varying size: emplace_back with arrays and subscript operator")
{
    std::array expected_firsts{1.f, 2.f};
    std::array expected_seconds{-1.f, -2.f, -3.f, -4.f, -5.f};
    TwoVarying vector{1, expected_firsts.size() * sizeof(float) + expected_seconds.size() * sizeof(float)};
    vector.emplace_back(10u, expected_firsts, expected_seconds);
    auto&& [i, firsts, seconds] = vector[0];
    CHECK_EQ(10u, i);
    CHECK(test::range_equal(expected_firsts, firsts));
    CHECK(test::range_equal(expected_seconds, seconds));
}

TEST_CASE("ContiguousTest: one varying size: emplace_back with lists and subscript operator")
{
    std::list expected_firsts{1.f, 2.f};
    OneVarying vector{1, expected_firsts.size() * sizeof(float)};
    vector.emplace_back(10u, expected_firsts);
    auto&& [i, firsts] = vector[0];
    CHECK_EQ(10u, i);
    CHECK(test::range_equal(expected_firsts, firsts));
}

TEST_CASE("ContiguousTest: one varying size: subscript operator returns a reference")
{
    OneVarying vector{1, 0};
    vector.emplace_back(10, std::initializer_list<float>{});
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

TEST_CASE("ContiguousTest: one varying size: emplace_back c-style array")
{
    float carray[]{0.1f, 0.2f};
    OneVarying vector{1, std::size(carray) * sizeof(float)};
    vector.emplace_back(10, carray);
    auto&& [i, array] = vector[0];
    CHECK_EQ(10u, i);
    CHECK(test::range_equal(carray, array));
}

TEST_CASE("ContiguousTest: two fixed size: emplace_back with lists and subscript operator")
{
    std::list expected_firsts{1.f, 2.f};
    std::vector expected_seconds{-1.f, -2.f};
    TwoFixed vector{2, {expected_firsts.size(), expected_seconds.size()}};
    vector.emplace_back(expected_firsts, 10u, expected_seconds);
    vector.emplace_back(expected_firsts, 10u, expected_seconds);
    for (size_t i = 0; i < vector.size(); ++i)
    {
        auto&& [firsts, j, seconds] = vector[i];
        CHECK_EQ(10u, j);
        CHECK(test::range_equal(expected_firsts, firsts));
        CHECK(test::range_equal(expected_seconds, seconds));
    }
}

TEST_CASE("ContiguousTest: one fixed size: emplace_back with iterator and subscript operator")
{
    std::vector expected_elements{1.f, 2.f};
    OneFixed vector{1, {expected_elements.size()}};
    SUBCASE("begin() iterator") { vector.emplace_back(10u, expected_elements.begin()); }
    SUBCASE("data() iterator") { vector.emplace_back(10u, expected_elements.data()); }
    auto&& [i, elements] = vector[0];
    CHECK_EQ(10u, i);
    CHECK(test::range_equal(expected_elements, elements));
}

const std::string test_string_one{"a very long test string"};
const std::string test_string_two{"another very long test string"};

TEST_CASE("ContiguousTest: std::string emplace_back with iterator and subscript operator")
{
    cntgs::ContiguousVector<cntgs::FixedSize<std::string>, std::string, const std::string*> vector{1, {1}};
    std::vector v{test_string_one};
    SUBCASE("emplace_back range") { vector.emplace_back(v, test_string_two, &test_string_two); }
    SUBCASE("emplace_back iterator") { vector.emplace_back(v.begin(), test_string_two, &test_string_two); }
    auto&& [fixed, string, string_ptr] = vector[0];
    CHECK_EQ(1, fixed.size());
    CHECK_EQ(test_string_one, fixed[0]);
    CHECK_EQ(test_string_two, string);
    CHECK_EQ(test_string_two, *string_ptr);
}

TEST_CASE("ContiguousTest: std::string TypeErasedVector")
{
    cntgs::ContiguousVector<std::string> vector{2};
    vector.emplace_back(test_string_one);
    vector.emplace_back(test_string_two);
    auto erased = cntgs::type_erase(std::move(vector));
    cntgs::ContiguousVector<std::string> restored;
    SUBCASE("by move") { restored = cntgs::ContiguousVector<std::string>{std::move(erased)}; }
    SUBCASE("by lvalue reference")
    {
        for (size_t i = 0; i < 2; i++)
        {
            restored = cntgs::ContiguousVector<std::string>{erased};
        }
    }
    auto&& [string_one] = restored[0];
    CHECK_EQ(test_string_one, string_one);
    auto&& [string_two] = restored[1];
    CHECK_EQ(test_string_two, string_two);
}

template <class T>
void check_iterator(T&& vector)
{
    auto begin = vector.begin();
    using IterTraits = std::iterator_traits<decltype(begin)>;
    CHECK(std::is_same_v<std::random_access_iterator_tag, typename IterTraits::iterator_category>);
    std::for_each(vector.begin()++, ++vector.begin(), [&](auto&& elem) {
        auto&& [uinteger, floats] = vector[0];
        CHECK_EQ(uinteger, cntgs::get<0>(elem));
        CHECK(test::range_equal(cntgs::get<1>(elem), floats));
    });
    std::for_each(vector.begin() + 1, vector.end(), [&](auto&& elem) {
        auto&& [uinteger, floats] = vector[1];
        CHECK_EQ(uinteger, cntgs::get<0>(elem));
        CHECK(test::range_equal(cntgs::get<1>(elem), floats));
    });
}

TEST_CASE("ContiguousTest: one fixed size: begin() end()")
{
    OneFixed vector{2, {2}};
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

TEST_CASE("ContiguousTest: swap and iter_swap with ContiguousVectorIterator of std::unique_ptr")
{
    cntgs::ContiguousVector<std::unique_ptr<int>, cntgs::FixedSize<std::unique_ptr<int>>> vector{2, {1}};
    std::array v{std::make_unique<int>(20)};
    vector.emplace_back(std::make_unique<int>(10), std::make_move_iterator(v.begin()));
    vector.emplace_back(std::make_unique<int>(30), std::array{std::make_unique<int>(40)});
    SUBCASE("std::iter_swap") { std::iter_swap(vector.begin(), ++vector.begin()); }
    SUBCASE("cntgs::swap") { cntgs::swap(vector[0], vector[1]); }
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

TEST_CASE("ContiguousTest: std::rotate with ContiguousVectorIterator of std::unique_ptr")
{
    cntgs::ContiguousVector<std::unique_ptr<int>, cntgs::FixedSize<std::unique_ptr<int>>> vector{2, {1}};
    std::array v{std::make_unique<int>(20)};
    vector.emplace_back(std::make_unique<int>(10), std::make_move_iterator(v.begin()));
    vector.emplace_back(std::make_unique<int>(30), std::array{std::make_unique<int>(40)});
    std::rotate(vector.begin(), ++vector.begin(), vector.end());
    auto&& [a, b] = vector[0];
    CHECK_EQ(30, *a);
    CHECK_EQ(40, *b.front());
}

TEST_CASE("ContiguousTest: OneFixed construct with unique_ptr and span")
{
    std::array elements{1.f, 2.f};
    std::optional<OneFixed> vector;
    const auto memory_size = 2 * (sizeof(uint32_t) * 2 * sizeof(float));
    auto ptr = std::make_unique<std::byte[]>(memory_size);
    SUBCASE("unique_ptr") { vector.emplace(memory_size, std::move(ptr), 2, std::array{elements.size()}); }
    SUBCASE("span") { vector.emplace(cntgs::Span<std::byte>{ptr.get(), memory_size}, 2, std::array{elements.size()}); }
    CHECK(vector);
    vector->emplace_back(10u, elements);
    auto&& [i, e] = (*vector)[0];
    CHECK_EQ(10u, i);
    CHECK(test::range_equal(elements, e));
}

TEST_CASE("ContiguousTest: NoSpecialParameter construct with unique_ptr and span")
{
    std::optional<NoSpecialParameter> vector;
    const auto memory_size = 2 * (sizeof(uint32_t) * sizeof(float));
    auto ptr = std::make_unique<std::byte[]>(memory_size);
    SUBCASE("unique_ptr") { vector.emplace(memory_size, std::move(ptr), 2); }
    SUBCASE("span") { vector.emplace(cntgs::Span<std::byte>{ptr.get(), memory_size}, 2); }
    CHECK(vector);
    vector->emplace_back(10u, 5.f);
    auto&& [i, f] = (*vector)[0];
    CHECK_EQ(10u, i);
    CHECK_EQ(5.f, f);
}

TEST_CASE("ContiguousTest: type_erase OneFixed and restore")
{
    std::array elements{1.f, 2.f};
    OneFixed vector{2, {elements.size()}};
    vector.emplace_back(10u, elements);
    auto erased = cntgs::type_erase(std::move(vector));
    OneFixed restored;
    SUBCASE("by move") { restored = OneFixed{std::move(erased)}; }
    SUBCASE("by lvalue reference") { restored = OneFixed{erased}; }
    auto&& [i, e] = restored[0];
    CHECK_EQ(10u, i);
    CHECK(test::range_equal(elements, e));
}

using TwoNonSpecialAligned = cntgs::ContiguousVector<char, cntgs::AlignAs<uint32_t, 8>>;
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

TEST_CASE("ContiguousTest: with alignment: size() and capacity()")
{
    std::array firsts{1.f, 2.f};
    std::array seconds{1.f, 2.f};
    std::optional<std::variant<TwoNonSpecialAligned, OneVaryingAligned, TwoVaryingAligned, OneFixedAligned,
                               TwoFixedAligned, OneFixedOneVaryingAligned>>
        vector;
    SUBCASE("no special parameter")
    {
        TwoNonSpecialAligned v{2};
        v.emplace_back('a', 10u);
        vector.emplace(std::move(v));
    }
    SUBCASE("one varying")
    {
        OneVaryingAligned v{2, firsts.size() * sizeof(float)};
        v.emplace_back(firsts, 10u);
        vector.emplace(std::move(v));
    }
    SUBCASE("two varying")
    {
        TwoVaryingAligned v{2, firsts.size() * sizeof(float) + seconds.size() * sizeof(float)};
        v.emplace_back(10u, firsts, seconds);
        vector.emplace(std::move(v));
    }
    SUBCASE("one fixed")
    {
        OneFixedAligned v{2, {firsts.size()}};
        v.emplace_back(10u, firsts);
        vector.emplace(std::move(v));
    }
    SUBCASE("two fixed")
    {
        TwoFixedAligned v{2, {firsts.size(), seconds.size()}};
        v.emplace_back(firsts, 10u, seconds);
        vector.emplace(std::move(v));
    }
    SUBCASE("one fixed one varying")
    {
        OneFixedOneVaryingAligned v{2, seconds.size() * sizeof(float), {firsts.size()}};
        v.emplace_back(firsts, 10u, seconds);
        vector.emplace(std::move(v));
    }
    CHECK(vector);
    std::visit(
        [&](auto&& v) {
            CHECK_EQ(1, v.size());
            CHECK_EQ(2, v.capacity());
            CHECK_FALSE(v.empty());
        },
        *vector);
}

template <std::size_t Alignment, class T>
void check_alignment(T* t)
{
    void* ptr = reinterpret_cast<void*>(t);
    auto size = std::numeric_limits<size_t>::max();
    std::align(Alignment, sizeof(uint32_t), ptr, size);
    CHECK_EQ(t, ptr);
}

template <std::size_t Alignment, class T>
void check_alignment(T&& t)
{
    check_alignment<Alignment>(std::data(t));
}

TEST_CASE("ContiguousTest: with alignment: emplace_back() and subscript operator")
{
    std::array firsts{1.f, 2.f};
    std::array seconds{1.f, 2.f};
    SUBCASE("no special parameter")
    {
        TwoNonSpecialAligned vector{5};
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
    SUBCASE("one varying")
    {
        OneVaryingAligned vector{5, 5 * firsts.size() * sizeof(float)};
        for (uint32_t i = 0; i < 5; ++i)
        {
            vector.emplace_back(firsts, i);
        }
        for (uint32_t i = 0; i < 5; ++i)
        {
            auto&& [a, b] = vector[i];
            CHECK(test::range_equal(firsts, a));
            CHECK_EQ(i, b);
            check_alignment<16>(a);
        }
    }
    SUBCASE("two varying")
    {
        TwoVaryingAligned vector{5, 5 * (firsts.size() * sizeof(float) + seconds.size() * sizeof(float))};
        for (uint32_t i = 0; i < 5; ++i)
        {
            vector.emplace_back(i, firsts, seconds);
        }
        for (uint32_t i = 0; i < 5; ++i)
        {
            auto&& [a, b, c] = vector[i];
            CHECK_EQ(i, a);
            CHECK(test::range_equal(firsts, b));
            CHECK(test::range_equal(seconds, c));
            check_alignment<8>(b);
            check_alignment<8>(c);
        }
    }
    SUBCASE("one fixed")
    {
        OneFixedAligned vector{5, {firsts.size()}};
        for (uint32_t i = 0; i < 5; ++i)
        {
            vector.emplace_back(i, firsts);
        }
        for (uint32_t i = 0; i < 5; ++i)
        {
            auto&& [a, b] = vector[i];
            CHECK_EQ(i, a);
            CHECK(test::range_equal(firsts, b));
            check_alignment<32>(b);
        }
    }
    SUBCASE("two fixed")
    {
        TwoFixedAligned vector{5, {firsts.size(), seconds.size()}};
        for (uint32_t i = 0; i < 5; ++i)
        {
            vector.emplace_back(firsts, i, seconds);
        }
        for (uint32_t i = 0; i < 5; ++i)
        {
            auto&& [a, b, c] = vector[i];
            CHECK(test::range_equal(firsts, a));
            CHECK_EQ(i, b);
            CHECK(test::range_equal(seconds, c));
            check_alignment<8>(a);
            check_alignment<16>(&b);
        }
    }
    SUBCASE("two fixed alt")
    {
        std::array seconds_uint{50u, 100u, 150u};
        TwoFixedAlignedAlt vector{5, {firsts.size(), seconds_uint.size()}};
        for (uint32_t i = 0; i < 5; ++i)
        {
            vector.emplace_back(firsts, seconds_uint, i);
        }
        for (uint32_t i = 0; i < 5; ++i)
        {
            auto&& [a, b, c] = vector[i];
            CHECK(test::range_equal(firsts, a));
            CHECK(test::range_equal(seconds_uint, b));
            CHECK_EQ(i, c);
            check_alignment<32>(a);
        }
    }
    SUBCASE("one fixed one varying")
    {
        OneFixedOneVaryingAligned vector{5, 5 * seconds.size() * sizeof(float), {firsts.size()}};
        for (uint32_t i = 0; i < 5; ++i)
        {
            vector.emplace_back(firsts, i, seconds);
        }
        for (uint32_t i = 0; i < 5; ++i)
        {
            auto&& [a, b, c] = vector[i];
            CHECK(test::range_equal(firsts, a));
            CHECK_EQ(i, b);
            CHECK(test::range_equal(seconds, c));
            check_alignment<16>(a);
            check_alignment<8>(c);
        }
    }
}
}  // namespace test_contiguous
