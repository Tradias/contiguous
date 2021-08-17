// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_CNTGS_CONTIGUOUS_HPP
#define CNTGS_CNTGS_CONTIGUOUS_HPP

// #include "cntgs/element.hpp"
// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_CNTGS_ELEMENT_HPP
#define CNTGS_CNTGS_ELEMENT_HPP

// #include "cntgs/detail/allocator.hpp"
// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_DETAIL_ALLOCATOR_HPP
#define CNTGS_DETAIL_ALLOCATOR_HPP

// #include "cntgs/detail/memory.hpp"
// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_DETAIL_MEMORY_HPP
#define CNTGS_DETAIL_MEMORY_HPP

// #include "cntgs/detail/attributes.hpp"
// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_DETAIL_ATTRIBUTES_HPP
#define CNTGS_DETAIL_ATTRIBUTES_HPP

#ifdef _MSC_VER
#define CNTGS_RESTRICT_RETURN __declspec(restrict)
#else
#define CNTGS_RESTRICT_RETURN __attribute__((malloc)) __attribute__((returns_nonnull))
#endif

#define CNTGS_RESTRICT __restrict

#endif  // CNTGS_DETAIL_ATTRIBUTES_HPP

// #include "cntgs/detail/iterator.hpp"
// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_DETAIL_ITERATOR_HPP
#define CNTGS_DETAIL_ITERATOR_HPP

// #include "cntgs/detail/typeTraits.hpp"
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

#ifdef __cpp_lib_concepts
#include <concepts>
#endif

namespace cntgs::detail
{
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

template <class>
struct IsByte : std::false_type
{
};

template <>
struct IsByte<char> : std::true_type
{
};

template <>
struct IsByte<signed char> : std::true_type
{
};

template <>
struct IsByte<unsigned char> : std::true_type
{
};

template <>
struct IsByte<std::byte> : std::true_type
{
};

#ifdef __cpp_char8_t
template <>
struct IsByte<char8_t> : std::true_type
{
};
#endif

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

template <class, class, class = std::void_t<>>
struct AreEqualityComparable : std::false_type
{
};

template <class Lhs, class Rhs>
struct AreEqualityComparable<Lhs, Rhs,
                             std::void_t<decltype(std::declval<const Lhs&>() == std::declval<const Rhs&>()),
                                         decltype(std::declval<const Rhs&>() == std::declval<const Lhs&>())>>
    : std::true_type
{
};

template <class T>
inline constexpr auto IS_NOTRHOW_EQUALITY_COMPARABLE = noexcept(std::declval<const T&>() == std::declval<const T&>());

template <class T>
inline constexpr auto IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE = noexcept(std::declval<const T&>() <
                                                                       std::declval<const T&>());

template <class T, class U>
inline constexpr auto MEMCPY_COMPATIBLE =
    detail::EQUAL_SIZEOF<T, U>&& std::is_trivially_copyable_v<T>&& std::is_trivially_copyable_v<U>&&
        std::is_floating_point_v<T> == std::is_floating_point_v<U>;

// Implementation taken from MSVC _Can_memcmp_elements
template <class T, class U = T,
          bool = (detail::EQUAL_SIZEOF<T, U> && std::is_integral_v<T> && !std::is_volatile_v<T> &&
                  std::is_integral_v<U> && !std::is_volatile_v<U>)>
inline constexpr auto EQUALITY_MEMCMP_COMPATIBLE = std::is_same_v<T, bool> || std::is_same_v<U, bool> || T(-1) == U(-1);

template <>
inline constexpr auto EQUALITY_MEMCMP_COMPATIBLE<std::byte, std::byte, false> = true;

template <class T, class U>
inline constexpr auto EQUALITY_MEMCMP_COMPATIBLE<T*, U*, false> =
    std::is_same_v<std::remove_cv_t<T>, std::remove_cv_t<U>>;

template <class T, class U>
inline constexpr auto EQUALITY_MEMCMP_COMPATIBLE<T, U, false> = false;

template <class T>
struct EqualityMemcmpCompatible
    : std::bool_constant<detail::EQUALITY_MEMCMP_COMPATIBLE<std::remove_const_t<T>, std::remove_const_t<T>>>
{
};

// Implementation taken from MSVC+GCC _Lex_compare_check_element_types_helper and __is_memcmp_ordered
template <class T, bool = detail::IsByte<T>::value>
struct LexicographicalMemcmpCompatible : std::bool_constant<((T(-1) > T(1)) && !std::is_volatile_v<T>)>
{
};

template <>
struct LexicographicalMemcmpCompatible<std::byte, false> : std::true_type
{
};

template <class T>
struct LexicographicalMemcmpCompatible<T, false> : std::false_type
{
};

template <class T>
using LexicographicalMemcmpCompatibleT = detail::LexicographicalMemcmpCompatible<std::remove_const_t<T>>;

template <class T>
inline constexpr auto LEXICOGRAPHICAL_MEMCMP_COMPATIBLE = detail::LexicographicalMemcmpCompatibleT<T>::value;
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_TYPETRAITS_HPP


#include <iterator>
#include <version>

namespace cntgs::detail
{
template <class, class = std::void_t<>>
struct HasOperatorArrow : std::false_type
{
};

template <class T>
struct HasOperatorArrow<T, std::void_t<decltype(std::declval<const T&>().operator->())>> : std::true_type
{
};

template <class T>
struct ArrowProxy
{
    T t;

    constexpr const T* operator->() const noexcept { return &t; }
};

template <class I>
constexpr auto operator_arrow_produces_pointer_to_iterator_reference_type() noexcept
{
    if constexpr (detail::HasOperatorArrow<I>::value)
    {
        return std::is_same_v<decltype(std::declval<const I&>().operator->()),
                              std::add_pointer_t<typename std::iterator_traits<I>::reference>>;
    }
    else
    {
        return false;
    }
}

template <class I>
inline constexpr auto CONTIGUOUS_ITERATOR_V =
    detail::DerivedFrom<typename std::iterator_traits<I>::iterator_category, std::random_access_iterator_tag>&&
        std::is_lvalue_reference_v<typename std::iterator_traits<I>::reference>&&
            std::is_same_v<typename std::iterator_traits<I>::value_type,
                           detail::RemoveCvrefT<typename std::iterator_traits<I>::reference>>&&
            detail::operator_arrow_produces_pointer_to_iterator_reference_type<I>();
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_ITERATOR_HPP

// #include "cntgs/detail/range.hpp"
// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_DETAIL_RANGE_HPP
#define CNTGS_DETAIL_RANGE_HPP

#include <iterator>
#include <type_traits>
#include <utility>

namespace cntgs::detail
{
template <class, class = std::void_t<>>
struct HasDataAndSize : std::false_type
{
};

template <class T>
struct HasDataAndSize<T, std::void_t<decltype(std::data(std::declval<T&>())), decltype(std::size(std::declval<T&>()))>>
    : std::true_type
{
};

template <class, class = std::void_t<>>
struct IsRange : std::false_type
{
};

template <class T>
struct IsRange<T, std::void_t<decltype(std::begin(std::declval<T&>())), decltype(std::end(std::declval<T&>()))>>
    : std::true_type
{
};
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_RANGE_HPP

// #include "cntgs/detail/typeTraits.hpp"

// #include "cntgs/detail/utility.hpp"
// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_DETAIL_UTILITY_HPP
#define CNTGS_DETAIL_UTILITY_HPP

// #include "cntgs/span.hpp"
// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_CNTGS_SPAN_HPP
#define CNTGS_CNTGS_SPAN_HPP

#include <cstddef>
#include <iterator>
#include <version>

#ifdef __cpp_lib_span
#include <span>
#endif

namespace cntgs
{
template <class T>
struct Span
{
    using element_type = T;
    using value_type = std::remove_cv_t<T>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using iterator = T*;
    using reverse_iterator = std::reverse_iterator<iterator>;

    iterator first;
    iterator last;

    Span() = default;

    template <class U>
    constexpr explicit Span(const Span<U>& other) noexcept : first(other.first), last(other.last)
    {
    }

    Span(const Span& other) = default;

    constexpr Span(iterator first, iterator last) noexcept : first(first), last(last) {}

    constexpr Span(iterator first, size_type size) noexcept(noexcept(first + size)) : first(first), last(first + size)
    {
    }

    [[nodiscard]] constexpr iterator begin() const noexcept { return this->first; }

    [[nodiscard]] constexpr iterator end() const noexcept { return this->last; }

    [[nodiscard]] constexpr reverse_iterator rbegin() const noexcept { return reverse_iterator{this->end()}; }

    [[nodiscard]] constexpr reverse_iterator rend() const noexcept { return reverse_iterator{this->begin()}; }

    [[nodiscard]] constexpr bool empty() const noexcept { return this->first == this->last; }

    [[nodiscard]] constexpr size_type size() const noexcept { return this->last - this->first; }

    [[nodiscard]] constexpr pointer data() const noexcept { return this->first; }

    [[nodiscard]] constexpr reference operator[](size_type i) const noexcept { return this->first[i]; }

    [[nodiscard]] constexpr reference front() const noexcept { return this->first[0]; }

    [[nodiscard]] constexpr reference back() const noexcept { return *(this->last - 1); }

#ifdef __cpp_lib_span
    constexpr operator std::span<T>() const noexcept { return std::span<T>{first, last}; }
#endif
};
}  // namespace cntgs

#endif  // CNTGS_CNTGS_SPAN_HPP


#include <utility>

namespace cntgs::detail
{
template <class T>
struct MoveDefaultingValue
{
    T value;

    constexpr explicit MoveDefaultingValue(T value) noexcept : value(value) {}

    ~MoveDefaultingValue() = default;

    MoveDefaultingValue(const MoveDefaultingValue&) = default;

    constexpr MoveDefaultingValue(MoveDefaultingValue&& other) noexcept : value(other.value) { other.value = T{}; }

    MoveDefaultingValue& operator=(const MoveDefaultingValue& other) = default;

    constexpr MoveDefaultingValue& operator=(MoveDefaultingValue&& other) noexcept
    {
        this->value = other.value;
        other.value = T{};
        return *this;
    }
};

template <class T, bool = (std::is_empty_v<T> && !std::is_final_v<T>)>
class EmptyBaseOptimization
{
  private:
    T value;

  public:
    EmptyBaseOptimization() = default;

    constexpr explicit EmptyBaseOptimization(const T& value) noexcept(std::is_nothrow_copy_constructible_v<T>)
        : value{value}
    {
    }

    constexpr explicit EmptyBaseOptimization(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>)
        : value{std::move(value)}
    {
    }

    constexpr auto& get() noexcept { return value; }

    constexpr const auto& get() const noexcept { return value; }
};

template <class T>
class EmptyBaseOptimization<T, true> : private T
{
  public:
    EmptyBaseOptimization() = default;

    constexpr explicit EmptyBaseOptimization(const T& value) noexcept(std::is_nothrow_copy_constructible_v<T>)
        : T{value}
    {
    }

    constexpr explicit EmptyBaseOptimization(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>)
        : T{std::move(value)}
    {
    }

    constexpr T& get() noexcept { return *this; }

    constexpr const T& get() const noexcept { return *this; }
};

template <class T>
constexpr auto as_const(cntgs::Span<T> value) noexcept
{
    return cntgs::Span<std::add_const_t<T>>{value};
}

template <class T>
constexpr decltype(auto) as_const(T& value) noexcept
{
    return std::as_const(value);
}

template <class T>
constexpr decltype(auto) as_ref(T* value) noexcept
{
    return (*value);
}

template <class T>
constexpr decltype(auto) as_ref(T& value) noexcept
{
    return (value);
}

template <class T>
constexpr decltype(auto) as_const_ref(T&& value) noexcept
{
    return detail::as_const(detail::as_ref(std::forward<T>(value)));
}
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_UTILITY_HPP

// #include "cntgs/span.hpp"


#include <algorithm>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <limits>
#include <memory>
#include <version>

namespace cntgs::detail
{
using Byte = std::underlying_type_t<std::byte>;

template <std::size_t N>
struct alignas(N) AlignedByte
{
    std::byte byte;
};

template <class T>
auto memcpy(const T* CNTGS_RESTRICT source, std::byte* CNTGS_RESTRICT target, std::size_t size) noexcept
{
    std::memcpy(target, source, size * sizeof(T));
    return target + size * sizeof(T);
}

template <class Range, class TargetIterator>
constexpr auto uninitialized_move(Range&& source, TargetIterator&& target)
{
#ifdef __cpp_lib_ranges
    return std::ranges::uninitialized_move(
               std::forward<Range>(source),
               std::ranges::subrange{std::forward<TargetIterator>(target), std::unreachable_sentinel})
        .out;
#else
    return std::uninitialized_move(std::begin(source), std::end(source), std::forward<TargetIterator>(target));
#endif
}

template <class Range, class TargetIterator>
constexpr auto uninitialized_copy(Range&& source, TargetIterator&& target)
{
#ifdef __cpp_lib_ranges
    return std::ranges::uninitialized_copy(
               std::forward<Range>(source),
               std::ranges::subrange{std::forward<TargetIterator>(target), std::unreachable_sentinel})
        .out;
#else
    return std::uninitialized_copy(std::begin(source), std::end(source), std::forward<TargetIterator>(target));
#endif
}

template <class SourceIterator, class DifferenceType, class TargetIterator>
constexpr auto uninitialized_copy_n(SourceIterator&& source, DifferenceType count, TargetIterator&& target)
{
#ifdef __cpp_lib_ranges
    return std::ranges::uninitialized_copy_n(std::forward<SourceIterator>(source),
                                             static_cast<std::iter_difference_t<SourceIterator>>(count),
                                             std::forward<TargetIterator>(target), std::unreachable_sentinel)
        .out;
#else
    return std::uninitialized_copy_n(std::forward<SourceIterator>(source), count, std::forward<TargetIterator>(target));
#endif
}

template <bool IgnoreAliasing, class TargetType, class Range>
auto uninitialized_range_construct(Range&& CNTGS_RESTRICT range, TargetType* CNTGS_RESTRICT address)
{
    using RangeValueType = typename std::iterator_traits<decltype(std::begin(range))>::value_type;
    if constexpr (IgnoreAliasing && detail::HasDataAndSize<std::decay_t<Range>>{} &&
                  detail::MEMCPY_COMPATIBLE<TargetType, RangeValueType>)
    {
        return detail::memcpy(std::data(range), reinterpret_cast<std::byte*>(address), std::size(range));
    }
    else
    {
        if constexpr (!std::is_lvalue_reference_v<Range>)
        {
            return reinterpret_cast<std::byte*>(detail::uninitialized_move(std::forward<Range>(range), address));
        }
        else
        {
            return reinterpret_cast<std::byte*>(detail::uninitialized_copy(std::forward<Range>(range), address));
        }
    }
}

template <bool IgnoreAliasing, class TargetType, class Range>
auto uninitialized_construct(Range&& CNTGS_RESTRICT range, TargetType* CNTGS_RESTRICT address, std::size_t)
    -> std::enable_if_t<detail::IsRange<Range>::value, std::byte*>
{
    return detail::uninitialized_range_construct<IgnoreAliasing>(std::forward<Range>(range), address);
}

template <bool IgnoreAliasing, class TargetType, class Iterator>
auto uninitialized_construct(const Iterator& CNTGS_RESTRICT iterator, TargetType* CNTGS_RESTRICT address,
                             std::size_t size) -> std::enable_if_t<!detail::IsRange<Iterator>::value, std::byte*>
{
    using IteratorValueType = typename std::iterator_traits<Iterator>::value_type;
    if constexpr (IgnoreAliasing && std::is_pointer_v<Iterator> &&
                  detail::MEMCPY_COMPATIBLE<TargetType, IteratorValueType>)
    {
        return detail::memcpy(iterator, reinterpret_cast<std::byte*>(address), size);
    }
    else if constexpr (IgnoreAliasing && detail::CONTIGUOUS_ITERATOR_V<Iterator> &&
                       detail::MEMCPY_COMPATIBLE<TargetType, IteratorValueType>)
    {
        return detail::memcpy(iterator.operator->(), reinterpret_cast<std::byte*>(address), size);
    }
    else
    {
        return reinterpret_cast<std::byte*>(detail::uninitialized_copy_n(iterator, size, address));
    }
}

#ifdef __cpp_lib_ranges
using std::construct_at;
#else
template <class T, class... Args>
constexpr T* construct_at(T* ptr, Args&&... args)
{
    return ::new (const_cast<void*>(static_cast<const volatile void*>(ptr))) T(std::forward<Args>(args)...);
}
#endif

#ifdef __cpp_lib_assume_aligned
using std::assume_aligned;
#else
template <std::size_t Alignment, class T>
[[nodiscard]] constexpr T* assume_aligned(T* const ptr) noexcept
{
    return static_cast<T*>(::__builtin_assume_aligned(ptr, Alignment));
}
#endif

[[nodiscard]] constexpr auto align(std::uintptr_t position, std::size_t alignment) noexcept
{
    return (position - 1u + alignment) & (alignment * std::numeric_limits<std::size_t>::max());
}

template <std::size_t Alignment>
[[nodiscard]] constexpr auto align(std::uintptr_t position) noexcept
{
    if constexpr (Alignment > 1)
    {
        return detail::align(position, Alignment);
    }
    else
    {
        return position;
    }
}

template <std::size_t Alignment, class T>
[[nodiscard]] constexpr T* align(T* ptr) noexcept
{
    if constexpr (Alignment > 1)
    {
        const auto uintptr = reinterpret_cast<std::uintptr_t>(ptr);
        const auto aligned = detail::align<Alignment>(uintptr);
        return detail::assume_aligned<Alignment>(reinterpret_cast<T*>(aligned));
    }
    else
    {
        return ptr;
    }
}

template <bool NeedsAlignment, std::size_t Alignment, class T>
[[nodiscard]] constexpr auto align_if(T* ptr) noexcept
{
    if constexpr (NeedsAlignment && Alignment > 1)
    {
        ptr = detail::align<Alignment>(ptr);
    }
    return detail::assume_aligned<Alignment>(ptr);
}
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_MEMORY_HPP

// #include "cntgs/detail/utility.hpp"


#include <cstddef>
#include <memory>
#include <type_traits>

namespace cntgs::detail
{
using TypeErasedAllocator = std::aligned_storage_t<32>;

template <class Allocator>
class AllocatorAwarePointer
{
  private:
    using AllocatorTraits = std::allocator_traits<Allocator>;
    using Pointer = typename AllocatorTraits::pointer;

