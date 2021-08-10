// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_CNTGS_ELEMENT_HPP
#define CNTGS_CNTGS_ELEMENT_HPP

#include "cntgs/detail/allocator.hpp"
#include "cntgs/detail/elementTraits.hpp"
#include "cntgs/detail/forward.hpp"
#include "cntgs/detail/memory.hpp"
#include "cntgs/detail/parameterListTraits.hpp"
#include "cntgs/detail/sizeGetter.hpp"
#include "cntgs/detail/utility.hpp"
#include "cntgs/detail/vectorTraits.hpp"
#include "cntgs/reference.hpp"

#include <cstddef>
#include <cstring>
#include <memory>
#include <tuple>

namespace cntgs
{
template <class... Types>
using ContiguousElement = cntgs::BasicContiguousElement<std::allocator<std::byte>, Types...>;

template <class Allocator, class... Types>
class BasicContiguousElement
{
  private:
    using ListTraits = detail::ParameterListTraits<Types...>;
    using VectorTraits = detail::ContiguousVectorTraits<Types...>;
    using ElementTraits = detail::ElementTraitsT<Types...>;
    using AllocatorTraits = std::allocator_traits<Allocator>;
    using StorageElementType = detail::AlignedByte<ElementTraits::template ParameterTraitsAt<0>::ALIGNMENT>;
    using StorageType = detail::AllocatorAwarePointer<
        typename std::allocator_traits<Allocator>::template rebind_alloc<StorageElementType>>;
    using Reference = typename VectorTraits::ReferenceType;

  public:
    using allocator_type = Allocator;

    StorageType memory;
    Reference reference;

    template <bool IsConst>
    /*implicit*/ BasicContiguousElement(const cntgs::BasicContiguousReference<IsConst, Types...>& other,
                                        const allocator_type& allocator = {})
        : memory(other.size_in_bytes(), allocator), reference(this->store_and_load(other, other.size_in_bytes()))
    {
    }

    template <bool IsConst>
    /*implicit*/ BasicContiguousElement(cntgs::BasicContiguousReference<IsConst, Types...>&& other,
                                        const allocator_type& allocator = {})
        : memory(other.size_in_bytes(), allocator), reference(this->store_and_load(other, other.size_in_bytes()))
    {
    }

    /*implicit*/ BasicContiguousElement(const BasicContiguousElement& other)
        : memory(other.memory), reference(this->store_and_load(other.reference, other.reference.size_in_bytes()))
    {
    }

    template <class OtherAllocator>
    explicit BasicContiguousElement(const BasicContiguousElement<OtherAllocator, Types...>& other)
        : memory(other.memory), reference(this->store_and_load(other.reference, other.reference.size_in_bytes()))
    {
    }

    template <class OtherAllocator>
    BasicContiguousElement(const BasicContiguousElement<OtherAllocator, Types...>& other,
                           const allocator_type& allocator)
        : memory(other.reference.size_in_bytes(), allocator),
          reference(this->store_and_load(other.reference, other.reference.size_in_bytes()))
    {
    }

    BasicContiguousElement(BasicContiguousElement&&) = default;

    template <class OtherAllocator>
    explicit constexpr BasicContiguousElement(BasicContiguousElement<OtherAllocator, Types...>&& other) noexcept
        : memory(std::move(other.memory)), reference(std::move(other.reference))
    {
    }

    template <class OtherAllocator>
    constexpr BasicContiguousElement(
        BasicContiguousElement<OtherAllocator, Types...>&& other,
        const allocator_type& allocator) noexcept(detail::AreEqualityComparable<allocator_type, OtherAllocator>::value&&
                                                      AllocatorTraits::is_always_equal::value)
        : memory(this->acquire_memory(other, allocator)), reference(this->acquire_reference(other, allocator))
    {
    }

    ~BasicContiguousElement() noexcept { this->destruct(); }

    BasicContiguousElement& operator=(const BasicContiguousElement& other)
    {
        this->copy_assign(other);
        return *this;
    }

    BasicContiguousElement& operator=(BasicContiguousElement&& other) noexcept(
        AllocatorTraits::is_always_equal::value || AllocatorTraits::propagate_on_container_move_assignment::value)
    {
        this->move_assign(std::move(other));
        return *this;
    }

    template <bool IsConst>
    constexpr BasicContiguousElement& operator=(const cntgs::BasicContiguousReference<IsConst, Types...>&
                                                    other) noexcept(ListTraits::IS_NOTHROW_COPY_ASSIGNABLE)
    {
        this->reference = other;
        return *this;
    }

    template <bool IsConst>
    constexpr BasicContiguousElement& operator=(cntgs::BasicContiguousReference<IsConst, Types...>&& other) noexcept(
        IsConst ? ListTraits::IS_NOTHROW_COPY_ASSIGNABLE : ListTraits::IS_NOTHROW_MOVE_ASSIGNABLE)
    {
        this->reference = std::move(other);
        return *this;
    }

