#pragma once

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
}  // namespace cntgs