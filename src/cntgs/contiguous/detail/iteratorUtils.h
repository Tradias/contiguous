#ifndef CNTGS_DETAIL_ITERATORUTILS_H
#define CNTGS_DETAIL_ITERATORUTILS_H

#include "cntgs/contiguous/detail/typeUtils.h"

#include <iterator>
#include <version>

namespace cntgs::detail
{
template <class T>
struct ArrowProxy
{
    T t;

    constexpr const T* operator->() const noexcept { return &t; }
};

template <class I>
inline constexpr auto CONTIGUOUS_ITERATOR_V =
    detail::DerivedFrom<typename std::iterator_traits<I>::iterator_category, std::random_access_iterator_tag>&&
        std::is_lvalue_reference_v<typename std::iterator_traits<I>::reference>&&
            std::is_same_v<typename std::iterator_traits<I>::value_type,
                           detail::RemoveCvrefT<typename std::iterator_traits<I>::reference>>&&
                std::is_same_v<decltype(std::declval<const I&>().operator->()),
                               std::add_pointer_t<typename std::iterator_traits<I>::reference>>;
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_ITERATORUTILS_H