    [[nodiscard]] constexpr allocator_type get_allocator() const noexcept { return this->memory.get_allocator(); }

    template <bool IsConst>
    [[nodiscard]] constexpr auto operator==(const cntgs::BasicContiguousReference<IsConst, Types...>& other) const
        noexcept(ListTraits::IS_NOTHROW_EQUALITY_COMPARABLE)
    {
        return this->reference == other;
    }

    [[nodiscard]] constexpr auto operator==(const BasicContiguousElement& other) const
        noexcept(ListTraits::IS_NOTHROW_EQUALITY_COMPARABLE)
    {
        return this->reference == other.reference;
    }

    template <bool IsConst>
    [[nodiscard]] constexpr auto operator!=(const cntgs::BasicContiguousReference<IsConst, Types...>& other) const
        noexcept(ListTraits::IS_NOTHROW_EQUALITY_COMPARABLE)
    {
        return !(this->reference == other);
    }

    [[nodiscard]] constexpr auto operator!=(const BasicContiguousElement& other) const
        noexcept(ListTraits::IS_NOTHROW_EQUALITY_COMPARABLE)
    {
        return !(this->reference == other.reference);
    }

    template <bool IsConst>
    [[nodiscard]] constexpr auto operator<(const cntgs::BasicContiguousReference<IsConst, Types...>& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return this->reference < other;
    }

    [[nodiscard]] constexpr auto operator<(const BasicContiguousElement& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return this->reference < other.reference;
    }

    template <bool IsConst>
    [[nodiscard]] constexpr auto operator<=(const cntgs::BasicContiguousReference<IsConst, Types...>& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return !(other < this->reference);
    }

    [[nodiscard]] constexpr auto operator<=(const BasicContiguousElement& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return !(other.reference < this->reference);
    }

    template <bool IsConst>
    [[nodiscard]] constexpr auto operator>(const cntgs::BasicContiguousReference<IsConst, Types...>& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return other < this->reference;
    }

    [[nodiscard]] constexpr auto operator>(const BasicContiguousElement& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return other.reference < this->reference;
    }

    template <bool IsConst>
    [[nodiscard]] constexpr auto operator>=(const cntgs::BasicContiguousReference<IsConst, Types...>& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return !(this->reference < other);
    }

    [[nodiscard]] constexpr auto operator>=(const BasicContiguousElement& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return !(this->reference < other.reference);
    }

  private:
    template <class SourceReference>
    auto store_and_load(SourceReference& source, std::size_t memory_size) const
    {
        return this->store_and_load(source, memory_size, this->memory_begin());
    }

    template <class SourceReference>
    auto store_and_load(SourceReference& source, std::size_t memory_size, std::byte* target_memory) const
    {
        static constexpr auto USE_MOVE = !std::is_const_v<SourceReference> && !SourceReference::IS_CONST;
        std::memcpy(target_memory, source.data_begin(), memory_size);
        auto target =
            ElementTraits::template load_element_at<detail::IgnoreFirstAlignmentNeeds,
                                                    detail::ContiguousReferenceSizeGetter>(target_memory, source.tuple);
        ElementTraits::template construct_if_non_trivial<USE_MOVE>(source, target);
        return Reference{target};
    }

    template <class OtherAllocator>
    auto acquire_memory(BasicContiguousElement<OtherAllocator, Types...>& other, const allocator_type& allocator) const
    {
        if constexpr (detail::AreEqualityComparable<allocator_type, OtherAllocator>::value)
        {
            if constexpr (AllocatorTraits::is_always_equal::value)
            {
                return std::move(other.memory);
            }
            else
            {
                if (allocator == other.memory.get_allocator())
                {
                    return std::move(other.memory);
                }
            }
        }
        return StorageType(other.memory.size(), allocator);
    }

    template <class OtherAllocator>
    auto acquire_reference(BasicContiguousElement<OtherAllocator, Types...>& other,
                           [[maybe_unused]] const allocator_type& allocator) const
    {
        if constexpr (detail::AreEqualityComparable<allocator_type, OtherAllocator>::value)
        {
            if constexpr (AllocatorTraits::is_always_equal::value)
            {
                return std::move(other.reference);
            }
            else
            {
                if (allocator == other.memory.get_allocator())
                {
                    return std::move(other.reference);
                }
            }
        }
        return this->store_and_load(other.reference, other.memory.size());
    }

    auto memory_begin() const noexcept { return BasicContiguousElement::memory_begin(this->memory); }

    static auto memory_begin(const StorageType& memory) noexcept
    {
        return detail::assume_aligned<ElementTraits::template ParameterTraitsAt<0>::ALIGNMENT>(
            reinterpret_cast<std::byte*>(memory.get()));
    }

