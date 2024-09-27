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

    template <class Allocator, class... Parameter>
    explicit TypeErasedVector(cntgs::BasicContiguousVector<Allocator, Parameter...>&& vector) noexcept
        : memory_size(vector.memory_.size()),
          max_element_count(vector.max_element_count_),
          memory(vector.memory_.release()),
          is_memory_owned(true),
          fixed_sizes(
              detail::convert_array_to_size<detail::MAX_FIXED_SIZE_VECTOR_PARAMETER>(vector.locator_.fixed_sizes())),
          locator(detail::type_erase_element_locator(*vector.locator_)),
          allocator(detail::type_erase_allocator(vector.get_allocator())),
          destructor(&TypeErasedVector::destruct<Allocator, Parameter...>)
    {
    }

    TypeErasedVector(const TypeErasedVector&) = delete;
    TypeErasedVector(TypeErasedVector&&) = default;

    TypeErasedVector& operator=(const TypeErasedVector&) = delete;
    TypeErasedVector& operator=(TypeErasedVector&&) = default;

    ~TypeErasedVector() noexcept { destructor(*this); }

  private:
    template <class Allocator, class... Parameter>
    static void destruct(cntgs::TypeErasedVector& erased)
    {
        if (erased.is_memory_owned.value_)
        {
            cntgs::BasicContiguousVector<Allocator, Parameter...>(std::move(erased));
        }
    }
};
}  // namespace cntgs

#endif  // CNTGS_CNTGS_TYPEERASEDVECTOR_HPP
