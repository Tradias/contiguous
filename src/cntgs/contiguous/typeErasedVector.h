#pragma once

#include "cntgs/contiguous/detail/elementLocator.h"
#include "cntgs/contiguous/detail/vectorTraits.h"

namespace cntgs
{
class TypeErasedVector
{
  private:
    using Traits = detail::ContiguousVectorTraits<>;
    using SizeType = Traits::SizeType;

  public:
    SizeType memory_size;
    SizeType max_element_count;
    Traits::StorageType memory;
    std::array<SizeType, detail::MAX_FIXED_SIZE_VECTOR_PARAMETER> fixed_sizes;
    detail::TypeErasedElementLocator locator;
};
}  // namespace cntgs