  public:
    using allocator_type = Allocator;
    using pointer = Pointer;
    using value_type = typename AllocatorTraits::value_type;

  private:
    struct Impl : detail::EmptyBaseOptimization<Allocator>
    {
        using Base = detail::EmptyBaseOptimization<Allocator>;

        Pointer ptr{};
        std::size_t size{};

        Impl() = default;

        constexpr Impl(Pointer ptr, std::size_t size, const Allocator& allocator) noexcept
            : Base{allocator}, ptr(ptr), size(size)
        {
        }
    };

    Impl impl;

    constexpr auto allocate() { return AllocatorTraits::allocate(this->get_allocator(), this->size()); }

    constexpr void deallocate() noexcept
    {
        if (this->get())
        {
            AllocatorTraits::deallocate(this->get_allocator(), this->get(), this->size());
        }
    }

    static constexpr auto allocate_if_not_zero(std::size_t size, Allocator allocator)
    {
#ifdef __cpp_lib_is_constant_evaluated
        if (std::is_constant_evaluated() && size == 0)
        {
            return Pointer{};
        }
#endif
        return AllocatorTraits::allocate(allocator, size);
    }

  public:
    AllocatorAwarePointer() = default;

    constexpr AllocatorAwarePointer(std::size_t size, const Allocator& allocator)
        : impl(AllocatorAwarePointer::allocate_if_not_zero(size, allocator), size, allocator)
    {
    }

    constexpr AllocatorAwarePointer(pointer ptr, std::size_t size, const Allocator& allocator) noexcept
        : impl(ptr, size, allocator)
    {
    }

    constexpr AllocatorAwarePointer(const AllocatorAwarePointer& other)
        : AllocatorAwarePointer(other.size(),
                                AllocatorTraits::select_on_container_copy_construction(other.get_allocator()))
    {
    }

    constexpr AllocatorAwarePointer(AllocatorAwarePointer&& other) noexcept
        : impl(other.release(), other.size(), other.get_allocator())
    {
    }

#if __cpp_constexpr_dynamic_alloc
    constexpr
#endif
        ~AllocatorAwarePointer() noexcept
    {
        this->deallocate();
    }

    constexpr AllocatorAwarePointer& operator=(const AllocatorAwarePointer& other)
    {
        if (this != std::addressof(other))
        {
            if constexpr (AllocatorTraits::propagate_on_container_copy_assignment::value &&
                          !AllocatorTraits::is_always_equal::value)
            {
                if (this->get_allocator() != other.get_allocator())
                {
                    this->deallocate();
                    this->propagate_on_container_copy_assignment(other);
                    this->size() = other.size();
                    this->get() = this->allocate();
                    return *this;
                }
            }
            this->propagate_on_container_copy_assignment(other);
            if (this->size() < other.size() || !this->get())
            {
                this->deallocate();
                this->size() = other.size();
                this->get() = this->allocate();
            }
        }
        return *this;
    }

    constexpr AllocatorAwarePointer& operator=(AllocatorAwarePointer&& other) noexcept
    {
        if (this != std::addressof(other))
        {
            this->propagate_on_container_move_assignment(other);
            this->deallocate();
            this->get() = other.release();
            this->size() = other.size();
        }
        return *this;
    }

    constexpr decltype(auto) get_allocator() noexcept { return this->impl.get(); }

    constexpr auto get_allocator() const noexcept { return this->impl.get(); }

    constexpr auto& get() noexcept { return this->impl.ptr; }

    constexpr auto get() const noexcept { return this->impl.ptr; }

    constexpr auto& size() noexcept { return this->impl.size; }

    constexpr auto size() const noexcept { return this->impl.size; }

    constexpr explicit operator bool() const noexcept { return this->get() != nullptr; }

    constexpr auto release() noexcept { return std::exchange(this->impl.ptr, nullptr); }

    constexpr void reset(AllocatorAwarePointer&& other) noexcept
    {
        this->deallocate();
        this->get() = other.release();
        this->size() = other.size();
    }

    constexpr void propagate_on_container_copy_assignment(const AllocatorAwarePointer& other) noexcept
    {
        if constexpr (AllocatorTraits::propagate_on_container_copy_assignment::value)
        {
            this->get_allocator() = other.get_allocator();
        }
    }

    constexpr void propagate_on_container_move_assignment(AllocatorAwarePointer& other) noexcept
    {
        if constexpr (AllocatorTraits::propagate_on_container_move_assignment::value)
        {
            this->get_allocator() = std::move(other.get_allocator());
        }
    }
};

template <class Allocator>
constexpr void swap(detail::AllocatorAwarePointer<Allocator>& lhs,
                    detail::AllocatorAwarePointer<Allocator>& rhs) noexcept
{
    using std::swap;
    if constexpr (std::allocator_traits<Allocator>::propagate_on_container_swap::value)
    {
        swap(lhs.get_allocator(), rhs.get_allocator());
    }
    swap(lhs.get(), rhs.get());
    swap(lhs.size(), rhs.size());
}

template <class T>
auto type_erase_allocator(T&& allocator) noexcept
{
    detail::TypeErasedAllocator result;
    detail::construct_at(reinterpret_cast<detail::RemoveCvrefT<T>*>(&result), std::forward<T>(allocator));
    return result;
}
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_ALLOCATOR_HPP

// #include "cntgs/detail/elementTraits.hpp"
// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_DETAIL_ELEMENTTRAITS_HPP
#define CNTGS_DETAIL_ELEMENTTRAITS_HPP

// #include "cntgs/detail/algorithm.hpp"
// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_DETAIL_ALGORITHM_HPP
#define CNTGS_DETAIL_ALGORITHM_HPP

// #include "cntgs/detail/memory.hpp"


#include <algorithm>
#include <array>
#include <cstddef>
#include <utility>

namespace cntgs::detail
{
// Some compilers (e.g. MSVC) perform hand rolled optimizations or call C functions if the argument type
// fulfills certain criteria. These checks are not always performed correctly for std::byte, therefore
// cast it to a more reliable type.

template <class T>
constexpr auto trivial_swap_ranges(T* begin, T* end, T* begin2) noexcept
{
    return std::swap_ranges(reinterpret_cast<detail::Byte*>(begin), reinterpret_cast<detail::Byte*>(end),
                            reinterpret_cast<detail::Byte*>(begin2));
}

template <class T>
constexpr auto trivial_equal(const T* begin, const T* end, const T* begin2, const T* end2) noexcept
{
    return std::equal(reinterpret_cast<const detail::Byte*>(begin), reinterpret_cast<const detail::Byte*>(end),
                      reinterpret_cast<const detail::Byte*>(begin2), reinterpret_cast<const detail::Byte*>(end2));
}

template <class T>
constexpr auto trivial_lexicographical_compare(const T* begin, const T* end, const T* begin2, const T* end2) noexcept
{
    return std::lexicographical_compare(
        reinterpret_cast<const detail::Byte*>(begin), reinterpret_cast<const detail::Byte*>(end),
        reinterpret_cast<const detail::Byte*>(begin2), reinterpret_cast<const detail::Byte*>(end2));
}
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_ALGORITHM_HPP

// #include "cntgs/detail/attributes.hpp"

// #include "cntgs/detail/forward.hpp"
// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_DETAIL_FORWARD_HPP
#define CNTGS_DETAIL_FORWARD_HPP

#include <cstddef>

namespace cntgs
{
template <class T>
struct VaryingSize;

template <class T>
struct FixedSize;

template <class T, std::size_t Alignment = 1>
struct AlignAs;

template <class... Option>
struct Options;

template <class T>
struct Allocator;

template <class Options, class... T>
class BasicContiguousVector;

template <bool IsConst, class Options, class... Parameter>
class ContiguousVectorIterator;

class TypeErasedVector;

template <bool IsConst, class... Parameter>
class BasicContiguousReference;

template <class Allocator, class... Parameter>
class BasicContiguousElement;
}  // namespace cntgs

#endif  // CNTGS_DETAIL_FORWARD_HPP

// #include "cntgs/detail/memory.hpp"

// #include "cntgs/detail/parameterListTraits.hpp"
// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_DETAIL_PARAMETERLISTTRAITS_HPP
#define CNTGS_DETAIL_PARAMETERLISTTRAITS_HPP

// #include "cntgs/detail/array.hpp"
// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_DETAIL_ARRAY_HPP
#define CNTGS_DETAIL_ARRAY_HPP

#include <array>
#include <cstddef>
#include <utility>

namespace cntgs::detail
{
template <class T, std::size_t N>
struct Array
{
    std::array<T, N> array;
};

template <class T>
struct Array<T, 0>
{
    Array() = default;

    constexpr explicit Array(const std::array<T, 0>&) noexcept {}
};

template <std::size_t I, class T, std::size_t N>
constexpr decltype(auto) get(const detail::Array<T, N>& array) noexcept
{
    return std::get<I>(array.array);
}

template <std::size_t, class T>
constexpr auto get(const detail::Array<T, 0>&) noexcept
{
    return T{};
}

template <std::size_t N, template <class, std::size_t> class ArrayLike, class T, std::size_t K, std::size_t... I>
constexpr auto convert_array_to_size(const ArrayLike<T, K>& array, std::index_sequence<I...>)
{
    return ArrayLike<T, N>{get<I>(array)...};
}

template <std::size_t N, template <class, std::size_t> class ArrayLike, class T, std::size_t K>
constexpr auto convert_array_to_size(const ArrayLike<T, K>& array)
{
    return detail::convert_array_to_size<N>(array, std::make_index_sequence<std::min(N, K)>{});
}
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_ARRAY_HPP

// #include "cntgs/detail/parameterTraits.hpp"
// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_DETAIL_PARAMETERTRAITS_HPP
#define CNTGS_DETAIL_PARAMETERTRAITS_HPP

// #include "cntgs/detail/attributes.hpp"

// #include "cntgs/detail/memory.hpp"

// #include "cntgs/detail/parameterType.hpp"
// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_DETAIL_PARAMETERTYPE_HPP
#define CNTGS_DETAIL_PARAMETERTYPE_HPP

namespace cntgs::detail
{
enum class ParameterType
{
    PLAIN,
    FIXED_SIZE,
    VARYING_SIZE
};
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_PARAMETERTYPE_HPP

// #include "cntgs/detail/typeTraits.hpp"

// #include "cntgs/parameter.hpp"
// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_CNTGS_PARAMETER_HPP
#define CNTGS_CNTGS_PARAMETER_HPP

#include <cstddef>

namespace cntgs
{
/// Parameter that causes values to be stored along with a `std::size_t`. When used with
/// [cntgs::BasicContiguousVector]() every element of the vector can have a different number of values of type `T`
///
/// \param T User-defined or built-in type, optionally wrapped in [cntgs::AlignAs]()
template <class T>
struct VaryingSize
{
};

/// When used with [cntgs::BasicContiguousVector]() every element of the vector has a fixed number of values of type
/// `T`.
///
/// \param T User-defined or built-in type, optionally wrapped in [cntgs::AlignAs]()
template <class T>
struct FixedSize
{
};

/// When used with [cntgs::BasicContiguousVector]() every element of the vector is stored with an alignment of
/// [Alignment](<> "cntgs::AlignAs<T, Alignment>.Alignment").
///
/// \param T User-defined or built-in type
/// \param Alignment Desired alignment of the type
template <class T, std::size_t Alignment>
struct AlignAs
{
};

template <class... Option>
struct Options
{
};

/// An allocator that is used to acquire/release memory and to construct/destroy the elements in that
/// memory. Must satisfy [Allocator](https://en.cppreference.com/w/cpp/named_req/Allocator).
template <class T>
struct Allocator
{
};
}  // namespace cntgs

#endif  // CNTGS_CNTGS_PARAMETER_HPP

// #include "cntgs/span.hpp"


#include <algorithm>
#include <cstddef>
#include <memory>
#include <new>
#include <type_traits>

namespace cntgs::detail
{
template <class T>
struct ParameterTraits : detail::ParameterTraits<cntgs::AlignAs<T, 1>>
{
};

template <class T, std::size_t Alignment>
struct ParameterTraits<cntgs::AlignAs<T, Alignment>>
{
    using ValueType = T;
    using PointerType = std::add_pointer_t<T>;
    using ReferenceType = std::add_lvalue_reference_t<T>;
    using ConstReferenceType = std::add_lvalue_reference_t<std::add_const_t<T>>;

    static constexpr auto TYPE = detail::ParameterType::PLAIN;
    static constexpr auto ALIGNMENT = Alignment;
    static constexpr auto VALUE_BYTES = sizeof(T);
    static constexpr auto ALIGNED_SIZE_IN_MEMORY = std::max(VALUE_BYTES, ALIGNMENT);

    template <bool NeedsAlignment, bool>
    static auto load(std::byte* address, std::size_t) noexcept
    {
        address = detail::align_if<NeedsAlignment, ALIGNMENT>(address);
        auto result = std::launder(reinterpret_cast<PointerType>(address));
        return std::pair{result, address + VALUE_BYTES};
    }

    static auto load_(std::byte* address, std::size_t) noexcept
    {
        address = detail::assume_aligned<ALIGNMENT>(address);
        auto result = std::launder(reinterpret_cast<PointerType>(address));
        return result;
    }

    template <bool NeedsAlignment, bool, class Arg>
    CNTGS_RESTRICT_RETURN static std::byte* store(Arg&& arg, std::byte* CNTGS_RESTRICT address, std::size_t)
    {
        address = reinterpret_cast<std::byte*>(detail::align_if<NeedsAlignment, ALIGNMENT>(address));
        detail::construct_at(reinterpret_cast<T*>(address), std::forward<Arg>(arg));
        return address + VALUE_BYTES;
    }

    static constexpr auto aligned_size_in_memory(std::size_t) noexcept { return ALIGNED_SIZE_IN_MEMORY; }

    static constexpr auto guaranteed_size_in_memory(std::size_t) noexcept { return VALUE_BYTES; }

    static auto data_begin(ConstReferenceType reference) noexcept
    {
        return detail::assume_aligned<ALIGNMENT>(reinterpret_cast<const std::byte*>(std::addressof(reference)));
    }

    static auto data_begin(ReferenceType reference) noexcept
    {
        return detail::assume_aligned<ALIGNMENT>(reinterpret_cast<std::byte*>(std::addressof(reference)));
    }

    static auto data_end(ConstReferenceType reference) noexcept { return data_begin(reference) + VALUE_BYTES; }

    static auto data_end(ReferenceType reference) noexcept { return data_begin(reference) + VALUE_BYTES; }

    static constexpr void copy(ConstReferenceType source,
                               ReferenceType target) noexcept(std::is_nothrow_copy_assignable_v<T>)
    {
        target = source;
    }

    static constexpr void move(T&& source, ReferenceType target) noexcept(std::is_nothrow_move_assignable_v<T>)
    {
        target = std::move(source);
    }

    static constexpr void move(ReferenceType source,
                               ReferenceType target) noexcept(std::is_nothrow_move_assignable_v<T>)
    {
        target = std::move(source);
    }

    static constexpr void uninitialized_copy(ConstReferenceType source,
                                             T* target) noexcept(std::is_nothrow_copy_constructible_v<T>)
    {
        detail::construct_at(target, source);
    }

    static constexpr void uninitialized_move(ReferenceType source,
                                             T* target) noexcept(std::is_nothrow_move_constructible_v<T>)
    {
        detail::construct_at(target, std::move(source));
    }

    static constexpr void swap(ReferenceType lhs, ReferenceType rhs) noexcept(std::is_nothrow_swappable_v<T>)
    {
        using std::swap;
        swap(lhs, rhs);
    }

    static constexpr auto equal(ConstReferenceType lhs, ConstReferenceType rhs) { return lhs == rhs; }

    static constexpr auto lexicographical_compare(ConstReferenceType lhs, ConstReferenceType rhs) { return lhs < rhs; }

