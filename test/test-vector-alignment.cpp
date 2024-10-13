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

namespace test_vector_alignment
{
using namespace cntgs;
using namespace test;

TEST_CASE("ContiguousVector: PlainAligned size() and capacity()")
{
    PlainAligned v{2};
    v.emplace_back('a', 10u);
    check_size1_and_capacity2(v);
}

TEST_CASE("ContiguousVector: OneVaryingAligned size() and capacity()")
{
    OneVaryingAligned v{2, FLOATS1.size() * sizeof(float)};
    v.emplace_back(FLOATS1.size(), FLOATS1, 10u);
    check_size1_and_capacity2(v);
}

TEST_CASE("ContiguousVector: TwoVaryingAligned size() and capacity()")
{
    TwoVaryingAligned v{2, FLOATS1.size() * sizeof(float) + FLOATS2.size() * sizeof(float)};
    v.emplace_back(10u, FLOATS1.size(), FLOATS1, FLOATS2.size(), FLOATS2);
    check_size1_and_capacity2(v);
}

TEST_CASE("ContiguousVector: OneFixedAligned size(), capacity() and memory_consumption()")
{
    TestMemoryResource resource;
    Checked<OneFixedAligned<TestAllocator<>>, 32> v{2, {FLOATS1.size()}, resource.get_allocator()};
    v.emplace_back(10u, FLOATS1);
    check_size1_and_capacity2(v);
    v.emplace_back(10u, FLOATS1);
    CHECK_EQ(v[0], v[1]);
    const auto size = 32 + FLOATS1.size() * sizeof(float);
    const auto expected = 2 * test::align(size, 32);
    CHECK_EQ(expected, v.memory_consumption());
    CHECK_EQ(expected, resource.bytes_allocated);
}

TEST_CASE("ContiguousVector: TwoFixedAligned size() and capacity()")
{
    TestMemoryResource resource;
    TwoFixedAligned<TestAllocator<>> v{2, {FLOATS1.size(), FLOATS2.size()}, resource.get_allocator()};
    v.emplace_back(FLOATS1, 10u, FLOATS2);
    check_size1_and_capacity2(v);
    const auto size = (FLOATS1.size() * sizeof(float) + 7 + sizeof(uint32_t) + FLOATS2.size() * sizeof(float));
    const auto expected = 2 * test::align(size, 8);
    CHECK_EQ(expected, v.memory_consumption());
    CHECK_EQ(expected, resource.bytes_allocated);
}

TEST_CASE("ContiguousVector: OneFixedOneVaryingAligned size() and capacity()")
{
    OneFixedOneVaryingAligned v{2, FLOATS2.size() * sizeof(float), {FLOATS1.size()}};
    v.emplace_back(FLOATS1, 10u, FLOATS2.size(), FLOATS2);
    check_size1_and_capacity2(v);
}

TEST_CASE("ContiguousVector: OneFixedAligned emplace_back->reserve->emplace_back")
{
    Checked<OneFixedAligned<>> v{1, {FLOATS1.size()}};
    v.emplace_back(1u, FLOATS1);
    v.reserve(2);
    v.emplace_back(2u, FLOATS1);
    check_equal_using_get(v[0], 1u, FLOATS1);
    check_equal_using_get(v[1], 2u, FLOATS1);
}

TEST_CASE("ContiguousVector: PlainAligned emplace_back() and subscript operator")
{
    Checked<PlainAligned, 8> vector{5};
    for (uint32_t i = 0; i < 5; ++i)
    {
        vector.emplace_back('a', i);
    }
    for (uint32_t i = 0; i < 5; ++i)
    {
        check_equal_using_get(vector[i], 'a', i);
        auto&& [a, b] = vector[i];
        check_alignment(&b, 8);
    }
}

TEST_CASE("ContiguousVector: OneVaryingAligned emplace_back() and subscript operator")
{
    CheckedSoft<OneVaryingAligned, 16> vector{5, 5 * FLOATS1.size() * sizeof(float)};
    for (uint32_t i = 0; i < 5; ++i)
    {
        vector.emplace_back(FLOATS1.size(), FLOATS1, i);
    }
    for (uint32_t i = 0; i < 5; ++i)
    {
        check_equal_using_get(vector[i], FLOATS1.size(), FLOATS1, i);
        auto&& [a, b, c] = vector[i];
        check_alignment(&a, 8);
        check_alignment(b, 16);
    }
}

TEST_CASE("ContiguousVector: TwoVaryingAligned emplace_back() and subscript operator")
{
    CheckedSoft<TwoVaryingAligned, 16> vector{5, 5 * (FLOATS1.size() * sizeof(float) + FLOATS2.size() * sizeof(float))};
    for (uint32_t i = 0; i < 5; ++i)
    {
        vector.emplace_back(i, FLOATS1.size(), FLOATS1, FLOATS2.size(), FLOATS2);
    }
    for (uint32_t i = 0; i < 5; ++i)
    {
        check_equal_using_get(vector[i], i, FLOATS1.size(), FLOATS1, FLOATS2.size(), FLOATS2);
        auto&& [a, b, c, d, e] = vector[i];
        check_alignment(c, 8);
        check_alignment(e, 16);
    }
    CHECK_EQ(336, vector.memory_consumption());
}

TEST_CASE("ContiguousVector: OneFixedAligned emplace_back() and subscript operator")
{
    Checked<OneFixedAligned<>, 32> vector{5, {FLOATS1.size()}};
    for (uint32_t i = 0; i < 5; ++i)
    {
        vector.emplace_back(i, FLOATS1);
    }
    for (uint32_t i = 0; i < 5; ++i)
    {
        check_equal_using_get(vector[i], i, FLOATS1);
        auto&& [a, b] = vector[i];
        check_alignment(b, 32);
    }
}

TEST_CASE("ContiguousVector: TwoFixedAligned emplace_back() and subscript operator")
{
    Checked<TwoFixedAligned<>, 16> vector{5, {FLOATS1.size(), FLOATS2.size()}};
    for (uint32_t i = 0; i < 5; ++i)
    {
        vector.emplace_back(FLOATS1, i, FLOATS2);
    }
    for (uint32_t i = 0; i < 5; ++i)
    {
        check_equal_using_get(vector[i], FLOATS1, i, FLOATS2);
        auto&& [a, b, c] = vector[i];
        check_alignment(a, 8);
        check_alignment(&b, 16);
    }
}

TEST_CASE("ContiguousVector: TwoFixedAlignedAlt emplace_back() and subscript operator")
{
    std::array uint2{50u, 100u, 150u};
    Checked<TwoFixedAlignedAlt, 32> vector{5, {FLOATS1.size(), uint2.size()}};
    for (uint32_t i = 0; i < 5; ++i)
    {
        vector.emplace_back(FLOATS1, uint2, i);
    }
    for (uint32_t i = 0; i < 5; ++i)
    {
        check_equal_using_get(vector[i], FLOATS1, uint2, i);
        auto&& [a, b, c] = vector[i];
        check_alignment(a, 32);
    }
}

TEST_CASE("ContiguousVector: OneFixedOneVaryingAligned emplace_back() and subscript operator")
{
    Checked<OneFixedOneVaryingAligned, 16> vector{5, 5 * FLOATS2.size() * sizeof(float), {FLOATS1.size()}};
    for (uint32_t i = 0; i < 5; ++i)
    {
        vector.emplace_back(FLOATS1, i, FLOATS2.size(), FLOATS2);
    }
    for (uint32_t i = 0; i < 5; ++i)
    {
        check_equal_using_get(vector[i], FLOATS1, i, FLOATS2.size(), FLOATS2);
        auto&& [a, b, c, d] = vector[i];
        check_alignment(a, 16);
        check_alignment(d, 8);
    }
}

struct Sixteen
{
    double a, b;

