#pragma once

#include "cntgs/contiguous/detail/elementLocator.h"
#include "cntgs/contiguous/detail/vectorTraits.h"

namespace cntgs
{
class TypeErasedVector
{
  private:
    using Traits = detail::ContiguousVectorTraits<>;

  public:
    std::size_t memory_size;
    std::size_t max_element_count;
    Traits::StorageType memory;
    std::array<std::size_t, detail::MAX_FIXED_SIZE_VECTOR_PARAMETER> fixed_sizes;
    detail::TypeErasedElementLocator locator;
};
}  // namespace cntgs