    static constexpr void destroy(ReferenceType value) noexcept { value.~T(); }
};

template <class T, std::size_t Alignment>
struct BaseContiguousParameterTraits
{
    using Self = BaseContiguousParameterTraits<T, Alignment>;

    static auto data_end(const cntgs::Span<std::add_const_t<T>>& value) noexcept
    {
        return reinterpret_cast<const std::byte*>(std::end(value));
    }

    static auto data_end(const cntgs::Span<T>& value) noexcept { return reinterpret_cast<std::byte*>(std::end(value)); }

    static constexpr void uninitialized_copy(
        const cntgs::Span<std::add_const_t<T>>& source,
        const cntgs::Span<T>& target) noexcept(std::is_nothrow_copy_constructible_v<T>)
    {
        std::uninitialized_copy(Self::begin(source), std::end(source), Self::begin(target));
    }

    static constexpr void uninitialized_copy(const cntgs::Span<T>& source, const cntgs::Span<T>& target) noexcept(
        std::is_nothrow_copy_constructible_v<T>)
    {
        std::uninitialized_copy(Self::begin(source), std::end(source), Self::begin(target));
    }

    static constexpr void uninitialized_move(const cntgs::Span<T>& source, const cntgs::Span<T>& target) noexcept(
        std::is_nothrow_move_constructible_v<T>)
    {
        std::uninitialized_copy(std::make_move_iterator(Self::begin(source)), std::make_move_iterator(std::end(source)),
                                Self::begin(target));
    }

    static constexpr auto equal(const cntgs::Span<T>& source, const cntgs::Span<T>& target)
    {
        return std::equal(Self::begin(source), std::end(source), Self::begin(target));
    }

    static constexpr auto equal(const cntgs::Span<T>& source, const cntgs::Span<std::add_const_t<T>>& target)
    {
        return std::equal(Self::begin(source), std::end(source), Self::begin(target));
    }

    static constexpr auto equal(const cntgs::Span<std::add_const_t<T>>& source, const cntgs::Span<T>& target)
    {
        return std::equal(Self::begin(source), std::end(source), Self::begin(target));
    }

    static constexpr auto equal(const cntgs::Span<std::add_const_t<T>>& source,
                                const cntgs::Span<std::add_const_t<T>>& target)
    {
        return std::equal(Self::begin(source), std::end(source), Self::begin(target));
    }

    static constexpr auto lexicographical_compare(const cntgs::Span<T>& source, const cntgs::Span<T>& target)
    {
        return std::lexicographical_compare(Self::begin(source), std::end(source), Self::begin(target),
                                            std::end(target));
    }

    static constexpr auto lexicographical_compare(const cntgs::Span<T>& source,
                                                  const cntgs::Span<std::add_const_t<T>>& target)
    {
        return std::lexicographical_compare(Self::begin(source), std::end(source), Self::begin(target),
                                            std::end(target));
    }

    static constexpr auto lexicographical_compare(const cntgs::Span<std::add_const_t<T>>& source,
                                                  const cntgs::Span<T>& target)
    {
        return std::lexicographical_compare(Self::begin(source), std::end(source), Self::begin(target),
                                            std::end(target));
    }

    static constexpr auto lexicographical_compare(const cntgs::Span<std::add_const_t<T>>& source,
                                                  const cntgs::Span<std::add_const_t<T>>& target)
    {
        return std::lexicographical_compare(Self::begin(source), std::end(source), Self::begin(target),
                                            std::end(target));
    }

    static constexpr void destroy(const cntgs::Span<T>& value) noexcept
    {
        std::destroy(Self::begin(value), std::end(value));
    }

    template <class U>
    static constexpr auto begin(const cntgs::Span<U>& value) noexcept
    {
        return detail::assume_aligned<Alignment>(std::begin(value));
    }
};

template <class T>
struct ParameterTraits<cntgs::VaryingSize<T>> : ParameterTraits<cntgs::VaryingSize<cntgs::AlignAs<T, 1>>>
{
};

template <class T, std::size_t Alignment>
struct ParameterTraits<cntgs::VaryingSize<cntgs::AlignAs<T, Alignment>>> : BaseContiguousParameterTraits<T, Alignment>
{
    using ValueType = T;
    using PointerType = cntgs::Span<T>;
    using ReferenceType = cntgs::Span<T>;
    using ConstReferenceType = cntgs::Span<std::add_const_t<T>>;
    using IteratorType = T*;

    static constexpr auto TYPE = detail::ParameterType::VARYING_SIZE;
    static constexpr auto ALIGNMENT = Alignment;
    static constexpr auto MEMORY_OVERHEAD = sizeof(std::size_t);
    static constexpr auto ALIGNED_SIZE_IN_MEMORY = MEMORY_OVERHEAD + ALIGNMENT - 1;

    template <bool NeedsAlignment, bool IsSizeProvided>
    static auto load(std::byte* address, std::size_t size) noexcept
    {
        if constexpr (!IsSizeProvided)
        {
            size = *reinterpret_cast<std::size_t*>(address);
        }
        address += MEMORY_OVERHEAD;
        const auto first_byte = detail::align_if<NeedsAlignment, ALIGNMENT>(address);
        const auto first = std::launder(reinterpret_cast<IteratorType>(first_byte));
        const auto last = std::launder(reinterpret_cast<IteratorType>(first_byte) + size);
        return std::pair{PointerType{first, last}, reinterpret_cast<std::byte*>(last)};
    }

    template <bool NeedsAlignment, bool IgnoreAliasing, class Range>
    CNTGS_RESTRICT_RETURN static std::byte* store(Range&& range, std::byte* CNTGS_RESTRICT address, std::size_t)
    {
        const auto size = reinterpret_cast<std::size_t*>(address);
        address += MEMORY_OVERHEAD;
        const auto aligned_address =
            reinterpret_cast<IteratorType>(detail::align_if<NeedsAlignment, ALIGNMENT>(address));
        auto* new_address =
            detail::uninitialized_range_construct<IgnoreAliasing>(std::forward<Range>(range), aligned_address);
        *size = reinterpret_cast<IteratorType>(new_address) - aligned_address;
        return new_address;
    }

    static constexpr auto aligned_size_in_memory(std::size_t) noexcept { return ALIGNED_SIZE_IN_MEMORY; }

    static auto data_begin(const cntgs::Span<std::add_const_t<T>>& value) noexcept
    {
        return reinterpret_cast<const std::byte*>(ParameterTraits::begin(value)) - MEMORY_OVERHEAD;
    }

    static auto data_begin(const cntgs::Span<T>& value) noexcept
    {
        return reinterpret_cast<std::byte*>(ParameterTraits::begin(value)) - MEMORY_OVERHEAD;
    }
};

template <class T>
struct ParameterTraits<cntgs::FixedSize<T>> : ParameterTraits<cntgs::FixedSize<cntgs::AlignAs<T, 1>>>
{
};

template <class T, std::size_t Alignment>
struct ParameterTraits<cntgs::FixedSize<cntgs::AlignAs<T, Alignment>>> : BaseContiguousParameterTraits<T, Alignment>
{
    using ValueType = T;
    using PointerType = cntgs::Span<T>;
    using ReferenceType = cntgs::Span<T>;
    using ConstReferenceType = cntgs::Span<std::add_const_t<T>>;
    using IteratorType = T*;

    static constexpr auto TYPE = detail::ParameterType::FIXED_SIZE;
    static constexpr auto ALIGNMENT = Alignment;
    static constexpr auto VALUE_BYTES = sizeof(T);

    template <bool NeedsAlignment, bool>
    static auto load(std::byte* address, std::size_t size) noexcept
    {
        const auto first =
            std::launder(reinterpret_cast<IteratorType>(detail::align_if<NeedsAlignment, ALIGNMENT>(address)));
        const auto last = first + size;
        return std::pair{PointerType{first, last}, reinterpret_cast<std::byte*>(last)};
    }

    static auto load_(std::byte* address, std::size_t size) noexcept
    {
        const auto first = std::launder(reinterpret_cast<IteratorType>(detail::assume_aligned<ALIGNMENT>(address)));
        const auto last = first + size;
        return PointerType{first, last};
    }

    template <bool NeedsAlignment, bool IgnoreAliasing, class RangeOrIterator>
    CNTGS_RESTRICT_RETURN static std::byte* store(RangeOrIterator&& range_or_iterator,
                                                  std::byte* CNTGS_RESTRICT address, std::size_t size)
    {
        const auto aligned_address =
            reinterpret_cast<IteratorType>(detail::align_if<NeedsAlignment, ALIGNMENT>(address));
        return detail::uninitialized_construct<IgnoreAliasing>(std::forward<RangeOrIterator>(range_or_iterator),
                                                               aligned_address, size);
    }

    static constexpr auto aligned_size_in_memory(std::size_t fixed_size) noexcept
    {
        if constexpr (ALIGNMENT == 1)
        {
            return fixed_size * VALUE_BYTES;
        }
        else
        {
            return std::max(fixed_size * VALUE_BYTES, ALIGNMENT);
        }
    }

    static constexpr auto guaranteed_size_in_memory(std::size_t fixed_size) noexcept
    {
        return fixed_size * VALUE_BYTES;
    }

    static auto data_begin(const cntgs::Span<std::add_const_t<T>>& value) noexcept
    {
        return reinterpret_cast<const std::byte*>(ParameterTraits::begin(value));
    }

    static auto data_begin(const cntgs::Span<T>& value) noexcept
    {
        return reinterpret_cast<std::byte*>(ParameterTraits::begin(value));
    }

    static void copy(const cntgs::Span<std::add_const_t<T>>& source,
                     const cntgs::Span<T>& target) noexcept(std::is_nothrow_copy_assignable_v<T>)
    {
        std::copy(std::begin(source), std::end(source), std::begin(target));
    }

    static void copy(const cntgs::Span<T>& source,
                     const cntgs::Span<T>& target) noexcept(std::is_nothrow_copy_assignable_v<T>)
    {
        std::copy(std::begin(source), std::end(source), std::begin(target));
    }

    static void move(const cntgs::Span<T>& source,
                     const cntgs::Span<T>& target) noexcept(std::is_nothrow_move_assignable_v<T>)
    {
        std::move(std::begin(source), std::end(source), std::begin(target));
    }

    static void swap(const cntgs::Span<T>& lhs, const cntgs::Span<T>& rhs) noexcept(std::is_nothrow_swappable_v<T>)
    {
        std::swap_ranges(std::begin(lhs), std::end(lhs), std::begin(rhs));
    }
};
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_PARAMETERTRAITS_HPP

// #include "cntgs/detail/parameterType.hpp"

// #include "cntgs/detail/typeTraits.hpp"


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

template <class... Parameter>
struct ParameterListTraits
{
    template <std::size_t K>
    using ParameterTraitsAt = detail::ParameterTraits<std::tuple_element_t<K, std::tuple<Parameter...>>>;

    static constexpr auto IS_NOTHROW_COPY_CONSTRUCTIBLE =
        (std::is_nothrow_copy_constructible_v<typename detail::ParameterTraits<Parameter>::ValueType> && ...);
    static constexpr auto IS_NOTHROW_MOVE_CONSTRUCTIBLE =
        (std::is_nothrow_move_constructible_v<typename detail::ParameterTraits<Parameter>::ValueType> && ...);
    static constexpr auto IS_NOTHROW_COPY_ASSIGNABLE =
        (std::is_nothrow_copy_assignable_v<typename detail::ParameterTraits<Parameter>::ValueType> && ...);
    static constexpr auto IS_NOTHROW_MOVE_ASSIGNABLE =
        (std::is_nothrow_move_assignable_v<typename detail::ParameterTraits<Parameter>::ValueType> && ...);
    static constexpr auto IS_NOTHROW_SWAPPABLE =
        (std::is_nothrow_swappable_v<typename detail::ParameterTraits<Parameter>::ValueType> && ...);
    static constexpr auto IS_NOTHROW_EQUALITY_COMPARABLE =
        (detail::IS_NOTRHOW_EQUALITY_COMPARABLE<typename detail::ParameterTraits<Parameter>::ValueType> && ...);
    static constexpr auto IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE =
        (detail::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE<typename detail::ParameterTraits<Parameter>::ValueType> && ...);

    static constexpr auto IS_TRIVIALLY_DESTRUCTIBLE =
        (std::is_trivially_destructible_v<typename detail::ParameterTraits<Parameter>::ValueType> && ...);
    static constexpr auto IS_TRIVIALLY_COPY_CONSTRUCTIBLE =
        (std::is_trivially_copy_constructible_v<typename detail::ParameterTraits<Parameter>::ValueType> && ...);
    static constexpr auto IS_TRIVIALLY_MOVE_CONSTRUCTIBLE =
        (std::is_trivially_move_constructible_v<typename detail::ParameterTraits<Parameter>::ValueType> && ...);
    static constexpr auto IS_TRIVIALLY_COPY_ASSIGNABLE =
        (std::is_trivially_copy_assignable_v<typename detail::ParameterTraits<Parameter>::ValueType> && ...);
    static constexpr auto IS_TRIVIALLY_MOVE_ASSIGNABLE =
        (std::is_trivially_move_assignable_v<typename detail::ParameterTraits<Parameter>::ValueType> && ...);
    static constexpr auto IS_TRIVIALLY_SWAPPABLE =
        (detail::IS_TRIVIALLY_SWAPPABLE<typename detail::ParameterTraits<Parameter>::ValueType> && ...);
    static constexpr auto IS_EQUALITY_MEMCMPABLE =
        (detail::EQUALITY_MEMCMP_COMPATIBLE<typename detail::ParameterTraits<Parameter>::ValueType> && ...);
    static constexpr auto IS_LEXICOGRAPHICAL_MEMCMPABLE =
        (detail::LEXICOGRAPHICAL_MEMCMP_COMPATIBLE<typename detail::ParameterTraits<Parameter>::ValueType> && ...);

    static constexpr auto CONTIGUOUS_COUNT =
        (std::size_t{} + ... + (detail::ParameterTraits<Parameter>::TYPE != detail::ParameterType::PLAIN));
    static constexpr auto CONTIGUOUS_FIXED_SIZE_COUNT =
        (std::size_t{} + ... + (detail::ParameterTraits<Parameter>::TYPE == detail::ParameterType::FIXED_SIZE));

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

    static constexpr auto make_index_sequence() noexcept { return std::make_index_sequence<sizeof...(Parameter)>{}; }
};
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_PARAMETERLISTTRAITS_HPP

// #include "cntgs/detail/parameterTraits.hpp"

// #include "cntgs/detail/reference.hpp"
// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_DETAIL_REFERENCE_HPP
#define CNTGS_DETAIL_REFERENCE_HPP

// #include "cntgs/detail/forward.hpp"

// #include "cntgs/detail/parameterTraits.hpp"

// #include "cntgs/detail/tuple.hpp"
// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_DETAIL_TUPLE_HPP
#define CNTGS_DETAIL_TUPLE_HPP

// #include "cntgs/detail/parameterTraits.hpp"

// #include "cntgs/detail/typeTraits.hpp"


#include <tuple>

namespace cntgs::detail
{
template <class... Parameter>
using ToTupleOfContiguousPointer = std::tuple<typename detail::ParameterTraits<Parameter>::PointerType...>;

template <bool IsConst, class... Parameter>
using ToTupleOfContiguousReferences =
    detail::ConditionalT<IsConst, std::tuple<typename detail::ParameterTraits<Parameter>::ConstReferenceType...>,
                         std::tuple<typename detail::ParameterTraits<Parameter>::ReferenceType...>>;
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_TUPLE_HPP


namespace std
{
template <std::size_t I, bool IsConst, class... Parameter>
struct tuple_element<I, ::cntgs::BasicContiguousReference<IsConst, Parameter...>>
    : std::tuple_element<I, ::cntgs::detail::ToTupleOfContiguousReferences<IsConst, Parameter...>>
{
};

template <bool IsConst, class... Parameter>
struct tuple_size<::cntgs::BasicContiguousReference<IsConst, Parameter...>>
    : std::integral_constant<std::size_t, sizeof...(Parameter)>
{
};
}  // namespace std

namespace cntgs
{
template <std::size_t I, bool IsConst, class... Parameter>
[[nodiscard]] constexpr std::tuple_element_t<I, cntgs::BasicContiguousReference<IsConst, Parameter...>> get(
    const cntgs::BasicContiguousReference<IsConst, Parameter...>& reference) noexcept;
}  // namespace cntgs

#endif  // CNTGS_DETAIL_REFERENCE_HPP

// #include "cntgs/detail/sizeGetter.hpp"
// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_DETAIL_SIZEGETTER_HPP
#define CNTGS_DETAIL_SIZEGETTER_HPP

// #include "cntgs/detail/array.hpp"

// #include "cntgs/detail/forward.hpp"

// #include "cntgs/detail/parameterTraits.hpp"

// #include "cntgs/detail/parameterType.hpp"

// #include "cntgs/detail/reference.hpp"

// #include "cntgs/detail/typeTraits.hpp"


#include <array>
#include <cstddef>
#include <tuple>

namespace cntgs::detail
{
template <class... Parameter>
class FixedSizeGetter
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

