#ifndef CNTGS_CONTIGUOUS_PARAMETER_H
#define CNTGS_CONTIGUOUS_PARAMETER_H

#include <cstddef>

namespace cntgs
{
/// Parameter that causes values to be stored along with a `std::size_t`. When used with
/// [cntgs::BasicContiguousVector]() every element of the vector can have a different number of values of type `T`
///
/// \param T User-defined or built-in type, optionally wrapped in [cntgs::AlignAs]()
template <class T>
struct VaryingSize
{
};

/// When used with [cntgs::BasicContiguousVector]() every element of the vector has a fixed number of values of type
/// `T`.
///
/// \param T User-defined or built-in type, optionally wrapped in [cntgs::AlignAs]()
template <class T>
struct FixedSize
{
};

/// When used with [cntgs::BasicContiguousVector]() every element of the vector is stored with an alignment of
/// [Alignment](<> "cntgs::AlignAs<T, Alignment>.Alignment").
///
/// \param T User-defined or built-in type
/// \param Alignment Desired alignment of the type
template <class T, std::size_t Alignment>
struct AlignAs
{
};
}  // namespace cntgs

#endif  // CNTGS_CONTIGUOUS_PARAMETER_H
