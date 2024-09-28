// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "utils/check.hpp"
#include "utils/doctest.hpp"
#include "utils/fixture.hpp"
#include "utils/noexcept.hpp"
#include "utils/testMemoryResource.hpp"
#include "utils/typedefs.hpp"

#include <cntgs/contiguous.hpp>

#include <array>
#include <version>

#ifdef __cpp_lib_span
#include <span>
#endif

namespace test_vector
{
using namespace cntgs;
using namespace test;

TEST_CASE("ContiguousVector: TwoFixed get_fixed_size<I>()")
{
    TwoFixed vector{2, {10, 20}};
    CHECK_EQ(10, vector.get_fixed_size<0>());
    CHECK_EQ(20, vector.get_fixed_size<1>());
}

TEST_CASE("ContiguousVector: one fixed one varying size: correct memory_consumption()")
{
    using Vector =
        ContiguousVectorWithAllocator<TestAllocator<>, cntgs::FixedSize<uint16_t>, uint32_t, cntgs::VaryingSize<float>>;
    const auto varying_byte_count = 6 * sizeof(float);
    TestMemoryResource resource;
    Vector vector{2, varying_byte_count, {3}, resource.get_allocator()};
    const auto bytes_allocated = resource.bytes_allocated;
    vector.emplace_back(std::initializer_list<uint16_t>{1, 2, 3}, 10, std::array{0.f, 0.1f, 0.2f});
    vector.emplace_back(std::initializer_list<uint16_t>{4, 5, 6}, 11, std::array{0.3f, 0.4f, 0.5f});
    const auto expected =
        2 * (3 * sizeof(uint16_t) + sizeof(std::size_t) + sizeof(std::size_t) + sizeof(uint32_t)) + varying_byte_count;
    CHECK_EQ(expected, vector.memory_consumption());
    CHECK_EQ(expected, bytes_allocated);
    CHECK_EQ(expected + 3 * sizeof(std::size_t), resource.bytes_allocated);  // reallocations
}

TEST_CASE("ContiguousVector: TwoFixed correct memory_consumption()")
{
    TwoFixed vector{2, {1, 2}};
    const auto expected = 2 * (1 * sizeof(float) + sizeof(uint32_t) + 2 * sizeof(float));
    CHECK_EQ(expected, vector.memory_consumption());
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

template <bool IsNoexcept>
using NoexceptVector = cntgs::ContiguousVector<cntgs::FixedSize<Noexcept<IsNoexcept>>>;

TEST_CASE("ContiguousVector: ContiguousVector is conditionally nothrow")
{
    check_conditionally_nothrow_comparison<NoexceptVector>();
    check_always_nothrow_move_construct<NoexceptVector>();
    CHECK(std::is_nothrow_move_assignable_v<NoexceptVector<true>>);
    CHECK(std::is_nothrow_move_assignable_v<NoexceptVector<false>>);
    CHECK_FALSE(std::is_nothrow_move_assignable_v<pmr::ContiguousVector<float>>);
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

TEST_CASE("ContiguousVector: OneFixedUniquePtr with polymorphic_allocator")
{
    TestPmrMemoryResource resource;
    auto vector = fixed_vector_of_unique_ptrs(resource.get_allocator());
    resource.check_was_used(vector.get_allocator());
}

template <class Vector>
void check_varying_vector_unique_ptrs(const Vector& vector)
{
    CHECK_EQ(2, vector.size());
    check_equal_using_get(vector[0], array_two_unique_ptr(10, 20), 30);
    check_equal_using_get(vector[1], array_one_unique_ptr(40), 50);
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
    TestPmrMemoryResource resource;
    auto vector = varying_vector_of_unique_ptrs(resource.get_allocator());
    TestPmrMemoryResource resource2;
    using std::swap;
    SUBCASE("swap into smaller vector")
    {
        decltype(vector) vector2{0, 0, resource2.get_allocator()};
        auto buffer = resource.buffer;
        swap(vector2, vector);
        CHECK(std::equal(buffer.begin(), buffer.end(), resource.buffer.begin()));
        check_varying_vector_unique_ptrs(vector2);
    }
    SUBCASE("swap into larger vector")
    {
        decltype(vector) vector2{3, 10, resource2.get_allocator()};
        auto buffer = resource.buffer;
        swap(vector2, vector);
        CHECK(std::equal(buffer.begin(), buffer.end(), resource.buffer.begin()));
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
    check_equal_using_get(vector[0], std::array{STRING2, STRING2}, STRING2);
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
}  // namespace test_vector
