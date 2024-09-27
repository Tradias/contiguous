// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_DETAIL_ITERATOR_HPP
#define CNTGS_DETAIL_ITERATOR_HPP

#include "cntgs/detail/typeTraits.hpp"

#include <iterator>
#include <version>

namespace cntgs::detail
{
template <class, class = void>
inline constexpr bool HAS_OPERATOR_ARROW = false;

template <class T>
inline constexpr bool HAS_OPERATOR_ARROW<T, std::void_t<decltype(std::declval<const T&>().operator->())>> = true;

template <class T>
struct ArrowProxy
{
    T t_;

    constexpr const T* operator->() const noexcept { return &t_; }
};

template <class I>
constexpr auto operator_arrow_produces_pointer_to_iterator_reference_type() noexcept
{
    if constexpr (detail::HAS_OPERATOR_ARROW<I>)
    {
        return std::is_same_v<decltype(std::declval<const I&>().operator->()),
                              std::add_pointer_t<typename std::iterator_traits<I>::reference>>;
    }
    else
    {
        return false;
    }
}

template <class I>
inline constexpr bool CONTIGUOUS_ITERATOR_V =
    detail::IS_DERIVED_FROM<typename std::iterator_traits<I>::iterator_category, std::random_access_iterator_tag> &&
    std::is_lvalue_reference_v<typename std::iterator_traits<I>::reference> &&
    std::is_same_v<typename std::iterator_traits<I>::value_type,
                   detail::RemoveCvrefT<typename std::iterator_traits<I>::reference>> &&
    detail::operator_arrow_produces_pointer_to_iterator_reference_type<I>();
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_ITERATOR_HPP
