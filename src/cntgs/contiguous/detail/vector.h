#pragma once

#include "cntgs/contiguous/detail/parameterTraits.h"
#include "cntgs/contiguous/detail/typeUtils.h"
#include "cntgs/contiguous/span.h"

#include <array>
#include <cstddef>

namespace cntgs::detail
{
template <class T>
constexpr auto dereference(const cntgs::Span<T>& memory) noexcept
{
    return memory;
}

template <class T>
constexpr decltype(auto) dereference(T&& memory) noexcept
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
            if constexpr (detail::ParameterTraits<T>::IS_FIXED_SIZE)
            {
                std::get<I>(fixed_size_indices) = index;
                ++index;
            }
        }(),
        ...);
    return fixed_size_indices;
}
}  // namespace cntgs::detail