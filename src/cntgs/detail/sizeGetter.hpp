// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_DETAIL_SIZEGETTER_HPP
#define CNTGS_DETAIL_SIZEGETTER_HPP

#include "cntgs/detail/array.hpp"
#include "cntgs/detail/forward.hpp"
#include "cntgs/detail/parameterTraits.hpp"
#include "cntgs/detail/parameterType.hpp"
#include "cntgs/detail/reference.hpp"
#include "cntgs/detail/typeUtils.hpp"

#include <array>
#include <cstddef>
#include <tuple>

namespace cntgs::detail
{
template <class... Types>
class FixedSizeGetter
{
  private:
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

  public:
    template <class Type>
    static constexpr auto CAN_PROVIDE_SIZE = detail::ParameterType::FIXED_SIZE == detail::ParameterTraits<Type>::TYPE;

    template <class, std::size_t I, std::size_t N>
    static constexpr auto get(const detail::Array<std::size_t, N>& fixed_sizes) noexcept
    {
        return detail::get<std::get<I>(FIXED_SIZE_INDICES)>(fixed_sizes);
    }
};

class ContiguousReferenceSizeGetter
{
  public:
    template <class Type>
    static constexpr auto CAN_PROVIDE_SIZE = detail::ParameterType::PLAIN != detail::ParameterTraits<Type>::TYPE;

    template <class Type, std::size_t I, bool IsConst, class... Types>
    static constexpr auto get([[maybe_unused]] const cntgs::BasicContiguousReference<IsConst, Types...>& tuple) noexcept
    {
        if constexpr (detail::ParameterType::PLAIN != detail::ParameterTraits<Type>::TYPE)
        {
            return cntgs::get<I>(tuple).size();
        }
        else
        {
            return std::size_t{};
        }
    }
};
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_SIZEGETTER_HPP