    template <class Type, std::size_t I, bool IsConst, class... Parameter>
    static constexpr auto get(
        [[maybe_unused]] const cntgs::BasicContiguousReference<IsConst, Parameter...>& tuple) noexcept
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

// #include "cntgs/detail/typeTraits.hpp"

// #include "cntgs/detail/vectorTraits.hpp"
// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_DETAIL_VECTORTRAITS_HPP
#define CNTGS_DETAIL_VECTORTRAITS_HPP

// #include "cntgs/detail/forward.hpp"

// #include "cntgs/detail/tuple.hpp"


#include <tuple>

namespace cntgs::detail
{
template <class... Parameter>
struct ContiguousVectorTraits
{
    using ReferenceType = cntgs::BasicContiguousReference<false, Parameter...>;
    using ConstReferenceType = cntgs::BasicContiguousReference<true, Parameter...>;
    using PointerType = detail::ToTupleOfContiguousPointer<Parameter...>;
};
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_VECTORTRAITS_HPP


#include <array>
#include <cstddef>
#include <limits>
#include <numeric>
#include <type_traits>

namespace cntgs::detail
{
struct DefaultAlignmentNeeds
{
    template <std::size_t>
    static constexpr auto VALUE = true;
};

struct IgnoreFirstAlignmentNeeds
{
    template <std::size_t I>
    static constexpr auto VALUE = I != 0;
};

template <std::size_t Alignment>
constexpr auto alignment_offset(std::size_t position) noexcept
{
    return detail::align<Alignment>(position) - position;
}

template <bool UseMove, class Type, class Source, class Target>
constexpr void construct_one_if_non_trivial([[maybe_unused]] Source&& source, [[maybe_unused]] const Target& target)
{
    using ValueType = typename detail::ParameterTraits<Type>::ValueType;
    if constexpr (UseMove && !std::is_trivially_move_constructible_v<ValueType>)
    {
        detail::ParameterTraits<Type>::uninitialized_move(source, target);
    }
    else if constexpr (!UseMove && !std::is_trivially_copy_constructible_v<ValueType>)
    {
        detail::ParameterTraits<Type>::uninitialized_copy(source, target);
    }
}

template <class, class...>
class ElementTraits;

template <std::size_t... I, class... Parameter>
class ElementTraits<std::index_sequence<I...>, Parameter...>
{
  private:
    using ListTraits = detail::ParameterListTraits<Parameter...>;
    using FixedSizesArray = typename ListTraits::FixedSizesArray;
    using ContiguousPointer = typename detail::ContiguousVectorTraits<Parameter...>::PointerType;
    using ContiguousReference = typename detail::ContiguousVectorTraits<Parameter...>::ReferenceType;
    using AlignmentNeeds = detail::ConditionalT<ListTraits::IS_FIXED_SIZE_OR_PLAIN, detail::IgnoreFirstAlignmentNeeds,
                                                detail::DefaultAlignmentNeeds>;
    using FixedSizeGetter = detail::FixedSizeGetter<Parameter...>;

    static constexpr auto SKIP = std::numeric_limits<std::size_t>::max();
    static constexpr auto MANUAL = SKIP - 1;

    template <template <class> class Predicate>
    static constexpr auto calculate_consecutive_indices() noexcept
    {
        std::array<std::size_t, sizeof...(Parameter)> consecutive_indices{((void)I, SKIP)...};
        [[maybe_unused]] std::size_t index{};
        (
            [&]
            {
                if constexpr (Predicate<typename detail::ParameterTraits<Parameter>::ValueType>::value)
                {
                    consecutive_indices[index] = I;
                }
                else
                {
                    index = I + 1;
                    consecutive_indices[I] = MANUAL;
                }
            }(),
            ...);
        return consecutive_indices;
    }

    static constexpr std::array CONSECUTIVE_TRIVIALLY_ASSIGNABLE_INDICES{
        calculate_consecutive_indices<std::is_trivially_copy_assignable>(),
        calculate_consecutive_indices<std::is_trivially_move_assignable>()};

    static constexpr auto CONSECUTIVE_TRIVIALLY_SWAPPABLE_INDICES{
        calculate_consecutive_indices<detail::IsTriviallySwappable>()};

    static constexpr auto CONSECUTIVE_EQUALITY_MEMCMPABLE_INDICES{
        calculate_consecutive_indices<detail::EqualityMemcmpCompatible>()};

    static constexpr auto CONSECUTIVE_LEXICOGRAPHICAL_MEMCMPABLE_INDICES{
        calculate_consecutive_indices<detail::LexicographicalMemcmpCompatibleT>()};

    template <std::size_t K, std::size_t L, bool IsLhsConst, bool IsRhsConst>
    static constexpr auto get_data_begin_and_end(
        const cntgs::BasicContiguousReference<IsLhsConst, Parameter...>& lhs,
        const cntgs::BasicContiguousReference<IsRhsConst, Parameter...>& rhs) noexcept
    {
        return std::tuple{ParameterTraitsAt<K>::data_begin(cntgs::get<K>(lhs)),
                          ParameterTraitsAt<L>::data_end(cntgs::get<L>(lhs)),
                          ParameterTraitsAt<K>::data_begin(cntgs::get<K>(rhs))};
    }

    template <bool IgnoreAliasing, class... Args>
    static std::byte* emplace_at(std::byte* CNTGS_RESTRICT address, const FixedSizesArray& fixed_sizes, Args&&... args)
    {
        ((address =
              detail::ParameterTraits<Parameter>::template store<AlignmentNeeds::template VALUE<I>, IgnoreAliasing>(
                  std::forward<Args>(args), address, FixedSizeGetter::template get<Parameter, I>(fixed_sizes))),
         ...);
        return address;
    }

  public:
    template <std::size_t K>
    using ParameterTraitsAt = typename ListTraits::template ParameterTraitsAt<K>;

    template <class... Args>
    CNTGS_RESTRICT_RETURN static std::byte* emplace_at(std::byte* CNTGS_RESTRICT address,
                                                       const FixedSizesArray& fixed_sizes, Args&&... args)
    {
        return ElementTraits::template emplace_at<true>(address, fixed_sizes, std::forward<Args>(args)...);
    }

    template <class... Args>
    static std::byte* emplace_at_aliased(std::byte* CNTGS_RESTRICT address, const FixedSizesArray& fixed_sizes,
                                         Args&&... args)
    {
        return ElementTraits::template emplace_at<false>(address, fixed_sizes, std::forward<Args>(args)...);
    }

    template <class AlignmentNeedsType = ElementTraits::AlignmentNeeds,
              class FixedSizeGetterType = ElementTraits::FixedSizeGetter, class FixedSizesType>
    static auto load_element_at(std::byte* CNTGS_RESTRICT address, const FixedSizesType& fixed_sizes) noexcept
    {
        ContiguousPointer result;
        ((std::tie(std::get<I>(result), address) = detail::ParameterTraits<Parameter>::template load<
              AlignmentNeedsType::template VALUE<I>, FixedSizeGetterType::template CAN_PROVIDE_SIZE<Parameter>>(
              address, FixedSizeGetterType::template get<Parameter, I>(fixed_sizes))),
         ...);
        return result;
    }

    template <std::size_t K>
    static constexpr auto get([[maybe_unused]] const std::array<std::size_t, sizeof...(I)>& fixed_sizes) noexcept
    {
        if constexpr (0 == K)
        {
            return std::size_t{};
        }
        else
        {
            return std::get<K - 1>(fixed_sizes);
        }
    }

    static constexpr auto calculate_offsets(const FixedSizesArray& fixed_sizes) noexcept
    {
        std::array<std::size_t, sizeof...(I)> result{};
        if constexpr (ListTraits::IS_FIXED_SIZE_OR_PLAIN)
        {
            ((std::get<I>(result) = detail::ParameterTraits<Parameter>::guaranteed_size_in_memory(
                                        FixedSizeGetter::template get<Parameter, I>(fixed_sizes)) +
                                    ElementTraits::get<I>(result) +
                                    detail::alignment_offset<detail::ParameterTraits<Parameter>::ALIGNMENT>(
                                        ElementTraits::get<I>(result))),
             ...);
        }
        else
        {
            ((std::get<I>(result) = detail::ParameterTraits<Parameter>::aligned_size_in_memory(
                                        FixedSizeGetter::template get<Parameter, I>(fixed_sizes)) +
                                    ElementTraits::get<I>(result) +
                                    detail::alignment_offset<detail::ParameterTraits<Parameter>::ALIGNMENT>(
                                        ElementTraits::get<I>(result))),
             ...);
        }
        return result;
    }

    static constexpr auto calculate_element_size2(const FixedSizesArray& fixed_sizes) noexcept
    {
        std::size_t result{};
        if constexpr (ListTraits::IS_FIXED_SIZE_OR_PLAIN)
        {
            ((result += detail::ParameterTraits<Parameter>::guaranteed_size_in_memory(
                            FixedSizeGetter::template get<Parameter, I>(fixed_sizes)) +
                        detail::alignment_offset<detail::ParameterTraits<Parameter>::ALIGNMENT>(result)),
             ...);
        }
        else
        {
            ((result += detail::ParameterTraits<Parameter>::aligned_size_in_memory(
                            FixedSizeGetter::template get<Parameter, I>(fixed_sizes)) +
                        detail::alignment_offset<detail::ParameterTraits<Parameter>::ALIGNMENT>(result)),
             ...);
        }
        return result + detail::alignment_offset<ParameterTraitsAt<0>::ALIGNMENT>(result);
    }

    static constexpr auto calculate_element_size(const FixedSizesArray& fixed_sizes) noexcept
    {
        const auto last_offset = ElementTraits::calculate_offsets(fixed_sizes).back();
        const auto result = last_offset + detail::alignment_offset<ParameterTraitsAt<0>::ALIGNMENT>(last_offset);
        auto a = calculate_element_size2(fixed_sizes);
        return result;
    }

    template <bool UseMove, bool IsConst>
    static constexpr void construct_if_non_trivial(const cntgs::BasicContiguousReference<IsConst, Parameter...>& source,
                                                   const ContiguousPointer& target)
    {
        (detail::construct_one_if_non_trivial<UseMove, Parameter>(cntgs::get<I>(source), std::get<I>(target)), ...);
    }

    template <bool UseMove, std::size_t K, bool IsLhsConst>
    static void assign_one(const cntgs::BasicContiguousReference<IsLhsConst, Parameter...>& source,
                           const ContiguousReference& target)
    {
        static constexpr auto INDEX = std::get<K>(std::get<UseMove>(CONSECUTIVE_TRIVIALLY_ASSIGNABLE_INDICES));
        if constexpr (INDEX == MANUAL)
        {
            if constexpr (UseMove)
            {
                ParameterTraitsAt<K>::move(cntgs::get<K>(source), cntgs::get<K>(target));
            }
            else
            {
                ParameterTraitsAt<K>::copy(cntgs::get<K>(source), cntgs::get<K>(target));
            }
        }
        else if constexpr (INDEX != SKIP)
        {
            const auto [source_start, source_end, target_start] =
                ElementTraits::template get_data_begin_and_end<K, INDEX>(source, target);
            std::memmove(target_start, source_start, source_end - source_start);
        }
    }

    template <bool UseMove, bool IsLhsConst>
    static void assign(const cntgs::BasicContiguousReference<IsLhsConst, Parameter...>& source,
                       const ContiguousReference& target)
    {
        (ElementTraits::template assign_one<UseMove, I>(source, target), ...);
    }

    template <std::size_t K>
    static void swap_one(const ContiguousReference& lhs, const ContiguousReference& rhs)
    {
        static constexpr auto INDEX = std::get<K>(CONSECUTIVE_TRIVIALLY_SWAPPABLE_INDICES);
        if constexpr (INDEX == MANUAL)
        {
            ParameterTraitsAt<K>::swap(cntgs::get<K>(lhs), cntgs::get<K>(rhs));
        }
        else if constexpr (INDEX != SKIP)
        {
            const auto [lhs_start, lhs_end, rhs_start] =
                ElementTraits::template get_data_begin_and_end<K, INDEX>(lhs, rhs);
            detail::trivial_swap_ranges(lhs_start, lhs_end, rhs_start);
        }
    }

    static void swap(const ContiguousReference& lhs, const ContiguousReference& rhs)
    {
        (ElementTraits::template swap_one<I>(lhs, rhs), ...);
    }

    template <std::size_t K, bool IsLhsConst, bool IsRhsConst>
    static constexpr auto equal_one(const cntgs::BasicContiguousReference<IsLhsConst, Parameter...>& lhs,
                                    const cntgs::BasicContiguousReference<IsRhsConst, Parameter...>& rhs)
    {
        constexpr auto INDEX = std::get<K>(CONSECUTIVE_EQUALITY_MEMCMPABLE_INDICES);
        if constexpr (INDEX == MANUAL)
        {
            return ParameterTraitsAt<K>::equal(cntgs::get<K>(lhs), cntgs::get<K>(rhs));
        }
        else if constexpr (INDEX != SKIP)
        {
            const auto [lhs_start, lhs_end, rhs_start] =
                ElementTraits::template get_data_begin_and_end<K, INDEX>(lhs, rhs);
            const auto rhs_end = ParameterTraitsAt<INDEX>::data_end(cntgs::get<INDEX>(rhs));
            return detail::trivial_equal(lhs_start, lhs_end, rhs_start, rhs_end);
        }
        else
        {
            return true;
        }
    }

    template <bool IsLhsConst, bool IsRhsConst>
    static constexpr auto equal(const cntgs::BasicContiguousReference<IsLhsConst, Parameter...>& lhs,
                                const cntgs::BasicContiguousReference<IsRhsConst, Parameter...>& rhs)
    {
        return (ElementTraits::template equal_one<I>(lhs, rhs) && ...);
    }

    template <std::size_t K, bool IsLhsConst, bool IsRhsConst>
    static constexpr auto lexicographical_compare_one(
        const cntgs::BasicContiguousReference<IsLhsConst, Parameter...>& lhs,
        const cntgs::BasicContiguousReference<IsRhsConst, Parameter...>& rhs)
    {
        constexpr auto INDEX = std::get<K>(CONSECUTIVE_LEXICOGRAPHICAL_MEMCMPABLE_INDICES);
        if constexpr (INDEX == MANUAL)
        {
            return ParameterTraitsAt<K>::lexicographical_compare(cntgs::get<K>(lhs), cntgs::get<K>(rhs));
        }
        else if constexpr (INDEX != SKIP)
        {
            const auto [lhs_start, lhs_end, rhs_start] =
                ElementTraits::template get_data_begin_and_end<K, INDEX>(lhs, rhs);
            const auto rhs_end = ParameterTraitsAt<INDEX>::data_end(cntgs::get<INDEX>(rhs));
            return detail::trivial_lexicographical_compare(lhs_start, lhs_end, rhs_start, rhs_end);
        }
        else
        {
            return true;
        }
    }

    template <bool IsLhsConst, bool IsRhsConst>
    static constexpr auto lexicographical_compare(const cntgs::BasicContiguousReference<IsLhsConst, Parameter...>& lhs,
                                                  const cntgs::BasicContiguousReference<IsRhsConst, Parameter...>& rhs)
    {
        return (ElementTraits::template lexicographical_compare_one<I>(lhs, rhs) && ...);
    }

    static void destruct(const ContiguousReference& reference) noexcept
    {
        (detail::ParameterTraits<Parameter>::destroy(cntgs::get<I>(reference)), ...);
    }
};

template <class... Parameter>
using ElementTraitsT = detail::ElementTraits<std::make_index_sequence<sizeof...(Parameter)>, Parameter...>;
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_ELEMENTTRAITS_HPP

// #include "cntgs/detail/forward.hpp"

// #include "cntgs/detail/memory.hpp"

// #include "cntgs/detail/parameterListTraits.hpp"

// #include "cntgs/detail/sizeGetter.hpp"

// #include "cntgs/detail/utility.hpp"

// #include "cntgs/detail/vectorTraits.hpp"

// #include "cntgs/reference.hpp"
// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_CNTGS_REFERENCE_HPP
#define CNTGS_CNTGS_REFERENCE_HPP

// #include "cntgs/detail/attributes.hpp"

// #include "cntgs/detail/elementTraits.hpp"

// #include "cntgs/detail/forward.hpp"

// #include "cntgs/detail/parameterListTraits.hpp"

// #include "cntgs/detail/reference.hpp"

// #include "cntgs/detail/tuple.hpp"

// #include "cntgs/detail/typeTraits.hpp"


#include <cstddef>
#include <cstring>
#include <tuple>

namespace cntgs
{
template <class... Parameter>
using ContiguousReference = cntgs::BasicContiguousReference<false, Parameter...>;

template <class... Parameter>
using ContiguousConstReference = cntgs::BasicContiguousReference<true, Parameter...>;

/// Reference type
template <bool IsConst, class... Parameter>
class BasicContiguousReference
{
  private:
    using ListTraits = detail::ParameterListTraits<Parameter...>;
    using ElementTraits = detail::ElementTraitsT<Parameter...>;
    using PointerTuple = detail::ToTupleOfContiguousPointer<Parameter...>;

    static constexpr auto IS_CONST = IsConst;

  public:
    PointerTuple tuple;

    BasicContiguousReference() = default;
    ~BasicContiguousReference() = default;

    BasicContiguousReference(const BasicContiguousReference&) = default;
    BasicContiguousReference(BasicContiguousReference&&) = default;

