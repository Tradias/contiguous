#pragma once

#include "cntgs/contiguous/detail/elementLocator.h"
#include "cntgs/contiguous/detail/memory.h"
#include "cntgs/contiguous/detail/utility.h"
#include "cntgs/contiguous/detail/vectorTraits.h"

#include <memory>

namespace cntgs
{
class TypeErasedVector
{
  public:
    std::size_t max_element_count;
    std::byte* memory;
    detail::MoveDefaultingValue<bool> is_memory_owned;
    std::array<std::size_t, detail::MAX_FIXED_SIZE_VECTOR_PARAMETER> fixed_sizes;
    detail::TypeErasedElementLocator locator;
    void (*destructor)(cntgs::TypeErasedVector&);
    detail::TypeErasedDeleter deleter;

    TypeErasedVector(std::size_t max_element_count, std::byte* memory, bool is_memory_owned,
                     detail::TypeErasedDeleter deleter,
                     const std::array<std::size_t, detail::MAX_FIXED_SIZE_VECTOR_PARAMETER>& fixed_sizes,
                     detail::TypeErasedElementLocator locator, void (*destructor)(cntgs::TypeErasedVector&)) noexcept
        : max_element_count(max_element_count),
          memory(std::move(memory)),
          is_memory_owned(is_memory_owned),
          fixed_sizes(fixed_sizes),
          locator(std::move(locator)),
          destructor(destructor),
          deleter(std::move(deleter))
    {
    }

    TypeErasedVector(const TypeErasedVector&) = delete;
    TypeErasedVector(TypeErasedVector&&) = default;

    TypeErasedVector& operator=(const TypeErasedVector&) = delete;
    TypeErasedVector& operator=(TypeErasedVector&&) = default;

    ~TypeErasedVector() noexcept { destructor(*this); }
};
}  // namespace cntgs