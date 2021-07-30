#ifndef CNTGS_DETAIL_PARAMETERTRAITS_HPP
#define CNTGS_DETAIL_PARAMETERTRAITS_HPP

#include "cntgs/contiguous/detail/attributes.hpp"
#include "cntgs/contiguous/detail/memory.hpp"
#include "cntgs/contiguous/detail/parameterType.hpp"
#include "cntgs/contiguous/detail/typeUtils.hpp"
#include "cntgs/contiguous/parameter.hpp"
#include "cntgs/contiguous/span.hpp"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <new>
#include <type_traits>

namespace cntgs::detail
{
template <class T>
struct ParameterTraits : detail::ParameterTraits<cntgs::AlignAs<T, 1>>
{
};

template <class T, std::size_t Alignment>
struct ParameterTraits<cntgs::AlignAs<T, Alignment>>
{
    using ValueType = T;
    using PointerType = std::add_pointer_t<T>;
    using ReferenceType = std::add_lvalue_reference_t<T>;
    using ConstReferenceType = std::add_lvalue_reference_t<std::add_const_t<T>>;

    static constexpr auto TYPE = detail::ParameterType::PLAIN;
    static constexpr auto ALIGNMENT = Alignment;
    static constexpr auto VALUE_BYTES = sizeof(T);
    static constexpr auto ALIGNED_SIZE_IN_MEMORY = std::max(VALUE_BYTES, ALIGNMENT);

    template <bool NeedsAlignment>
    static auto load(std::byte* address, std::size_t) noexcept
    {
        address = detail::align_if<NeedsAlignment, ALIGNMENT>(address);
        auto result = std::launder(reinterpret_cast<PointerType>(address));
        return std::pair{result, address + VALUE_BYTES};
    }

    template <bool NeedsAlignment, bool, class Arg>
    CNTGS_RESTRICT_RETURN static std::byte* store(Arg&& arg, std::byte* CNTGS_RESTRICT address, std::size_t)
    {
        address = reinterpret_cast<std::byte*>(detail::align_if<NeedsAlignment, ALIGNMENT>(address));
        detail::construct_at(reinterpret_cast<T*>(address), std::forward<Arg>(arg));
        return address + VALUE_BYTES;
    }

    static constexpr auto aligned_size_in_memory(std::size_t) noexcept { return ALIGNED_SIZE_IN_MEMORY; }

    static constexpr auto guaranteed_size_in_memory(std::size_t) noexcept { return VALUE_BYTES; }

    static auto data_begin(ConstReferenceType reference) noexcept
    {
        return detail::assume_aligned<ALIGNMENT>(reinterpret_cast<const std::byte*>(std::addressof(reference)));
    }

    static auto data_begin(ReferenceType reference) noexcept
    {
        return detail::assume_aligned<ALIGNMENT>(reinterpret_cast<std::byte*>(std::addressof(reference)));
    }

    static auto data_end(ConstReferenceType reference) noexcept { return data_begin(reference) + VALUE_BYTES; }

    static auto data_end(ReferenceType reference) noexcept { return data_begin(reference) + VALUE_BYTES; }

    static constexpr void copy(ConstReferenceType source,
                               ReferenceType target) noexcept(std::is_nothrow_copy_assignable_v<T>)
    {
        target = source;
    }

    static constexpr void move(T&& source, ReferenceType target) noexcept(std::is_nothrow_move_assignable_v<T>)
    {
        target = std::move(source);
    }

    static constexpr void move(ReferenceType source,
                               ReferenceType target) noexcept(std::is_nothrow_move_assignable_v<T>)
    {
        target = std::move(source);
    }

    static constexpr void uninitialized_copy(ConstReferenceType source,
                                             T* target) noexcept(std::is_nothrow_copy_constructible_v<T>)
    {
        detail::construct_at(target, source);
    }

    static constexpr void uninitialized_move(ReferenceType source,
                                             T* target) noexcept(std::is_nothrow_move_constructible_v<T>)
    {
        detail::construct_at(target, std::move(source));
    }

    static constexpr void swap(ReferenceType lhs, ReferenceType rhs) noexcept(std::is_nothrow_swappable_v<T>)
    {
        using std::swap;
        swap(lhs, rhs);
    }

    static constexpr auto equal(ConstReferenceType lhs, ConstReferenceType rhs) { return lhs == rhs; }

    static constexpr auto lexicographical_compare(ConstReferenceType lhs, ConstReferenceType rhs) { return lhs < rhs; }

    static constexpr void destroy(ReferenceType value) noexcept { value.~T(); }
};

template <class T, std::size_t Alignment>
struct BaseContiguousParameterTraits
{
    using Self = BaseContiguousParameterTraits<T, Alignment>;

