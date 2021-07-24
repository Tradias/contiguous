#ifndef CNTGS_CONTIGUOUS_PARAMETER_H
#define CNTGS_CONTIGUOUS_PARAMETER_H

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

#endif  // CNTGS_CONTIGUOUS_PARAMETER_H
