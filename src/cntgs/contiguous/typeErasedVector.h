#pragma once

#include "cntgs/contiguous/detail/elementLocator.h"
#include "cntgs/contiguous/detail/vectorTraits.h"

namespace cntgs
{
class TypeErasedVector
{
  private:
    using Traits = detail::ContiguousVectorTraitsT<>;
    using SizeType = Traits::SizeType;

  public:
    SizeType memory_size;
    SizeType max_element_count;
    Traits::StorageType memory;
    std::byte* last_element;
    std::array<SizeType, Traits::MAX_FIXED_SIZE_VECTOR_PARAMETER> fixed_sizes;
    std::array<std::byte, detail::MAX_ELEMENT_LOCATOR_SIZE> locator;
};
}  // namespace cntgs