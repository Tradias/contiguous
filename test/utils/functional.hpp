// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_UTILS_FUNCTIONAL_HPP
#define CNTGS_UTILS_FUNCTIONAL_HPP

namespace cntgs::test
{
struct Identity
{
    template <class T>
    auto operator()(T&& t) -> decltype(std::forward<T>(t))
    {
        return std::forward<T>(t);
    }
};

template <class Vector>
struct ToValueType
{
    template <class T>
    auto operator()(T&& t)
    {
        return typename Vector::value_type{std::forward<T>(t)};
    }
};
}  // namespace cntgs::test

#endif  // CNTGS_UTILS_FUNCTIONAL_HPP
