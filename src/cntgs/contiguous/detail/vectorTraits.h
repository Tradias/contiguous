#pragma once

#include "cntgs/contiguous/detail/forward.h"
#include "cntgs/contiguous/detail/math.h"
#include "cntgs/contiguous/detail/tuple.h"
#include "cntgs/contiguous/detail/vector.h"

#include <tuple>
#include <utility>

namespace cntgs::detail
{
template <class T, class U>
struct FixedSizeGetterImplementation
{
    template <std::size_t I, std::size_t N>
    static constexpr auto get(const std::array<std::size_t, N>&) noexcept
    {
        return std::size_t{};
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
        constexpr auto INDEX = std::get<I>(FIXED_SIZE_INDICES);
        return std::get<INDEX>(fixed_sizes);
    }
};

struct BaseVectorTraits
{
    using DifferenceType = std::ptrdiff_t;
    using SizeType = std::size_t;
};

template <class T>
struct VectorTraits : BaseVectorTraits
{
};

template <class... Types>
struct VectorTraits<cntgs::ContiguousVector<Types...>> : BaseVectorTraits
{
    using Tuple = std::tuple<Types...>;
    using ValueReturnType = detail::ToContiguousTupleOfValueReturnType<Tuple>;
    using ReferenceReturnType = detail::ToContiguousTupleOfReferenceReturnType<Tuple>;
    using ConstReferenceReturnType = detail::ToContiguousTupleOfConstReferenceReturnType<Tuple>;
    using PointerReturnType = detail::ToContiguousTupleOfPointerReturnType<Tuple>;
    using StorageType = detail::MaybeOwnedPtr<std::byte[]>;

    template <class T>
    using FixedSizeGetter = detail::FixedSizeGetterImplementation<T, detail::TypeList<Types...>>;

    static constexpr auto TYPE_COUNT = sizeof...(Types);
    static constexpr auto SIZE_IN_MEMORY = (SizeType{} + ... + detail::ParameterTraits<Types>::SIZE_IN_MEMORY);
    static constexpr auto CONTIGUOUS_COUNT = (SizeType{} + ... + detail::ParameterTraits<Types>::IS_CONTIGUOUS);
    static constexpr auto CONTIGUOUS_FIXED_SIZE_COUNT =
        (SizeType{} + ... + detail::ParameterTraits<Types>::IS_FIXED_SIZE);
    static constexpr auto MAX_ALIGNMENT = detail::max_size_t_of<detail::ParameterTraits<Types>::ALIGNMENT...>();

    static constexpr auto IS_TRIVIALLY_DESTRUCTIBLE =
        (std::is_trivially_destructible_v<typename detail::ParameterTraits<Types>::value_type> && ...);
    static constexpr bool IS_MIXED =
        CONTIGUOUS_FIXED_SIZE_COUNT != 0 && CONTIGUOUS_FIXED_SIZE_COUNT != CONTIGUOUS_COUNT;
    static constexpr bool IS_ALL_FIXED_SIZE =
        CONTIGUOUS_FIXED_SIZE_COUNT != 0 && CONTIGUOUS_FIXED_SIZE_COUNT == CONTIGUOUS_COUNT;
    static constexpr bool IS_ALL_VARYING_SIZE = CONTIGUOUS_FIXED_SIZE_COUNT == 0 && CONTIGUOUS_COUNT != 0;
    static constexpr bool IS_NONE_SPECIAL = CONTIGUOUS_COUNT == 0;

#ifdef CNTGS_MAX_FIXED_SIZE_VECTOR_PARAMETER
    static constexpr auto MAX_FIXED_SIZE_VECTOR_PARAMETER = CNTGS_MAX_FIXED_SIZE_VECTOR_PARAMETER;
#else
    static constexpr auto MAX_FIXED_SIZE_VECTOR_PARAMETER = 32;
#endif

    static_assert(MAX_FIXED_SIZE_VECTOR_PARAMETER > CONTIGUOUS_FIXED_SIZE_COUNT,
                  "Maximum number of FixedSize vector parameter exceeded. Define CNTGS_MAX_FIXED_SIZE_VECTOR_PARAMETER "
                  "to a higher limit.");
};

template <class... Types>
using ContiguousVectorTraits = VectorTraits<cntgs::ContiguousVector<Types...>>;
}  // namespace cntgs::detail