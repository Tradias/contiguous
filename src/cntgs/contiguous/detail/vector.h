#pragma once

#include "cntgs/contiguous/detail/traits.h"
#include "cntgs/contiguous/detail/typeUtils.h"
#include "cntgs/contiguous/span.h"

#include <array>
#include <cstddef>
#include <tuple>
#include <utility>

namespace cntgs::detail
{
template <class T>
constexpr auto dereference(const cntgs::Span<T>& memory) noexcept
{
    return memory;
}

template <class T>
constexpr decltype(auto) dereference(T* memory) noexcept
{
    return *memory;
}

constexpr auto calculate_element_addresses_size(std::size_t element_count) noexcept
{
    return element_count * sizeof(std::byte*);
}

template <class... T, size_t... I>
constexpr auto calculate_fixed_size_indices(detail::TypeList<T...>, std::index_sequence<I...>)
{
    std::array<size_t, sizeof...(T)> fixed_size_indices{};
    std::size_t index = 0;
    (
        [&] {
            if constexpr (detail::ContiguousTraits<T>::IS_FIXED_SIZE)
            {
                std::get<I>(fixed_size_indices) = index;
                ++index;
            }
        }(),
        ...);
    return fixed_size_indices;
}

template <class Parameter, class...>
struct FixedSizeGetter
{
    template <std::size_t I, std::size_t N>
    static constexpr auto get(const std::array<std::size_t, N>&)
    {
        return std::size_t{0};
    }
};

template <class T, class... Types>
struct FixedSizeGetter<cntgs::FixedSize<T>, Types...>
{
    static constexpr auto FIXED_SIZE_INDICES = detail::calculate_fixed_size_indices(
        cntgs::detail::TypeList<Types...>{}, std::make_index_sequence<sizeof...(Types)>{});

    template <std::size_t I, std::size_t N>
    static constexpr auto get(const std::array<std::size_t, N>& fixed_sizes)
    {
        static constexpr auto INDEX = std::get<I>(FIXED_SIZE_INDICES);
        return std::get<INDEX>(fixed_sizes);
    }
};

template <std::size_t N, class... T, size_t... I>
constexpr auto calculate_fixed_size_memory_consumption(const std::array<std::size_t, N>& fixed_sizes,
                                                       detail::TypeList<T...>, std::index_sequence<I...>)
{
    return ((detail::ContiguousTraits<T>::VALUE_BYTES * detail::FixedSizeGetter<T, T...>::get<I>(fixed_sizes)) + ...);
}
}  // namespace cntgs::detail