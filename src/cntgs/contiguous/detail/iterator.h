#pragma once

namespace cntgs::detail
{
template <class T>
struct ArrowProxy
{
    T t;

    constexpr const T* operator->() const noexcept { return &t; }
};
}  // namespace cntgs::detail