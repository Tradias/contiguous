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
template <class... Parameter>
using ContiguousElement = cntgs::BasicContiguousElement<std::allocator<std::byte>, Parameter...>;

template <class Allocator, class... Parameter>
class BasicContiguousElement
{
  private:
    using ListTraits = detail::ParameterListTraits<Parameter...>;
    using VectorTraits = detail::ContiguousVectorTraits<Parameter...>;
    using ElementTraits = detail::ElementTraitsT<Parameter...>;
    using AllocatorTraits = std::allocator_traits<Allocator>;
    using StorageElementType = detail::AlignedByte<ElementTraits::template ParameterTraitsAt<0>::ALIGNMENT>;
    using StorageType = detail::AllocatorAwarePointer<
        typename std::allocator_traits<Allocator>::template rebind_alloc<StorageElementType>>;
    using Reference = typename VectorTraits::ReferenceType;

  public:
    using allocator_type = Allocator;

    StorageType memory_;
    Reference reference_;

    template <bool IsConst>
    /*implicit*/ BasicContiguousElement(const cntgs::BasicContiguousReference<IsConst, Parameter...>& other,
                                        const allocator_type& allocator = {})
        : memory_(other.size_in_bytes(), allocator), reference_(store_and_load(other, other.size_in_bytes()))
    {
    }

    template <bool IsConst>
    /*implicit*/ BasicContiguousElement(cntgs::BasicContiguousReference<IsConst, Parameter...>&& other,
                                        const allocator_type& allocator = {})
        : memory_(other.size_in_bytes(), allocator), reference_(store_and_load(other, other.size_in_bytes()))
    {
    }

    /*implicit*/ BasicContiguousElement(const BasicContiguousElement& other)
        : memory_(other.memory_), reference_(store_and_load(other.reference_, other.reference_.size_in_bytes()))
    {
    }

    template <class OtherAllocator>
    explicit BasicContiguousElement(const BasicContiguousElement<OtherAllocator, Parameter...>& other)
        : memory_(other.memory_), reference_(store_and_load(other.reference_, other.reference_.size_in_bytes()))
    {
    }

    template <class OtherAllocator>
    BasicContiguousElement(const BasicContiguousElement<OtherAllocator, Parameter...>& other,
                           const allocator_type& allocator)
        : memory_(other.reference_.size_in_bytes(), allocator),
          reference_(store_and_load(other.reference_, other.reference_.size_in_bytes()))
    {
    }

    BasicContiguousElement(BasicContiguousElement&&) = default;

    template <class OtherAllocator>
    constexpr explicit BasicContiguousElement(BasicContiguousElement<OtherAllocator, Parameter...>&& other) noexcept
        : memory_(std::move(other.memory_)), reference_(std::move(other.reference_))
    {
    }

    template <class OtherAllocator>
    constexpr BasicContiguousElement(
        BasicContiguousElement<OtherAllocator, Parameter...>&& other,
        const allocator_type&
            allocator) noexcept(detail::AreEqualityComparable<allocator_type, OtherAllocator>::value &&
                                AllocatorTraits::is_always_equal::value)
        : memory_(acquire_memory(other, allocator)), reference_(acquire_reference(other, allocator))
    {
    }

    ~BasicContiguousElement() noexcept { destruct(); }

    BasicContiguousElement& operator=(const BasicContiguousElement& other)
    {
        if (this != std::addressof(other))
        {
            copy_assign(other);
        }
        return *this;
    }

    BasicContiguousElement& operator=(BasicContiguousElement&& other) noexcept(
        AllocatorTraits::is_always_equal::value || AllocatorTraits::propagate_on_container_move_assignment::value)
    {
        if (this != std::addressof(other))
        {
            move_assign(std::move(other));
        }
        return *this;
    }

    template <bool IsConst>
    constexpr BasicContiguousElement& operator=(const cntgs::BasicContiguousReference<IsConst, Parameter...>&
                                                    other) noexcept(ListTraits::IS_NOTHROW_COPY_ASSIGNABLE)
    {
        reference_ = other;
        return *this;
    }

    template <bool IsConst>
    constexpr BasicContiguousElement& operator=(cntgs::BasicContiguousReference<IsConst, Parameter...>&&
                                                    other) noexcept(IsConst ? ListTraits::IS_NOTHROW_COPY_ASSIGNABLE
                                                                            : ListTraits::IS_NOTHROW_MOVE_ASSIGNABLE)
    {
        reference_ = std::move(other);
        return *this;
    }

    [[nodiscard]] constexpr allocator_type get_allocator() const noexcept { return memory_.get_allocator(); }

