#pragma once

#include "cntgs/contiguous/detail/forward.h"
#include "cntgs/contiguous/detail/math.h"
#include "cntgs/contiguous/detail/tuple.h"
#include "cntgs/contiguous/detail/vector.h"
#include "cntgs/contiguous/tuple.h"

#include <tuple>
#include <utility>

namespace cntgs::detail
{
#ifdef CNTGS_MAX_FIXED_SIZE_VECTOR_PARAMETER
static constexpr auto MAX_FIXED_SIZE_VECTOR_PARAMETER = CNTGS_MAX_FIXED_SIZE_VECTOR_PARAMETER;
#else
static constexpr auto MAX_FIXED_SIZE_VECTOR_PARAMETER = 15;
#endif

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
        detail::TypeList<Types...>{}, std::make_index_sequence<sizeof...(Types)>{});

    template <std::size_t I, std::size_t N>
    static constexpr auto get(const std::array<std::size_t, N>& fixed_sizes) noexcept
    {
        constexpr auto INDEX = std::get<I>(FIXED_SIZE_INDICES);
        return std::get<INDEX>(fixed_sizes);
    }
};

template <class T>
struct VectorTraits
{
};

template <class... Types>
struct VectorTraits<cntgs::ContiguousVector<Types...>>
{
    using ValueReturnType = cntgs::ContiguousElement<Types...>;
    using ReferenceReturnType = cntgs::ContiguousTuple<detail::ContiguousTupleQualifier::REFERENCE, Types...>;
    using ConstReferenceReturnType =
        cntgs::ContiguousTuple<detail::ContiguousTupleQualifier::CONST_REFERENCE, Types...>;
    using PointerReturnType = detail::ToContiguousTupleOfPointerReturnType<std::tuple<Types...>>;
    using StorageType = detail::MaybeOwnedPtr<std::byte[]>;

    template <class T>
    using FixedSizeGetter = detail::FixedSizeGetterImplementation<T, detail::TypeList<Types...>>;

    static constexpr auto TYPE_COUNT = sizeof...(Types);
    static constexpr auto CONTIGUOUS_COUNT =
        (std::size_t{} + ... + (detail::ParameterTraits<Types>::TYPE != detail::ParameterType::PLAIN));
    static constexpr auto CONTIGUOUS_FIXED_SIZE_COUNT =
        (std::size_t{} + ... + (detail::ParameterTraits<Types>::TYPE == detail::ParameterType::FIXED_SIZE));
    static constexpr auto MAX_ALIGNMENT = detail::MAX_SIZE_T_OF<detail::ParameterTraits<Types>::ALIGNMENT...>;

    static constexpr auto IS_NOTHROW_DESTRUCTIBLE =
        (std::is_nothrow_destructible_v<typename detail::ParameterTraits<Types>::ValueType> && ...);
    static constexpr auto IS_TRIVIALLY_DESTRUCTIBLE =
        (std::is_trivially_destructible_v<typename detail::ParameterTraits<Types>::ValueType> && ...);
    static constexpr auto IS_TRIVIALLY_MOVE_CONSTRUCTIBLE =
        (std::is_trivially_move_constructible_v<typename detail::ParameterTraits<Types>::ValueType> && ...);

    static constexpr bool IS_MIXED =
        CONTIGUOUS_FIXED_SIZE_COUNT != 0 && CONTIGUOUS_FIXED_SIZE_COUNT != CONTIGUOUS_COUNT;
    static constexpr bool IS_ALL_FIXED_SIZE =
        CONTIGUOUS_FIXED_SIZE_COUNT != 0 && CONTIGUOUS_FIXED_SIZE_COUNT == CONTIGUOUS_COUNT;
    static constexpr bool IS_ALL_VARYING_SIZE = CONTIGUOUS_FIXED_SIZE_COUNT == 0 && CONTIGUOUS_COUNT != 0;
    static constexpr bool IS_NONE_SPECIAL = CONTIGUOUS_COUNT == 0;

    static_assert(detail::MAX_FIXED_SIZE_VECTOR_PARAMETER > CONTIGUOUS_FIXED_SIZE_COUNT,
                  "Maximum number of FixedSize vector parameter exceeded. Define CNTGS_MAX_FIXED_SIZE_VECTOR_PARAMETER "
                  "to a higher limit.");
};

template <class... Types>
using ContiguousVectorTraits = VectorTraits<cntgs::ContiguousVector<Types...>>;
}  // namespace cntgs::detail