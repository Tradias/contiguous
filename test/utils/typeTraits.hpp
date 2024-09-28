// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_UTILS_TYPETRAITS_HPP
#define CNTGS_UTILS_TYPETRAITS_HPP

#include <type_traits>

namespace test
{
template <class T>
struct RemoveCvref
{
    using Type = std::remove_cv_t<std::remove_reference_t<T>>;
};

template <class T>
using RemoveCvrefT = typename RemoveCvref<T>::Type;
}  // namespace test

#endif  // CNTGS_UTILS_TYPETRAITS_HPP
