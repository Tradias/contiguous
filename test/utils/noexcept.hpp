// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_UTILS_NOEXCEPT_HPP
#define CNTGS_UTILS_NOEXCEPT_HPP

namespace test
{
template <bool IsNoexcept>
struct Noexcept
{
    Noexcept() noexcept(IsNoexcept);
    Noexcept(const Noexcept&) noexcept(IsNoexcept);
    Noexcept(Noexcept&&) noexcept(IsNoexcept);
    Noexcept& operator=(const Noexcept&) noexcept(IsNoexcept);
    Noexcept& operator=(Noexcept&&) noexcept(IsNoexcept);
    bool operator==(const Noexcept&) const noexcept(IsNoexcept);
    bool operator<(const Noexcept&) const noexcept(IsNoexcept);
};
}  // namespace test

#endif  // CNTGS_UTILS_NOEXCEPT_HPP
