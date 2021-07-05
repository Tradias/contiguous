#pragma once

#include "cntgs/contiguous/detail/tupleQualifier.h"

#include <cstddef>

namespace cntgs
{
template <class T>
struct VaryingSize;

template <class T>
struct FixedSize;

template <class... Types>
class BasicContiguousVector;

template <class Vector>
class ContiguousVectorIterator;

class TypeErasedVector;

template <class T, std::size_t Alignment = 1>
struct AlignAs;

template <detail::ContiguousTupleQualifier Qualifier, class... Types>
class ContiguousTuple;

template <class... Types>
class ContiguousElement;

template <class... Types>
struct Options;
}  // namespace cntgs