    template <bool OtherIsConst>
    /*implicit*/ constexpr BasicContiguousReference(
        const cntgs::BasicContiguousReference<OtherIsConst, Parameter...>& other) noexcept
        : tuple(other.tuple)
    {
    }

    template <class Allocator>
    /*implicit*/ constexpr BasicContiguousReference(
        const cntgs::BasicContiguousElement<Allocator, Parameter...>& other) noexcept
        : BasicContiguousReference(other.reference)
    {
    }

    template <class Allocator>
    /*implicit*/ constexpr BasicContiguousReference(
        cntgs::BasicContiguousElement<Allocator, Parameter...>& other) noexcept
        : BasicContiguousReference(other.reference)
    {
    }

    constexpr BasicContiguousReference& operator=(const BasicContiguousReference& other) noexcept(
        ListTraits::IS_NOTHROW_COPY_ASSIGNABLE)
    {
        this->assign(other);
        return *this;
    }

    template <bool OtherIsConst>
    constexpr BasicContiguousReference& operator=(const cntgs::BasicContiguousReference<OtherIsConst, Parameter...>&
                                                      other) noexcept(ListTraits::IS_NOTHROW_COPY_ASSIGNABLE)
    {
        this->assign(other);
        return *this;
    }

    template <class Allocator>
    constexpr BasicContiguousReference& operator=(const cntgs::BasicContiguousElement<Allocator, Parameter...>&
                                                      other) noexcept(ListTraits::IS_NOTHROW_COPY_ASSIGNABLE)
    {
        this->assign(other.reference);
        return *this;
    }

    constexpr BasicContiguousReference& operator=(BasicContiguousReference&& other) noexcept(
        ListTraits::IS_NOTHROW_MOVE_ASSIGNABLE)
    {
        this->assign(other);
        return *this;
    }

    template <bool OtherIsConst>
    constexpr BasicContiguousReference& operator=(cntgs::BasicContiguousReference<OtherIsConst, Parameter...>&&
                                                      other) noexcept(OtherIsConst
                                                                          ? ListTraits::IS_NOTHROW_COPY_ASSIGNABLE
                                                                          : ListTraits::IS_NOTHROW_MOVE_ASSIGNABLE)
    {
        this->assign(other);
        return *this;
    }

    template <class Allocator>
    constexpr BasicContiguousReference& operator=(
        cntgs::BasicContiguousElement<Allocator, Parameter...>&& other) noexcept(ListTraits::IS_NOTHROW_MOVE_ASSIGNABLE)
    {
        this->assign(other.reference);
        return *this;
    }

    [[nodiscard]] constexpr std::size_t size_in_bytes() const noexcept { return this->data_end() - this->data_begin(); }

    [[nodiscard]] constexpr auto data_begin() const noexcept
    {
        return ElementTraits::template ParameterTraitsAt<0>::data_begin(cntgs::get<0>(*this));
    }

    [[nodiscard]] constexpr auto data_end() const noexcept
    {
        return ElementTraits::template ParameterTraitsAt<sizeof...(Parameter) - 1>::data_end(
            cntgs::get<sizeof...(Parameter) - 1>(*this));
    }

    template <bool OtherIsConst>
    [[nodiscard]] constexpr auto operator==(const cntgs::BasicContiguousReference<OtherIsConst, Parameter...>& other)
        const noexcept(ListTraits::IS_NOTHROW_EQUALITY_COMPARABLE)
    {
        return ElementTraits::equal(*this, other);
    }

    template <class Allocator>
    [[nodiscard]] constexpr auto operator==(const cntgs::BasicContiguousElement<Allocator, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTHROW_EQUALITY_COMPARABLE)
    {
        return *this == other.reference;
    }

    template <bool OtherIsConst>
    [[nodiscard]] constexpr auto operator!=(const cntgs::BasicContiguousReference<OtherIsConst, Parameter...>& other)
        const noexcept(ListTraits::IS_NOTHROW_EQUALITY_COMPARABLE)
    {
        return !(*this == other);
    }

    template <class Allocator>
    [[nodiscard]] constexpr auto operator!=(const cntgs::BasicContiguousElement<Allocator, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTHROW_EQUALITY_COMPARABLE)
    {
        return !(*this == other.reference);
    }

    template <bool OtherIsConst>
    [[nodiscard]] constexpr auto operator<(const cntgs::BasicContiguousReference<OtherIsConst, Parameter...>& other)
        const noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return ElementTraits::lexicographical_compare(*this, other);
    }

    template <class Allocator>
    [[nodiscard]] constexpr auto operator<(const cntgs::BasicContiguousElement<Allocator, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return *this < other.reference;
    }

    template <bool OtherIsConst>
    [[nodiscard]] constexpr auto operator<=(const cntgs::BasicContiguousReference<OtherIsConst, Parameter...>& other)
        const noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return !(other < *this);
    }

    template <class Allocator>
    [[nodiscard]] constexpr auto operator<=(const cntgs::BasicContiguousElement<Allocator, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return !(other.reference < *this);
    }

    template <bool OtherIsConst>
    [[nodiscard]] constexpr auto operator>(const cntgs::BasicContiguousReference<OtherIsConst, Parameter...>& other)
        const noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return other < *this;
    }

    template <class Allocator>
    [[nodiscard]] constexpr auto operator>(const cntgs::BasicContiguousElement<Allocator, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return other.reference < *this;
    }

    template <bool OtherIsConst>
    [[nodiscard]] constexpr auto operator>=(const cntgs::BasicContiguousReference<OtherIsConst, Parameter...>& other)
        const noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return !(*this < other);
    }

    template <class Allocator>
    [[nodiscard]] constexpr auto operator>=(const cntgs::BasicContiguousElement<Allocator, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return !(*this < other.reference);
    }

  private:
    friend cntgs::BasicContiguousReference<!IsConst, Parameter...>;

    template <class, class...>
    friend class BasicContiguousVector;

    template <class, class...>
    friend class BasicContiguousElement;

    template <bool, class, class...>
    friend class ContiguousVectorIterator;

    constexpr explicit BasicContiguousReference(const PointerTuple& tuple) noexcept : tuple(tuple) {}

    template <class Reference>
    void assign(Reference& other) const
    {
        static constexpr auto USE_MOVE = !std::is_const_v<Reference> && !Reference::IS_CONST;
        ElementTraits::template assign<USE_MOVE>(other, *this);
    }
};

template <bool IsConst, class... Parameter>
constexpr void swap(const cntgs::BasicContiguousReference<IsConst, Parameter...>& lhs,
                    const cntgs::BasicContiguousReference<IsConst, Parameter...>&
                        rhs) noexcept(detail::ParameterListTraits<Parameter...>::IS_NOTHROW_SWAPPABLE)
{
    detail::ElementTraitsT<Parameter...>::swap(rhs, lhs);
}

template <std::size_t I, bool IsConst, class... Parameter>
[[nodiscard]] constexpr std::tuple_element_t<I, cntgs::BasicContiguousReference<IsConst, Parameter...>> get(
    const cntgs::BasicContiguousReference<IsConst, Parameter...>& reference) noexcept
{
    if constexpr (IsConst)
    {
        return detail::as_const_ref(std::get<I>(reference.tuple));
    }
    else
    {
        return detail::as_ref(std::get<I>(reference.tuple));
    }
}
}  // namespace cntgs

#endif  // CNTGS_CNTGS_REFERENCE_HPP


#include <cstddef>
#include <cstring>
#include <memory>
#include <tuple>

namespace cntgs
{
template <class... Parameter>
using ContiguousElement = cntgs::BasicContiguousElement<std::allocator<std::byte>, Parameter...>;

template <class Allocator, class... Parameter>
class BasicContiguousElement
{
  private:
    using ListTraits = detail::ParameterListTraits<Parameter...>;
    using VectorTraits = detail::ContiguousVectorTraits<Parameter...>;
    using ElementTraits = detail::ElementTraitsT<Parameter...>;
    using AllocatorTraits = std::allocator_traits<Allocator>;
    using StorageElementType = detail::AlignedByte<ElementTraits::template ParameterTraitsAt<0>::ALIGNMENT>;
    using StorageType = detail::AllocatorAwarePointer<
        typename std::allocator_traits<Allocator>::template rebind_alloc<StorageElementType>>;
    using Reference = typename VectorTraits::ReferenceType;

  public:
    using allocator_type = Allocator;

    StorageType memory;
    Reference reference;

    template <bool IsConst>
    /*implicit*/ BasicContiguousElement(const cntgs::BasicContiguousReference<IsConst, Parameter...>& other,
                                        const allocator_type& allocator = {})
        : memory(other.size_in_bytes(), allocator), reference(this->store_and_load(other, other.size_in_bytes()))
    {
    }

    template <bool IsConst>
    /*implicit*/ BasicContiguousElement(cntgs::BasicContiguousReference<IsConst, Parameter...>&& other,
                                        const allocator_type& allocator = {})
        : memory(other.size_in_bytes(), allocator), reference(this->store_and_load(other, other.size_in_bytes()))
    {
    }

    /*implicit*/ BasicContiguousElement(const BasicContiguousElement& other)
        : memory(other.memory), reference(this->store_and_load(other.reference, other.reference.size_in_bytes()))
    {
    }

    template <class OtherAllocator>
    explicit BasicContiguousElement(const BasicContiguousElement<OtherAllocator, Parameter...>& other)
        : memory(other.memory), reference(this->store_and_load(other.reference, other.reference.size_in_bytes()))
    {
    }

    template <class OtherAllocator>
    BasicContiguousElement(const BasicContiguousElement<OtherAllocator, Parameter...>& other,
                           const allocator_type& allocator)
        : memory(other.reference.size_in_bytes(), allocator),
          reference(this->store_and_load(other.reference, other.reference.size_in_bytes()))
    {
    }

    BasicContiguousElement(BasicContiguousElement&&) = default;

    template <class OtherAllocator>
    constexpr explicit BasicContiguousElement(BasicContiguousElement<OtherAllocator, Parameter...>&& other) noexcept
        : memory(std::move(other.memory)), reference(std::move(other.reference))
    {
    }

    template <class OtherAllocator>
    constexpr BasicContiguousElement(
        BasicContiguousElement<OtherAllocator, Parameter...>&& other,
        const allocator_type& allocator) noexcept(detail::AreEqualityComparable<allocator_type, OtherAllocator>::value&&
                                                      AllocatorTraits::is_always_equal::value)
        : memory(this->acquire_memory(other, allocator)), reference(this->acquire_reference(other, allocator))
    {
    }

    ~BasicContiguousElement() noexcept { this->destruct(); }

    BasicContiguousElement& operator=(const BasicContiguousElement& other)
    {
        if (this != std::addressof(other))
        {
            this->copy_assign(other);
        }
        return *this;
    }

    BasicContiguousElement& operator=(BasicContiguousElement&& other) noexcept(
        AllocatorTraits::is_always_equal::value || AllocatorTraits::propagate_on_container_move_assignment::value)
    {
        if (this != std::addressof(other))
        {
            this->move_assign(std::move(other));
        }
        return *this;
    }

    template <bool IsConst>
    constexpr BasicContiguousElement& operator=(const cntgs::BasicContiguousReference<IsConst, Parameter...>&
                                                    other) noexcept(ListTraits::IS_NOTHROW_COPY_ASSIGNABLE)
    {
        this->reference = other;
        return *this;
    }

    template <bool IsConst>
    constexpr BasicContiguousElement& operator=(cntgs::BasicContiguousReference<IsConst, Parameter...>&&
                                                    other) noexcept(IsConst ? ListTraits::IS_NOTHROW_COPY_ASSIGNABLE
                                                                            : ListTraits::IS_NOTHROW_MOVE_ASSIGNABLE)
    {
        this->reference = std::move(other);
        return *this;
    }

    [[nodiscard]] constexpr allocator_type get_allocator() const noexcept { return this->memory.get_allocator(); }

    template <bool IsConst>
    [[nodiscard]] constexpr auto operator==(const cntgs::BasicContiguousReference<IsConst, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTHROW_EQUALITY_COMPARABLE)
    {
        return this->reference == other;
    }

    [[nodiscard]] constexpr auto operator==(const BasicContiguousElement& other) const
        noexcept(ListTraits::IS_NOTHROW_EQUALITY_COMPARABLE)
    {
        return this->reference == other.reference;
    }

    template <bool IsConst>
    [[nodiscard]] constexpr auto operator!=(const cntgs::BasicContiguousReference<IsConst, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTHROW_EQUALITY_COMPARABLE)
    {
        return !(this->reference == other);
    }

    [[nodiscard]] constexpr auto operator!=(const BasicContiguousElement& other) const
        noexcept(ListTraits::IS_NOTHROW_EQUALITY_COMPARABLE)
    {
        return !(this->reference == other.reference);
    }

    template <bool IsConst>
    [[nodiscard]] constexpr auto operator<(const cntgs::BasicContiguousReference<IsConst, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return this->reference < other;
    }

    [[nodiscard]] constexpr auto operator<(const BasicContiguousElement& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return this->reference < other.reference;
    }

    template <bool IsConst>
    [[nodiscard]] constexpr auto operator<=(const cntgs::BasicContiguousReference<IsConst, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return !(other < this->reference);
    }

    [[nodiscard]] constexpr auto operator<=(const BasicContiguousElement& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return !(other.reference < this->reference);
    }

    template <bool IsConst>
    [[nodiscard]] constexpr auto operator>(const cntgs::BasicContiguousReference<IsConst, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return other < this->reference;
    }

    [[nodiscard]] constexpr auto operator>(const BasicContiguousElement& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return other.reference < this->reference;
    }

    template <bool IsConst>
    [[nodiscard]] constexpr auto operator>=(const cntgs::BasicContiguousReference<IsConst, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return !(this->reference < other);
    }

    [[nodiscard]] constexpr auto operator>=(const BasicContiguousElement& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return !(this->reference < other.reference);
    }

  private:
    template <class SourceReference>
    auto store_and_load(SourceReference& source, std::size_t memory_size) const
    {
        return this->store_and_load(source, memory_size, this->memory_begin());
    }

    template <class SourceReference>
    auto store_and_load(SourceReference& source, std::size_t memory_size, std::byte* target_memory) const
    {
        static constexpr auto USE_MOVE = !std::is_const_v<SourceReference> && !SourceReference::IS_CONST;
        std::memcpy(target_memory, source.data_begin(), memory_size);
        auto target =
            ElementTraits::template load_element_at<detail::IgnoreFirstAlignmentNeeds,
                                                    detail::ContiguousReferenceSizeGetter>(target_memory, source);
        ElementTraits::template construct_if_non_trivial<USE_MOVE>(source, target);
        return Reference{target};
    }

    template <class OtherAllocator>
    auto acquire_memory(BasicContiguousElement<OtherAllocator, Parameter...>& other,
                        [[maybe_unused]] const allocator_type& allocator) const
    {
        if constexpr (detail::AreEqualityComparable<allocator_type, OtherAllocator>::value)
        {
            if constexpr (AllocatorTraits::is_always_equal::value)
            {
                return std::move(other.memory);
            }
            else
            {
                if (allocator == other.memory.get_allocator())
                {
                    return std::move(other.memory);
                }
                return StorageType(other.memory.size(), allocator);
            }
        }
        else
        {
            return StorageType(other.memory.size(), allocator);
        }
    }

    template <class OtherAllocator>
    auto acquire_reference(BasicContiguousElement<OtherAllocator, Parameter...>& other,
                           [[maybe_unused]] const allocator_type& allocator) const
    {
        if constexpr (detail::AreEqualityComparable<allocator_type, OtherAllocator>::value)
        {
            if constexpr (AllocatorTraits::is_always_equal::value)
            {
                return std::move(other.reference);
            }
            else
            {
                if (allocator == other.memory.get_allocator())
                {
                    return std::move(other.reference);
                }
                return this->store_and_load(other.reference, other.memory.size());
            }
        }
        else
        {
            return this->store_and_load(other.reference, other.memory.size());
        }
    }

    auto memory_begin() const noexcept { return BasicContiguousElement::memory_begin(this->memory); }

    static auto memory_begin(const StorageType& memory) noexcept
    {
        return detail::assume_aligned<ElementTraits::template ParameterTraitsAt<0>::ALIGNMENT>(
            reinterpret_cast<std::byte*>(memory.get()));
    }

    template <class OtherAllocator>
    void copy_assign(const BasicContiguousElement<OtherAllocator, Parameter...>& other)
    {
        if (this != std::addressof(other))
        {
            if constexpr (ListTraits::IS_FIXED_SIZE_OR_PLAIN &&
                          (!AllocatorTraits::propagate_on_container_copy_assignment::value ||
                           AllocatorTraits::is_always_equal::value))
            {
                this->reference = other.reference;
                this->memory.propagate_on_container_copy_assignment(other.memory);
            }
            else
            {
                this->destruct();
                this->memory = other.memory;
                this->store_and_construct_reference_inplace(other.reference, other.memory.size());
            }
        }
    }

    template <class OtherAllocator>
    constexpr void steal(BasicContiguousElement<OtherAllocator, Parameter...>&& other) noexcept
    {
        this->destruct();
        this->memory = std::move(other.memory);
        this->reference.tuple = std::move(other.reference.tuple);
    }

