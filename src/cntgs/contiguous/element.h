#pragma once

#include "cntgs/contiguous/detail/elementTraits.h"
#include "cntgs/contiguous/detail/fixedSizeGetter.h"
#include "cntgs/contiguous/detail/forward.h"
#include "cntgs/contiguous/detail/memory.h"
#include "cntgs/contiguous/detail/parameterListTraits.h"
#include "cntgs/contiguous/detail/tupleQualifier.h"
#include "cntgs/contiguous/detail/utility.h"
#include "cntgs/contiguous/detail/vectorTraits.h"
#include "cntgs/contiguous/tuple.h"

#include <cstddef>
#include <cstring>
#include <memory>
#include <tuple>

namespace cntgs
{
template <class... Types>
using ContiguousElement = cntgs::BasicContiguousElement<std::allocator<void>, Types...>;

template <class Allocator, class... Types>
class BasicContiguousElement
{
  private:
    using ListTraits = detail::ParameterListTraits<Types...>;
    using VectorTraits = detail::ContiguousVectorTraits<Types...>;
    using ElementTraits = detail::ElementTraitsT<Types...>;
    using AllocatorTraits = std::allocator_traits<Allocator>;
    using StorageElementType = detail::AlignedByte<ElementTraits::template ParameterTraitsAt<0>::ALIGNMENT>;
    using StorageType = detail::AllocateUniquePtr<StorageElementType[], Allocator>;
    using Tuple = typename VectorTraits::ReferenceReturnType;

  public:
    using allocator_type = Allocator;

    StorageType memory;
    Tuple tuple;

    BasicContiguousElement() = default;

    template <detail::ContiguousTupleQualifier Qualifier>
    /*implicit*/ BasicContiguousElement(const cntgs::ContiguousTuple<Qualifier, Types...>& other,
                                        allocator_type allocator = {})
        : memory(
              detail::allocate_unique_for_overwrite<StorageElementType[]>(other.size_in_bytes(), std::move(allocator))),
          tuple(this->store_and_load(other, other.size_in_bytes()))
    {
    }

    template <detail::ContiguousTupleQualifier Qualifier>
    /*implicit*/ BasicContiguousElement(cntgs::ContiguousTuple<Qualifier, Types...>&& other,
                                        allocator_type allocator = {})
        : memory(
              detail::allocate_unique_for_overwrite<StorageElementType[]>(other.size_in_bytes(), std::move(allocator))),
          tuple(this->store_and_load(other, other.size_in_bytes()))
    {
    }

    /*implicit*/ BasicContiguousElement(const BasicContiguousElement& other)
        : BasicContiguousElement(
              other, AllocatorTraits::select_on_container_copy_construction(other.memory.get_deleter().get_allocator()))
    {
    }

    template <class TAllocator>
    /*implicit*/ BasicContiguousElement(const BasicContiguousElement<TAllocator, Types...>& other)
        : BasicContiguousElement(
              other, AllocatorTraits::select_on_container_copy_construction(other.memory.get_deleter().get_allocator()))
    {
    }

    template <class TAllocator>
    BasicContiguousElement(const BasicContiguousElement<TAllocator, Types...>& other, allocator_type allocator)
        : memory(detail::allocate_unique_for_overwrite<StorageElementType[]>(other.tuple.size_in_bytes(),
                                                                             std::move(allocator))),
          tuple(this->store_and_load(other.tuple, other.tuple.size_in_bytes()))
    {
    }

    BasicContiguousElement(BasicContiguousElement&&) = default;

    template <class TAllocator>
    BasicContiguousElement(BasicContiguousElement<TAllocator, Types...>&& other)
        : memory(std::move(other.memory)), tuple(std::move(other.tuple))
    {
    }

    template <class TAllocator>
    BasicContiguousElement(BasicContiguousElement<TAllocator, Types...>&& other, allocator_type allocator)
        : memory(allocator == other.memory.get_deleter().get_allocator()
                     ? std::move(other.memory)
                     : detail::allocate_unique_for_overwrite<StorageElementType[]>(other.tuple.size_in_bytes(),
                                                                                   std::move(allocator))),
          tuple(allocator == other.memory.get_deleter().get_allocator()
                    ? std::move(other.tuple)
                    : this->store_and_load(other.tuple, other.tuple.size_in_bytes()))
    {
    }

    BasicContiguousElement& operator=(const BasicContiguousElement& other) noexcept(
        ListTraits::IS_NOTHROW_COPY_ASSIGNABLE)
    {
        this->tuple = other.tuple;
        return *this;
    }

