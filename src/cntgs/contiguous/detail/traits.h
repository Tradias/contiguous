#pragma once

#include "cntgs/contiguous/detail/memory.h"
#include "cntgs/contiguous/parameter.h"
#include "cntgs/contiguous/span.h"

#include <cstddef>
#include <type_traits>

namespace cntgs::detail
{
template <class T>
struct ContiguousTraits
{
    using Type = T;
    using ReturnType = std::add_pointer_t<T>;
    using ConstReturnType = std::add_const_t<std::add_pointer_t<T>>;
    using value_type = T;
    using iterator_type = ReturnType;

    static constexpr bool IS_CONTIGUOUS = false;
    static constexpr bool IS_FIXED_SIZE = false;
    static constexpr auto SIZE_IN_MEMORY = sizeof(value_type);
    static constexpr auto VALUE_BYTES = SIZE_IN_MEMORY;

    static auto from_address(std::byte* address, std::size_t) noexcept
    {
        auto result = reinterpret_cast<ReturnType>(address);
        return std::pair{result, address + SIZE_IN_MEMORY};
    }

    template <class Arg>
    static constexpr auto store_contiguously(Arg&& arg, std::byte* address, std::size_t)
    {
        using RawArg = std::remove_const_t<std::remove_reference_t<Arg>>;
        new (address) RawArg(std::forward<Arg>(arg));
        return address + sizeof(RawArg);
    }
};

template <class T>
struct ContiguousTraits<cntgs::VaryingSize<T>>
{
    using Type = cntgs::VaryingSize<T>;
    using ReturnType = cntgs::Span<T>;
    using ConstReturnType = cntgs::Span<std::add_const_t<T>>;
    using value_type = T;
    using iterator_type = T*;

    static constexpr bool IS_CONTIGUOUS = true;
    static constexpr bool IS_FIXED_SIZE = false;
    static constexpr auto SIZE_IN_MEMORY = sizeof(iterator_type);
    static constexpr auto VALUE_BYTES = sizeof(value_type);

    static auto from_address(std::byte* address, std::size_t) noexcept
    {
        ReturnType result;
        result.last = *reinterpret_cast<iterator_type*>(address);
        address += SIZE_IN_MEMORY;
        result.first = reinterpret_cast<iterator_type>(address);
        return std::pair{result, reinterpret_cast<std::byte*>(result.last)};
    }

    template <class Range>
    static auto store_contiguously(const Range& range, std::byte* address, std::size_t)
    {
        const auto start = reinterpret_cast<iterator_type*>(address);
        address += SIZE_IN_MEMORY;
        auto new_address = detail::copy_range_ignore_aliasing(range, address);
        *start = reinterpret_cast<iterator_type>(new_address);
        return new_address;
    }
};

template <class T>
struct ContiguousTraits<cntgs::FixedSize<T>>
{
    using Type = cntgs::FixedSize<T>;
    using ReturnType = cntgs::Span<T>;
    using ConstReturnType = cntgs::Span<std::add_const_t<T>>;
    using value_type = T;
    using iterator_type = T*;

    static constexpr bool IS_CONTIGUOUS = true;
    static constexpr bool IS_FIXED_SIZE = true;
    static constexpr auto SIZE_IN_MEMORY = std::size_t(0);
    static constexpr auto VALUE_BYTES = sizeof(value_type);

    static auto from_address(std::byte* address, std::size_t size) noexcept
    {
        ReturnType result;
        result.first = reinterpret_cast<iterator_type>(address);
        result.last = result.first + size;
        return std::pair{result, address + size * VALUE_BYTES};
    }

    template <class RangeOrIterator>
    static auto store_contiguously(RangeOrIterator&& range_or_iterator, std::byte* address, std::size_t size)
    {
        return detail::copy_ignore_aliasing(range_or_iterator, address, size);
    }
};

template <class T>
static constexpr auto IS_CONTIGUOUS = detail::ContiguousTraits<T>::IS_CONTIGUOUS;
}  // namespace cntgs::detail