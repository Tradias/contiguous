#pragma once

#include "cntgs/contiguous/detail/parameterTraits.h"
#include "cntgs/contiguous/detail/parameterType.h"
#include "cntgs/contiguous/detail/typeUtils.h"

#include <array>
#include <cstddef>
#include <tuple>

namespace cntgs::detail
{
template <class T, class U>
struct FixedSizeGetter
{
    template <std::size_t I, std::size_t N>
    static constexpr auto get(const std::array<std::size_t, N>&) noexcept
    {
        return std::size_t{};
    }
};

template <class T, class... Types>
struct FixedSizeGetter<cntgs::FixedSize<T>, detail::TypeList<Types...>>
{
    template <std::size_t... I>
    static constexpr auto calculate_fixed_size_indices(std::index_sequence<I...>) noexcept
    {
        std::array<std::size_t, sizeof...(Types)> fixed_size_indices{};
        [[maybe_unused]] std::size_t index = 0;
        (
            [&] {
                if constexpr (detail::ParameterTraits<Types>::TYPE == detail::ParameterType::FIXED_SIZE)
                {
                    std::get<I>(fixed_size_indices) = index;
                    ++index;
                }
            }(),
            ...);
        return fixed_size_indices;
    }

    static constexpr auto FIXED_SIZE_INDICES =
        calculate_fixed_size_indices(std::make_index_sequence<sizeof...(Types)>{});

    template <std::size_t I, std::size_t N>
    static constexpr auto get(const std::array<std::size_t, N>& fixed_sizes) noexcept
    {
        return std::get<std::get<I>(FIXED_SIZE_INDICES)>(fixed_sizes);
    }
};

template <class T>
struct ContiguousReturnTypeSizeGetter
{
    template <std::size_t I, class... U>
    static constexpr auto get(const std::tuple<U...>&) noexcept
    {
        return std::size_t{};
    }
};

template <class T>
struct ContiguousReturnTypeSizeGetter<cntgs::FixedSize<T>>
{
    template <std::size_t I, class... U>
    static constexpr auto get(const std::tuple<U...>& tuple) noexcept
    {
        return std::get<I>(tuple).size();
    }
};
}  // namespace cntgs::detail