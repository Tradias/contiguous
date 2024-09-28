// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_UTILS_CHECK_HPP
#define CNTGS_UTILS_CHECK_HPP

#include "utils/doctest.hpp"
#include "utils/range.hpp"
#include "utils/typeTraits.hpp"

#include <cntgs/contiguous.hpp>

#include <cstddef>
#include <string_view>
#include <type_traits>
#include <utility>

namespace cntgs::test
{
namespace detail
{
template <class>
inline constexpr bool IS_UNIQUE_PTR = false;

template <class T>
inline constexpr bool IS_UNIQUE_PTR<std::unique_ptr<T>> = true;

template <class Lhs, class Rhs>
bool check_equal(const Lhs& lhs, const Rhs& rhs);

template <class T>
bool check_unique_ptr_equal_to(const std::unique_ptr<T>& lhs, const T& rhs)
{
    if (!lhs)
    {
        CHECK(lhs);
        return false;
    }
    return detail::check_equal(rhs, *lhs);
}

template <class T>
bool check_unique_ptr_equal_to(const std::unique_ptr<T>& lhs, std::nullptr_t rhs)
{
    CHECK_EQ(lhs, rhs);
    return lhs == rhs;
}

template <class Lhs, class Rhs>
bool check_equal(const Lhs& lhs, const Rhs& rhs)
{
    if constexpr (std::is_constructible_v<std::string_view, const Lhs&>)
    {
        CHECK_EQ(lhs, rhs);
        return lhs == rhs;
    }
    if constexpr (test::IS_RANGE<test::RemoveCvrefT<Lhs>>)
    {
        auto result = test::range_equal(lhs, rhs,
                                        [](auto&& lhs_value, auto&& rhs_value)
                                        {
                                            return detail::check_equal(lhs_value, rhs_value);
                                        });
        CHECK(result);
        return result;
    }
    else if constexpr (IS_UNIQUE_PTR<Lhs> && !IS_UNIQUE_PTR<Rhs>)
    {
        return detail::check_unique_ptr_equal_to(lhs, rhs);
    }
    else if constexpr (!IS_UNIQUE_PTR<Lhs> && IS_UNIQUE_PTR<Rhs>)
    {
        return detail::check_unique_ptr_equal_to(rhs, lhs);
    }
    else if constexpr (IS_UNIQUE_PTR<Lhs> && IS_UNIQUE_PTR<Rhs>)
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
    else
    {
        CHECK_EQ(lhs, rhs);
        return lhs == rhs;
    }
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

template <class T>
auto check_size1_and_capacity2(T& v)
{
    CHECK_EQ(1, v.size());
    CHECK_EQ(2, v.capacity());
    CHECK_FALSE(v.empty());
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

template <class T>
void check_alignment(T* ptr, std::size_t alignment)
{
    auto* void_ptr = static_cast<void*>(ptr);
    auto size = std::numeric_limits<size_t>::max();
    std::align(alignment, sizeof(T), void_ptr, size);
    CHECK_EQ(void_ptr, ptr);
}

template <class T>
void check_alignment(cntgs::Span<T>& span, std::size_t alignment)
{
    check_alignment(std::data(span), alignment);
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
}  // namespace cntgs::test

#endif  // CNTGS_UTILS_CHECK_HPP
