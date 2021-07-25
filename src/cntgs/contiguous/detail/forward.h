#ifndef CNTGS_DETAIL_FORWARD_H
#define CNTGS_DETAIL_FORWARD_H

#include "cntgs/contiguous/referenceQualifier.h"

#include <cstddef>

namespace cntgs
{
template <class T>
struct VaryingSize;

template <class T>
struct FixedSize;

template <class Allocator, class... T>
class BasicContiguousVector;

template <class Vector>
class ContiguousVectorIterator;

class TypeErasedVector;

template <class T, std::size_t Alignment = 1>
struct AlignAs;

template <cntgs::ContiguousReferenceQualifier Qualifier, class... Types>
class ContiguousReference;

template <class Allocator, class... Types>
class BasicContiguousElement;
}  // namespace cntgs

#endif  // CNTGS_DETAIL_FORWARD_H
