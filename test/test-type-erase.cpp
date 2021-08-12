// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "utils/check.hpp"
#include "utils/fixture.hpp"
#include "utils/range.hpp"
#include "utils/typedefs.hpp"

#include <cntgs/contiguous.hpp>
#include <doctest/doctest.h>

namespace test_type_erase
{
TEST_SUITE_BEGIN(CNTGS_TEST_CPP_VERSION);

using namespace cntgs;
using namespace cntgs::test;

TEST_CASE("TypeErasedVector: type_erase OneFixed string and destruct")
{
    using Vector = cntgs::ContiguousVector<std::string>;
    Vector vector{2};
    vector.emplace_back(STRING1);
    cntgs::TypeErasedVector erased{std::move(vector)};
}

TEST_CASE("TypeErasedVector: type_erase OneFixed and restore and move assign")
{
    OneFixed vector{2, {FLOATS2.size()}};
    vector.emplace_back(10u, FLOATS2);
    auto expected{vector};
    cntgs::TypeErasedVector erased{std::move(vector)};
    OneFixed restored;
    restored = OneFixed(std::move(erased));
    CHECK_EQ(expected, restored);
}

TEST_CASE("TypeErasedVector: type_erase OneFixed, restore and copy assign")
{
    using Vector = cntgs::ContiguousVector<std::string>;
    Vector vector{2};
    vector.emplace_back(STRING1);
    auto vector_copy{vector};
    vector_copy.emplace_back(STRING2);
    cntgs::TypeErasedVector erased{std::move(vector)};
    Vector restored{std::move(erased)};
    restored = vector_copy;
    CHECK_EQ(vector_copy, restored);
}

TEST_CASE("TypeErasedVector: type_erase empty OneFixed, restore and copy assign")
{
    using Vector = cntgs::ContiguousVector<std::string>;
    Vector vector{0};
    auto vector_copy{vector};
    cntgs::TypeErasedVector erased{std::move(vector)};
    Vector restored{std::move(erased)};
    restored = vector_copy;
    restored.reserve(1);
    restored.emplace_back(STRING1);
    auto&& [a] = restored.back();
    CHECK_EQ(STRING1, a);
}

TEST_CASE("TypeErasedVector: std::string TypeErasedVector")
{
    using Vector = cntgs::ContiguousVector<std::string>;
    Vector vector{2};
    vector.emplace_back(STRING1);
    vector.emplace_back(STRING2);
    auto expected{vector};
    cntgs::TypeErasedVector erased{std::move(vector)};
    auto move_constructed_erased{std::move(erased)};
    Vector restored{std::move(move_constructed_erased)};
    CHECK_EQ(expected, restored);
}

TEST_SUITE_END();
}  // namespace test_type_erase
