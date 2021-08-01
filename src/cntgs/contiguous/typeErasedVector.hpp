#ifndef CNTGS_CONTIGUOUS_TYPEERASEDVECTOR_HPP
#define CNTGS_CONTIGUOUS_TYPEERASEDVECTOR_HPP

#include "cntgs/contiguous/detail/allocator.hpp"
#include "cntgs/contiguous/detail/array.hpp"
#include "cntgs/contiguous/detail/elementLocator.hpp"
#include "cntgs/contiguous/detail/utility.hpp"

#include <cstddef>

namespace cntgs
{
class TypeErasedVector
{
  public:
    std::size_t memory_size;
    std::size_t max_element_count;
    std::byte* memory;
    detail::MoveDefaultingValue<bool> is_memory_owned;
    detail::Array<std::size_t, detail::MAX_FIXED_SIZE_VECTOR_PARAMETER> fixed_sizes;
    detail::TypeErasedElementLocator locator;
    void (*destructor)(cntgs::TypeErasedVector&);
    detail::TypeErasedAllocator allocator;

    TypeErasedVector(std::size_t memory_size, std::size_t max_element_count, std::byte* memory, bool is_memory_owned,
                     detail::TypeErasedAllocator allocator,
                     const detail::Array<std::size_t, detail::MAX_FIXED_SIZE_VECTOR_PARAMETER>& fixed_sizes,
                     detail::TypeErasedElementLocator locator, void (*destructor)(cntgs::TypeErasedVector&)) noexcept
        : memory_size(memory_size),
          max_element_count(max_element_count),
          memory(memory),
          is_memory_owned(is_memory_owned),
          fixed_sizes(fixed_sizes),
          locator(locator),
          destructor(destructor),
          allocator(allocator)
    {
    }

    TypeErasedVector(const TypeErasedVector&) = delete;
    TypeErasedVector(TypeErasedVector&&) = default;

    TypeErasedVector& operator=(const TypeErasedVector&) = delete;
    TypeErasedVector& operator=(TypeErasedVector&&) = default;

    ~TypeErasedVector() noexcept { destructor(*this); }
};
}  // namespace cntgs

#endif  // CNTGS_CONTIGUOUS_TYPEERASEDVECTOR_HPP