    template <bool IsConst>
    [[nodiscard]] constexpr auto operator==(const cntgs::BasicContiguousReference<IsConst, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTHROW_EQUALITY_COMPARABLE)
    {
        return reference_ == other;
    }

    [[nodiscard]] constexpr auto operator==(const BasicContiguousElement& other) const
        noexcept(ListTraits::IS_NOTHROW_EQUALITY_COMPARABLE)
    {
        return reference_ == other.reference_;
    }

    template <bool IsConst>
    [[nodiscard]] constexpr auto operator!=(const cntgs::BasicContiguousReference<IsConst, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTHROW_EQUALITY_COMPARABLE)
    {
        return !(reference_ == other);
    }

    [[nodiscard]] constexpr auto operator!=(const BasicContiguousElement& other) const
        noexcept(ListTraits::IS_NOTHROW_EQUALITY_COMPARABLE)
    {
        return !(reference_ == other.reference_);
    }

    template <bool IsConst>
    [[nodiscard]] constexpr auto operator<(const cntgs::BasicContiguousReference<IsConst, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return reference_ < other;
    }

    [[nodiscard]] constexpr auto operator<(const BasicContiguousElement& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return reference_ < other.reference_;
    }

    template <bool IsConst>
    [[nodiscard]] constexpr auto operator<=(const cntgs::BasicContiguousReference<IsConst, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return !(other < reference_);
    }

    [[nodiscard]] constexpr auto operator<=(const BasicContiguousElement& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return !(other.reference_ < reference_);
    }

    template <bool IsConst>
    [[nodiscard]] constexpr auto operator>(const cntgs::BasicContiguousReference<IsConst, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return other < reference_;
    }

    [[nodiscard]] constexpr auto operator>(const BasicContiguousElement& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return other.reference_ < reference_;
    }

    template <bool IsConst>
    [[nodiscard]] constexpr auto operator>=(const cntgs::BasicContiguousReference<IsConst, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return !(reference_ < other);
    }

    [[nodiscard]] constexpr auto operator>=(const BasicContiguousElement& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return !(reference_ < other.reference_);
    }

  private:
    template <class SourceReference>
    auto store_and_load(SourceReference& source, std::size_t memory_size) const
    {
        return store_and_load(source, memory_size, memory_begin());
    }

    template <class SourceReference>
    auto store_and_load(SourceReference& source, std::size_t memory_size, std::byte* target_memory) const
    {
        static constexpr auto USE_MOVE = !std::is_const_v<SourceReference> && !SourceReference::IS_CONST;
        std::memcpy(target_memory, source.data_begin(), memory_size);
        auto target =
            ElementTraits::template load_element_at<detail::IgnoreFirstAlignmentNeeds,
                                                    detail::ContiguousReferenceSizeGetter>(target_memory, source);
        ElementTraits::template construct_if_non_trivial<USE_MOVE>(source, target);
        return Reference{target};
    }

    template <class OtherAllocator>
    auto acquire_memory(BasicContiguousElement<OtherAllocator, Parameter...>& other,
                        [[maybe_unused]] const allocator_type& allocator) const
    {
        if constexpr (detail::AreEqualityComparable<allocator_type, OtherAllocator>::value)
        {
            if constexpr (AllocatorTraits::is_always_equal::value)
            {
                return std::move(other.memory_);
            }
            else
            {
                if (allocator == other.memory_.get_allocator())
                {
                    return std::move(other.memory_);
                }
                return StorageType(other.memory_.size(), allocator);
            }
        }
        else
        {
            return StorageType(other.memory_.size(), allocator);
        }
    }

    template <class OtherAllocator>
    auto acquire_reference(BasicContiguousElement<OtherAllocator, Parameter...>& other,
                           [[maybe_unused]] const allocator_type& allocator) const
    {
        if constexpr (detail::AreEqualityComparable<allocator_type, OtherAllocator>::value)
        {
            if constexpr (AllocatorTraits::is_always_equal::value)
            {
                return std::move(other.reference_);
            }
            else
            {
                if (allocator == other.memory_.get_allocator())
                {
                    return std::move(other.reference_);
                }
                return store_and_load(other.reference_, other.memory_.size());
            }
        }
        else
        {
            return store_and_load(other.reference_, other.memory_.size());
        }
    }

    auto memory_begin() const noexcept { return BasicContiguousElement::memory_begin(memory_); }

    static auto memory_begin(const StorageType& memory) noexcept
    {
        return detail::assume_aligned<ElementTraits::template ParameterTraitsAt<0>::ALIGNMENT>(
            reinterpret_cast<std::byte*>(memory.get()));
    }