    static auto data_begin(const cntgs::Span<std::add_const_t<T>>& value) noexcept
    {
        return reinterpret_cast<const std::byte*>(Self::begin(value));
    }

    static auto data_begin(const cntgs::Span<T>& value) noexcept
    {
        return reinterpret_cast<std::byte*>(Self::begin(value));
    }

    static auto data_end(const cntgs::Span<std::add_const_t<T>>& value) noexcept
    {
        return reinterpret_cast<const std::byte*>(std::end(value));
    }

    static auto data_end(const cntgs::Span<T>& value) noexcept { return reinterpret_cast<std::byte*>(std::end(value)); }

    static constexpr void uninitialized_copy(
        const cntgs::Span<std::add_const_t<T>>& source,
        const cntgs::Span<T>& target) noexcept(std::is_nothrow_copy_constructible_v<T>)
    {
        std::uninitialized_copy(Self::begin(source), std::end(source), Self::begin(target));
    }

    static constexpr void uninitialized_copy(const cntgs::Span<T>& source, const cntgs::Span<T>& target) noexcept(
        std::is_nothrow_copy_constructible_v<T>)
    {
        std::uninitialized_copy(Self::begin(source), std::end(source), Self::begin(target));
    }

    static constexpr void uninitialized_move(const cntgs::Span<T>& source, const cntgs::Span<T>& target) noexcept(
        std::is_nothrow_move_constructible_v<T>)
    {
        std::uninitialized_copy(std::make_move_iterator(Self::begin(source)), std::make_move_iterator(std::end(source)),
                                Self::begin(target));
    }

    static constexpr auto equal(const cntgs::Span<T>& source, const cntgs::Span<T>& target)
    {
        return std::equal(Self::begin(source), std::end(source), Self::begin(target));
    }

    static constexpr auto equal(const cntgs::Span<T>& source, const cntgs::Span<std::add_const_t<T>>& target)
    {
        return std::equal(Self::begin(source), std::end(source), Self::begin(target));
    }

    static constexpr auto equal(const cntgs::Span<std::add_const_t<T>>& source, const cntgs::Span<T>& target)
    {
        return std::equal(Self::begin(source), std::end(source), Self::begin(target));
    }

    static constexpr auto equal(const cntgs::Span<std::add_const_t<T>>& source,
                                const cntgs::Span<std::add_const_t<T>>& target)
    {
        return std::equal(Self::begin(source), std::end(source), Self::begin(target));
    }

    static constexpr auto lexicographical_compare(const cntgs::Span<T>& source, const cntgs::Span<T>& target)
    {
        return std::lexicographical_compare(Self::begin(source), std::end(source), Self::begin(target),
                                            std::end(target));
    }

    static constexpr auto lexicographical_compare(const cntgs::Span<T>& source,
                                                  const cntgs::Span<std::add_const_t<T>>& target)
    {
        return std::lexicographical_compare(Self::begin(source), std::end(source), Self::begin(target),
                                            std::end(target));
    }

    static constexpr auto lexicographical_compare(const cntgs::Span<std::add_const_t<T>>& source,
                                                  const cntgs::Span<T>& target)
    {
        return std::lexicographical_compare(Self::begin(source), std::end(source), Self::begin(target),
                                            std::end(target));
    }

    static constexpr auto lexicographical_compare(const cntgs::Span<std::add_const_t<T>>& source,
                                                  const cntgs::Span<std::add_const_t<T>>& target)
    {
        return std::lexicographical_compare(Self::begin(source), std::end(source), Self::begin(target),
                                            std::end(target));
    }

    static constexpr void destroy(const cntgs::Span<T>& value) noexcept
    {
        std::destroy(Self::begin(value), std::end(value));
    }

    template <class U>
    static constexpr auto begin(const cntgs::Span<U>& value) noexcept
    {
        return detail::assume_aligned<Alignment>(std::begin(value));
    }
};

template <class T>
struct ParameterTraits<cntgs::VaryingSize<T>> : ParameterTraits<cntgs::VaryingSize<cntgs::AlignAs<T, 1>>>
{
};

template <class T, std::size_t Alignment>
struct ParameterTraits<cntgs::VaryingSize<cntgs::AlignAs<T, Alignment>>> : BaseContiguousParameterTraits<T, Alignment>
{
    using ValueType = T;
    using PointerType = cntgs::Span<T>;
    using ReferenceType = cntgs::Span<T>;
    using ConstReferenceType = cntgs::Span<std::add_const_t<T>>;
    using IteratorType = T*;

    static constexpr auto TYPE = detail::ParameterType::VARYING_SIZE;
    static constexpr auto ALIGNMENT = Alignment;
    static constexpr auto MEMORY_OVERHEAD = sizeof(std::size_t);
    static constexpr auto ALIGNED_SIZE_IN_MEMORY = MEMORY_OVERHEAD + ALIGNMENT - 1;

