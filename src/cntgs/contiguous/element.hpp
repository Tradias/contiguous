#ifndef CNTGS_CONTIGUOUS_ELEMENT_HPP
#define CNTGS_CONTIGUOUS_ELEMENT_HPP

#include "cntgs/contiguous/detail/elementTraits.hpp"
#include "cntgs/contiguous/detail/forward.hpp"
#include "cntgs/contiguous/detail/memory.hpp"
#include "cntgs/contiguous/detail/parameterListTraits.hpp"
#include "cntgs/contiguous/detail/sizeGetter.hpp"
#include "cntgs/contiguous/detail/utility.hpp"
#include "cntgs/contiguous/detail/vectorTraits.hpp"
#include "cntgs/contiguous/reference.hpp"

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

    BasicContiguousElement() = default;

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
    explicit constexpr BasicContiguousElement(BasicContiguousElement<OtherAllocator, Types...>&& other)
        : memory(std::move(other.memory)), reference(std::move(other.reference))
    {
    }

    template <class OtherAllocator>
    constexpr BasicContiguousElement(BasicContiguousElement<OtherAllocator, Types...>&& other,
                                     const allocator_type& allocator)
        : memory(this->acquire_memory(other, allocator)), reference(this->acquire_reference(other, allocator))
    {
    }

    ~BasicContiguousElement() noexcept
    {
        if constexpr (!ListTraits::IS_TRIVIALLY_DESTRUCTIBLE)
        {
            if (this->memory)
            {
                ElementTraits::destruct(this->reference);
            }
        }
    }

    BasicContiguousElement& operator=(const BasicContiguousElement& other) noexcept(
        ListTraits::IS_NOTHROW_COPY_ASSIGNABLE)
    {
        this->reference = other.reference;
        return *this;
    }

    template <class OtherAllocator>
    BasicContiguousElement& operator=(const BasicContiguousElement<OtherAllocator, Types...>& other) noexcept(
        ListTraits::IS_NOTHROW_COPY_ASSIGNABLE)
    {
        this->reference = other.reference;
        return *this;
    }

    BasicContiguousElement& operator=(BasicContiguousElement&& other) noexcept(ListTraits::IS_NOTHROW_MOVE_ASSIGNABLE)
    {
        this->reference = std::move(other.reference);
        return *this;
    }

    template <class OtherAllocator>
    BasicContiguousElement& operator=(BasicContiguousElement<OtherAllocator, Types...>&& other) noexcept(
        ListTraits::IS_NOTHROW_MOVE_ASSIGNABLE)
    {
        this->reference = std::move(other.reference);
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
    {
        return this->reference == other;
    }

    [[nodiscard]] constexpr auto operator==(const BasicContiguousElement& other) const
    {
        return this->reference == other.reference;
    }

    template <bool IsConst>
    [[nodiscard]] constexpr auto operator!=(const cntgs::BasicContiguousReference<IsConst, Types...>& other) const
    {
        return !(this->reference == other);
    }

    [[nodiscard]] constexpr auto operator!=(const BasicContiguousElement& other) const
    {
        return !(this->reference == other.reference);
    }

    template <bool IsConst>
    [[nodiscard]] constexpr auto operator<(const cntgs::BasicContiguousReference<IsConst, Types...>& other) const
    {
        return this->reference < other;
    }

    [[nodiscard]] constexpr auto operator<(const BasicContiguousElement& other) const
    {
        return this->reference < other.reference;
    }

    template <bool IsConst>
    [[nodiscard]] constexpr auto operator<=(const cntgs::BasicContiguousReference<IsConst, Types...>& other) const
    {
        return !(other < this->reference);
    }

    [[nodiscard]] constexpr auto operator<=(const BasicContiguousElement& other) const
    {
        return !(other.reference < this->reference);
    }

    template <bool IsConst>
    [[nodiscard]] constexpr auto operator>(const cntgs::BasicContiguousReference<IsConst, Types...>& other) const
    {
        return other < this->reference;
    }

    [[nodiscard]] constexpr auto operator>(const BasicContiguousElement& other) const
    {
        return other.reference < this->reference;
    }

    template <bool IsConst>
    [[nodiscard]] constexpr auto operator>=(const cntgs::BasicContiguousReference<IsConst, Types...>& other) const
    {
        return !(this->reference < other);
    }

    [[nodiscard]] constexpr auto operator>=(const BasicContiguousElement& other) const
    {
        return !(this->reference < other.reference);
    }

  private:
    template <class SourceReference>
    auto store_and_load(SourceReference& source, std::size_t memory_size) const
    {
        static constexpr auto USE_MOVE = !std::is_const_v<SourceReference> && !SourceReference::IS_CONST;
        const auto begin = this->memory_begin();
        std::memcpy(begin, source.data_begin(), memory_size);
        auto target =
            ElementTraits::template load_element_at<detail::IgnoreFirstAlignmentNeeds,
                                                    detail::ContiguousReferenceSizeGetter>(begin, source.tuple);
        ElementTraits::template construct_if_non_trivial<USE_MOVE>(source, target);
        return Reference{target};
    }

    template <class OtherAllocator>
    auto acquire_memory(BasicContiguousElement<OtherAllocator, Types...>& other, const allocator_type& allocator) const
    {
        if constexpr (detail::AreEqualityComparable<allocator_type, OtherAllocator>::value)
        {
            if (allocator == other.memory.get_allocator())
            {
                return std::move(other.memory);
            }
        }
        return StorageType(other.reference.size_in_bytes(), allocator);
    }

    template <class OtherAllocator>
    auto acquire_reference(BasicContiguousElement<OtherAllocator, Types...>& other,
                           [[maybe_unused]] const allocator_type& allocator) const
    {
        if constexpr (detail::AreEqualityComparable<allocator_type, OtherAllocator>::value)
        {
            if (allocator == other.memory.get_allocator())
            {
                return std::move(other.reference);
            }
        }
        return this->store_and_load(other.reference, other.reference.size_in_bytes());
    }

    auto memory_begin() const noexcept
    {
        return detail::assume_aligned<ElementTraits::template ParameterTraitsAt<0>::ALIGNMENT>(
            reinterpret_cast<std::byte*>(this->memory.get()));
    }
};

template <class Allocator, class... T>
constexpr void swap(cntgs::BasicContiguousElement<Allocator, T...>& lhs,
                    cntgs::BasicContiguousElement<Allocator, T...>& rhs) noexcept
{
    using std::swap;
    swap(lhs.memory, rhs.memory);
    auto temp{lhs.reference.tuple};
    detail::construct_at(&lhs.reference.tuple, rhs.reference.tuple);
    detail::construct_at(&rhs.reference.tuple, temp);
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

#endif  // CNTGS_CONTIGUOUS_ELEMENT_HPP