    template <class OtherAllocator>
    void copy_assign(const BasicContiguousElement<OtherAllocator, Parameter...>& other)
    {
        if (this != std::addressof(other))
        {
            if constexpr (ListTraits::IS_FIXED_SIZE_OR_PLAIN &&
                          (!AllocatorTraits::propagate_on_container_copy_assignment::value ||
                           AllocatorTraits::is_always_equal::value))
            {
                reference_ = other.reference_;
                memory_.propagate_on_container_copy_assignment(other.memory_);
            }
            else
            {
                destruct();
                memory_ = other.memory_;
                store_and_construct_reference_inplace(other.reference_, other.memory_.size());
            }
        }
    }

    template <class OtherAllocator>
    constexpr void steal(BasicContiguousElement<OtherAllocator, Parameter...>&& other) noexcept
    {
        destruct();
        memory_ = std::move(other.memory_);
        reference_.tuple_ = std::move(other.reference_.tuple_);
    }

    template <class OtherAllocator>
    constexpr void move_assign(BasicContiguousElement<OtherAllocator, Parameter...>&& other)
    {
        if constexpr (AllocatorTraits::is_always_equal::value ||
                      AllocatorTraits::propagate_on_container_move_assignment::value)
        {
            steal(std::move(other));
        }
        else
        {
            if (get_allocator() == other.get_allocator())
            {
                steal(std::move(other));
            }
            else
            {
                if constexpr (ListTraits::IS_FIXED_SIZE_OR_PLAIN)
                {
                    reference_ = std::move(other.reference_);
                    memory_.propagate_on_container_move_assignment(other.memory_);
                }
                else
                {
                    const auto other_size_in_bytes = other.reference_.size_in_bytes();
                    if (other_size_in_bytes > memory_.size())
                    {
                        // allocate memory first because it might throw
                        StorageType new_memory{other.memory_.size(), get_allocator()};
                        destruct();
                        reference_.tuple_ = store_and_load(other.reference_, other_size_in_bytes,
                                                                       BasicContiguousElement::memory_begin(new_memory))
                                                      .tuple_;
                        memory_ = std::move(new_memory);
                    }
                    else
                    {
                        destruct();
                        store_and_construct_reference_inplace(other.reference_, other_size_in_bytes);
                    }
                }
            }
        }
    }

    template <class SourceReference>
    void store_and_construct_reference_inplace(SourceReference& other, std::size_t memory_size)
    {
        reference_.tuple_ = store_and_load(other, memory_size).tuple_;
    }

    void destruct() noexcept
    {
        if constexpr (!ListTraits::IS_TRIVIALLY_DESTRUCTIBLE)
        {
            if (memory_)
            {
                ElementTraits::destruct(reference_);
            }
        }
    }
};

template <class Allocator, class... T>
constexpr void swap(cntgs::BasicContiguousElement<Allocator, T...>& lhs,
                    cntgs::BasicContiguousElement<Allocator, T...>& rhs) noexcept
{
    detail::swap(lhs.memory_, rhs.memory_);
    std::swap(lhs.reference_.tuple_, rhs.reference_.tuple_);
}

template <std::size_t I, class Allocator, class... Parameter>
[[nodiscard]] constexpr decltype(auto) get(cntgs::BasicContiguousElement<Allocator, Parameter...>& element) noexcept
{
    return cntgs::get<I>(element.reference_);
}

template <std::size_t I, class Allocator, class... Parameter>
[[nodiscard]] constexpr decltype(auto) get(
    const cntgs::BasicContiguousElement<Allocator, Parameter...>& element) noexcept
{
    return detail::as_const(cntgs::get<I>(element.reference_));
}

template <std::size_t I, class Allocator, class... Parameter>
[[nodiscard]] constexpr decltype(auto) get(cntgs::BasicContiguousElement<Allocator, Parameter...>&& element) noexcept
{
    return std::move(cntgs::get<I>(element.reference_));
}

template <std::size_t I, class Allocator, class... Parameter>
[[nodiscard]] constexpr decltype(auto) get(
    const cntgs::BasicContiguousElement<Allocator, Parameter...>&& element) noexcept
{
    return detail::as_const(std::move(cntgs::get<I>(element.reference_)));
}
}  // namespace cntgs

namespace std
{
template <std::size_t I, class Allocator, class... Parameter>
struct tuple_element<I, ::cntgs::BasicContiguousElement<Allocator, Parameter...>>
    : std::tuple_element<I, decltype(::cntgs::BasicContiguousElement<Allocator, Parameter...>::reference)>
{
};

template <class Allocator, class... Parameter>
struct tuple_size<::cntgs::BasicContiguousElement<Allocator, Parameter...>>
    : std::integral_constant<std::size_t, sizeof...(Parameter)>
{
};
}  // namespace std

#endif  // CNTGS_CNTGS_ELEMENT_HPP
