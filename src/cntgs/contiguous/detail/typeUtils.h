#pragma once

#include <type_traits>
#include <utility>
#include <version>

#ifdef __cpp_lib_concepts
#include <concepts>
#endif

namespace cntgs::detail
{
template <class...>
struct TypeList
{
};

#ifdef __cpp_lib_concepts
template <class Derived, class Base>
concept DerivedFrom = std::derived_from<Derived, Base>;
#else
template <class Derived, class Base>
inline constexpr auto DerivedFrom =
    std::is_base_of_v<Base, Derived>&& std::is_convertible_v<const volatile Derived*, const volatile Base*>;
#endif

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
    static constexpr auto VALUE = false;
};

template <>
struct EqualSizeof<false>
{
    template <class T, class U>
    static constexpr auto VALUE = sizeof(T) == sizeof(U);
};

template <class T, class U>
inline constexpr auto EQUAL_SIZEOF =
    EqualSizeof<(std::is_same_v<void, T> || std::is_same_v<void, U>)>::template VALUE<T, U>;

template <class...>
inline constexpr auto FALSE_V = false;

namespace has_adl_swap_detail
{
void swap();  // undefined (deliberate shadowing)

template <class, class = void>
struct HasADLSwap : std::false_type
{
};
template <class T>
struct HasADLSwap<T, std::void_t<decltype(swap(std::declval<T&>(), std::declval<T&>()))>> : std::true_type
{
};
}  // namespace has_adl_swap_detail
using has_adl_swap_detail::HasADLSwap;

// Implementation taken from MSVC _Is_trivially_swappable
template <class T>
using IsTriviallySwappable =
    std::conjunction<std::is_trivially_destructible<T>, std::is_trivially_move_constructible<T>,
                     std::is_trivially_move_assignable<T>, std::negation<HasADLSwap<T>>>;

template <class T>
inline constexpr auto IS_TRIVIALLY_SWAPPABLE = detail::IsTriviallySwappable<T>::value;

template <bool B>
struct Conditional
{
    template <class T, class U>
    using Type = T;
};

template <>
struct Conditional<false>
{
    template <class T, class U>
    using Type = U;
};

template <bool B, class T, class U>
using ConditionalT = typename detail::Conditional<B>::template Type<T, U>;

template <class T, class U>
inline constexpr auto MEMCPY_COMPATIBLE =
    detail::EQUAL_SIZEOF<T, U>&& std::is_trivially_copyable_v<T>&& std::is_trivially_copyable_v<U>&&
        std::is_floating_point_v<T> == std::is_floating_point_v<U>;

// Implementation taken from MSVC _Can_memcmp_elements
template <class T, class U = T,
          bool = (detail::EQUAL_SIZEOF<T, U> && std::is_integral_v<T> && !std::is_volatile_v<T> &&
                  std::is_integral_v<U> && !std::is_volatile_v<U>)>
inline constexpr auto EQUALITY_MEMCMP_COMPATIBLE = std::is_same_v<T, bool> || std::is_same_v<U, bool> ||
                                                   static_cast<T>(-1) == static_cast<U>(-1);

template <>
inline constexpr auto EQUALITY_MEMCMP_COMPATIBLE<std::byte, std::byte, false> = true;

template <class T, class U>
inline constexpr auto EQUALITY_MEMCMP_COMPATIBLE<T*, U*, false> =
    std::is_same_v<std::remove_cv_t<T>, std::remove_cv_t<U>>;

template <class T, class U>
inline constexpr auto EQUALITY_MEMCMP_COMPATIBLE<T, U, false> = false;

template <class T, class U = T>
struct EqualityMemcmpCompatible : std::bool_constant<detail::EQUALITY_MEMCMP_COMPATIBLE<T, U>>
{
};

// Implementation taken from MSVC _Lex_compare_check_element_types_helper
template <class T, class U = T>
struct LexicographicalMemcmpCompatible
    : std::bool_constant<(std::is_same_v<unsigned char, T> && std::is_same_v<unsigned char, U>)>
{
};

template <>
struct LexicographicalMemcmpCompatible<std::byte, std::byte> : std::true_type
{
};

template <class T, class U = T>
inline constexpr auto LEXICOGRAPHICAL_MEMCMP_COMPATIBLE = detail::LexicographicalMemcmpCompatible<T, U>::value;
}  // namespace cntgs::detail