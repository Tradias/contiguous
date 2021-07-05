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
    void (*deleter)(cntgs::TypeErasedVector&);

    TypeErasedVector(std::size_t memory_size, std::size_t max_element_count, Traits::StorageType memory,
                     std::array<std::size_t, detail::MAX_FIXED_SIZE_VECTOR_PARAMETER> fixed_sizes,
                     detail::TypeErasedElementLocator locator, void (*deleter)(cntgs::TypeErasedVector&)) noexcept
        : memory_size(memory_size),
          max_element_count(max_element_count),
          memory(std::move(memory)),
          fixed_sizes(fixed_sizes),
          locator(std::move(locator)),
          deleter(deleter)
    {
    }

    TypeErasedVector(const TypeErasedVector&) = delete;
    TypeErasedVector(TypeErasedVector&&) = default;

    TypeErasedVector& operator=(const TypeErasedVector&) = delete;
    TypeErasedVector& operator=(TypeErasedVector&&) = default;

    ~TypeErasedVector() noexcept { deleter(*this); }
};
}  // namespace cntgs