    bool operator==(const Sixteen& other) const noexcept { return a == other.a && b == other.b; }
};

struct Twelve
{
    uint32_t a{}, b{}, c{};
};

TEST_CASE("ContiguousVector: Aligned with matching leading/trailing alignment emplace_back() and subscript operator")
{
    using V = cntgs::ContiguousVector<cntgs::AlignAs<std::size_t, 8>, cntgs::VaryingSize<cntgs::AlignAs<float, 16>>,
                                      uint32_t, cntgs::FixedSize<cntgs::AlignAs<Sixteen, 16>>>;
    CheckedSoft<V, 16> vector{5, 5 * FLOATS2.size() * sizeof(float), {1}};
    const auto sixteens = std::array{Sixteen{1, 2}};
    for (uint32_t i = 0; i < 5; ++i)
    {
        vector.emplace_back(FLOATS2.size(), FLOATS2, i, sixteens);
    }
    for (uint32_t i = 0; i < 5; ++i)
    {
        check_equal_using_get(vector[i], FLOATS2.size(), FLOATS2, i, sixteens);
        auto&& [a, b, c, d] = vector[i];
        check_alignment(b, 16);
        check_alignment(d, 16);
    }
}

TEST_CASE("ContiguousVector: Larger alignment after VaryingSize")
{
    using V = cntgs::ContiguousVector<cntgs::AlignAs<std::size_t, 8>, cntgs::VaryingSize<cntgs::AlignAs<double, 8>>,
                                      Twelve, cntgs::AlignAs<float, 16>>;
    CheckedSoft<V, 16> vector{5, 5 * sizeof(double)};
    const auto doubles = std::array{1.};
    for (uint32_t i = 0; i < 5; ++i)
    {
        vector.emplace_back(doubles.size(), doubles, Twelve{i}, 42.f);
    }
    for (uint32_t i = 0; i < 5; ++i)
    {
        check_equal_using_get(vector[i], doubles.size(), doubles);
        auto&& [a, b, c, d] = vector[i];
        check_alignment(b, 8);
        check_alignment(&d, 16);
    }
    CHECK_EQ(272, vector.memory_consumption());
}

TEST_CASE("ContiguousVector: Larger alignment after VaryingSize with matching trailing alignment")
{
    using V =
        cntgs::ContiguousVector<double, cntgs::AlignAs<std::size_t, 8>, cntgs::VaryingSize<cntgs::AlignAs<Sixteen, 8>>,
                                Twelve, cntgs::AlignAs<float, 16>>;
    Checked<V, 16> vector{5, 5 * sizeof(Sixteen)};
    const auto sixteens = std::array{Sixteen{1, 2}};
    for (uint32_t i = 0; i < 5; ++i)
    {
        vector.emplace_back(i, sixteens.size(), sixteens, Twelve{i}, 42.f);
    }
    for (uint32_t i = 0; i < 5; ++i)
    {
        check_equal_using_get(vector[i], i, sixteens.size(), sixteens);
        auto&& [a, b, c, d, e] = vector[i];
        check_alignment(c, 8);
        check_alignment(&e, 16);
    }
}

TEST_CASE("ContiguousVector: Larger alignment after VaryingSize with increased and matching trailing alignment")
{
    using V = cntgs::ContiguousVector<Bytes<16>, cntgs::AlignAs<std::size_t, 8>,
                                      cntgs::VaryingSize<cntgs::AlignAs<Bytes<32>, 16>>, cntgs::AlignAs<float, 32>>;
    CheckedSoft<V, 16> vector{5, 5 * sizeof(Bytes<32>)};
    for (uint32_t i = 0; i < 5; ++i)
    {
        vector.emplace_back(Bytes<16>{}, 1, std::array{Bytes<32>{}}, 42.f);
    }
    for (uint32_t i = 0; i < 5; ++i)
    {
        test::check_equal(42.f, cntgs::get<3>(vector[1]));
        auto&& [a, b, c, d] = vector[i];
        check_alignment(c, 16);
        check_alignment(&d, 32);
    }
}

TEST_CASE("ContiguousVector: Matching trailing alignment after VaryingSize is not mistaken for actual alignment")
{
    using V = cntgs::ContiguousVector<cntgs::AlignAs<std::size_t, 8>, cntgs::VaryingSize<Bytes<4>>, Bytes<12>,
                                      cntgs::AlignAs<std::size_t, 8>, cntgs::VaryingSize<cntgs::AlignAs<Bytes<32>, 16>>,
                                      cntgs::AlignAs<float, 32>>;
    CheckedSoft<V, 16> vector{5, 5 * (sizeof(Bytes<4>) + sizeof(Bytes<32>))};
    for (uint32_t i = 0; i < 5; ++i)
    {
        vector.emplace_back(1, std::array{Bytes<4>{}}, Bytes<12>{}, 1, std::array{Bytes<32>{}}, 42.f);
    }
    for (uint32_t i = 0; i < 5; ++i)
    {
        test::check_equal(42.f, cntgs::get<5>(vector[i]));
        auto&& [a, b, c, d, e, f] = vector[i];
        check_alignment(e, 16);
        check_alignment(&f, 32);
    }
}

TEST_CASE("ContiguousVector: Compile-time known VaryingSize trailing alignments are correct")
{
    using V = cntgs::ContiguousVector<double, cntgs::AlignAs<std::size_t, 8>,
                                      cntgs::VaryingSize<cntgs::AlignAs<Sixteen, 16>>, cntgs::AlignAs<float, 16>>;
    Checked<V, 16> vector{5, 5 * sizeof(Sixteen)};
    const auto sixteens = std::array{Sixteen{1, 2}};
    for (uint32_t i = 0; i < 5; ++i)
    {
        vector.emplace_back(i, sixteens.size(), sixteens, 42.f);
    }
    for (uint32_t i = 0; i < 5; ++i)
    {
        check_equal_using_get(vector[i], i, sixteens.size(), sixteens, 42.f);
        auto&& [a, b, c, d] = vector[i];
        check_alignment(c, 16);
        check_alignment(&d, 16);
    }
}
}  // namespace test_vector_alignment