    template <class OtherAllocator>
    constexpr void move_assign(BasicContiguousElement<OtherAllocator, Parameter...>&& other)
    {
        if constexpr (AllocatorTraits::is_always_equal::value ||
                      AllocatorTraits::propagate_on_container_move_assignment::value)
        {
            this->steal(std::move(other));
        }
        else
        {
            if (this->get_allocator() == other.get_allocator())
            {
                this->steal(std::move(other));
            }
            else
            {
                if constexpr (ListTraits::IS_FIXED_SIZE_OR_PLAIN)
                {
                    this->reference = std::move(other.reference);
                    this->memory.propagate_on_container_move_assignment(other.memory);
                }
                else
                {
                    const auto other_size_in_bytes = other.reference.size_in_bytes();
                    if (other_size_in_bytes > this->memory.size())
                    {
                        // allocate memory first because it might throw
                        StorageType new_memory{other.memory.size(), this->get_allocator()};
                        this->destruct();
                        this->reference.tuple = this->store_and_load(other.reference, other_size_in_bytes,
                                                                     BasicContiguousElement::memory_begin(new_memory))
                                                    .tuple;
                        this->memory = std::move(new_memory);
                    }
                    else
                    {
                        this->destruct();
                        this->store_and_construct_reference_inplace(other.reference, other_size_in_bytes);
                    }
                }
            }
        }
    }

    template <class SourceReference>
    void store_and_construct_reference_inplace(SourceReference& other, std::size_t memory_size)
    {
        this->reference.tuple = this->store_and_load(other, memory_size).tuple;
    }

    void destruct() noexcept
    {
        if constexpr (!ListTraits::IS_TRIVIALLY_DESTRUCTIBLE)
        {
            if (this->memory)
            {
                ElementTraits::destruct(this->reference);
            }
        }
    }
};

template <class Allocator, class... T>
constexpr void swap(cntgs::BasicContiguousElement<Allocator, T...>& lhs,
                    cntgs::BasicContiguousElement<Allocator, T...>& rhs) noexcept
{
    detail::swap(lhs.memory, rhs.memory);
    std::swap(lhs.reference.tuple, rhs.reference.tuple);
}

template <std::size_t I, class Allocator, class... Parameter>
[[nodiscard]] constexpr decltype(auto) get(cntgs::BasicContiguousElement<Allocator, Parameter...>& element) noexcept
{
    return cntgs::get<I>(element.reference);
}

template <std::size_t I, class Allocator, class... Parameter>
[[nodiscard]] constexpr decltype(auto) get(
    const cntgs::BasicContiguousElement<Allocator, Parameter...>& element) noexcept
{
    return detail::as_const(cntgs::get<I>(element.reference));
}

template <std::size_t I, class Allocator, class... Parameter>
[[nodiscard]] constexpr decltype(auto) get(cntgs::BasicContiguousElement<Allocator, Parameter...>&& element) noexcept
{
    return std::move(cntgs::get<I>(element.reference));
}

template <std::size_t I, class Allocator, class... Parameter>
[[nodiscard]] constexpr decltype(auto) get(
    const cntgs::BasicContiguousElement<Allocator, Parameter...>&& element) noexcept
{
    return detail::as_const(std::move(cntgs::get<I>(element.reference)));
}
}  // namespace cntgs

namespace std
{
template <std::size_t I, class Allocator, class... Parameter>
struct tuple_element<I, ::cntgs::BasicContiguousElement<Allocator, Parameter...>>
    : std::tuple_element<I, decltype(::cntgs::BasicContiguousElement<Allocator, Parameter...>::reference)>
{
};

template <class Allocator, class... Parameter>
struct tuple_size<::cntgs::BasicContiguousElement<Allocator, Parameter...>>
    : std::integral_constant<std::size_t, sizeof...(Parameter)>
{
};
}  // namespace std

#endif  // CNTGS_CNTGS_ELEMENT_HPP

// #include "cntgs/iterator.hpp"
// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_CNTGS_ITERATOR_HPP
#define CNTGS_CNTGS_ITERATOR_HPP

// #include "cntgs/detail/elementLocator.hpp"
// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_DETAIL_ELEMENTLOCATOR_HPP
#define CNTGS_DETAIL_ELEMENTLOCATOR_HPP

// #include "cntgs/detail/array.hpp"

// #include "cntgs/detail/elementTraits.hpp"

// #include "cntgs/detail/memory.hpp"

// #include "cntgs/detail/parameterListTraits.hpp"

// #include "cntgs/detail/parameterTraits.hpp"

// #include "cntgs/detail/sizeGetter.hpp"

// #include "cntgs/detail/typeTraits.hpp"

// #include "cntgs/detail/utility.hpp"

// #include "cntgs/detail/vectorTraits.hpp"


#include <algorithm>
#include <array>
#include <type_traits>

namespace cntgs::detail
{
template <class Locator>
auto move_elements_forward(std::size_t from, std::size_t to, std::byte* memory_begin, const Locator& locator) noexcept
{
    const auto target = locator.element_address(to, memory_begin);
    const auto source = locator.element_address(from, memory_begin);
    const auto count = static_cast<std::size_t>(locator.data_end() - source);
    std::memmove(target, source, count);
    return source - target;
}

class BaseElementLocator
{
  protected:
    static constexpr auto RESERVED_BYTES_PER_ELEMENT = sizeof(std::byte*);

    std::byte** last_element_address{};
    std::byte* last_element{};

    BaseElementLocator() = default;

    constexpr BaseElementLocator(std::byte** last_element_address, std::byte* last_element) noexcept
        : last_element_address(last_element_address), last_element(last_element)
    {
    }

  public:
    static constexpr auto reserved_bytes(std::size_t element_count) noexcept
    {
        return element_count * RESERVED_BYTES_PER_ELEMENT;
    }

    bool empty(std::byte* memory_begin) const noexcept
    {
        return this->last_element_address == reinterpret_cast<std::byte**>(memory_begin);
    }

    std::size_t size(std::byte* memory_begin) const noexcept
    {
        return this->last_element_address - reinterpret_cast<std::byte**>(memory_begin);
    }

    static auto element_address(std::size_t index, std::byte* memory_begin) noexcept
    {
        const auto element_addresses_begin = reinterpret_cast<std::byte**>(memory_begin);
        return element_addresses_begin[index];
    }

    constexpr auto data_end() const noexcept { return last_element; }

    void resize(std::size_t new_size, std::byte* memory_begin) noexcept
    {
        this->last_element = BaseElementLocator::element_address(new_size, memory_begin);
        this->last_element_address = reinterpret_cast<std::byte**>(memory_begin) + new_size;
    }

    void move_elements_forward(std::size_t from, std::size_t to, std::byte* memory_begin) const noexcept
    {
        const auto diff = detail::move_elements_forward(from, to, memory_begin, *this);
        const auto first_element_address = reinterpret_cast<std::byte**>(memory_begin);
        std::transform(first_element_address + from, this->last_element_address, first_element_address + to,
                       [&](auto address)
                       {
                           return address - diff;
                       });
    }
};

template <bool IsAllFixedSize, class... Parameter>
class ElementLocator : public BaseElementLocator
{
  private:
    using ElementTraits = detail::ElementTraitsT<Parameter...>;
    using FixedSizesArray = typename detail::ParameterListTraits<Parameter...>::FixedSizesArray;

  public:
    ElementLocator() = default;

    ElementLocator(std::size_t max_element_count, std::byte* memory_begin, const FixedSizesArray&) noexcept
        : BaseElementLocator(reinterpret_cast<std::byte**>(memory_begin),
                             ElementLocator::calculate_element_start(max_element_count, memory_begin))
    {
    }

    ElementLocator(ElementLocator& old_locator, std::size_t old_max_element_count, std::byte* old_memory_begin,
                   std::size_t new_max_element_count, std::byte* new_memory_begin) noexcept
    {
        this->trivially_copy_into(old_locator, old_max_element_count, old_memory_begin, new_max_element_count,
                                  new_memory_begin);
    }

    template <class... Args>
    void emplace_back(const FixedSizesArray& fixed_sizes, Args&&... args)
    {
        *this->last_element_address = last_element;
        ++this->last_element_address;
        last_element = ElementTraits::emplace_at(last_element, fixed_sizes, std::forward<Args>(args)...);
    }

    template <class... Args>
    static void emplace_at(std::size_t index, std::byte* memory_begin, const FixedSizesArray& fixed_sizes,
                           Args&&... args)
    {
        const auto element_addresses_begin = reinterpret_cast<std::byte**>(memory_begin);
        element_addresses_begin[index + 1] =
            ElementTraits::emplace_at_aliased(element_addresses_begin[index], fixed_sizes, std::forward<Args>(args)...);
    }

    template <class AlignmentNeedsType = detail::DefaultAlignmentNeeds,
              class FixedSizeGetterType = detail::FixedSizeGetter<Parameter...>, class FixedSizesType = FixedSizesArray>
    static auto load_element_at(std::size_t index, std::byte* memory_begin, const FixedSizesType& fixed_sizes) noexcept
    {
        return ElementTraits::template load_element_at<AlignmentNeedsType, FixedSizeGetterType>(
            ElementLocator::element_address(index, memory_begin), fixed_sizes);
    }

    void trivially_copy_into(std::size_t old_max_element_count, std::byte* CNTGS_RESTRICT old_memory_begin,
                             std::size_t new_max_element_count, std::byte* CNTGS_RESTRICT new_memory_begin) noexcept
    {
        this->trivially_copy_into(*this, old_max_element_count, old_memory_begin, new_max_element_count,
                                  new_memory_begin);
    }

    static constexpr auto calculate_new_memory_size(std::size_t max_element_count, std::size_t varying_size_bytes,
                                                    const FixedSizesArray& fixed_sizes) noexcept
    {
        constexpr auto ALIGNMENT_OVERHEAD = ElementTraits::template ParameterTraitsAt<0>::ALIGNMENT - 1;
        return varying_size_bytes + ElementTraits::calculate_element_size(fixed_sizes) * max_element_count +
               ElementLocator::reserved_bytes(max_element_count) + ALIGNMENT_OVERHEAD;
    }

  private:
    void trivially_copy_into(ElementLocator& old_locator, std::size_t old_max_element_count,
                             std::byte* CNTGS_RESTRICT old_memory_begin, std::size_t new_max_element_count,
                             std::byte* CNTGS_RESTRICT new_memory_begin) noexcept
    {
        const auto new_start = ElementLocator::calculate_element_start(new_max_element_count, new_memory_begin);
        const auto old_start = ElementLocator::calculate_element_start(old_max_element_count, old_memory_begin);
        const auto size_diff = std::distance(new_memory_begin, new_start) - std::distance(old_memory_begin, old_start);
        const auto new_last_element_address = ElementLocator::copy_element_addresses(
            new_memory_begin, old_memory_begin, old_locator.last_element_address, size_diff);
        const auto old_used_memory_size = std::distance(old_start, old_locator.last_element);
        std::memcpy(new_start, old_start, old_used_memory_size);
        this->last_element_address = new_last_element_address;
        this->last_element = new_memory_begin + std::distance(old_memory_begin, old_locator.last_element) + size_diff;
    }

    static auto copy_element_addresses(std::byte* new_memory_begin, std::byte* old_memory_begin,
                                       std::byte** old_last_element_address, std::ptrdiff_t size_diff) noexcept
    {
        auto new_last_element_address = reinterpret_cast<std::byte**>(new_memory_begin);
        std::for_each(reinterpret_cast<std::byte**>(old_memory_begin), old_last_element_address,
                      [&](std::byte* element)
                      {
                          *new_last_element_address =
                              new_memory_begin + std::distance(old_memory_begin, element) + size_diff;
                          ++new_last_element_address;
                      });
        return new_last_element_address;
    }

    static constexpr auto calculate_element_start(std::size_t max_element_count, std::byte* memory_begin) noexcept
    {
        return detail::align<ElementTraits::template ParameterTraitsAt<0>::ALIGNMENT>(
            memory_begin + BaseElementLocator::reserved_bytes(max_element_count));
    }
};

class BaseAllFixedSizeElementLocator
{
  protected:
    std::size_t element_count{};
    std::size_t stride{};
    std::byte* start{};

    BaseAllFixedSizeElementLocator() = default;

    constexpr BaseAllFixedSizeElementLocator(std::size_t element_count, std::size_t stride, std::byte* start) noexcept
        : element_count(element_count), stride(stride), start(start)
    {
    }

  public:
    static constexpr auto reserved_bytes(std::size_t) noexcept { return std::size_t{}; }

    constexpr bool empty(const std::byte*) const noexcept { return this->element_count == std::size_t{}; }

    constexpr std::size_t size(const std::byte*) const noexcept { return this->element_count; }

    constexpr auto element_address(std::size_t index, const std::byte*) const noexcept
    {
        return this->start + this->stride * index;
    }

    constexpr auto data_end() const noexcept { return this->start + this->stride * this->element_count; }

    constexpr void resize(std::size_t new_size, const std::byte*) noexcept { this->element_count = new_size; }

    void move_elements_forward(std::size_t from, std::size_t to, const std::byte*) const noexcept
    {
        detail::move_elements_forward(from, to, {}, *this);
    }
};

template <class... Parameter>
class ElementLocator<true, Parameter...> : public BaseAllFixedSizeElementLocator
{
  private:
    using ElementTraits = detail::ElementTraitsT<Parameter...>;
    using FixedSizesArray = typename detail::ParameterListTraits<Parameter...>::FixedSizesArray;
    using ContiguousPointer = typename detail::ContiguousVectorTraits<Parameter...>::PointerType;
    using OffsetArray = std::array<std::size_t, sizeof...(Parameter) - 1>;

    OffsetArray offsets{};

  public:
    ElementLocator() = default;

    constexpr ElementLocator(std::size_t, std::byte* memory_begin, const FixedSizesArray& fixed_sizes) noexcept
        : BaseAllFixedSizeElementLocator({}, ElementTraits::calculate_element_size(fixed_sizes),
                                         ElementLocator::calculate_element_start(memory_begin))
    {
        this->offsets =
            detail::convert_array_to_size<sizeof...(Parameter) - 1>(ElementTraits::calculate_offsets(fixed_sizes));
    }

    ElementLocator(ElementLocator& old_locator, std::size_t, const std::byte*, std::size_t,
                   std::byte* new_memory_begin) noexcept
        : BaseAllFixedSizeElementLocator(old_locator.element_count, old_locator.stride, {})
    {
        this->offsets = old_locator.offsets;
        this->trivially_copy_into(old_locator, new_memory_begin);
    }

    template <class... Args>
    void emplace_back(const FixedSizesArray& fixed_sizes, Args&&... args)
    {
        auto last_element = this->element_address(this->element_count, {});
        ++this->element_count;
        ElementTraits::emplace_at(last_element, fixed_sizes, std::forward<Args>(args)...);
    }

    template <class... Args>
    void emplace_at(std::size_t index, const std::byte*, const FixedSizesArray& fixed_sizes, Args&&... args)
    {
        ElementTraits::emplace_at_aliased(this->element_address(index, {}), fixed_sizes, std::forward<Args>(args)...);
    }

    template <class AlignmentNeedsType = detail::DefaultAlignmentNeeds,
              class FixedSizeGetterType = detail::FixedSizeGetter<Parameter...>, class FixedSizesType = FixedSizesArray>
    auto load_element_at(std::size_t index, const std::byte*, const FixedSizesType& fixed_sizes) const noexcept
    {
        return ElementLocator::template load_element_at<FixedSizeGetterType>(
            this->element_address(index, {}), fixed_sizes, std::make_index_sequence<sizeof...(Parameter)>{});
    }

    void trivially_copy_into(std::size_t, const std::byte*, std::size_t, std::byte* new_memory_begin) noexcept
    {
        this->trivially_copy_into(*this, new_memory_begin);
    }

    constexpr auto calculate_new_memory_size(std::size_t max_element_count, std::size_t varying_size_bytes,
                                             const FixedSizesArray&) noexcept
    {
        constexpr auto ALIGNMENT_OVERHEAD = ElementTraits::template ParameterTraitsAt<0>::ALIGNMENT - 1;
        return varying_size_bytes + this->stride * max_element_count +
               ElementLocator::reserved_bytes(max_element_count) + ALIGNMENT_OVERHEAD;
    }

  private:
    template <class FixedSizeGetterType, class FixedSizesType, std::size_t... I>
    auto load_element_at(std::byte* CNTGS_RESTRICT address, const FixedSizesType& fixed_sizes,
                         std::index_sequence<I...>) const noexcept
    {
        ContiguousPointer result;
        ((std::tie(std::get<I>(result), std::ignore) = detail::ParameterTraits<Parameter>::template load<
              false, FixedSizeGetterType::template CAN_PROVIDE_SIZE<Parameter>>(
              address + ElementLocator::get<I>(this->offsets),
              FixedSizeGetterType::template get<Parameter, I>(fixed_sizes))),
         ...);
        return result;
    }

    template <std::size_t I>
    static constexpr auto get([[maybe_unused]] const OffsetArray& array) noexcept
    {
        if constexpr (0 == I)
        {
            return std::size_t{};
        }
        else
        {
            return std::get<I - 1>(array);
        }
    }

