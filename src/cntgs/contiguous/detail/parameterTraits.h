#pragma once

#include "cntgs/contiguous/detail/attributes.h"
#include "cntgs/contiguous/detail/memory.h"
#include "cntgs/contiguous/detail/parameterType.h"
#include "cntgs/contiguous/detail/typeUtils.h"
#include "cntgs/contiguous/parameter.h"
#include "cntgs/contiguous/span.h"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <new>
#include <type_traits>

namespace cntgs::detail
{
template <class T>
struct ParameterTraits : detail::ParameterTraits<cntgs::AlignAs<T, 0>>
{
};

template <class T, std::size_t Alignment>
struct ParameterTraits<cntgs::AlignAs<T, Alignment>>
{
    using Type = T;
    using ValueType = T;
    using PointerReturnType = std::add_pointer_t<T>;
    using ReferenceReturnType = std::add_lvalue_reference_t<T>;
    using ConstReferenceReturnType = std::add_lvalue_reference_t<std::add_const_t<T>>;

    static constexpr auto TYPE = detail::ParameterType::PLAIN;
    static constexpr auto ALIGNMENT = Alignment;
    static constexpr auto VALUE_BYTES = sizeof(T);
    static constexpr auto ALIGNED_SIZE_IN_MEMORY = detail::MAX_SIZE_T_OF<VALUE_BYTES, ALIGNMENT>;

    template <bool NeedsAlignment>
    static auto load(std::byte* address, std::size_t) noexcept
    {
        address = static_cast<std::byte*>(detail::align_if<NeedsAlignment, ALIGNMENT>(address));
        auto result = std::launder(reinterpret_cast<PointerReturnType>(address));
        return std::pair{result, address + ALIGNED_SIZE_IN_MEMORY};
    }

    template <bool NeedsAlignment, class Arg>
    CNTGS_RESTRICT_RETURN static std::byte* store(Arg&& arg, std::byte* CNTGS_RESTRICT address, std::size_t)
    {
        address = reinterpret_cast<std::byte*>(detail::align_if<NeedsAlignment, ALIGNMENT>(address));
        detail::construct_at(reinterpret_cast<T*>(address), std::forward<Arg>(arg));
        return address + ALIGNED_SIZE_IN_MEMORY;
    }

    static constexpr auto aligned_size_in_memory(std::size_t) noexcept { return ALIGNED_SIZE_IN_MEMORY; }

    static auto start_address(ConstReferenceReturnType return_type) noexcept
    {
        return reinterpret_cast<const std::byte*>(std::addressof(return_type));
    }

    static auto start_address(ReferenceReturnType return_type) noexcept
    {
        return reinterpret_cast<std::byte*>(std::addressof(return_type));
    }

    static auto end_address(ConstReferenceReturnType return_type) noexcept
    {
        return start_address(return_type) + VALUE_BYTES;
    }

    static auto end_address(ReferenceReturnType return_type) noexcept
    {
        return start_address(return_type) + VALUE_BYTES;
    }

    static constexpr void copy(ConstReferenceReturnType source,
                               ReferenceReturnType target) noexcept(std::is_nothrow_copy_assignable_v<T>)
    {
        target = source;
    }

    static constexpr void move(T&& source, ReferenceReturnType target) noexcept(std::is_nothrow_move_assignable_v<T>)
    {
        target = std::move(source);
    }

    static constexpr void move(ReferenceReturnType source,
                               ReferenceReturnType target) noexcept(std::is_nothrow_move_assignable_v<T>)
    {
        target = std::move(source);
    }

    static constexpr void uninitialized_copy(ConstReferenceReturnType source,
                                             T* target) noexcept(std::is_nothrow_copy_constructible_v<T>)
    {
        detail::construct_at(target, source);
    }

    static constexpr void uninitialized_move(ReferenceReturnType source,
                                             T* target) noexcept(std::is_nothrow_move_constructible_v<T>)
    {
        detail::construct_at(target, std::move(source));
    }

    static constexpr void swap(ReferenceReturnType lhs,
                               ReferenceReturnType rhs) noexcept(std::is_nothrow_swappable_v<T>)
    {
        using std::swap;
        swap(lhs, rhs);
    }

    static constexpr void destroy(ReferenceReturnType value) noexcept { value.~T(); }
};

template <class T>
struct BaseContiguousParameterTraits
{
    static auto start_address(const cntgs::Span<std::add_const_t<T>>& value) noexcept
    {
        return reinterpret_cast<const std::byte*>(value.data());
    }

    static auto start_address(const cntgs::Span<T>& value) noexcept
    {
        return reinterpret_cast<std::byte*>(value.data());
    }

    static auto end_address(const cntgs::Span<std::add_const_t<T>>& value) noexcept
    {
        return reinterpret_cast<const std::byte*>(value.data() + value.size());
    }

    static auto end_address(const cntgs::Span<T>& value) noexcept
    {
        return reinterpret_cast<std::byte*>(value.data() + value.size());
    }

    static void uninitialized_copy(const cntgs::Span<std::add_const_t<T>>& source,
                                   const cntgs::Span<T>& target) noexcept(std::is_nothrow_copy_constructible_v<T>)
    {
        std::uninitialized_copy(std::begin(source), std::end(source), std::begin(target));
    }