    template <class OtherAllocator>
    void copy_assign(const BasicContiguousElement<OtherAllocator, Types...>& other)
    {
        if (this != std::addressof(other))
        {
            if constexpr (ListTraits::IS_FIXED_SIZE_OR_PLAIN &&
                          (!AllocatorTraits::propagate_on_container_copy_assignment::value ||
                           AllocatorTraits::is_always_equal::value))
            {
                this->reference = other.reference;
                this->memory.propagate_on_container_copy_assignment(other.memory);
            }
            else
            {
                this->destruct();
                this->memory = other.memory;
                this->store_and_construct_reference_inplace(other.reference, other.memory.size());
            }
        }
    }

    template <class OtherAllocator>
    constexpr void steal(BasicContiguousElement<OtherAllocator, Types...>&& other) noexcept
    {
        this->destruct();
        this->memory = std::move(other.memory);
        detail::construct_at(std::addressof(this->reference), std::move(other.reference));
    }

    template <class OtherAllocator>
    constexpr void move_assign(BasicContiguousElement<OtherAllocator, Types...>&& other)
    {
        if (this == std::addressof(other))
        {
            return;
        }
        if constexpr (AllocatorTraits::is_always_equal::value ||
                      AllocatorTraits::propagate_on_container_move_assignment::value)
        {
            this->steal(std::move(other));
        }
        else
        {
            if (this->get_allocator() == other.get_allocator())
            {
                this->steal(std::move(other));
            }
            else
            {
                if constexpr (ListTraits::IS_FIXED_SIZE_OR_PLAIN)
                {
                    this->reference = std::move(other.reference);
                    this->memory.propagate_on_container_move_assignment(other.memory);
                }
                else
                {
                    const auto other_size_in_bytes = other.reference.size_in_bytes();
                    if (other_size_in_bytes > this->memory.size())
                    {
                        // allocate memory first because it might throw
                        StorageType new_memory{other.memory.size(), this->get_allocator()};
                        this->destruct();
                        detail::construct_at(std::addressof(this->reference),
                                             this->store_and_load(other.reference, other_size_in_bytes,
                                                                  BasicContiguousElement::memory_begin(new_memory)));
                        this->memory = std::move(new_memory);
                    }
                    else
                    {
                        this->destruct();
                        this->store_and_construct_reference_inplace(other.reference, other_size_in_bytes);
                    }
                }
            }
        }
    }

    template <class SourceReference>
    void store_and_construct_reference_inplace(SourceReference& other, std::size_t memory_size)
    {
        detail::construct_at(std::addressof(this->reference), this->store_and_load(other, memory_size));
    }

    void destruct() noexcept
    {
        if constexpr (!ListTraits::IS_TRIVIALLY_DESTRUCTIBLE)
        {
            if (this->memory)
            {
                ElementTraits::destruct(this->reference);
            }
        }
    }
};

template <class Allocator, class... T>
constexpr void swap(cntgs::BasicContiguousElement<Allocator, T...>& lhs,
                    cntgs::BasicContiguousElement<Allocator, T...>& rhs) noexcept
{
    detail::swap(lhs.memory, rhs.memory);
    auto temp{lhs.reference};
    detail::construct_at(&lhs.reference, rhs.reference);
    detail::construct_at(&rhs.reference, temp);
}

template <std::size_t I, class Allocator, class... Types>
[[nodiscard]] constexpr decltype(auto) get(cntgs::BasicContiguousElement<Allocator, Types...>& element) noexcept
{
    return cntgs::get<I>(element.reference);
}

template <std::size_t I, class Allocator, class... Types>
[[nodiscard]] constexpr decltype(auto) get(const cntgs::BasicContiguousElement<Allocator, Types...>& element) noexcept
{
    return detail::as_const(cntgs::get<I>(element.reference));
}

template <std::size_t I, class Allocator, class... Types>
[[nodiscard]] constexpr decltype(auto) get(cntgs::BasicContiguousElement<Allocator, Types...>&& element) noexcept
{
    return cntgs::get<I>(std::move(element.reference));
}

template <std::size_t I, class Allocator, class... Types>
[[nodiscard]] constexpr decltype(auto) get(const cntgs::BasicContiguousElement<Allocator, Types...>&& element) noexcept
{
    return detail::as_const(cntgs::get<I>(std::move(element.reference)));
}
}  // namespace cntgs

namespace std
{
template <std::size_t I, class Allocator, class... Types>
struct tuple_element<I, ::cntgs::BasicContiguousElement<Allocator, Types...>>
    : std::tuple_element<I, decltype(::cntgs::BasicContiguousElement<Allocator, Types...>::reference)>
{
};

template <class Allocator, class... Types>
struct tuple_size<::cntgs::BasicContiguousElement<Allocator, Types...>>
    : std::integral_constant<std::size_t, sizeof...(Types)>
{
};
}  // namespace std

#endif  // CNTGS_CNTGS_ELEMENT_HPP