    //     template <class FixedSizeGetterType, class FixedSizesType, std::size_t... I>
    // auto load_element_at(std::byte* CNTGS_RESTRICT address, const FixedSizesType& fixed_sizes,
    //                      std::index_sequence<I...>) const noexcept
    // {
    //     return ContiguousPointer{detail::ParameterTraits<Parameter>::load_(
    //         address + std::get<I>(this->offsets), FixedSizeGetterType::template get<Parameter, I>(fixed_sizes))...};
    // }

    void trivially_copy_into(const ElementLocator& old_locator, std::byte* new_memory_begin) noexcept
    {
        const auto new_start = ElementLocator::calculate_element_start(new_memory_begin);
        std::memcpy(new_start, old_locator.start, old_locator.element_count * old_locator.stride);
        this->start = new_start;
    }

    static constexpr auto calculate_element_start(std::byte* memory_begin) noexcept
    {
        return detail::align<ElementTraits::template ParameterTraitsAt<0>::ALIGNMENT>(memory_begin);
    }
};

template <class... Parameter>
using ElementLocatorT =
    detail::ElementLocator<detail::ParameterListTraits<Parameter...>::IS_FIXED_SIZE_OR_PLAIN, Parameter...>;

template <class... Parameter>
class ElementLocatorAndFixedSizes
    : private detail::EmptyBaseOptimization<typename detail::ParameterListTraits<Parameter...>::FixedSizesArray>
{
  private:
    static constexpr auto HAS_FIXED_SIZES = detail::ParameterListTraits<Parameter...>::CONTIGUOUS_FIXED_SIZE_COUNT > 0;

    using FixedSizesArray = typename detail::ParameterListTraits<Parameter...>::FixedSizesArray;
    using Locator = detail::ElementLocatorT<Parameter...>;
    using Base = detail::EmptyBaseOptimization<FixedSizesArray>;

  public:
    Locator locator;

    ElementLocatorAndFixedSizes() = default;

    constexpr ElementLocatorAndFixedSizes(const Locator& locator, const FixedSizesArray& fixed_sizes) noexcept
        : Base{fixed_sizes}, locator(locator)
    {
    }

    constexpr ElementLocatorAndFixedSizes(std::size_t max_element_count, std::byte* memory,
                                          const FixedSizesArray& fixed_sizes) noexcept
        : Base{fixed_sizes}, locator(max_element_count, memory, fixed_sizes)
    {
    }

    constexpr auto operator->() noexcept { return std::addressof(this->locator); }

    constexpr auto operator->() const noexcept { return std::addressof(this->locator); }

    constexpr auto& operator*() noexcept { return this->locator; }

    constexpr const auto& operator*() const noexcept { return this->locator; }

    constexpr auto& fixed_sizes() noexcept { return Base::get(); }

    constexpr const auto& fixed_sizes() const noexcept { return Base::get(); }
};

using TypeErasedElementLocator = std::aligned_storage_t<
    std::max(sizeof(detail::ElementLocator<false, bool>), sizeof(detail::ElementLocator<true, bool>)),
    std::max(alignof(detail::ElementLocator<false, bool>), alignof(detail::ElementLocator<true, bool>))>;

template <class T>
auto type_erase_element_locator(T&& locator) noexcept
{
    detail::TypeErasedElementLocator result;
    detail::construct_at(reinterpret_cast<detail::RemoveCvrefT<T>*>(&result), std::forward<T>(locator));
    return result;
}
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_ELEMENTLOCATOR_HPP

// #include "cntgs/detail/forward.hpp"

// #include "cntgs/detail/iterator.hpp"

// #include "cntgs/detail/typeTraits.hpp"


#include <iterator>

namespace cntgs
{
template <bool IsConst, class Options, class... Parameter>
class ContiguousVectorIterator
{
  private:
    using Vector = cntgs::BasicContiguousVector<Options, Parameter...>;
    using ElementLocatorAndFixedSizes = detail::ElementLocatorAndFixedSizes<Parameter...>;
    using SizeType = typename Vector::size_type;
    using MemoryPointer = typename std::allocator_traits<typename Vector::allocator_type>::pointer;

  public:
    using value_type = typename Vector::value_type;
    using reference = detail::ConditionalT<IsConst, typename Vector::const_reference, typename Vector::reference>;
    using pointer = detail::ArrowProxy<reference>;
    using difference_type = typename Vector::difference_type;
    using iterator_category = std::random_access_iterator_tag;

    ContiguousVectorIterator() = default;

    constexpr ContiguousVectorIterator(const Vector& vector, SizeType index) noexcept
        : i(index), memory(vector.memory.get()), locator(vector.locator)
    {
    }

    constexpr explicit ContiguousVectorIterator(const Vector& vector) noexcept
        : ContiguousVectorIterator(vector, SizeType{})
    {
    }

    template <bool OtherIsConst>
    /*implicit*/ constexpr ContiguousVectorIterator(
        const ContiguousVectorIterator<OtherIsConst, Options, Parameter...>& other) noexcept
        : i(other.i), memory(other.memory), locator(other.locator)
    {
    }

    ContiguousVectorIterator(const ContiguousVectorIterator&) = default;
    ContiguousVectorIterator(ContiguousVectorIterator&&) = default;

    template <bool OtherIsConst>
    constexpr ContiguousVectorIterator& operator=(
        const ContiguousVectorIterator<OtherIsConst, Options, Parameter...>& other) noexcept
    {
        this->i = other.i;
        this->memory = other.memory;
        this->locator = other.locator;
        return *this;
    }

    ContiguousVectorIterator& operator=(const ContiguousVectorIterator&) = default;
    ContiguousVectorIterator& operator=(ContiguousVectorIterator&&) = default;

    [[nodiscard]] constexpr auto index() const noexcept { return this->i; }

    [[nodiscard]] constexpr auto data() const noexcept -> detail::ConditionalT<IsConst, const std::byte*, std::byte*>
    {
        return this->locator->element_address(this->i, this->memory);
    }

    [[nodiscard]] constexpr reference operator*() noexcept
    {
        return reference{this->locator->load_element_at(i, this->memory, this->locator.fixed_sizes())};
    }

    [[nodiscard]] constexpr reference operator*() const noexcept
    {
        return reference{this->locator->load_element_at(i, this->memory, this->locator.fixed_sizes())};
    }

    [[nodiscard]] constexpr pointer operator->() const noexcept { return {*(*this)}; }

    constexpr ContiguousVectorIterator& operator++() noexcept
    {
        ++this->i;
        return *this;
    }

    constexpr ContiguousVectorIterator operator++(int) noexcept
    {
        auto copy{*this};
        ++(*this);
        return copy;
    }

    constexpr ContiguousVectorIterator& operator--() noexcept
    {
        --this->i;
        return *this;
    }

    constexpr ContiguousVectorIterator operator--(int) noexcept
    {
        auto copy{*this};
        --(*this);
        return copy;
    }

    [[nodiscard]] constexpr ContiguousVectorIterator operator+(difference_type diff) const noexcept
    {
        auto copy{*this};
        copy.i += diff;
        return copy;
    }

    [[nodiscard]] constexpr difference_type operator+(ContiguousVectorIterator it) const noexcept
    {
        return this->i + it.i;
    }

    constexpr ContiguousVectorIterator& operator+=(difference_type diff) noexcept
    {
        this->i += diff;
        return *this;
    }

    [[nodiscard]] constexpr ContiguousVectorIterator operator-(difference_type diff) const noexcept
    {
        auto copy{*this};
        copy.i -= diff;
        return copy;
    }

    [[nodiscard]] constexpr difference_type operator-(ContiguousVectorIterator it) const noexcept
    {
        return this->i - it.i;
    }

    constexpr ContiguousVectorIterator& operator-=(difference_type diff) noexcept
    {
        this->i -= diff;
        return *this;
    }

    [[nodiscard]] reference operator[](difference_type diff) const noexcept { return *(*this + diff); }

    [[nodiscard]] constexpr bool operator==(const ContiguousVectorIterator& other) const noexcept
    {
        return this->i == other.i && this->memory == other.memory;
    }

    [[nodiscard]] constexpr bool operator!=(const ContiguousVectorIterator& other) const noexcept
    {
        return !(*this == other);
    }

    [[nodiscard]] constexpr bool operator<(const ContiguousVectorIterator& other) const noexcept
    {
        return this->i < other.i && this->memory == other.memory;
    }

    [[nodiscard]] constexpr bool operator>(const ContiguousVectorIterator& other) const noexcept
    {
        return other < *this;
    }

    [[nodiscard]] constexpr bool operator<=(const ContiguousVectorIterator& other) const noexcept
    {
        return !(*this > other);
    }

    [[nodiscard]] constexpr bool operator>=(const ContiguousVectorIterator& other) const noexcept
    {
        return !(*this < other);
    }

  private:
    friend cntgs::ContiguousVectorIterator<!IsConst, Options, Parameter...>;

    SizeType i{};
    MemoryPointer memory;
    ElementLocatorAndFixedSizes locator;
};
}  // namespace cntgs

#endif  // CNTGS_CNTGS_ITERATOR_HPP

// #include "cntgs/parameter.hpp"

// #include "cntgs/reference.hpp"

// #include "cntgs/span.hpp"

// #include "cntgs/typeErasedVector.hpp"
// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_CNTGS_TYPEERASEDVECTOR_HPP
#define CNTGS_CNTGS_TYPEERASEDVECTOR_HPP

// #include "cntgs/detail/allocator.hpp"

// #include "cntgs/detail/array.hpp"

// #include "cntgs/detail/elementLocator.hpp"

// #include "cntgs/detail/utility.hpp"


#include <cstddef>

namespace cntgs
{
class TypeErasedVector
{
  public:
    std::size_t memory_size;
    std::size_t max_element_count;
    std::byte* memory;
    detail::MoveDefaultingValue<bool> is_memory_owned;
    detail::Array<std::size_t, detail::MAX_FIXED_SIZE_VECTOR_PARAMETER> fixed_sizes;
    detail::TypeErasedElementLocator locator;
    detail::TypeErasedAllocator allocator;
    void (*destructor)(cntgs::TypeErasedVector&);

    template <class Allocator, class... Parameter>
    explicit TypeErasedVector(cntgs::BasicContiguousVector<Allocator, Parameter...>&& vector) noexcept
        : memory_size(vector.memory.size()),
          max_element_count(vector.max_element_count),
          memory(vector.memory.release()),
          is_memory_owned(true),
          fixed_sizes(
              detail::convert_array_to_size<detail::MAX_FIXED_SIZE_VECTOR_PARAMETER>(vector.locator.fixed_sizes())),
          locator(detail::type_erase_element_locator(*vector.locator)),
          allocator(detail::type_erase_allocator(vector.get_allocator())),
          destructor(&TypeErasedVector::destruct<Allocator, Parameter...>)
    {
    }

    TypeErasedVector(const TypeErasedVector&) = delete;
    TypeErasedVector(TypeErasedVector&&) = default;

    TypeErasedVector& operator=(const TypeErasedVector&) = delete;
    TypeErasedVector& operator=(TypeErasedVector&&) = default;

    ~TypeErasedVector() noexcept { destructor(*this); }

  private:
    template <class Allocator, class... Parameter>
    static void destruct(cntgs::TypeErasedVector& erased)
    {
        if (erased.is_memory_owned.value)
        {
            cntgs::BasicContiguousVector<Allocator, Parameter...>(std::move(erased));
        }
    }
};
}  // namespace cntgs

#endif  // CNTGS_CNTGS_TYPEERASEDVECTOR_HPP

// #include "cntgs/vector.hpp"
// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_CNTGS_VECTOR_HPP
#define CNTGS_CNTGS_VECTOR_HPP

// #include "cntgs/detail/algorithm.hpp"

// #include "cntgs/detail/allocator.hpp"

// #include "cntgs/detail/array.hpp"

// #include "cntgs/detail/elementLocator.hpp"

// #include "cntgs/detail/forward.hpp"

// #include "cntgs/detail/memory.hpp"

// #include "cntgs/detail/optionsParser.hpp"
// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_DETAIL_OPTIONSPARSER_HPP
#define CNTGS_DETAIL_OPTIONSPARSER_HPP

// #include "cntgs/detail/typeTraits.hpp"

// #include "cntgs/parameter.hpp"


#include <memory>
#include <type_traits>

namespace cntgs::detail
{
template <class = void>
struct AllocatorOptionParser : std::false_type
{
    using Allocator = std::allocator<std::byte>;
};

template <class T>
struct AllocatorOptionParser<cntgs::Allocator<T>> : std::true_type
{
    using Allocator = typename std::allocator_traits<T>::template rebind_alloc<std::byte>;
};

template <class... Option>
struct OptionsParser
{
    template <template <class = void> class Parser>
    using Parse =
        detail::ConditionalT<std::disjunction<Parser<Option>...>::value, std::disjunction<Parser<Option>...>, Parser<>>;