    static void uninitialized_copy(const cntgs::Span<T>& source,
                                   const cntgs::Span<T>& target) noexcept(std::is_nothrow_copy_constructible_v<T>)
    {
        std::uninitialized_copy(std::begin(source), std::end(source), std::begin(target));
    }

    static void uninitialized_move(const cntgs::Span<T>& source,
                                   const cntgs::Span<T>& target) noexcept(std::is_nothrow_move_constructible_v<T>)
    {
        std::uninitialized_copy(std::make_move_iterator(std::begin(source)), std::make_move_iterator(std::end(source)),
                                std::begin(target));
    }

    static void destroy(const cntgs::Span<T>& value) noexcept { std::destroy(std::begin(value), std::end(value)); }
};

template <class T>
struct ParameterTraits<cntgs::VaryingSize<T>> : ParameterTraits<cntgs::VaryingSize<cntgs::AlignAs<T, 0>>>
{
};

template <class T, std::size_t Alignment>
struct ParameterTraits<cntgs::VaryingSize<cntgs::AlignAs<T, Alignment>>> : BaseContiguousParameterTraits<T>
{
    using Type = cntgs::VaryingSize<T>;
    using ValueType = T;
    using PointerReturnType = cntgs::Span<T>;
    using ReferenceReturnType = cntgs::Span<T>;
    using ConstReferenceReturnType = cntgs::Span<std::add_const_t<T>>;
    using IteratorType = T*;

    static constexpr auto TYPE = detail::ParameterType::VARYING_SIZE;
    static constexpr auto ALIGNMENT = Alignment;
    static constexpr auto MEMORY_OVERHEAD = sizeof(std::size_t);
    static constexpr auto ALIGNED_SIZE_IN_MEMORY = MEMORY_OVERHEAD + ALIGNMENT;

    template <bool NeedsAlignment>
    static auto load(std::byte* address, std::size_t) noexcept
    {
        const auto size = *reinterpret_cast<std::size_t*>(address);
        address += MEMORY_OVERHEAD;
        const auto first_byte = static_cast<std::byte*>(detail::align_if<NeedsAlignment, ALIGNMENT>(address));
        const auto first = std::launder(reinterpret_cast<IteratorType>(first_byte));
        const auto last = std::launder(reinterpret_cast<IteratorType>(first_byte + size));
        return std::pair{PointerReturnType{first, last}, reinterpret_cast<std::byte*>(last)};
    }

    template <bool NeedsAlignment, class Range>
    CNTGS_RESTRICT_RETURN static std::byte* store(Range&& range, std::byte* CNTGS_RESTRICT address, std::size_t)
    {
        const auto size = reinterpret_cast<std::size_t*>(address);
        address += MEMORY_OVERHEAD;
        address = reinterpret_cast<std::byte*>(detail::align_if<NeedsAlignment, ALIGNMENT>(address));
        auto* new_address = detail::copy_range_ignore_aliasing<T>(std::forward<Range>(range), address);
        *size = new_address - address;
        return new_address;
    }

    static constexpr auto aligned_size_in_memory(std::size_t) noexcept { return ALIGNED_SIZE_IN_MEMORY; }
};

template <class T>
struct ParameterTraits<cntgs::FixedSize<T>> : ParameterTraits<cntgs::FixedSize<cntgs::AlignAs<T, 0>>>
{
};

template <class T, std::size_t Alignment>
struct ParameterTraits<cntgs::FixedSize<cntgs::AlignAs<T, Alignment>>> : BaseContiguousParameterTraits<T>
{
    using Type = cntgs::FixedSize<T>;
    using ValueType = T;
    using PointerReturnType = cntgs::Span<T>;
    using ReferenceReturnType = cntgs::Span<T>;
    using ConstReferenceReturnType = cntgs::Span<std::add_const_t<T>>;
    using IteratorType = T*;

    static constexpr auto TYPE = detail::ParameterType::FIXED_SIZE;
    static constexpr auto ALIGNMENT = Alignment;
    static constexpr auto VALUE_BYTES = sizeof(T);

    template <bool NeedsAlignment>
    static auto load(std::byte* address, std::size_t size) noexcept
    {
        const auto first =
            std::launder(static_cast<IteratorType>(detail::align_if<NeedsAlignment, ALIGNMENT>(address)));
        const auto last = first + size;
        return std::pair{PointerReturnType{first, last}, reinterpret_cast<std::byte*>(last)};
    }

    template <bool NeedsAlignment, class RangeOrIterator>
    CNTGS_RESTRICT_RETURN static std::byte* store(RangeOrIterator&& range_or_iterator,
                                                  std::byte* CNTGS_RESTRICT address, std::size_t size)
    {
        address = reinterpret_cast<std::byte*>(detail::align_if<NeedsAlignment, ALIGNMENT>(address));
        return detail::copy_ignore_aliasing<T>(std::forward<RangeOrIterator>(range_or_iterator), address, size);
    }

    static constexpr auto aligned_size_in_memory(std::size_t fixed_size) noexcept
    {
        if constexpr (ALIGNMENT == 0)
        {
            return fixed_size * VALUE_BYTES;
        }
        else
        {
            return std::max(fixed_size * VALUE_BYTES, ALIGNMENT);
        }
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
