// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "utils/check.hpp"
#include "utils/fixture.hpp"
#include "utils/functional.hpp"
#include "utils/range.hpp"
#include "utils/typedefs.hpp"

#include <cntgs/contiguous.hpp>
#include <doctest/doctest.h>

namespace test_contiguous
{
TEST_SUITE_BEGIN(CNTGS_TEST_CPP_VERSION);

using namespace cntgs;
using namespace cntgs::test;

TEST_CASE("ContiguousIterator: OneVarying sizeof(iterator)")
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
                      test::check_equal_using_get(vector[0], cntgs::get<0>(elem), cntgs::get<1>(elem));
                  });
    std::for_each(((vector.begin() + 2) - 1), vector.end(),
                  [&](auto&& elem)
                  {
                      test::check_equal_using_get(vector[1], cntgs::get<0>(elem), cntgs::get<1>(elem));
                  });
}

TEST_CASE("ContiguousIterator: OneFixed begin() end()")
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

TEST_CASE("ContiguousIterator: std::rotate with ContiguousVectorIterator of FixedSize std::unique_ptr")
{
    cntgs::ContiguousVector<std::unique_ptr<int>, cntgs::FixedSize<std::unique_ptr<int>>> vector{2, {1}};
    vector.emplace_back(std::make_unique<int>(10), array_one_unique_ptr(20));
    vector.emplace_back(std::make_unique<int>(30), array_one_unique_ptr(40));
    std::rotate(vector.begin(), ++vector.begin(), vector.end());
    auto&& [a, b] = vector[0];
    CHECK_EQ(30, *a);
    CHECK_EQ(40, *b.front());
}

TEST_SUITE_END();
}  // namespace test_contiguous
