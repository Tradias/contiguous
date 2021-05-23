#pragma once

#include "cntgs/contiguous/detail/memory.h"
#include "cntgs/contiguous/detail/typeUtils.h"
#include "cntgs/contiguous/parameter.h"
#include "cntgs/contiguous/span.h"

#include <cstddef>
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
    static constexpr auto SIZE_IN_MEMORY = sizeof(value_type);
    static constexpr auto VALUE_BYTES = SIZE_IN_MEMORY;
    static constexpr auto ALIGNMENT = Alignment;
    static constexpr auto MEMORY_OVERHEAD = std::size_t{};

    template <bool NeedsAlignment>
    static auto from_address(std::byte* address, std::size_t) noexcept
    {
        address = reinterpret_cast<std::byte*>(detail::align_if<NeedsAlignment, ALIGNMENT>(address));
        auto result = std::launder(reinterpret_cast<PointerReturnType>(address));
        return std::pair{result, address + detail::MAX_SIZE_T_OF<SIZE_IN_MEMORY, ALIGNMENT>};
    }

    template <bool NeedsAlignment, class Arg>
    static constexpr auto store_contiguously(Arg&& arg, std::byte* address, std::size_t)
    {
        address = reinterpret_cast<std::byte*>(detail::align_if<NeedsAlignment, ALIGNMENT>(address));
        new (address) Type(std::forward<Arg>(arg));
        return address + detail::MAX_SIZE_T_OF<SIZE_IN_MEMORY, ALIGNMENT>;
    }
};

template <class T>
struct ParameterTraits<cntgs::VaryingSize<T>> : ParameterTraits<cntgs::VaryingSize<cntgs::AlignAs<T, 0>>>
{
};

template <class T, std::size_t Alignment>
struct ParameterTraits<cntgs::VaryingSize<cntgs::AlignAs<T, Alignment>>>
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
    static constexpr auto SIZE_IN_MEMORY = std::size_t{};
    static constexpr auto VALUE_BYTES = sizeof(value_type);
    static constexpr auto ALIGNMENT = Alignment;
    static constexpr auto MEMORY_OVERHEAD = sizeof(iterator_type);

    template <bool NeedsAlignment>
    static auto from_address(std::byte* address, std::size_t) noexcept
    {
        const auto last = *reinterpret_cast<iterator_type*>(address);
        address += MEMORY_OVERHEAD;
        const auto first = reinterpret_cast<iterator_type>(detail::align_if<NeedsAlignment, ALIGNMENT>(address));
        return std::pair{PointerReturnType{first, last}, reinterpret_cast<std::byte*>(last)};
    }

    template <bool NeedsAlignment, class Range>
    static auto store_contiguously(const Range& range, std::byte* address, std::size_t)
    {
        const auto start = std::launder(reinterpret_cast<iterator_type*>(address));
        address += MEMORY_OVERHEAD;
        address = reinterpret_cast<std::byte*>(detail::align_if<NeedsAlignment, ALIGNMENT>(address));
        auto new_address = detail::copy_range_ignore_aliasing<value_type>(range, address);
        *start = std::launder(reinterpret_cast<iterator_type>(new_address));
        return new_address;
    }
};

template <class T>
struct ParameterTraits<cntgs::FixedSize<T>> : ParameterTraits<cntgs::FixedSize<cntgs::AlignAs<T, 0>>>
{
};

template <class T, std::size_t Alignment>
struct ParameterTraits<cntgs::FixedSize<cntgs::AlignAs<T, Alignment>>>
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
    static constexpr auto SIZE_IN_MEMORY = std::size_t{};
    static constexpr auto VALUE_BYTES = sizeof(value_type);
    static constexpr auto ALIGNMENT = Alignment;
    static constexpr auto MEMORY_OVERHEAD = std::size_t{};

    template <bool NeedsAlignment>
    static auto from_address(std::byte* address, std::size_t size) noexcept
    {
        const auto first =
            std::launder(reinterpret_cast<iterator_type>(detail::align_if<NeedsAlignment, ALIGNMENT>(address)));
        const auto last = first + size;
        return std::pair{PointerReturnType{first, last}, reinterpret_cast<std::byte*>(last)};
    }

    template <bool NeedsAlignment, class RangeOrIterator>
    static auto store_contiguously(RangeOrIterator&& range_or_iterator, std::byte* address, std::size_t size)
    {
        address = reinterpret_cast<std::byte*>(detail::align_if<NeedsAlignment, ALIGNMENT>(address));
        return detail::copy_ignore_aliasing<value_type>(range_or_iterator, address, size);
    }
};

template <class T>
static constexpr auto IS_CONTIGUOUS = detail::ParameterTraits<T>::IS_CONTIGUOUS;
}  // namespace cntgs::detail