// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_UTILS_NOEXCEPT_HPP
#define CNTGS_UTILS_NOEXCEPT_HPP

namespace cntgs::test
{
template <bool IsNoThrow>
struct Thrower
{
    Thrower() noexcept(IsNoThrow);
    Thrower(const Thrower&) noexcept(IsNoThrow);
    Thrower(Thrower&&) noexcept(IsNoThrow);
    Thrower& operator=(const Thrower&) noexcept(IsNoThrow);
    Thrower& operator=(Thrower&&) noexcept(IsNoThrow);
    bool operator==(const Thrower&) const noexcept(IsNoThrow);
    bool operator<(const Thrower&) const noexcept(IsNoThrow);
};
}  // namespace cntgs::test

#endif  // CNTGS_UTILS_NOEXCEPT_HPP
