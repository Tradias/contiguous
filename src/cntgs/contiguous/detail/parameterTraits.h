#pragma once

#include "cntgs/contiguous/detail/attributes.h"
#include "cntgs/contiguous/detail/memory.h"
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
    using ValueReturnType = T;
    using PointerReturnType = std::add_pointer_t<T>;
    using ReferenceReturnType = std::add_lvalue_reference_t<T>;
    using ConstReferenceReturnType = std::add_lvalue_reference_t<std::add_const_t<T>>;
    using value_type = T;
    using iterator_type = PointerReturnType;

    static constexpr bool IS_CONTIGUOUS = false;
    static constexpr bool IS_FIXED_SIZE = false;
    static constexpr auto VALUE_BYTES = sizeof(value_type);
    static constexpr auto ALIGNMENT = Alignment;
    static constexpr auto MEMORY_OVERHEAD = std::size_t{};
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
        detail::construct_at(reinterpret_cast<value_type*>(address), std::forward<Arg>(arg));
        return address + ALIGNED_SIZE_IN_MEMORY;
    }

    static constexpr auto aligned_size_in_memory(std::size_t) noexcept { return ALIGNED_SIZE_IN_MEMORY; }

    static constexpr auto start_address(PointerReturnType return_type) noexcept
    {
        return reinterpret_cast<std::byte*>(return_type);
    }

    template <class U>
    static auto start_address(U&& return_type) noexcept
    {
        return reinterpret_cast<std::byte*>(std::addressof(return_type));
    }

    template <class U>
    static auto end_address(U&& return_type) noexcept
    {
        return start_address(return_type) + VALUE_BYTES;
    }

    static constexpr void copy(ConstReferenceReturnType source, ReferenceReturnType target) { target = source; }

    template <class Source>
    static constexpr void move(Source&& source, ReferenceReturnType target)
    {
        target = std::move(source);
    }

    template <class Source>
    static constexpr void uninitialized_move(Source&& source, value_type* target)
    {
        detail::construct_at(target, std::move(source));
    }

    static constexpr void swap(ReferenceReturnType lhs, ReferenceReturnType rhs)
    {
        using std::swap;
        swap(lhs, rhs);
    }

    static constexpr void destroy(ReferenceReturnType value) noexcept(std::is_nothrow_destructible_v<value_type>)
    {
        value.~value_type();
    }
};

template <class ValueType>
struct BaseContiguousParameterTraits
{
    template <class T>
    static auto start_address(const cntgs::Span<T>& value) noexcept
    {
        return reinterpret_cast<std::byte*>(value.data());
    }

    template <class T>
    static auto end_address(const cntgs::Span<T>& value) noexcept
    {
        return reinterpret_cast<std::byte*>(value.data() + value.size());
    }

    template <class T, class U>
    static void copy(const cntgs::Span<T>& source, cntgs::Span<U>& target)
    {
        const auto size = std::min(std::size(source), std::size(target));
        std::copy_n(std::begin(source), size, std::begin(target));
    }

    template <class Source, class T>
    static void move(Source&& source, cntgs::Span<T>& target)
    {
        const auto size = std::min(std::size(source), std::size(target));
        std::copy_n(std::make_move_iterator(std::begin(source)), size, std::begin(target));
    }

    template <class Source, class T>
    static void uninitialized_move(Source&& source, cntgs::Span<T>& target)
    {
        std::uninitialized_copy(std::make_move_iterator(std::begin(source)), std::make_move_iterator(std::end(source)),
                                std::begin(target));
    }

    template <class T>
    static void swap(cntgs::Span<T>& lhs, cntgs::Span<T>& rhs)
    {
        const auto size = std::min(std::size(lhs), std::size(rhs));
        std::swap_ranges(std::begin(lhs), std::begin(lhs) + size, std::begin(rhs));
    }

    template <class T>
    static void destroy(const cntgs::Span<T>& value) noexcept(std::is_nothrow_destructible_v<T>)
    {
        std::destroy(std::begin(value), std::end(value));
    }
};

