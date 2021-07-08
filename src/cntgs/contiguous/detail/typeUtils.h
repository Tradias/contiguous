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
static constexpr auto DerivedFrom =
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
static constexpr auto EQUAL_SIZEOF =
    EqualSizeof<(std::is_same_v<void, T> || std::is_same_v<void, U>)>::template VALUE<T, U>;

template <class...>
static constexpr auto FALSE_V = false;

namespace has_ADL_swap_detail
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
}  // namespace has_ADL_swap_detail
using has_ADL_swap_detail::HasADLSwap;

// Implementation taken from MSVC _Is_trivially_swappable
template <class T>
using IsTriviallySwappable =
    std::conjunction<std::is_trivially_destructible<T>, std::is_trivially_move_constructible<T>,
                     std::is_trivially_move_assignable<T>, std::negation<HasADLSwap<T>>>;

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
}  // namespace cntgs::detail