    using Allocator = typename Parse<detail::AllocatorOptionParser>::Allocator;
};
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_OPTIONSPARSER_HPP

// #include "cntgs/detail/parameterListTraits.hpp"

// #include "cntgs/detail/parameterTraits.hpp"

// #include "cntgs/detail/utility.hpp"

// #include "cntgs/detail/vectorTraits.hpp"

// #include "cntgs/element.hpp"

// #include "cntgs/iterator.hpp"

// #include "cntgs/parameter.hpp"

// #include "cntgs/reference.hpp"

// #include "cntgs/span.hpp"

// #include "cntgs/typeErasedVector.hpp"


#include <algorithm>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace cntgs
{
/// Alias template for [cntgs::BasicContiguousVector]() that uses [std::allocator]()
// begin-snippet: contiguous-vector-definition
template <class... Parameter>
using ContiguousVector = cntgs::BasicContiguousVector<cntgs::Options<>, Parameter...>;
// end-snippet

/// Container that stores the value of each specified parameter contiguously.
///
/// \param Option Any of [cntgs::Allocator]() wrapped into [cntgs::Options]().
/// \param Parameter Any of [cntgs::VaryingSize](), [cntgs::FixedSize](), [cntgs::AlignAs]() or a plain user-defined or
/// built-in type. The underlying type of each parameter must satisfy
/// [Erasable](https://en.cppreference.com/w/cpp/named_req/Erasable).
template <class... Option, class... Parameter>
class BasicContiguousVector<cntgs::Options<Option...>, Parameter...>
{
  private:
    using Self = cntgs::BasicContiguousVector<cntgs::Options<Option...>, Parameter...>;
    using ParsedOptions = detail::OptionsParser<Option...>;
    using Allocator = typename ParsedOptions::Allocator;
    using ListTraits = detail::ParameterListTraits<Parameter...>;
    using VectorTraits = detail::ContiguousVectorTraits<Parameter...>;
    using ElementLocator = detail::ElementLocatorT<Parameter...>;
    using ElementLocatorAndFixedSizes = detail::ElementLocatorAndFixedSizes<Parameter...>;
    using ElementTraits = detail::ElementTraitsT<Parameter...>;
    using AllocatorTraits = std::allocator_traits<Allocator>;
    using StorageType = detail::AllocatorAwarePointer<Allocator>;
    using FixedSizes = typename ListTraits::FixedSizes;
    using FixedSizesArray = typename ListTraits::FixedSizesArray;

    static constexpr bool IS_MIXED = ListTraits::IS_MIXED;
    static constexpr bool IS_ALL_FIXED_SIZE = ListTraits::IS_ALL_FIXED_SIZE;
    static constexpr bool IS_ALL_VARYING_SIZE = ListTraits::IS_ALL_VARYING_SIZE;
    static constexpr bool IS_ALL_PLAIN = ListTraits::IS_ALL_PLAIN;

  public:
    /// Type that can create copies of [cntgs::BasicContiguousVector::reference]() and
    /// [cntgs::BasicContiguousVector::const_reference]()
    using value_type = cntgs::BasicContiguousElement<Allocator, Parameter...>;

    /// A [cntgs::ContiguousReference]()
    /// \exclude target
    using reference = typename VectorTraits::ReferenceType;

    /// A [cntgs::ContiguousConstReference]()
    /// \exclude target
    using const_reference = typename VectorTraits::ConstReferenceType;
    using iterator = cntgs::ContiguousVectorIterator<false, cntgs::Options<Option...>, Parameter...>;
    using const_iterator = cntgs::ContiguousVectorIterator<true, cntgs::Options<Option...>, Parameter...>;
    using difference_type = std::ptrdiff_t;
    using size_type = std::size_t;
    using allocator_type = Allocator;

    size_type max_element_count{};
    StorageType memory{};
    ElementLocatorAndFixedSizes locator;

    BasicContiguousVector() = default;

    template <bool IsMixed = IS_MIXED>
    BasicContiguousVector(size_type max_element_count, size_type varying_size_bytes, const FixedSizes& fixed_sizes,
                          const allocator_type& allocator = {}, std::enable_if_t<IsMixed>* = nullptr)
        : BasicContiguousVector(max_element_count, varying_size_bytes, fixed_sizes, allocator)
    {
    }

    template <bool IsAllFixedSize = IS_ALL_FIXED_SIZE>
    constexpr BasicContiguousVector(size_type max_element_count, const FixedSizes& fixed_sizes,
                                    const allocator_type& allocator = {}, std::enable_if_t<IsAllFixedSize>* = nullptr)
        : BasicContiguousVector(max_element_count, size_type{}, fixed_sizes, allocator)
    {
    }

    template <bool IsAllVaryingSize = IS_ALL_VARYING_SIZE>
    BasicContiguousVector(size_type max_element_count, size_type varying_size_bytes,
                          const allocator_type& allocator = {}, std::enable_if_t<IsAllVaryingSize>* = nullptr)
        : BasicContiguousVector(max_element_count, varying_size_bytes, FixedSizes{}, allocator)
    {
    }

    template <bool IsNoneSpecial = IS_ALL_PLAIN>
    constexpr explicit BasicContiguousVector(size_type max_element_count, std::enable_if_t<IsNoneSpecial>* = nullptr)
        : BasicContiguousVector(max_element_count, size_type{}, FixedSizes{}, allocator_type{})
    {
    }

    template <bool IsNoneSpecial = IS_ALL_PLAIN>
    constexpr BasicContiguousVector(size_type max_element_count, const allocator_type& allocator,
                                    std::enable_if_t<IsNoneSpecial>* = nullptr)
        : BasicContiguousVector(max_element_count, size_type{}, FixedSizes{}, allocator)
    {
    }

    explicit BasicContiguousVector(cntgs::TypeErasedVector&& vector) noexcept
        : max_element_count(vector.max_element_count),
          memory(vector.memory, vector.memory_size,
                 *std::launder(reinterpret_cast<allocator_type*>(&vector.allocator))),
          locator(*std::launder(reinterpret_cast<ElementLocator*>(&vector.locator)),
                  detail::convert_array_to_size<ListTraits::CONTIGUOUS_FIXED_SIZE_COUNT>(vector.fixed_sizes))
    {
        vector.is_memory_owned.value = false;
    }

    BasicContiguousVector(const BasicContiguousVector& other)
        : max_element_count(other.max_element_count), memory(other.memory), locator(this->copy_construct_locator(other))
    {
    }

    BasicContiguousVector(BasicContiguousVector&&) = default;

    BasicContiguousVector& operator=(const BasicContiguousVector& other)
    {
        if (this != std::addressof(other))
        {
            this->copy_assign(other);
        }
        return *this;
    }

    constexpr BasicContiguousVector& operator=(BasicContiguousVector&& other) noexcept(
        AllocatorTraits::is_always_equal::value || AllocatorTraits::propagate_on_container_move_assignment::value)
    {
        if (this != std::addressof(other))
        {
            this->move_assign(std::move(other));
        }
        return *this;
    }

#if __cpp_constexpr_dynamic_alloc
    constexpr
#endif
        ~BasicContiguousVector() noexcept
    {
        this->destruct_if_owned();
    }

    template <class... Args>
    void emplace_back(Args&&... args)
    {
        this->locator->emplace_back(this->locator.fixed_sizes(), std::forward<Args>(args)...);
    }

    void pop_back() noexcept
    {
        ElementTraits::destruct(this->back());
        this->locator->resize(this->size() - size_type{1}, this->memory.get());
    }

    void reserve(size_type new_max_element_count, size_type new_varying_size_bytes = {})
    {
        if (this->max_element_count < new_max_element_count)
        {
            this->grow(new_max_element_count, new_varying_size_bytes);
        }
    }

    iterator erase(const_iterator position) noexcept(ListTraits::IS_NOTHROW_MOVE_CONSTRUCTIBLE)
    {
        iterator it_position{*this, position.index()};
        const auto next_position = position.index() + 1;
        ElementTraits::destruct(*it_position);
        this->move_elements_forward(next_position, it_position.index());
        this->locator->resize(this->size() - size_type{1}, this->memory.get());
        return it_position;
    }

    iterator erase(const_iterator first, const_iterator last) noexcept(ListTraits::IS_NOTHROW_MOVE_CONSTRUCTIBLE)
    {
        const auto current_size = this->size();
        iterator it_first{*this, first.index()};
        iterator it_last{*this, last.index()};
        BasicContiguousVector::destruct(it_first, it_last);
        if (last.index() < current_size && first.index() != last.index())
        {
            this->move_elements_forward(last.index(), first.index());
        }
        this->locator->resize(current_size - (last.index() - first.index()), this->memory.get());
        return it_first;
    }

    void clear() noexcept
    {
        this->destruct();
        this->locator->resize(0, this->memory.get());
    }

    [[nodiscard]] reference operator[](size_type i) noexcept
    {
        return reference{this->locator->load_element_at(i, this->memory.get(), this->locator.fixed_sizes())};
    }

    [[nodiscard]] const_reference operator[](size_type i) const noexcept
    {
        return const_reference{this->locator->load_element_at(i, this->memory.get(), this->locator.fixed_sizes())};
    }

    [[nodiscard]] reference front() noexcept { return (*this)[{}]; }

    [[nodiscard]] const_reference front() const noexcept { return (*this)[{}]; }

    [[nodiscard]] reference back() noexcept { return (*this)[this->size() - size_type{1}]; }

    [[nodiscard]] const_reference back() const noexcept { return (*this)[this->size() - size_type{1}]; }

    template <std::size_t I>
    [[nodiscard]] constexpr size_type get_fixed_size() const noexcept
    {
        return detail::get<I>(this->locator.fixed_sizes());
    }

    [[nodiscard]] constexpr bool empty() const noexcept { return this->locator->empty(this->memory.get()); }

    [[nodiscard]] constexpr std::byte* data() noexcept
    {
        return this->locator->element_address({}, this->memory.get());
    }

    [[nodiscard]] constexpr const std::byte* data() const noexcept
    {
        return this->locator->element_address({}, this->memory.get());
    }

    [[nodiscard]] constexpr std::byte* data_begin() noexcept { return this->data(); }

    [[nodiscard]] constexpr const std::byte* data_begin() const noexcept { return this->data(); }

    [[nodiscard]] constexpr std::byte* data_end() noexcept { return this->locator->data_end(); }

    [[nodiscard]] constexpr const std::byte* data_end() const noexcept { return this->locator->data_end(); }

    [[nodiscard]] constexpr size_type size() const noexcept { return this->locator->size(this->memory.get()); }

    [[nodiscard]] constexpr size_type capacity() const noexcept { return this->max_element_count; }

    [[nodiscard]] constexpr size_type memory_consumption() const noexcept { return this->memory.size(); }

    [[nodiscard]] constexpr iterator begin() noexcept { return iterator{*this}; }

    [[nodiscard]] constexpr const_iterator begin() const noexcept { return const_iterator{*this}; }

    [[nodiscard]] constexpr const_iterator cbegin() const noexcept { return this->begin(); }

    [[nodiscard]] constexpr iterator end() noexcept { return iterator{*this, this->size()}; }

    [[nodiscard]] constexpr const_iterator end() const noexcept { return const_iterator{*this, this->size()}; }

    [[nodiscard]] constexpr const_iterator cend() const noexcept { return this->end(); }

    [[nodiscard]] constexpr allocator_type get_allocator() const noexcept { return this->memory.get_allocator(); }

    template <class... TOption>
    [[nodiscard]] constexpr auto operator==(
        const cntgs::BasicContiguousVector<cntgs::Options<TOption...>, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTHROW_EQUALITY_COMPARABLE)
    {
        return this->equal(other);
    }

    template <class... TOption>
    [[nodiscard]] constexpr auto operator!=(
        const cntgs::BasicContiguousVector<cntgs::Options<TOption...>, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTHROW_EQUALITY_COMPARABLE)
    {
        return !(*this == other);
    }

    template <class... TOption>
    [[nodiscard]] constexpr auto operator<(
        const cntgs::BasicContiguousVector<cntgs::Options<TOption...>, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return this->lexicographical_compare(other);
    }

    template <class... TOption>
    [[nodiscard]] constexpr auto operator<=(
        const cntgs::BasicContiguousVector<cntgs::Options<TOption...>, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return !(other < *this);
    }

    template <class... TOption>
    [[nodiscard]] constexpr auto operator>(
        const cntgs::BasicContiguousVector<cntgs::Options<TOption...>, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return other < *this;
    }

    template <class... TOption>
    [[nodiscard]] constexpr auto operator>=(
        const cntgs::BasicContiguousVector<cntgs::Options<TOption...>, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return !(*this < other);
    }

    // private API
  private:
    constexpr BasicContiguousVector(std::byte* memory, size_type memory_size, bool is_memory_owned,
                                    size_type max_element_count, const FixedSizes& fixed_sizes,
                                    const allocator_type& allocator)
        : max_element_count(max_element_count),
          memory(memory, memory_size, is_memory_owned, allocator),
          locator(max_element_count, this->memory.get(), FixedSizesArray{fixed_sizes})
    {
    }

    constexpr BasicContiguousVector(size_type max_element_count, size_type varying_size_bytes,
                                    const FixedSizes& fixed_sizes, const allocator_type& allocator)
        : max_element_count(max_element_count),
          memory(
              Self::calculate_needed_memory_size(max_element_count, varying_size_bytes, FixedSizesArray{fixed_sizes}),
              allocator),
          locator(max_element_count, this->memory.get(), FixedSizesArray{fixed_sizes})
    {
    }

    [[nodiscard]] static constexpr auto calculate_needed_memory_size(size_type max_element_count,
                                                                     size_type varying_size_bytes,
                                                                     const FixedSizesArray& fixed_sizes) noexcept
    {
        constexpr auto ALIGNMENT_OVERHEAD = ListTraits::template ParameterTraitsAt<0>::ALIGNMENT - 1;
        return varying_size_bytes + ElementTraits::calculate_element_size(fixed_sizes) * max_element_count +
               ElementLocator::reserved_bytes(max_element_count) + ALIGNMENT_OVERHEAD;
    }

    void grow(size_type new_max_element_count, size_type new_varying_size_bytes)
    {
        const auto new_memory_size = this->locator->calculate_new_memory_size(
            new_max_element_count, new_varying_size_bytes, this->locator.fixed_sizes());
        StorageType new_memory{new_memory_size, this->get_allocator()};
        BasicContiguousVector::insert_into<true, true>(*this->locator, new_max_element_count, new_memory.get(), *this);
        this->max_element_count = new_max_element_count;
        this->memory.reset(std::move(new_memory));
    }

    template <bool UseMove, bool IsDestruct = false, class Self = BasicContiguousVector>
    static void insert_into(ElementLocator& locator, size_type new_max_element_count, std::byte* new_memory, Self& from)
    {
        static constexpr auto IS_TRIVIAL =
            UseMove ? ListTraits::IS_TRIVIALLY_MOVE_CONSTRUCTIBLE : ListTraits::IS_TRIVIALLY_COPY_CONSTRUCTIBLE;
        if constexpr (IS_TRIVIAL && (!IsDestruct || ListTraits::IS_TRIVIALLY_DESTRUCTIBLE))
        {
            locator.trivially_copy_into(from.max_element_count, from.memory.get(), new_max_element_count, new_memory);
        }
        else
        {
            ElementLocator new_locator{locator, from.max_element_count, from.memory.get(), new_max_element_count,
                                       new_memory};
            BasicContiguousVector::uninitialized_construct_if_non_trivial<UseMove>(from, new_memory, new_locator);
            if constexpr (IsDestruct)
            {
                from.destruct();
            }
            locator = new_locator;
        }
    }

    template <bool UseMove, class Self>
    static void uninitialized_construct_if_non_trivial(Self& self, [[maybe_unused]] std::byte* new_memory,
                                                       [[maybe_unused]] ElementLocator& new_locator)
    {
        static constexpr auto IS_TRIVIAL =
            UseMove ? ListTraits::IS_TRIVIALLY_MOVE_CONSTRUCTIBLE : ListTraits::IS_TRIVIALLY_COPY_CONSTRUCTIBLE;
        if constexpr (!IS_TRIVIAL)
        {
            for (size_type i{}; i < self.size(); ++i)
            {
                auto&& source = self[i];
                auto&& target =
                    new_locator.load_element_at<detail::DefaultAlignmentNeeds, detail::ContiguousReferenceSizeGetter>(
                        i, new_memory, source);
                ElementTraits::template construct_if_non_trivial<UseMove>(source, target);
            }
        }
    }

    void move_elements_forward(std::size_t from, std::size_t to)
    {
        if constexpr (ListTraits::IS_TRIVIALLY_MOVE_CONSTRUCTIBLE && ListTraits::IS_TRIVIALLY_DESTRUCTIBLE)
        {
            this->locator->move_elements_forward(from, to, this->memory.get());
        }
        else
        {
            for (auto i = to; from != this->size(); ++i, (void)++from)
            {
                this->emplace_at(i, (*this)[from], ListTraits::make_index_sequence());
            }
        }
    }

    template <std::size_t... I>
    void emplace_at(std::size_t i, const reference& element, std::index_sequence<I...>)
    {
        this->locator->emplace_at(i, this->memory.get(), this->locator.fixed_sizes(),
                                  std::move(cntgs::get<I>(element))...);
        ElementTraits::destruct(element);
    }

    constexpr void steal(BasicContiguousVector&& other) noexcept
    {
        this->destruct();
        this->max_element_count = other.max_element_count;
        this->memory = std::move(other.memory);
        this->locator = other.locator;
    }

    constexpr void move_assign(BasicContiguousVector&& other)
    {
        if constexpr (AllocatorTraits::is_always_equal::value ||
                      AllocatorTraits::propagate_on_container_move_assignment::value)
        {
            this->steal(std::move(other));
        }
        else
        {
            if (this->get_allocator() == other.get_allocator())
            {
                this->steal(std::move(other));
            }
            else
            {
                auto other_locator = other.locator;
                if (other.memory_consumption() > this->memory_consumption())
                {
                    // allocate memory first because it might throw
                    StorageType new_memory{other.memory_consumption(), this->get_allocator()};
                    this->destruct();
                    BasicContiguousVector::insert_into<true>(*other_locator, other.max_element_count, new_memory.get(),
                                                             other);
                    this->memory = std::move(new_memory);
                }
                else
                {
                    this->destruct();
                    BasicContiguousVector::insert_into<true>(*other_locator, other.max_element_count,
                                                             this->memory.get(), other);
                }
                this->max_element_count = other.max_element_count;
                this->locator = other_locator;
            }
        }
    }

    auto copy_construct_locator(const BasicContiguousVector& other)
    {
        auto other_locator = other.locator;
        BasicContiguousVector::insert_into<false>(*other_locator, other.max_element_count, this->memory.get(), other);
        return other_locator;
    }

    void copy_assign(const BasicContiguousVector& other)
    {
        this->destruct();
        this->memory = other.memory;
        auto other_locator = other.locator;
        BasicContiguousVector::insert_into<false>(*other_locator, other.max_element_count, this->memory.get(), other);
        this->max_element_count = other.max_element_count;
        this->locator = other_locator;
    }

    template <class... TOption>
    constexpr auto equal(const cntgs::BasicContiguousVector<cntgs::Options<TOption...>, Parameter...>& other) const
    {
        if constexpr (ListTraits::IS_EQUALITY_MEMCMPABLE)
        {
            if (this->empty())
            {
                return other.empty();
            }
            if (other.empty())
            {
                return false;
            }
            return detail::trivial_equal(this->data_begin(), this->data_end(), other.data_begin(), other.data_end());
        }
        else
        {
            return std::equal(this->begin(), this->end(), other.begin());
        }
    }
    template <class... TOption>
    constexpr auto lexicographical_compare(
        const cntgs::BasicContiguousVector<cntgs::Options<TOption...>, Parameter...>& other) const
    {
        if constexpr (ListTraits::IS_LEXICOGRAPHICAL_MEMCMPABLE && ListTraits::IS_FIXED_SIZE_OR_PLAIN)
        {
            if (this->empty())
            {
                return !other.empty();
            }
            if (other.empty())
            {
                return false;
            }
            return detail::trivial_lexicographical_compare(this->data_begin(), this->data_end(), other.data_begin(),
                                                           other.data_end());
        }
        else
        {
            return std::lexicographical_compare(this->begin(), this->end(), other.begin(), other.end());
        }
    }

    constexpr void destruct_if_owned() noexcept
    {
        if (this->memory)
        {
            this->destruct();
        }
    }

    constexpr void destruct() noexcept { BasicContiguousVector::destruct(this->begin(), this->end()); }

    static constexpr void destruct([[maybe_unused]] iterator first, [[maybe_unused]] iterator last) noexcept
    {
        if constexpr (!ListTraits::IS_TRIVIALLY_DESTRUCTIBLE)
        {
            std::for_each(first, last, ElementTraits::destruct);
        }
    }
};

template <class... Option, class... T>
constexpr void swap(cntgs::BasicContiguousVector<cntgs::Options<Option...>, T...>& lhs,
                    cntgs::BasicContiguousVector<cntgs::Options<Option...>, T...>& rhs) noexcept
{
    std::swap(lhs.max_element_count, rhs.max_element_count);
    detail::swap(lhs.memory, rhs.memory);
    std::swap(lhs.locator, rhs.locator);
}
}  // namespace cntgs

#endif  // CNTGS_CNTGS_VECTOR_HPP


#endif  // CNTGS_CNTGS_CONTIGUOUS_HPP
