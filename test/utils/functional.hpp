// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_UTILS_FUNCTIONAL_HPP
#define CNTGS_UTILS_FUNCTIONAL_HPP

namespace test
{
struct Identity
{
    template <class T>
    auto operator()(T&& t) -> decltype(static_cast<T&&>(t))
    {
        return static_cast<T&&>(t);
    }
};

template <class Vector>
struct ToValueType
{
    template <class T>
    auto operator()(T&& t)
    {
        return typename Vector::value_type{static_cast<T&&>(t)};
    }
};
}  // namespace test

#endif  // CNTGS_UTILS_FUNCTIONAL_HPP
