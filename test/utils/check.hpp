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

namespace test
{
template <class Lhs, class Rhs>
bool check_equal(const Lhs& lhs, const Rhs& rhs);

namespace detail
{
template <class>
inline constexpr bool IS_UNIQUE_PTR = false;

template <class T>
inline constexpr bool IS_UNIQUE_PTR<std::unique_ptr<T>> = true;

template <class T>
bool check_unique_ptr_equal_to(const std::unique_ptr<T>& lhs, const T& rhs)
{
    if (!lhs)
    {
        CHECK(lhs);
        return false;
    }
    return test::check_equal(rhs, *lhs);
}

template <class T>
bool check_unique_ptr_equal_to(const std::unique_ptr<T>& lhs, std::nullptr_t rhs)
{
    CHECK_EQ(lhs, rhs);
    return lhs == rhs;
}

template <class T, std::size_t... I, class... Args>
bool check_equal_using_get(T&& t, std::index_sequence<I...>, Args&&... args)
{
    return (test::check_equal(cntgs::get<I>(static_cast<T&&>(t)), args) && ...);
}
}  // namespace detail

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
                                            return test::check_equal(lhs_value, rhs_value);
                                        });
        CHECK(result);
        return result;
    }
    else if constexpr (detail::IS_UNIQUE_PTR<Lhs> && !detail::IS_UNIQUE_PTR<Rhs>)
    {
        return detail::check_unique_ptr_equal_to(lhs, rhs);
    }
    else if constexpr (!detail::IS_UNIQUE_PTR<Lhs> && detail::IS_UNIQUE_PTR<Rhs>)
    {
        return detail::check_unique_ptr_equal_to(rhs, lhs);
    }
    else if constexpr (detail::IS_UNIQUE_PTR<Lhs> && detail::IS_UNIQUE_PTR<Rhs>)
    {
        if (bool{lhs} == bool{rhs})
        {
            if (bool{lhs} && bool{rhs})
            {
                return test::check_equal(*lhs, *rhs);
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

template <class T, class... Args>
bool check_equal_using_get(T&& t, Args&&... args)
{
    return detail::check_equal_using_get(static_cast<T&&>(t), std::make_index_sequence<sizeof...(Args)>{}, args...);
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

inline std::uintptr_t align(std::uintptr_t ptr, std::size_t alignment)
{
    auto* void_ptr = reinterpret_cast<void*>(ptr);
    auto size = std::numeric_limits<size_t>::max();
    std::align(alignment, 0, void_ptr, size);
    return reinterpret_cast<std::uintptr_t>(void_ptr);
}

inline void check_alignment(void* ptr, std::size_t alignment)
{
    auto* void_ptr = ptr;
    auto size = std::numeric_limits<size_t>::max();
    std::align(alignment, 0, void_ptr, size);
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

template <class Options, class... Parameter>
std::size_t used_memory_size(const cntgs::BasicContiguousVector<Options, Parameter...>& vector,
                             std::size_t alignment = 1)
{
    return test::align(vector.data_end() - vector.allocation_begin(), alignment);
}

template <class Options, class... Parameter>
void check_all_memory_is_used(const cntgs::BasicContiguousVector<Options, Parameter...>& vector, std::size_t alignment)
{
    CHECK_EQ(test::used_memory_size(vector, alignment), vector.memory_consumption());
}

template <class Vector, std::size_t Alignment, bool IsSoft>
struct BasicChecked : Vector
{
    using Vector::Vector;

    ~BasicChecked() noexcept
    {
        if constexpr (IsSoft)
        {
            CHECK_LE(test::used_memory_size(*this, Alignment), this->memory_consumption());
        }
        else
        {
            CHECK_EQ(test::used_memory_size(*this, Alignment), this->memory_consumption());
        }
    }
};

template <class Vector, std::size_t Alignment = 1>
using Checked = BasicChecked<Vector, Alignment, false>;

template <class Vector, std::size_t Alignment = 1>
using CheckedSoft = BasicChecked<Vector, Alignment, true>;
}  // namespace test

#endif  // CNTGS_UTILS_CHECK_HPP
