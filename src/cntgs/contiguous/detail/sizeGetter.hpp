#ifndef CNTGS_DETAIL_SIZEGETTER_HPP
#define CNTGS_DETAIL_SIZEGETTER_HPP

#include "cntgs/contiguous/detail/array.hpp"
#include "cntgs/contiguous/detail/parameterTraits.hpp"
#include "cntgs/contiguous/detail/parameterType.hpp"
#include "cntgs/contiguous/detail/typeUtils.hpp"

#include <array>
#include <cstddef>
#include <tuple>

namespace cntgs::detail
{
template <class... Types>
struct FixedSizeGetter
{
    template <std::size_t... I>
    static constexpr auto calculate_fixed_size_indices(std::index_sequence<I...>) noexcept
    {
        std::array<std::size_t, sizeof...(Types)> fixed_size_indices{};
        [[maybe_unused]] std::size_t index{};
        (
            [&]
            {
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

    template <class Type, std::size_t I, std::size_t N>
    static constexpr auto get(const detail::Array<std::size_t, N>& fixed_sizes) noexcept
    {
        return detail::get<std::get<I>(FIXED_SIZE_INDICES)>(fixed_sizes);
    }
};

struct ContiguousReferenceSizeGetter
{
    template <class Type, std::size_t I, class... U>
    static constexpr auto get([[maybe_unused]] const std::tuple<U...>& tuple) noexcept
    {
        if constexpr (detail::ParameterType::FIXED_SIZE == detail::ParameterTraits<Type>::TYPE)
        {
            return std::get<I>(tuple).size();
        }
        else
        {
            return std::size_t{};
        }
    }
};
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_SIZEGETTER_HPP
