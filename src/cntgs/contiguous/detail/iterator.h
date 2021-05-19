#pragma once

#ifndef __cpp_concepts
#include "cntgs/contiguous/detail/typeUtils.h"
#endif

#include <iterator>

namespace cntgs::detail
{
template <class T>
struct ArrowProxy
{
    T t;

    constexpr const T* operator->() const noexcept { return &t; }
};

#ifdef __cpp_concepts
template <class I>
concept ContiguousIterator = std::contiguous_iterator<I>;
#else
template <class I>
static constexpr auto ContiguousIterator =
    detail::DerivedFrom<typename std::iterator_traits<I>::iterator_category, std::random_access_iterator_tag>&&
        std::is_lvalue_reference_v<typename std::iterator_traits<I>::reference>&&
            std::is_same_v<typename std::iterator_traits<I>::value_type,
                           detail::RemoveCvrefT<typename std::iterator_traits<I>::reference>>&&
                std::is_same_v<decltype(std::declval<const I&>().operator->()),
                               std::add_pointer_t<typename std::iterator_traits<I>::reference>>;
#endif
}  // namespace cntgs::detail