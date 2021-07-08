#pragma once

#include <cstddef>

namespace cntgs
{
template <class T>
struct VaryingSize
{
};

template <class T>
struct FixedSize
{
};

template <class T, std::size_t Alignment>
struct AlignAs
{
};
}  // namespace cntgs