template <class T>
struct ParameterTraits<cntgs::VaryingSize<T>> : ParameterTraits<cntgs::VaryingSize<cntgs::AlignAs<T, 0>>>
{
};

template <class T, std::size_t Alignment>
struct ParameterTraits<cntgs::VaryingSize<cntgs::AlignAs<T, Alignment>>> : BaseContiguousParameterTraits<T>
{
    using Type = cntgs::VaryingSize<T>;
    using ValueReturnType = cntgs::Span<T>;
    using PointerReturnType = cntgs::Span<T>;
    using ReferenceReturnType = cntgs::Span<T>;
    using ConstReferenceReturnType = cntgs::Span<std::add_const_t<T>>;
    using value_type = T;
    using iterator_type = T*;

    static constexpr bool IS_CONTIGUOUS = true;
    static constexpr bool IS_FIXED_SIZE = false;
    static constexpr auto VALUE_BYTES = sizeof(value_type);
    static constexpr auto ALIGNMENT = Alignment;
    static constexpr auto MEMORY_OVERHEAD = sizeof(std::size_t);
    static constexpr auto ALIGNED_SIZE_IN_MEMORY = MEMORY_OVERHEAD + ALIGNMENT;

    template <bool NeedsAlignment>
    static auto load(std::byte* address, std::size_t) noexcept
    {
        const auto size = *reinterpret_cast<std::size_t*>(address);
        address += MEMORY_OVERHEAD;
        const auto first_byte = static_cast<std::byte*>(detail::align_if<NeedsAlignment, ALIGNMENT>(address));
        const auto first = std::launder(reinterpret_cast<iterator_type>(first_byte));
        const auto last = std::launder(reinterpret_cast<iterator_type>(first_byte + size));
        return std::pair{PointerReturnType{first, last}, reinterpret_cast<std::byte*>(last)};
    }

    template <bool NeedsAlignment, class Range>
    CNTGS_RESTRICT_RETURN static std::byte* store(Range&& range, std::byte* CNTGS_RESTRICT address, std::size_t)
    {
        const auto size = reinterpret_cast<std::size_t*>(address);
        address += MEMORY_OVERHEAD;
        address = reinterpret_cast<std::byte*>(detail::align_if<NeedsAlignment, ALIGNMENT>(address));
        auto* new_address = detail::copy_range_ignore_aliasing<value_type>(std::forward<Range>(range), address);
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
    using ValueReturnType = cntgs::Span<T>;
    using PointerReturnType = cntgs::Span<T>;
    using ReferenceReturnType = cntgs::Span<T>;
    using ConstReferenceReturnType = cntgs::Span<std::add_const_t<T>>;
    using value_type = T;
    using iterator_type = T*;

    static constexpr bool IS_CONTIGUOUS = true;
    static constexpr bool IS_FIXED_SIZE = true;
    static constexpr auto VALUE_BYTES = sizeof(value_type);
    static constexpr auto ALIGNMENT = Alignment;
    static constexpr auto MEMORY_OVERHEAD = std::size_t{};

    template <bool NeedsAlignment>
    static auto load(std::byte* address, std::size_t size) noexcept
    {
        const auto first =
            std::launder(static_cast<iterator_type>(detail::align_if<NeedsAlignment, ALIGNMENT>(address)));
        const auto last = first + size;
        return std::pair{PointerReturnType{first, last}, reinterpret_cast<std::byte*>(last)};
    }

    template <bool NeedsAlignment, class RangeOrIterator>
    CNTGS_RESTRICT_RETURN static std::byte* store(RangeOrIterator&& range_or_iterator,
                                                  std::byte* CNTGS_RESTRICT address, std::size_t size)
    {
        address = reinterpret_cast<std::byte*>(detail::align_if<NeedsAlignment, ALIGNMENT>(address));
        return detail::copy_ignore_aliasing<value_type>(std::forward<RangeOrIterator>(range_or_iterator), address,
                                                        size);
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
};

template <class T>
static constexpr auto IS_CONTIGUOUS = detail::ParameterTraits<T>::IS_CONTIGUOUS;
}  // namespace cntgs::detail
