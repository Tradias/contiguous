// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_CNTGS_TYPEERASEDVECTOR_HPP
#define CNTGS_CNTGS_TYPEERASEDVECTOR_HPP

#include "cntgs/detail/allocator.hpp"
#include "cntgs/detail/array.hpp"
#include "cntgs/detail/elementLocator.hpp"
#include "cntgs/detail/utility.hpp"

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
    detail::TypeErasedAllocator allocator;
    void (*destructor)(cntgs::TypeErasedVector&);

    template <class Allocator, class... Types>
    TypeErasedVector(cntgs::BasicContiguousVector<Allocator, Types...>&& vector) noexcept
        : memory_size(vector.memory.size()),
          max_element_count(vector.max_element_count),
          memory(vector.memory.release()),
          is_memory_owned(true),
          fixed_sizes(
              detail::convert_array_to_size<detail::MAX_FIXED_SIZE_VECTOR_PARAMETER>(vector.locator.fixed_sizes())),
          locator(detail::type_erase_element_locator(*vector.locator)),
          allocator(detail::type_erase_allocator(vector.get_allocator())),
          destructor(
              []([[maybe_unused]] cntgs::TypeErasedVector& erased)
              {
                  if (erased.is_memory_owned.value)
                  {
                      cntgs::BasicContiguousVector<Allocator, Types...>(std::move(erased));
                  }
              })
    {
    }

    TypeErasedVector(const TypeErasedVector&) = delete;
    TypeErasedVector(TypeErasedVector&&) = default;

    TypeErasedVector& operator=(const TypeErasedVector&) = delete;
    TypeErasedVector& operator=(TypeErasedVector&&) = default;

    ~TypeErasedVector() noexcept { destructor(*this); }
};
}  // namespace cntgs

#endif  // CNTGS_CNTGS_TYPEERASEDVECTOR_HPP
