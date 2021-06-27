#pragma once

#include <type_traits>
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

template <class T, class U, bool B>
struct EqualSizeof;

template <class T, class U>
struct EqualSizeof<T, U, true>
{
    static constexpr auto VALUE = false;
};

template <class T, class U>
struct EqualSizeof<T, U, false>
{
    static constexpr auto VALUE = sizeof(T) == sizeof(U);
};

template <class T, class U>
static constexpr auto EQUAL_SIZEOF = EqualSizeof<T, U, (std::is_same_v<void, T> || std::is_same_v<void, U>)>::VALUE;
}  // namespace cntgs::detail