    template <bool NeedsAlignment>
    static auto load(std::byte* address, std::size_t) noexcept
    {
        const auto size = *reinterpret_cast<std::size_t*>(address);
        address += MEMORY_OVERHEAD;
        const auto first_byte = detail::align_if<NeedsAlignment, ALIGNMENT>(address);
        const auto first = std::launder(reinterpret_cast<IteratorType>(first_byte));
        const auto last = std::launder(reinterpret_cast<IteratorType>(first_byte + size));
        return std::pair{PointerType{first, last}, reinterpret_cast<std::byte*>(last)};
    }

    template <bool NeedsAlignment, bool IgnoreAliasing, class Range>
    CNTGS_RESTRICT_RETURN static std::byte* store(Range&& range, std::byte* CNTGS_RESTRICT address, std::size_t)
    {
        const auto size = reinterpret_cast<std::size_t*>(address);
        address += MEMORY_OVERHEAD;
        const auto aligned_address =
            reinterpret_cast<IteratorType>(detail::align_if<NeedsAlignment, ALIGNMENT>(address));
        auto* new_address =
            detail::uninitialized_range_construct<IgnoreAliasing>(std::forward<Range>(range), aligned_address);
        *size = new_address - reinterpret_cast<std::byte*>(aligned_address);
        return new_address;
    }

    static constexpr auto aligned_size_in_memory(std::size_t) noexcept { return ALIGNED_SIZE_IN_MEMORY; }
};

template <class T>
struct ParameterTraits<cntgs::FixedSize<T>> : ParameterTraits<cntgs::FixedSize<cntgs::AlignAs<T, 1>>>
{
};

template <class T, std::size_t Alignment>
struct ParameterTraits<cntgs::FixedSize<cntgs::AlignAs<T, Alignment>>> : BaseContiguousParameterTraits<T, Alignment>
{
    using ValueType = T;
    using PointerType = cntgs::Span<T>;
    using ReferenceType = cntgs::Span<T>;
    using ConstReferenceType = cntgs::Span<std::add_const_t<T>>;
    using IteratorType = T*;

    static constexpr auto TYPE = detail::ParameterType::FIXED_SIZE;
    static constexpr auto ALIGNMENT = Alignment;
    static constexpr auto VALUE_BYTES = sizeof(T);

    template <bool NeedsAlignment>
    static auto load(std::byte* address, std::size_t size) noexcept
    {
        const auto first =
            std::launder(reinterpret_cast<IteratorType>(detail::align_if<NeedsAlignment, ALIGNMENT>(address)));
        const auto last = first + size;
        return std::pair{PointerType{first, last}, reinterpret_cast<std::byte*>(last)};
    }

    template <bool NeedsAlignment, bool IgnoreAliasing, class RangeOrIterator>
    CNTGS_RESTRICT_RETURN static std::byte* store(RangeOrIterator&& range_or_iterator,
                                                  std::byte* CNTGS_RESTRICT address, std::size_t size)
    {
        const auto aligned_address =
            reinterpret_cast<IteratorType>(detail::align_if<NeedsAlignment, ALIGNMENT>(address));
        return detail::uninitialized_construct<IgnoreAliasing>(std::forward<RangeOrIterator>(range_or_iterator),
                                                               aligned_address, size);
    }

    static constexpr auto aligned_size_in_memory(std::size_t fixed_size) noexcept
    {
        if constexpr (ALIGNMENT == 1)
        {
            return fixed_size * VALUE_BYTES;
        }
        else
        {
            return std::max(fixed_size * VALUE_BYTES, ALIGNMENT);
        }
    }

    static constexpr auto guaranteed_size_in_memory(std::size_t fixed_size) noexcept
    {
        return fixed_size * VALUE_BYTES;
    }

    static void copy(const cntgs::Span<std::add_const_t<T>>& source,
                     const cntgs::Span<T>& target) noexcept(std::is_nothrow_copy_assignable_v<T>)
    {
        std::copy(std::begin(source), std::end(source), std::begin(target));
    }

    static void copy(const cntgs::Span<T>& source,
                     const cntgs::Span<T>& target) noexcept(std::is_nothrow_copy_assignable_v<T>)
    {
        std::copy(std::begin(source), std::end(source), std::begin(target));
    }

    static void move(const cntgs::Span<T>& source,
                     const cntgs::Span<T>& target) noexcept(std::is_nothrow_move_assignable_v<T>)
    {
        std::move(std::begin(source), std::end(source), std::begin(target));
    }

    static void swap(const cntgs::Span<T>& lhs, const cntgs::Span<T>& rhs) noexcept(std::is_nothrow_swappable_v<T>)
    {
        std::swap_ranges(std::begin(lhs), std::end(lhs), std::begin(rhs));
    }
};
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_PARAMETERTRAITS_HPP
