// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_UTILS_FIXTURES_HPP
#define CNTGS_UTILS_FIXTURES_HPP

#include "utils/typedefs.hpp"

#include <cntgs/contiguous.hpp>

#include <array>
#include <list>
#include <memory>
#include <string>

namespace cntgs::test
{
static constexpr std::array FLOATS1{1.f, 2.f};
static constexpr std::array FLOATS1_ALT{11.f, 22.f};
static constexpr std::array FLOATS2{-3.f, -4.f, -5.f};
static constexpr std::array FLOATS2_ALT{-33.f, -44.f, -55.f};
inline static const std::list FLOATS_LIST{1.f, 2.f};
inline static const std::string STRING1{"a very long test string"};
inline static const std::string STRING2{"another very long test string"};

inline auto floats1(float one = 1.f, float two = 2.f) { return std::array{one, two}; }

inline auto floats1(float one = 1.f, float two = 2.f, float three = 3.f) { return std::array{one, two, three}; }

inline auto array_one_unique_ptr(int v1 = 10) { return std::array{std::make_unique<int>(v1)}; }

inline auto array_one_unique_ptr(std::nullptr_t) { return std::array{std::unique_ptr<int>()}; }

inline auto array_two_unique_ptr(int v1 = 30, int v2 = 40)
{
    return std::array{std::make_unique<int>(v1), std::make_unique<int>(v2)};
}

template <class Allocator = std::allocator<int>>
auto fixed_vector_of_unique_ptrs(Allocator allocator = {})
{
    test::ContiguousVectorWithAllocator<Allocator, cntgs::FixedSize<std::unique_ptr<int>>, std::unique_ptr<int>> vector{
        2, {1}, allocator};
    vector.emplace_back(array_one_unique_ptr(10), std::make_unique<int>(20));
    vector.emplace_back(array_one_unique_ptr(30), std::make_unique<int>(40));
    return vector;
}

template <class Allocator = std::allocator<int>>
auto varying_vector_of_unique_ptrs(Allocator allocator = {})
{
    test::ContiguousVectorWithAllocator<Allocator, cntgs::VaryingSize<std::unique_ptr<int>>, std::unique_ptr<int>>
        vector{2, 3 * sizeof(std::unique_ptr<int>), allocator};
    vector.emplace_back(array_two_unique_ptr(10, 20), std::make_unique<int>(30));
    vector.emplace_back(array_one_unique_ptr(40), std::make_unique<int>(50));
    return vector;
}

template <class Allocator = std::allocator<int>>
auto fixed_vector_of_strings(Allocator allocator = {})
{
    test::ContiguousVectorWithAllocator<Allocator, cntgs::FixedSize<std::string>, std::string> vector{
        2, {2}, allocator};
    vector.emplace_back(std::array{STRING1, STRING2}, STRING1);
    vector.emplace_back(std::array{STRING1, STRING2}, STRING2);
    return vector;
}

template <class Allocator = std::allocator<int>>
auto varying_vector_of_strings(Allocator allocator = {})
{
    test::ContiguousVectorWithAllocator<Allocator, cntgs::VaryingSize<std::string>, std::string> vector{
        2, 3 * sizeof(std::string), allocator};
    vector.emplace_back(std::array{STRING1, STRING2}, STRING1);
    vector.emplace_back(std::array{STRING1}, STRING2);
    return vector;
}

inline auto one_varying_vector_for_elements()
{
    test::OneVarying vector{4, 4 * (FLOATS1.size() * 2 + FLOATS1_ALT.size() + 3) * sizeof(float)};
    vector.emplace_back(10u, FLOATS1);
    vector.emplace_back(20u, FLOATS1_ALT);
    vector.emplace_back(10u, FLOATS1);
    vector.emplace_back(15u, floats1(10.f, 20.f, 30.f));
    return vector;
}
}  // namespace cntgs::test

#endif  // CNTGS_UTILS_FIXTURES_HPP
