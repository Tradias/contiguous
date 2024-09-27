// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_DETAIL_TYPETRAITS_HPP
#define CNTGS_DETAIL_TYPETRAITS_HPP

#include <cstddef>
#include <type_traits>
#include <utility>
#include <version>

namespace cntgs::detail
{
template <class Derived, class Base>
inline constexpr auto IS_DERIVED_FROM =
    std::is_base_of_v<Base, Derived> && std::is_convertible_v<const volatile Derived*, const volatile Base*>;

#ifdef __cpp_lib_remove_cvref
template <class T>
using RemoveCvrefT = std::remove_cvref_t<T>;
#else
template <class T>
struct RemoveCvref
{
    using Type = std::remove_cv_t<std::remove_reference_t<T>>;
};

template <class T>
using RemoveCvrefT = typename RemoveCvref<T>::Type;
#endif

template <bool B>
struct EqualSizeof
{
    template <class, class>
    static constexpr bool VALUE = false;
};

template <>
struct EqualSizeof<false>
{
    template <class T, class U>
    static constexpr bool VALUE = sizeof(T) == sizeof(U);
};

template <class T, class U>
inline constexpr bool EQUAL_SIZEOF =
    EqualSizeof<(std::is_same_v<void, T> || std::is_same_v<void, U>)>::template VALUE<T, U>;

template <class T>
inline constexpr bool IS_BYTE = std::is_same_v<char, T> || std::is_same_v<signed char, T> ||
                                std::is_same_v<unsigned char, T> || std::is_same_v<std::byte, T>
#ifdef __cpp_char8_t
                                || std::is_same_v<char8_t, T>
#endif
    ;

namespace has_adl_swap_detail
{
void swap();  // undefined (deliberate shadowing)

template <class, class = void>
inline constexpr bool HAS_ADL_SWAP = false;

template <class T>
inline constexpr bool HAS_ADL_SWAP<T, std::void_t<decltype(swap(std::declval<T&>(), std::declval<T&>()))>> = true;
}  // namespace has_adl_swap_detail

// Implementation taken from MSVC _Is_trivially_swappable
template <class T>
inline constexpr bool IS_TRIVIALLY_SWAPPABLE =
    std::is_trivially_destructible_v<T> && std::is_trivially_move_constructible_v<T> &&
    std::is_trivially_move_assignable_v<T> && !has_adl_swap_detail::HAS_ADL_SWAP<T>;

template <class T>
struct IsTriviallySwappable : std::bool_constant<IS_TRIVIALLY_SWAPPABLE<T>>
{
};

template <bool B>
struct Conditional
{
    template <class T, class>
    using Type = T;
};

template <>
struct Conditional<false>
{
    template <class, class U>
    using Type = U;
};

template <bool B, class T, class U>
using ConditionalT = typename detail::Conditional<B>::template Type<T, U>;

template <class Lhs, class Rhs, class = void>
inline constexpr bool ARE_EQUALITY_COMPARABLE = false;

template <class Lhs, class Rhs>
inline constexpr bool
    ARE_EQUALITY_COMPARABLE<Lhs, Rhs,
                            std::void_t<decltype(std::declval<const Lhs&>() == std::declval<const Rhs&>()),
                                        decltype(std::declval<const Rhs&>() == std::declval<const Lhs&>())>> = true;

template <class T>
inline constexpr bool IS_NOTRHOW_EQUALITY_COMPARABLE = noexcept(std::declval<const T&>() == std::declval<const T&>());

template <class T>
inline constexpr bool IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE =
    noexcept(std::declval<const T&>() < std::declval<const T&>());

template <class T, class U>
inline constexpr bool MEMCPY_COMPATIBLE =
    detail::EQUAL_SIZEOF<T, U> && std::is_trivially_copyable_v<T> && std::is_trivially_copyable_v<U> &&
    std::is_floating_point_v<T> == std::is_floating_point_v<U>;

// Implementation taken from MSVC _Can_memcmp_elements
template <class T, class U = T,
          bool = (detail::EQUAL_SIZEOF<T, U> && std::is_integral_v<T> && !std::is_volatile_v<T> &&
                  std::is_integral_v<U> && !std::is_volatile_v<U>)>
inline constexpr bool EQUALITY_MEMCMP_COMPATIBLE = std::is_same_v<T, bool> || std::is_same_v<U, bool> || T(-1) == U(-1);

template <>
inline constexpr bool EQUALITY_MEMCMP_COMPATIBLE<std::byte, std::byte, false> = true;

template <class T, class U>
inline constexpr bool EQUALITY_MEMCMP_COMPATIBLE<T*, U*, false> =
    std::is_same_v<std::remove_cv_t<T>, std::remove_cv_t<U>>;

template <class T, class U>
inline constexpr bool EQUALITY_MEMCMP_COMPATIBLE<T, U, false> = false;

template <class T>
struct EqualityMemcmpCompatible
    : std::bool_constant<detail::EQUALITY_MEMCMP_COMPATIBLE<std::remove_const_t<T>, std::remove_const_t<T>>>
{
};

// Implementation taken from MSVC+GCC _Lex_compare_check_element_types_helper and __is_memcmp_ordered
template <class T, bool = detail::IS_BYTE<T>>
inline constexpr bool LEXICOGRAPHICAL_MEMCMP_COMPATIBLE = T(-1) > T(1) && !std::is_volatile_v<T>;

template <>
inline constexpr bool LEXICOGRAPHICAL_MEMCMP_COMPATIBLE<std::byte, false> = true;

template <class T>
inline constexpr bool LEXICOGRAPHICAL_MEMCMP_COMPATIBLE<T, false> = false;

template <class T>
struct LexicographicalMemcmpCompatible : std::bool_constant<LEXICOGRAPHICAL_MEMCMP_COMPATIBLE<T>>
{
};
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_TYPETRAITS_HPP
