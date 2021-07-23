#pragma once

#include "cntgs/contiguous/detail/tupleQualifier.h"

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

template <detail::ContiguousReferenceQualifier Qualifier, class... Types>
class ContiguousReference;

template <class Allocator, class... Types>
class BasicContiguousElement;
}  // namespace cntgs