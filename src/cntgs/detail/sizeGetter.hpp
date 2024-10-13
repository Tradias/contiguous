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

#include <array>
#include <cstddef>

namespace cntgs::detail
{
template <class... Parameter>
class SizeGetter
{
  private:
    template <std::size_t... I>
    static constexpr auto calculate_fixed_size_indices(std::index_sequence<I...>) noexcept
    {
        std::array<std::size_t, sizeof...(Parameter)> fixed_size_indices{};
        [[maybe_unused]] std::size_t index{};
        (
            [&]
            {
                if constexpr (detail::ParameterTraits<Parameter>::TYPE == detail::ParameterType::FIXED_SIZE)
                {
                    std::get<I>(fixed_size_indices) = index;
                    ++index;
                }
            }(),
            ...);
        return fixed_size_indices;
    }

    static constexpr auto FIXED_SIZE_INDICES =
        calculate_fixed_size_indices(std::make_index_sequence<sizeof...(Parameter)>{});

  public:
    template <std::size_t I, std::size_t N>
    static constexpr std::size_t get_fixed_size(const detail::Array<std::size_t, N>& fixed_sizes) noexcept
    {
        return detail::get<std::get<I>(FIXED_SIZE_INDICES)>(fixed_sizes);
    }

    template <class Type, std::size_t I, std::size_t N, class ContiguousPointer>
    static constexpr decltype(auto) get([[maybe_unused]] const detail::Array<std::size_t, N>& fixed_sizes,
                                        [[maybe_unused]] const ContiguousPointer& pointer) noexcept
    {
        if constexpr (detail::ParameterType::FIXED_SIZE == detail::ParameterTraits<Type>::TYPE)
        {
            return get_fixed_size<I>(fixed_sizes);
        }
        else if constexpr (detail::ParameterType::VARYING_SIZE == detail::ParameterTraits<Type>::TYPE)
        {
            return *std::get<I - 1>(pointer);
        }
        else
        {
            return std::size_t{};
        }
    }
};

class ContiguousReferenceSizeGetter
{
  public:
    template <class Type, std::size_t I, bool IsConst, class... Parameter, class T>
    static constexpr auto get([[maybe_unused]] const cntgs::BasicContiguousReference<IsConst, Parameter...>& tuple,
                              const T&) noexcept
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
