// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

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

template <class... Option>
struct Options;

template <class T>
struct Allocator;

template <class Options, class... T>
class BasicContiguousVector;

template <bool IsConst, class Options, class... Parameter>
class ContiguousVectorIterator;

template <bool IsConst, class... Parameter>
class BasicContiguousReference;

template <class Allocator, class... Parameter>
class BasicContiguousElement;
}  // namespace cntgs

#endif  // CNTGS_DETAIL_FORWARD_HPP
