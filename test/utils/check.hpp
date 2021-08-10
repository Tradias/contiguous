// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_UTILS_CHECK_HPP
#define CNTGS_UTILS_CHECK_HPP

#include "utils/range.hpp"
#include "utils/typeTraits.hpp"

#include <doctest/doctest.h>

#include <cstddef>
#include <functional>
#include <utility>

namespace cntgs::test
{
namespace detail
{
template <class Lhs, class Rhs>
bool check_equal(const Lhs& lhs, const Rhs& rhs)
{
    if constexpr (test::IsRange<test::RemoveCvrefT<Lhs>>::value)
    {
        auto result = test::range_equal(lhs, rhs,
                                        [](auto&& lhs_value, auto&& rhs_value)
                                        {
                                            return detail::check_equal(lhs_value, rhs_value);
                                        });
        CHECK(result);
        return result;
    }
    else
    {
        CHECK_EQ(lhs, rhs);
        return lhs == rhs;
    }
}

inline bool check_equal(const std::string& lhs, const std::string& rhs)
{
    CHECK_EQ(lhs, rhs);
    return lhs == rhs;
}

template <class T>
bool check_equal(const std::unique_ptr<T>& lhs, const std::unique_ptr<T>& rhs)
{
    if (bool{lhs} == bool{rhs})
    {
        if (bool{lhs} && bool{rhs})
        {
            return check_equal(*lhs, *rhs);
        }
        return true;
    }
    else
    {
        FAIL("unique_ptrs are not both null or non-null");
    }
    return false;
}

template <class T>
bool check_equal(const T& lhs, const std::unique_ptr<T>& rhs)
{
    if (!rhs)
    {
        CHECK(rhs);
        return false;
    }
    return check_equal(lhs, *rhs);
}

template <class T>
bool check_equal(const std::unique_ptr<T>& lhs, const T& rhs)
{
    return check_equal(rhs, lhs);
}

template <class T, std::size_t... I, class... Args>
bool check_equal_using_get(T&& t, std::index_sequence<I...>, Args&&... args)
{
    return (detail::check_equal(get<I>(std::forward<T>(t)), args) && ...);
}
}  // namespace detail

template <class T, class... Args>
bool check_equal_using_get(T&& t, Args&&... args)
{
    return detail::check_equal_using_get(std::forward<T>(t), std::make_index_sequence<sizeof...(Args)>{}, args...);
}
}  // namespace cntgs::test

#endif  // CNTGS_UTILS_CHECK_HPP
