#pragma once

#include "cntgs/contiguous/detail/memory.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <utility>

namespace cntgs::detail
{
// Some compilers (e.g. MSVC) perform handrolled optimizations or call C functions if the argument type
// fulfills certain criteria. These checks are not always performed correctly for std::byte, therefore
// cast it to a more reliable type.

template <class T>
constexpr auto trivial_swap_ranges(T* begin, T* end, T* begin2) noexcept
{
    return std::swap_ranges(reinterpret_cast<detail::Byte*>(begin), reinterpret_cast<detail::Byte*>(end),
                            reinterpret_cast<detail::Byte*>(begin2));
}

template <class T>
constexpr auto trivial_equal(const T* begin, const T* end, const T* begin2, const T* end2) noexcept
{
    return std::equal(reinterpret_cast<const detail::Byte*>(begin), reinterpret_cast<const detail::Byte*>(end),
                      reinterpret_cast<const detail::Byte*>(begin2), reinterpret_cast<const detail::Byte*>(end2));
}

template <class T>
constexpr auto trivial_lexicographical_compare(const T* begin, const T* end, const T* begin2, const T* end2) noexcept
{
    return std::lexicographical_compare(
        reinterpret_cast<const detail::Byte*>(begin), reinterpret_cast<const detail::Byte*>(end),
        reinterpret_cast<const detail::Byte*>(begin2), reinterpret_cast<const detail::Byte*>(end2));
}
}  // namespace cntgs::detail