#ifndef CNTGS_DETAIL_PARAMETERLISTTRAITS_HPP
#define CNTGS_DETAIL_PARAMETERLISTTRAITS_HPP

#include "cntgs/contiguous/detail/array.hpp"
#include "cntgs/contiguous/detail/parameterTraits.hpp"
#include "cntgs/contiguous/detail/parameterType.hpp"
#include "cntgs/contiguous/detail/typeUtils.hpp"

#include <array>
#include <cstddef>
#include <tuple>
#include <type_traits>

namespace cntgs::detail
{
#ifdef CNTGS_MAX_FIXED_SIZE_VECTOR_PARAMETER
inline constexpr auto MAX_FIXED_SIZE_VECTOR_PARAMETER = CNTGS_MAX_FIXED_SIZE_VECTOR_PARAMETER;
#else
inline constexpr auto MAX_FIXED_SIZE_VECTOR_PARAMETER = 15;
#endif

template <class... Types>
struct ParameterListTraits
{
    template <std::size_t K>
    using ParameterTraitsAt = detail::ParameterTraits<std::tuple_element_t<K, std::tuple<Types...>>>;

    static constexpr auto IS_NOTHROW_COPY_CONSTRUCTIBLE =
        (std::is_nothrow_copy_constructible_v<typename detail::ParameterTraits<Types>::ValueType> && ...);
    static constexpr auto IS_NOTHROW_MOVE_CONSTRUCTIBLE =
        (std::is_nothrow_move_constructible_v<typename detail::ParameterTraits<Types>::ValueType> && ...);
    static constexpr auto IS_NOTHROW_COPY_ASSIGNABLE =
        (std::is_nothrow_copy_assignable_v<typename detail::ParameterTraits<Types>::ValueType> && ...);
    static constexpr auto IS_NOTHROW_MOVE_ASSIGNABLE =
        (std::is_nothrow_move_assignable_v<typename detail::ParameterTraits<Types>::ValueType> && ...);
    static constexpr auto IS_NOTHROW_SWAPPABLE =
        (std::is_nothrow_swappable_v<typename detail::ParameterTraits<Types>::ValueType> && ...);
    static constexpr auto IS_NOTHROW_EQUALITY_COMPARABLE =
        (detail::IS_NOTRHOW_EQUALITY_COMPARABLE<typename detail::ParameterTraits<Types>::ValueType> && ...);
    static constexpr auto IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE =
        (detail::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE<typename detail::ParameterTraits<Types>::ValueType> && ...);

    static constexpr auto IS_TRIVIALLY_DESTRUCTIBLE =
        (std::is_trivially_destructible_v<typename detail::ParameterTraits<Types>::ValueType> && ...);
    static constexpr auto IS_TRIVIALLY_COPY_CONSTRUCTIBLE =
        (std::is_trivially_copy_constructible_v<typename detail::ParameterTraits<Types>::ValueType> && ...);
    static constexpr auto IS_TRIVIALLY_MOVE_CONSTRUCTIBLE =
        (std::is_trivially_move_constructible_v<typename detail::ParameterTraits<Types>::ValueType> && ...);
    static constexpr auto IS_TRIVIALLY_COPY_ASSIGNABLE =
        (std::is_trivially_copy_assignable_v<typename detail::ParameterTraits<Types>::ValueType> && ...);
    static constexpr auto IS_TRIVIALLY_MOVE_ASSIGNABLE =
        (std::is_trivially_move_assignable_v<typename detail::ParameterTraits<Types>::ValueType> && ...);
    static constexpr auto IS_TRIVIALLY_SWAPPABLE =
        (detail::IS_TRIVIALLY_SWAPPABLE<typename detail::ParameterTraits<Types>::ValueType> && ...);
    static constexpr auto IS_EQUALITY_MEMCMPABLE =
        (detail::EQUALITY_MEMCMP_COMPATIBLE<typename detail::ParameterTraits<Types>::ValueType> && ...);
    static constexpr auto IS_LEXICOGRAPHICAL_MEMCMPABLE =
        (detail::LEXICOGRAPHICAL_MEMCMP_COMPATIBLE<typename detail::ParameterTraits<Types>::ValueType> && ...);

    static constexpr auto CONTIGUOUS_COUNT =
        (std::size_t{} + ... + (detail::ParameterTraits<Types>::TYPE != detail::ParameterType::PLAIN));
    static constexpr auto CONTIGUOUS_FIXED_SIZE_COUNT =
        (std::size_t{} + ... + (detail::ParameterTraits<Types>::TYPE == detail::ParameterType::FIXED_SIZE));

    static constexpr bool IS_MIXED =
        CONTIGUOUS_FIXED_SIZE_COUNT != 0 && CONTIGUOUS_FIXED_SIZE_COUNT != CONTIGUOUS_COUNT;
    static constexpr bool IS_ALL_FIXED_SIZE =
        CONTIGUOUS_FIXED_SIZE_COUNT != 0 && CONTIGUOUS_FIXED_SIZE_COUNT == CONTIGUOUS_COUNT;
    static constexpr bool IS_ALL_VARYING_SIZE = CONTIGUOUS_FIXED_SIZE_COUNT == 0 && CONTIGUOUS_COUNT != 0;
    static constexpr bool IS_ALL_PLAIN = CONTIGUOUS_COUNT == 0;
    static constexpr bool IS_FIXED_SIZE_OR_PLAIN = IS_ALL_FIXED_SIZE || IS_ALL_PLAIN;

    using FixedSizes = std::array<std::size_t, CONTIGUOUS_FIXED_SIZE_COUNT>;
    using FixedSizesArray = detail::Array<std::size_t, CONTIGUOUS_FIXED_SIZE_COUNT>;

    static_assert(detail::MAX_FIXED_SIZE_VECTOR_PARAMETER > CONTIGUOUS_FIXED_SIZE_COUNT,
                  "Maximum number of FixedSize vector parameter exceeded. Define CNTGS_MAX_FIXED_SIZE_VECTOR_PARAMETER "
                  "to a higher limit.");

    static constexpr auto make_index_sequence() noexcept { return std::make_index_sequence<sizeof...(Types)>{}; }
};
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_PARAMETERLISTTRAITS_HPP
