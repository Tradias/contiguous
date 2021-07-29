#ifndef CNTGS_DETAIL_FORWARD_HPP
#define CNTGS_DETAIL_FORWARD_HPP

#include <cstddef>

namespace cntgs
{
template <class T>
struct VaryingSize;

template <class T>
struct FixedSize;

template <class T, std::size_t Alignment = 1>
struct AlignAs;

template <class Allocator, class... T>
class BasicContiguousVector;

template <bool IsConst, class Allocator, class... Types>
class ContiguousVectorIterator;

class TypeErasedVector;

template <bool IsConst, class... Types>
class BasicContiguousReference;

template <class Allocator, class... Types>
class BasicContiguousElement;
}  // namespace cntgs

#endif  // CNTGS_DETAIL_FORWARD_HPP