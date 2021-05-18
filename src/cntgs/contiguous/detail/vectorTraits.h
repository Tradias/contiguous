#pragma once

#include "cntgs/contiguous/detail/tuple.h"
#include "cntgs/contiguous/detail/vector.h"

#include <tuple>
#include <utility>

namespace cntgs
{
template <class... Types>
class ContiguousVector;

namespace detail
{
template <class T, class U>
struct FixedSizeGetterImplementation
{
    template <std::size_t I, std::size_t N>
    static constexpr auto get(const std::array<std::size_t, N>&) noexcept
    {
        return std::size_t{0};
    }
};

template <class T, class... Types>
struct FixedSizeGetterImplementation<cntgs::FixedSize<T>, detail::TypeList<Types...>>
{
    static constexpr auto FIXED_SIZE_INDICES = detail::calculate_fixed_size_indices(
        cntgs::detail::TypeList<Types...>{}, std::make_index_sequence<sizeof...(Types)>{});

    template <std::size_t I, std::size_t N>
    static constexpr auto get(const std::array<std::size_t, N>& fixed_sizes) noexcept
    {
        static constexpr auto INDEX = std::get<I>(FIXED_SIZE_INDICES);
        return std::get<INDEX>(fixed_sizes);
    }
};

template <class T>
struct ContiguousVectorTraits
{
};

template <class... Types>
struct ContiguousVectorTraits<cntgs::ContiguousVector<Types...>>
{
    using Tuple = std::tuple<Types...>;
    using ValueReturnType = detail::ToContiguousTupleOfValueReturnType<Tuple>;
    using ReferenceReturnType = detail::ToContiguousTupleOfReferenceReturnType<Tuple>;
    using ConstReferenceReturnType = detail::ToContiguousTupleOfConstReferenceReturnType<Tuple>;
    using PointerReturnType = detail::ToContiguousTupleOfPointerReturnType<Tuple>;

    template <class T>
    using FixedSizeGetter = detail::FixedSizeGetterImplementation<T, detail::TypeList<Types...>>;

    template <std::size_t I>
    using TypeAt = std::tuple_element_t<I, Tuple>;

    static constexpr auto TYPE_COUNT = sizeof...(Types);
    static constexpr auto SIZE_IN_MEMORY = (detail::ParameterTraits<Types>::SIZE_IN_MEMORY + ...);
    static constexpr auto CONTIGUOUS_COUNT = (detail::ParameterTraits<Types>::IS_CONTIGUOUS + ...);
    static constexpr auto CONTIGUOUS_FIXED_SIZE_COUNT = (detail::ParameterTraits<Types>::IS_FIXED_SIZE + ...);
};
}  // namespace detail
}  // namespace cntgs