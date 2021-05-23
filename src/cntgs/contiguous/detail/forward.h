#pragma once

#include <cstddef>

namespace cntgs
{
template <class T>
struct VaryingSize;

template <class T>
struct FixedSize;

template <class... Types>
class ContiguousVector;

template <class Vector>
class ContiguousVectorIterator;

class TypeErasedVector;

template <class T, std::size_t Alignment = 0>
struct AlignAs;

template <class... Types>
class ContiguousTuple;
}  // namespace cntgs