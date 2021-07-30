#ifndef CNTGS_DETAIL_ITERATORUTILS_HPP
#define CNTGS_DETAIL_ITERATORUTILS_HPP

#include "cntgs/contiguous/detail/typeUtils.hpp"

#include <iterator>
#include <version>

namespace cntgs::detail
{
template <class, class = std::void_t<>>
struct HasOperatorArrow : std::false_type
{
};

template <class T>
struct HasOperatorArrow<T, std::void_t<decltype(std::declval<const T&>().operator->())>> : std::true_type
{
};

template <class T>
struct ArrowProxy
{
    T t;

    constexpr const T* operator->() const noexcept { return &t; }
};

template <class I>
constexpr auto operator_arrow_produces_pointer_to_iterator_reference_type() noexcept
{
    if constexpr (detail::HasOperatorArrow<I>::value)
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
inline constexpr auto CONTIGUOUS_ITERATOR_V =
    detail::DerivedFrom<typename std::iterator_traits<I>::iterator_category, std::random_access_iterator_tag>&&
        std::is_lvalue_reference_v<typename std::iterator_traits<I>::reference>&&
            std::is_same_v<typename std::iterator_traits<I>::value_type,
                           detail::RemoveCvrefT<typename std::iterator_traits<I>::reference>>&&
            detail::operator_arrow_produces_pointer_to_iterator_reference_type<I>();
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_ITERATORUTILS_HPP
