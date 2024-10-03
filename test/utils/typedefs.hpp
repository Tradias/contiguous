// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_UTILS_TYPEDEFS_HPP
#define CNTGS_UTILS_TYPEDEFS_HPP

#include <cntgs/contiguous.hpp>

#include <memory>
#include <memory_resource>

namespace test
{
using UInt8 = unsigned char;

template <class Allocator, class... Parameter>
using ContiguousVectorWithAllocator =
    cntgs::BasicContiguousVector<cntgs::Options<cntgs::Allocator<Allocator>>, Parameter...>;

namespace pmr
{
template <class... Parameter>
using ContiguousVector = test::ContiguousVectorWithAllocator<std::pmr::polymorphic_allocator<std::byte>, Parameter...>;
}

using Plain = cntgs::ContiguousVector<uint32_t, float>;
using OneVarying = cntgs::ContiguousVector<uint32_t, cntgs::VaryingSize<float>>;
using TwoVarying = cntgs::ContiguousVector<uint32_t, cntgs::VaryingSize<float>, cntgs::VaryingSize<float>>;
using OneFixed = cntgs::ContiguousVector<uint32_t, cntgs::FixedSize<float>>;
using TwoFixed = cntgs::ContiguousVector<cntgs::FixedSize<float>, uint32_t, cntgs::FixedSize<float>>;
using OneFixedOneVarying = cntgs::ContiguousVector<cntgs::FixedSize<float>, uint32_t, cntgs::VaryingSize<float>>;
using OneFixedUniquePtr = cntgs::ContiguousVector<cntgs::FixedSize<std::unique_ptr<int>>, std::unique_ptr<int>>;
using OneVaryingUniquePtr = cntgs::ContiguousVector<cntgs::VaryingSize<std::unique_ptr<int>>, std::unique_ptr<int>>;

using PlainAligned = cntgs::ContiguousVector<char, cntgs::AlignAs<uint32_t, 8>>;
using OneVaryingAligned = cntgs::ContiguousVector<cntgs::VaryingSize<cntgs::AlignAs<float, 16>>, uint32_t>;
using TwoVaryingAligned = cntgs::ContiguousVector<uint32_t, cntgs::VaryingSize<cntgs::AlignAs<float, 8>>,
                                                  cntgs::VaryingSize<cntgs::AlignAs<float, 16>>>;
template <class Allocator = std::allocator<std::byte>>
using OneFixedAligned = ContiguousVectorWithAllocator<Allocator, uint32_t, cntgs::FixedSize<cntgs::AlignAs<float, 32>>>;
template <class Allocator = std::allocator<std::byte>>
using TwoFixedAligned = ContiguousVectorWithAllocator<Allocator, cntgs::FixedSize<cntgs::AlignAs<float, 8>>,
                                                      cntgs::AlignAs<uint32_t, 16>, cntgs::FixedSize<float>>;
using TwoFixedAlignedAlt =
    cntgs::ContiguousVector<cntgs::FixedSize<cntgs::AlignAs<float, 32>>, cntgs::FixedSize<uint32_t>, uint32_t>;
using OneFixedOneVaryingAligned = cntgs::ContiguousVector<cntgs::FixedSize<cntgs::AlignAs<float, 16>>, uint32_t,
                                                          cntgs::VaryingSize<cntgs::AlignAs<float, 8>>>;
}  // namespace test

#endif  // CNTGS_UTILS_TYPEDEFS_HPP