    template <class TAllocator>
    BasicContiguousElement& operator=(const BasicContiguousElement<TAllocator, Types...>& other) noexcept(
        ListTraits::IS_NOTHROW_COPY_ASSIGNABLE)
    {
        this->tuple = other.tuple;
        return *this;
    }

    BasicContiguousElement& operator=(BasicContiguousElement&& other) noexcept(ListTraits::IS_NOTHROW_MOVE_ASSIGNABLE)
    {
        this->tuple = std::move(other.tuple);
        return *this;
    }

    template <class TAllocator>
    BasicContiguousElement& operator=(BasicContiguousElement<TAllocator, Types...>&& other) noexcept(
        ListTraits::IS_NOTHROW_MOVE_ASSIGNABLE)
    {
        this->tuple = std::move(other.tuple);
        return *this;
    }

    template <detail::ContiguousTupleQualifier Qualifier>
    constexpr BasicContiguousElement& operator=(const cntgs::ContiguousTuple<Qualifier, Types...>& other) noexcept(
        ListTraits::IS_NOTHROW_COPY_ASSIGNABLE)
    {
        this->tuple = other;
        return *this;
    }

    template <detail::ContiguousTupleQualifier Qualifier>
    constexpr BasicContiguousElement& operator=(cntgs::ContiguousTuple<Qualifier, Types...>&& other) noexcept(
        ContiguousTuple<Qualifier, Types...>::IS_CONST ? ListTraits::IS_NOTHROW_MOVE_ASSIGNABLE
                                                       : ListTraits::IS_NOTHROW_COPY_ASSIGNABLE)
    {
        this->tuple = std::move(other);
        return *this;
    }

    allocator_type get_allocator() const noexcept { return this->memory.get_deleter().get_allocator(); }

    ~BasicContiguousElement() noexcept
    {
        if constexpr (!ListTraits::IS_TRIVIALLY_DESTRUCTIBLE)
        {
            if (this->memory)
            {
                ElementTraits::destruct(this->tuple);
            }
        }
    }

  private:
    template <class Tuple>
    auto store_and_load(Tuple& source, std::size_t memory_size)
    {
        static constexpr auto USE_MOVE = !std::is_const_v<Tuple> && !Tuple::IS_CONST;
        const auto begin = this->memory_begin();
        std::memcpy(begin, source.start_address(), memory_size);
        auto target =
            ElementTraits::template load_element_at<detail::IgnoreFirstAlignmentSelector,
                                                    detail::ContiguousReturnTypeSizeGetter>(begin, source.tuple);
        ElementTraits::template construct_if_non_trivial<USE_MOVE>(source, target);
        return Tuple{target};
    }

    [[nodiscard]] auto memory_begin() const noexcept
    {
        return detail::assume_aligned<ElementTraits::template ParameterTraitsAt<0>::ALIGNMENT>(
            reinterpret_cast<std::byte*>(this->memory.get()));
    }
};

template <class Allocator, class... T>
constexpr void swap(cntgs::BasicContiguousElement<Allocator, T...>& lhs,
                    cntgs::BasicContiguousElement<Allocator, T...>& rhs) noexcept
{
    std::swap(lhs.memory, rhs.memory);
    std::swap(lhs.tuple.tuple, rhs.tuple.tuple);
}

template <std::size_t I, class Allocator, class... Types>
[[nodiscard]] constexpr decltype(auto) get(cntgs::BasicContiguousElement<Allocator, Types...>& element) noexcept
{
    return cntgs::get<I>(element.tuple);
}

template <std::size_t I, class Allocator, class... Types>
[[nodiscard]] constexpr decltype(auto) get(const cntgs::BasicContiguousElement<Allocator, Types...>& element) noexcept
{
    return detail::as_const(cntgs::get<I>(element.tuple));
}

template <std::size_t I, class Allocator, class... Types>
[[nodiscard]] constexpr decltype(auto) get(cntgs::BasicContiguousElement<Allocator, Types...>&& element) noexcept
{
    return cntgs::get<I>(std::move(element.tuple));
}

template <std::size_t I, class Allocator, class... Types>
[[nodiscard]] constexpr decltype(auto) get(const cntgs::BasicContiguousElement<Allocator, Types...>&& element) noexcept
{
    return cntgs::get<I>(std::move(element.tuple));
}
}  // namespace cntgs

namespace std
{
template <std::size_t I, class Allocator, class... Types>
struct tuple_element<I, ::cntgs::BasicContiguousElement<Allocator, Types...>>
    : std::tuple_element<I, typename ::cntgs::BasicContiguousElement<Allocator, Types...>::Tuple>
{
};

template <class... Types>
struct tuple_size<::cntgs::BasicContiguousElement<Types...>> : std::integral_constant<std::size_t, sizeof...(Types)>
{
};
}  // namespace std
