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

template <class T, class = void>
inline constexpr bool IS_NOTRHOW_EQUALITY_COMPARABLE = false;

template <class T>
inline constexpr bool
    IS_NOTRHOW_EQUALITY_COMPARABLE<T, std::void_t<decltype(std::declval<const T&>() == std::declval<const T&>())>> =
        noexcept(std::declval<const T&>() == std::declval<const T&>());

template <class T, class = void>
inline constexpr bool IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE = false;

template <class T>
inline constexpr bool IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE<
    T, std::void_t<decltype(std::declval<const T&>() < std::declval<const T&>())>> =
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

#include <iterator>
#include <version>

namespace cntgs::detail
{
template <class, class = void>
inline constexpr bool HAS_OPERATOR_ARROW = false;

template <class T>
inline constexpr bool HAS_OPERATOR_ARROW<T, std::void_t<decltype(std::declval<const T&>().operator->())>> = true;

template <class T>
struct ArrowProxy
{
    T t_;

    constexpr const T* operator->() const noexcept { return &t_; }
};

template <class I>
constexpr auto operator_arrow_produces_pointer_to_iterator_reference_type() noexcept
{
    if constexpr (detail::HAS_OPERATOR_ARROW<I>)
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
inline constexpr bool CONTIGUOUS_ITERATOR_V =
    detail::IS_DERIVED_FROM<typename std::iterator_traits<I>::iterator_category, std::random_access_iterator_tag> &&
    std::is_lvalue_reference_v<typename std::iterator_traits<I>::reference> &&
    std::is_same_v<typename std::iterator_traits<I>::value_type,
                   detail::RemoveCvrefT<typename std::iterator_traits<I>::reference>> &&
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
template <class, class = void>
inline constexpr bool HAS_DATA_AND_SIZE = false;

template <class T>
inline constexpr bool HAS_DATA_AND_SIZE<
    T, std::void_t<decltype(std::data(std::declval<T&>())), decltype(std::size(std::declval<T&>()))>> = true;

template <class, class = void>
inline constexpr bool IS_RANGE = false;

template <class T>
inline constexpr bool
    IS_RANGE<T, std::void_t<decltype(std::begin(std::declval<T&>())), decltype(std::end(std::declval<T&>()))>> = true;
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_RANGE_HPP

// #include "cntgs/detail/typeTraits.hpp"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <limits>
#include <memory>

namespace cntgs::detail
{
using Byte = std::underlying_type_t<std::byte>;

template <std::size_t N>
struct Aligned
{
    alignas(N) std::byte v[N];
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
auto uninitialized_range_construct(Range&& range, TargetType* address)
{
    using RangeValueType = typename std::iterator_traits<decltype(std::begin(range))>::value_type;
    if constexpr (IgnoreAliasing && detail::HAS_DATA_AND_SIZE<std::decay_t<Range>> &&
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
auto uninitialized_construct(Range&& range, TargetType* address,
                             std::size_t) -> std::enable_if_t<detail::IS_RANGE<Range>, std::byte*>
{
    return detail::uninitialized_range_construct<IgnoreAliasing>(std::forward<Range>(range), address);
}

template <bool IgnoreAliasing, class TargetType, class Iterator>
auto uninitialized_construct(const Iterator& iterator, TargetType* address,
                             std::size_t size) -> std::enable_if_t<!detail::IS_RANGE<Iterator>, std::byte*>
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

[[nodiscard]] inline bool is_aligned(void* ptr, size_t alignment) noexcept
{
    void* void_ptr = ptr;
    auto size = std::numeric_limits<size_t>::max();
    std::align(alignment, 0, void_ptr, size);
    return void_ptr == ptr;
}

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

template <bool NeedsAlignment, std::size_t Alignment>
[[nodiscard]] constexpr auto align_if(std::uintptr_t position) noexcept
{
    if constexpr (NeedsAlignment && Alignment > 1)
    {
        position = detail::align<Alignment>(position);
    }
    return position;
}

template <class T>
[[nodiscard]] constexpr T extract_lowest_set_bit(T value) noexcept
{
    return value & (~value + T{1});
}

[[nodiscard]] constexpr std::size_t trailing_alignment(std::size_t byte_size, std::size_t alignment) noexcept
{
    return (std::min)(detail::extract_lowest_set_bit(byte_size), alignment);
}

inline constexpr auto SIZE_T_TRAILING_ALIGNMENT = detail::trailing_alignment(sizeof(std::size_t), alignof(std::size_t));
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_MEMORY_HPP

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

    iterator first_;
    iterator last_;

    Span() = default;

    template <class U>
    constexpr explicit Span(const Span<U>& other) noexcept : first_(other.first_), last_(other.last_)
    {
    }

    Span(const Span& other) = default;

    Span(Span&& other) = default;

    Span& operator=(const Span& other) = default;

    Span& operator=(Span&& other) = default;

    constexpr Span(iterator first, iterator last) noexcept : first_(first), last_(last) {}

    constexpr Span(iterator first, size_type size) noexcept(noexcept(first + size)) : first_(first), last_(first + size)
    {
    }

    [[nodiscard]] constexpr iterator begin() const noexcept { return first_; }

    [[nodiscard]] constexpr iterator end() const noexcept { return last_; }

    [[nodiscard]] constexpr reverse_iterator rbegin() const noexcept { return reverse_iterator{end()}; }

    [[nodiscard]] constexpr reverse_iterator rend() const noexcept { return reverse_iterator{begin()}; }

    [[nodiscard]] constexpr bool empty() const noexcept { return first_ == last_; }

    [[nodiscard]] constexpr size_type size() const noexcept { return last_ - first_; }

    [[nodiscard]] constexpr pointer data() const noexcept { return first_; }

    [[nodiscard]] constexpr reference operator[](size_type i) const noexcept { return first_[i]; }

    [[nodiscard]] constexpr reference front() const noexcept { return first_[0]; }

    [[nodiscard]] constexpr reference back() const noexcept { return *(last_ - 1); }

#ifdef __cpp_lib_span
    constexpr operator std::span<T>() const noexcept { return std::span<T>{first_, last_}; }
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
    T value_{};

    MoveDefaultingValue() = default;

    constexpr explicit MoveDefaultingValue(T value) noexcept : value_(value) {}

    ~MoveDefaultingValue() = default;

    MoveDefaultingValue(const MoveDefaultingValue&) = default;

    constexpr MoveDefaultingValue(MoveDefaultingValue&& other) noexcept : value_(other.value_) { other.value_ = T{}; }

    MoveDefaultingValue& operator=(const MoveDefaultingValue& other) = default;

    constexpr MoveDefaultingValue& operator=(MoveDefaultingValue&& other) noexcept
    {
        value_ = other.value_;
        other.value_ = T{};
        return *this;
    }
};

template <class T, bool = (std::is_empty_v<T> && !std::is_final_v<T>)>
class EmptyBaseOptimization
{
  private:
    T value_;

  public:
    EmptyBaseOptimization() = default;

    constexpr explicit EmptyBaseOptimization(const T& value) noexcept(std::is_nothrow_copy_constructible_v<T>)
        : value_{value}
    {
    }

    constexpr explicit EmptyBaseOptimization(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>)
        : value_{std::move(value)}
    {
    }

    constexpr auto& get() noexcept { return value_; }

    constexpr const auto& get() const noexcept { return value_; }
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

template <bool UseMove, class T>
constexpr decltype(auto) move_if(T& value)
{
    if constexpr (UseMove)
    {
        return std::move(value);
    }
    else
    {
        return (value);
    }
}
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_UTILITY_HPP

#include <cstddef>
#include <memory>
#include <type_traits>

namespace cntgs::detail
{
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

        Pointer ptr_{};
        std::size_t size_{};

        Impl() = default;

        constexpr Impl(Pointer ptr, std::size_t size, const Allocator& allocator) noexcept
            : Base{allocator}, ptr_(ptr), size_(size)
        {
        }
    };

    Impl impl_;

    constexpr auto allocate() { return AllocatorTraits::allocate(get_allocator(), size()); }

    constexpr void deallocate() noexcept
    {
        if (get())
        {
            AllocatorTraits::deallocate(get_allocator(), get(), size());
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
        : impl_(AllocatorAwarePointer::allocate_if_not_zero(size, allocator), size, allocator)
    {
    }

    constexpr AllocatorAwarePointer(pointer ptr, std::size_t size, const Allocator& allocator) noexcept
        : impl_(ptr, size, allocator)
    {
    }

    constexpr AllocatorAwarePointer(const AllocatorAwarePointer& other)
        : AllocatorAwarePointer(other.size(),
                                AllocatorTraits::select_on_container_copy_construction(other.get_allocator()))
    {
    }

    constexpr AllocatorAwarePointer(AllocatorAwarePointer&& other) noexcept
        : impl_(other.release(), other.size(), other.get_allocator())
    {
    }

#if __cpp_constexpr_dynamic_alloc
    constexpr
#endif
        ~AllocatorAwarePointer() noexcept
    {
        deallocate();
    }

    constexpr AllocatorAwarePointer& operator=(const AllocatorAwarePointer& other)
    {
        if (this != std::addressof(other))
        {
            if constexpr (AllocatorTraits::propagate_on_container_copy_assignment::value &&
                          !AllocatorTraits::is_always_equal::value)
            {
                if (get_allocator() != other.get_allocator())
                {
                    deallocate();
                    propagate_on_container_copy_assignment(other);
                    size() = other.size();
                    get() = allocate();
                    return *this;
                }
            }
            propagate_on_container_copy_assignment(other);
            if (size() < other.size() || !get())
            {
                deallocate();
                size() = other.size();
                get() = allocate();
            }
        }
        return *this;
    }

    constexpr AllocatorAwarePointer& operator=(AllocatorAwarePointer&& other) noexcept
    {
        if (this != std::addressof(other))
        {
            propagate_on_container_move_assignment(other);
            deallocate();
            get() = other.release();
            size() = other.size();
        }
        return *this;
    }

    constexpr decltype(auto) get_allocator() noexcept { return impl_.get(); }

    constexpr auto get_allocator() const noexcept { return impl_.get(); }

    constexpr auto& get() noexcept { return impl_.ptr_; }

    constexpr auto get() const noexcept { return impl_.ptr_; }

    constexpr auto& size() noexcept { return impl_.size_; }

    constexpr auto size() const noexcept { return impl_.size_; }

    constexpr explicit operator bool() const noexcept { return get() != nullptr; }

    constexpr auto release() noexcept { return std::exchange(impl_.ptr_, nullptr); }

    constexpr void reset(AllocatorAwarePointer&& other) noexcept
    {
        deallocate();
        get() = other.release();
        size() = other.size();
    }

    constexpr void propagate_on_container_copy_assignment(const AllocatorAwarePointer& other) noexcept
    {
        if constexpr (AllocatorTraits::propagate_on_container_copy_assignment::value)
        {
            get_allocator() = other.get_allocator();
        }
    }

    constexpr void propagate_on_container_move_assignment(AllocatorAwarePointer& other) noexcept
    {
        if constexpr (AllocatorTraits::propagate_on_container_move_assignment::value)
        {
            get_allocator() = std::move(other.get_allocator());
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
    std::array<T, N> array_;
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
    return std::get<I>(array.array_);
}

template <std::size_t, class T>
constexpr auto get(const detail::Array<T, 0>&) noexcept
{
    return T{};
}

template <std::size_t N, class T, std::size_t K, std::size_t... I>
constexpr auto convert_array_to_size(const detail::Array<T, K>& array, std::index_sequence<I...>)
{
    return detail::Array<T, N>{detail::get<I>(array)...};
}

template <std::size_t N, class T, std::size_t K>
constexpr auto convert_array_to_size(const detail::Array<T, K>& array)
{
    return detail::convert_array_to_size<N>(array, std::make_index_sequence<(std::min)(N, K)>{});
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
#include <cassert>
#include <cstddef>
#include <memory>
#include <new>
#include <type_traits>

namespace cntgs::detail
{
struct AlignedSizeInMemory
{
    std::size_t offset;
    std::size_t size;
    std::size_t padding;
};

template <class T>
struct VaryingSizeAddresses
{
    T* value;
    std::size_t* size;
};

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
    static constexpr auto TRAILING_ALIGNMENT = detail::trailing_alignment(VALUE_BYTES, ALIGNMENT);

    template <std::size_t PreviousAlignment, bool>
    static auto load(std::byte* address, std::size_t) noexcept
    {
        address = detail::align_if<(PreviousAlignment < ALIGNMENT), ALIGNMENT>(address);
        assert(detail::is_aligned(address, ALIGNMENT));
        auto result = std::launder(reinterpret_cast<PointerType>(address));
        return std::pair{result, address + VALUE_BYTES};
    }

    template <std::size_t PreviousAlignment, bool, class Arg>
    static std::byte* store(Arg&& arg, std::byte* address, std::size_t)
    {
        address = detail::align_if<(PreviousAlignment < ALIGNMENT), ALIGNMENT>(address);
        assert(detail::is_aligned(address, ALIGNMENT));
        detail::construct_at(reinterpret_cast<T*>(address), std::forward<Arg>(arg));
        return address + VALUE_BYTES;
    }

    template <std::size_t PreviousAlignment, std::size_t NextAlignment>
    static constexpr AlignedSizeInMemory aligned_size_in_memory(std::size_t offset, std::size_t) noexcept
    {
        const auto alignment_offset = detail::align_if<(PreviousAlignment < ALIGNMENT), ALIGNMENT>(offset);
        const auto size = alignment_offset - offset + VALUE_BYTES;
        const auto new_offset = offset + size;
        const auto padding_offset = detail::align_if<(TRAILING_ALIGNMENT < NextAlignment), NextAlignment>(new_offset);
        return {new_offset, size, padding_offset - new_offset};
    }

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

    static constexpr auto VALUE_BYTES = sizeof(T);

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
    static constexpr auto ALIGNMENT = alignof(std::size_t);
    static constexpr auto VALUE_ALIGNMENT = Alignment;
    static constexpr auto TRAILING_ALIGNMENT = detail::trailing_alignment(sizeof(T), ALIGNMENT);

    template <std::size_t PreviousAlignment, bool IsSizeProvided>
    static auto load(std::byte* address, std::size_t size) noexcept
    {
        const auto [value_address, size_address] = get_addresses<PreviousAlignment>(address);
        if constexpr (!IsSizeProvided)
        {
            size = *size_address;
        }
        const auto first = std::launder(reinterpret_cast<IteratorType>(value_address));
        const auto last = std::launder(reinterpret_cast<IteratorType>(value_address) + size);
        return std::pair{PointerType{first, last}, reinterpret_cast<std::byte*>(last)};
    }

    template <std::size_t PreviousAlignment, bool IgnoreAliasing, class Range>
    static std::byte* store(Range&& range, std::byte* address, std::size_t)
    {
        const auto [value_address, size_address] = get_addresses<PreviousAlignment>(address);
        auto* new_address =
            detail::uninitialized_range_construct<IgnoreAliasing>(std::forward<Range>(range), value_address);
        *size_address = reinterpret_cast<IteratorType>(new_address) - value_address;
        return new_address;
    }

    template <std::size_t PreviousAlignment, std::size_t NextAlignment>
    static constexpr AlignedSizeInMemory aligned_size_in_memory(std::size_t offset, std::size_t) noexcept
    {
        const auto size_t_alignment_offset = detail::align_if<(PreviousAlignment < ALIGNMENT), ALIGNMENT>(offset);
        const auto value_alignment_offset =
            detail::align_if<(detail::SIZE_T_TRAILING_ALIGNMENT < VALUE_ALIGNMENT), VALUE_ALIGNMENT>(
                size_t_alignment_offset + sizeof(std::size_t));
        const auto trailing_alignment =
            detail::trailing_alignment(sizeof(T), detail::extract_lowest_set_bit(value_alignment_offset));
        const auto next_alignment_difference =
            trailing_alignment < NextAlignment ? NextAlignment - trailing_alignment : 0;
        return {0, value_alignment_offset - offset + next_alignment_difference, 0};
    }

    static auto data_begin(const cntgs::Span<std::add_const_t<T>>& value) noexcept
    {
        return reinterpret_cast<const std::byte*>(ParameterTraits::begin(value)) - sizeof(std::size_t);
    }

    static auto data_begin(const cntgs::Span<T>& value) noexcept
    {
        return reinterpret_cast<std::byte*>(ParameterTraits::begin(value)) - sizeof(std::size_t);
    }

    template <std::size_t PreviousAlignment>
    static VaryingSizeAddresses<T> get_addresses(std::byte* address) noexcept
    {
        address = detail::align_if<(PreviousAlignment < ALIGNMENT), ALIGNMENT>(address);
        assert(detail::is_aligned(address, ALIGNMENT));
        auto size = reinterpret_cast<std::size_t*>(address);
        address += sizeof(std::size_t);
        const auto aligned_address = reinterpret_cast<IteratorType>(
            detail::align_if<(detail::SIZE_T_TRAILING_ALIGNMENT < VALUE_ALIGNMENT), VALUE_ALIGNMENT>(address));
        assert(detail::is_aligned(aligned_address, VALUE_ALIGNMENT));
        return {aligned_address, size};
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
    using ParameterTraits::BaseContiguousParameterTraits::VALUE_BYTES;
    static constexpr auto TRAILING_ALIGNMENT = detail::trailing_alignment(VALUE_BYTES, ALIGNMENT);

    template <std::size_t PreviousAlignment, bool>
    static auto load(std::byte* address, std::size_t size) noexcept
    {
        const auto first = std::launder(
            reinterpret_cast<IteratorType>(detail::align_if<(PreviousAlignment < ALIGNMENT), ALIGNMENT>(address)));
        assert(detail::is_aligned(first, ALIGNMENT));
        const auto last = first + size;
        return std::pair{PointerType{first, last}, reinterpret_cast<std::byte*>(last)};
    }

    template <std::size_t PreviousAlignment, bool IgnoreAliasing, class RangeOrIterator>
    static std::byte* store(RangeOrIterator&& range_or_iterator, std::byte* address, std::size_t size)
    {
        const auto aligned_address =
            reinterpret_cast<IteratorType>(detail::align_if<(PreviousAlignment < ALIGNMENT), ALIGNMENT>(address));
        assert(detail::is_aligned(aligned_address, ALIGNMENT));
        return detail::uninitialized_construct<IgnoreAliasing>(std::forward<RangeOrIterator>(range_or_iterator),
                                                               aligned_address, size);
    }

    template <std::size_t PreviousAlignment, std::size_t NextAlignment>
    static constexpr AlignedSizeInMemory aligned_size_in_memory(std::size_t offset, std::size_t fixed_size) noexcept
    {
        const auto alignment_offset = detail::align_if<(PreviousAlignment < ALIGNMENT), ALIGNMENT>(offset);
        const auto size = alignment_offset - offset + VALUE_BYTES * fixed_size;
        const auto new_offset = offset + size;
        const auto padding_offset = detail::align_if<(TRAILING_ALIGNMENT < NextAlignment), NextAlignment>(new_offset);
        return {new_offset, size, padding_offset - new_offset};
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

    static constexpr std::size_t LARGEST_LEADING_ALIGNMENT_UNTIL_VARYING_SIZE = []
    {
        bool stop{};
        std::size_t alignment{};
        (
            [&]
            {
                if constexpr (detail::ParameterTraits<Parameter>::TYPE == ParameterType::VARYING_SIZE)
                {
                    alignment = (std::max)(alignment, detail::ParameterTraits<Parameter>::VALUE_ALIGNMENT);
                    stop = true;
                }
                alignment = (std::max)(alignment, detail::ParameterTraits<Parameter>::ALIGNMENT);
                return stop;
            }() ||
            ...);
        return alignment;
    }();

    using FixedSizes = std::array<std::size_t, CONTIGUOUS_FIXED_SIZE_COUNT>;
    using FixedSizesArray = detail::Array<std::size_t, CONTIGUOUS_FIXED_SIZE_COUNT>;

    static_assert(detail::MAX_FIXED_SIZE_VECTOR_PARAMETER > CONTIGUOUS_FIXED_SIZE_COUNT,
                  "Maximum number of FixedSize vector parameter exceeded. Define CNTGS_MAX_FIXED_SIZE_VECTOR_PARAMETER "
                  "to a higher limit.");

    static constexpr auto make_index_sequence() noexcept { return std::make_index_sequence<sizeof...(Parameter)>{}; }

    template <std::size_t I>
    static constexpr std::size_t trailing_alignment() noexcept
    {
        return detail::trailing_alignment(ParameterTraitsAt<(I)>::VALUE_BYTES, ParameterTraitsAt<(I)>::ALIGNMENT);
    }

    template <std::size_t I>
    static constexpr std::size_t next_alignment() noexcept
    {
        if constexpr (sizeof...(Parameter) - 1 == I)
        {
            return LARGEST_LEADING_ALIGNMENT_UNTIL_VARYING_SIZE;
        }
        else
        {
            return ParameterTraitsAt<(I + 1)>::ALIGNMENT;
        }
    }

    template <std::size_t I>
    static constexpr std::size_t previous_alignment() noexcept
    {
        if constexpr (I == 0)
        {
            return LARGEST_LEADING_ALIGNMENT_UNTIL_VARYING_SIZE;
        }
        else
        {
            return trailing_alignment<(I - 1)>();
        }
    }
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
    static constexpr bool CAN_PROVIDE_SIZE = detail::ParameterType::FIXED_SIZE == detail::ParameterTraits<Type>::TYPE;

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
    static constexpr bool CAN_PROVIDE_SIZE = detail::ParameterType::PLAIN != detail::ParameterTraits<Type>::TYPE;

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
#include <type_traits>

namespace cntgs::detail
{
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

struct ElementSize
{
    std::size_t size;
    std::size_t stride;
};

template <class, class...>
class ElementTraits;

template <std::size_t... I, class... Parameter>
class ElementTraits<std::index_sequence<I...>, Parameter...>
{
  private:
    using ListTraits = detail::ParameterListTraits<Parameter...>;

  public:
    template <std::size_t K>
    using ParameterTraitsAt = typename ListTraits::template ParameterTraitsAt<K>;

  private:
    using FixedSizesArray = typename ListTraits::FixedSizesArray;
    using ContiguousPointer = typename detail::ContiguousVectorTraits<Parameter...>::PointerType;
    using ContiguousReference = typename detail::ContiguousVectorTraits<Parameter...>::ReferenceType;
    using FixedSizeGetter = detail::FixedSizeGetter<Parameter...>;

    static constexpr std::size_t LARGEST_LEADING_ALIGNMENT_UNTIL_VARYING_SIZE =
        ListTraits::LARGEST_LEADING_ALIGNMENT_UNTIL_VARYING_SIZE;
    static constexpr std::size_t SKIP = std::numeric_limits<std::size_t>::max();
    static constexpr std::size_t MANUAL = SKIP - 1;

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
        calculate_consecutive_indices<detail::LexicographicalMemcmpCompatible>()};

    template <std::size_t K, std::size_t L, bool IsLhsConst, bool IsRhsConst>
    static constexpr auto get_data_begin_and_end(
        const cntgs::BasicContiguousReference<IsLhsConst, Parameter...>& lhs,
        const cntgs::BasicContiguousReference<IsRhsConst, Parameter...>& rhs) noexcept
    {
        return std::tuple{ParameterTraitsAt<K>::data_begin(cntgs::get<K>(lhs)),
                          ParameterTraitsAt<L>::data_end(cntgs::get<L>(lhs)),
                          ParameterTraitsAt<K>::data_begin(cntgs::get<K>(rhs))};
    }

    template <class ParameterT, bool IgnoreAliasing, std::size_t K, class Args>
    static std::byte* store_one(std::byte* address, std::size_t fixed_size, Args&& args) noexcept
    {
        static constexpr auto PREVIOUS_ALIGNMENT = ListTraits::template previous_alignment<K>();
        return detail::ParameterTraits<ParameterT>::template store<PREVIOUS_ALIGNMENT, IgnoreAliasing>(
            std::forward<Args>(args), address, fixed_size);
    }

    template <bool IgnoreAliasing, class... Args>
    static std::byte* emplace_at(std::byte* address, const FixedSizesArray& fixed_sizes, Args&&... args)
    {
        ((address = store_one<Parameter, IgnoreAliasing, I>(
              address, FixedSizeGetter::template get<Parameter, I>(fixed_sizes), std::forward<Args>(args))),
         ...);
        return address;
    }

    template <class ParameterT, std::size_t K, class FixedSizeGetterType, class FixedSizesType>
    static auto load_one(std::byte* CNTGS_RESTRICT address, const FixedSizesType& fixed_sizes) noexcept
    {
        static constexpr auto PREVIOUS_ALIGNMENT = ListTraits::template previous_alignment<K>();
        static constexpr auto IS_SIZE_PROVIDED = FixedSizeGetterType::template CAN_PROVIDE_SIZE<ParameterT>;
        return detail::ParameterTraits<ParameterT>::template load<PREVIOUS_ALIGNMENT, IS_SIZE_PROVIDED>(
            address, FixedSizeGetterType::template get<ParameterT, K>(fixed_sizes));
    }

  public:
    using StorageElementType = detail::Aligned<LARGEST_LEADING_ALIGNMENT_UNTIL_VARYING_SIZE>;

    template <class StorageType, class Allocator>
    static constexpr StorageType allocate_memory(std::size_t size_in_bytes, const Allocator& allocator)
    {
        const auto remainder = size_in_bytes % LARGEST_LEADING_ALIGNMENT_UNTIL_VARYING_SIZE;
        auto count = size_in_bytes / LARGEST_LEADING_ALIGNMENT_UNTIL_VARYING_SIZE;
        count += remainder == 0 ? 0 : 1;
        return StorageType(count, allocator);
    }

    static constexpr std::byte* align_for_first_parameter(std::byte* address) noexcept
    {
        return detail::align_if<(ListTraits::template trailing_alignment<(sizeof...(I) - 1)>()) <
                                    LARGEST_LEADING_ALIGNMENT_UNTIL_VARYING_SIZE,
                                LARGEST_LEADING_ALIGNMENT_UNTIL_VARYING_SIZE>(address);
    }

    template <class... Args>
    CNTGS_RESTRICT_RETURN static std::byte* emplace_at(std::byte* CNTGS_RESTRICT address,
                                                       const FixedSizesArray& fixed_sizes, Args&&... args)
    {
        return emplace_at<true>(address, fixed_sizes, std::forward<Args>(args)...);
    }

    template <class... Args>
    static std::byte* emplace_at_aliased(std::byte* address, const FixedSizesArray& fixed_sizes, Args&&... args)
    {
        return emplace_at<false>(address, fixed_sizes, std::forward<Args>(args)...);
    }

    template <class FixedSizeGetterType = ElementTraits::FixedSizeGetter,
              class FixedSizesType = ElementTraits::FixedSizesArray>
    static ContiguousPointer load_element_at(std::byte* CNTGS_RESTRICT address,
                                             const FixedSizesType& fixed_sizes) noexcept
    {
        ContiguousPointer result;
        ((std::tie(std::get<I>(result), address) = load_one<Parameter, I, FixedSizeGetterType>(address, fixed_sizes)),
         ...);
        return result;
    }

    static constexpr ElementSize calculate_element_size(const FixedSizesArray& fixed_sizes) noexcept
    {
        std::size_t size{};
        std::size_t offset{};
        std::size_t padding{};
        (
            [&]
            {
                const auto [next_offset, next_size, next_padding] =
                    detail::ParameterTraits<Parameter>::template aligned_size_in_memory<
                        ListTraits::template previous_alignment<I>(), ListTraits::template next_alignment<I>()>(
                        offset, FixedSizeGetter::template get<Parameter, I>(fixed_sizes));
                size += next_size;
                offset = next_offset;
                if constexpr (I == sizeof...(Parameter) - 1)
                {
                    padding = next_padding;
                }
            }(),
            ...);
        return {size, size + padding};
    }

    static constexpr std::size_t calculate_needed_memory_size(std::size_t max_element_count,
                                                              std::size_t varying_size_bytes, ElementSize size) noexcept
    {
        const auto [element_size, element_stride] = size;
        const auto padding = max_element_count == 0 ? 0 : (element_stride - element_size);
        return varying_size_bytes + element_stride * max_element_count - padding;
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
    PointerTuple tuple_;

    BasicContiguousReference() = default;
    ~BasicContiguousReference() = default;

    BasicContiguousReference(const BasicContiguousReference&) = default;
    BasicContiguousReference(BasicContiguousReference&&) = default;

    template <bool OtherIsConst>
    /*implicit*/ constexpr BasicContiguousReference(
        const cntgs::BasicContiguousReference<OtherIsConst, Parameter...>& other) noexcept
        : tuple_(other.tuple_)
    {
    }

    template <class Allocator>
    /*implicit*/ constexpr BasicContiguousReference(
        const cntgs::BasicContiguousElement<Allocator, Parameter...>& other) noexcept
        : BasicContiguousReference(other.reference_)
    {
    }

    template <class Allocator>
    /*implicit*/ constexpr BasicContiguousReference(
        cntgs::BasicContiguousElement<Allocator, Parameter...>& other) noexcept
        : BasicContiguousReference(other.reference_)
    {
    }

    constexpr BasicContiguousReference& operator=(const BasicContiguousReference& other) noexcept(
        ListTraits::IS_NOTHROW_COPY_ASSIGNABLE)
    {
        assign(other);
        return *this;
    }

    template <bool OtherIsConst>
    constexpr BasicContiguousReference& operator=(const cntgs::BasicContiguousReference<OtherIsConst, Parameter...>&
                                                      other) noexcept(ListTraits::IS_NOTHROW_COPY_ASSIGNABLE)
    {
        assign(other);
        return *this;
    }

    template <class Allocator>
    constexpr BasicContiguousReference& operator=(const cntgs::BasicContiguousElement<Allocator, Parameter...>&
                                                      other) noexcept(ListTraits::IS_NOTHROW_COPY_ASSIGNABLE)
    {
        assign(other.reference);
        return *this;
    }

    constexpr BasicContiguousReference& operator=(BasicContiguousReference&& other) noexcept(
        ListTraits::IS_NOTHROW_MOVE_ASSIGNABLE)
    {
        assign(other);
        return *this;
    }

    template <bool OtherIsConst>
    constexpr BasicContiguousReference& operator=(cntgs::BasicContiguousReference<OtherIsConst, Parameter...>&&
                                                      other) noexcept(OtherIsConst
                                                                          ? ListTraits::IS_NOTHROW_COPY_ASSIGNABLE
                                                                          : ListTraits::IS_NOTHROW_MOVE_ASSIGNABLE)
    {
        assign(other);
        return *this;
    }

    template <class Allocator>
    constexpr BasicContiguousReference& operator=(
        cntgs::BasicContiguousElement<Allocator, Parameter...>&& other) noexcept(ListTraits::IS_NOTHROW_MOVE_ASSIGNABLE)
    {
        assign(other.reference_);
        return *this;
    }

    [[nodiscard]] constexpr std::size_t size_in_bytes() const noexcept { return data_end() - data_begin(); }

    [[nodiscard]] constexpr auto data_begin() const noexcept
    {
        return ElementTraits::template ParameterTraitsAt<0>::data_begin(cntgs::get<0>(*this));
    }

    [[nodiscard]] constexpr auto data_end() const noexcept
    {
        return ElementTraits::template ParameterTraitsAt<sizeof...(Parameter) - 1>::data_end(
            cntgs::get<sizeof...(Parameter) - 1>(*this));
    }

    friend constexpr void swap(const BasicContiguousReference& lhs,
                               const BasicContiguousReference& rhs) noexcept(ListTraits::IS_NOTHROW_SWAPPABLE)
    {
        ElementTraits::swap(rhs, lhs);
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
        return *this == other.reference_;
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
        return !(*this == other.reference_);
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
        return *this < other.reference_;
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
        return !(other.reference_ < *this);
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
        return other.reference_ < *this;
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
        return !(*this < other.reference_);
    }

  private:
    friend cntgs::BasicContiguousReference<!IsConst, Parameter...>;

    template <class, class...>
    friend class BasicContiguousVector;

    template <class, class...>
    friend class BasicContiguousElement;

    template <bool, class, class...>
    friend class ContiguousVectorIterator;

    constexpr explicit BasicContiguousReference(std::byte* CNTGS_RESTRICT address,
                                                const typename ListTraits::FixedSizesArray& fixed_sizes = {}) noexcept
        : BasicContiguousReference(ElementTraits::load_element_at(address, fixed_sizes))
    {
    }

    constexpr explicit BasicContiguousReference(const PointerTuple& tuple) noexcept : tuple_(tuple) {}

    template <class Reference>
    void assign(Reference& other) const
    {
        static constexpr auto USE_MOVE = !std::is_const_v<Reference> && !Reference::IS_CONST;
        ElementTraits::template assign<USE_MOVE>(other, *this);
    }
};

template <std::size_t I, bool IsConst, class... Parameter>
[[nodiscard]] constexpr std::tuple_element_t<I, cntgs::BasicContiguousReference<IsConst, Parameter...>> get(
    const cntgs::BasicContiguousReference<IsConst, Parameter...>& reference) noexcept
{
    if constexpr (IsConst)
    {
        return detail::as_const_ref(std::get<I>(reference.tuple_));
    }
    else
    {
        return detail::as_ref(std::get<I>(reference.tuple_));
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
    using StorageElementType = typename std::allocator_traits<Allocator>::value_type;
    using StorageType = detail::AllocatorAwarePointer<
        typename std::allocator_traits<Allocator>::template rebind_alloc<StorageElementType>>;
    using Reference = typename VectorTraits::ReferenceType;

  public:
    using allocator_type = Allocator;

    StorageType memory_;
    Reference reference_;

    template <bool IsConst>
    /*implicit*/ BasicContiguousElement(const cntgs::BasicContiguousReference<IsConst, Parameter...>& other,
                                        const allocator_type& allocator = {})
        : memory_(ElementTraits::template allocate_memory<StorageType>(other.size_in_bytes(), allocator)),
          reference_(store_and_load(other, other.size_in_bytes()))
    {
    }

    template <bool IsConst>
    /*implicit*/ BasicContiguousElement(cntgs::BasicContiguousReference<IsConst, Parameter...>&& other,
                                        const allocator_type& allocator = {})
        : memory_(ElementTraits::template allocate_memory<StorageType>(other.size_in_bytes(), allocator)),
          reference_(store_and_load(other, other.size_in_bytes()))
    {
    }

    /*implicit*/ BasicContiguousElement(const BasicContiguousElement& other)
        : memory_(other.memory_), reference_(store_and_load(other.reference_, other.reference_.size_in_bytes()))
    {
    }

    template <class OtherAllocator>
    explicit BasicContiguousElement(const BasicContiguousElement<OtherAllocator, Parameter...>& other)
        : memory_(other.memory_), reference_(store_and_load(other.reference_, other.reference_.size_in_bytes()))
    {
    }

    template <class OtherAllocator>
    BasicContiguousElement(const BasicContiguousElement<OtherAllocator, Parameter...>& other,
                           const allocator_type& allocator)
        : memory_(ElementTraits::template allocate_memory<StorageType>(other.reference_.size_in_bytes(), allocator)),
          reference_(store_and_load(other.reference_, other.reference_.size_in_bytes()))
    {
    }

    BasicContiguousElement(BasicContiguousElement&&) = default;

    template <class OtherAllocator>
    constexpr explicit BasicContiguousElement(BasicContiguousElement<OtherAllocator, Parameter...>&& other) noexcept
        : memory_(std::move(other.memory_)), reference_(std::move(other.reference_))
    {
    }

    template <class OtherAllocator>
    constexpr BasicContiguousElement(
        BasicContiguousElement<OtherAllocator, Parameter...>&& other,
        const allocator_type& allocator) noexcept(detail::ARE_EQUALITY_COMPARABLE<allocator_type, OtherAllocator> &&
                                                  AllocatorTraits::is_always_equal::value)
        : memory_(acquire_memory(other, allocator)), reference_(acquire_reference(other, allocator))
    {
    }

    ~BasicContiguousElement() noexcept { destruct(); }

    BasicContiguousElement& operator=(const BasicContiguousElement& other)
    {
        if (this != std::addressof(other))
        {
            copy_assign(other);
        }
        return *this;
    }

    BasicContiguousElement& operator=(BasicContiguousElement&& other) noexcept(
        AllocatorTraits::is_always_equal::value || AllocatorTraits::propagate_on_container_move_assignment::value)
    {
        if (this != std::addressof(other))
        {
            move_assign(std::move(other));
        }
        return *this;
    }

    template <bool IsConst>
    constexpr BasicContiguousElement& operator=(const cntgs::BasicContiguousReference<IsConst, Parameter...>&
                                                    other) noexcept(ListTraits::IS_NOTHROW_COPY_ASSIGNABLE)
    {
        reference_ = other;
        return *this;
    }

    template <bool IsConst>
    constexpr BasicContiguousElement& operator=(cntgs::BasicContiguousReference<IsConst, Parameter...>&&
                                                    other) noexcept(IsConst ? ListTraits::IS_NOTHROW_COPY_ASSIGNABLE
                                                                            : ListTraits::IS_NOTHROW_MOVE_ASSIGNABLE)
    {
        reference_ = std::move(other);
        return *this;
    }

    [[nodiscard]] constexpr allocator_type get_allocator() const noexcept { return memory_.get_allocator(); }

    friend constexpr void swap(BasicContiguousElement& lhs, BasicContiguousElement& rhs) noexcept
    {
        detail::swap(lhs.memory_, rhs.memory_);
        std::swap(lhs.reference_.tuple_, rhs.reference_.tuple_);
    }

    template <bool IsConst>
    [[nodiscard]] constexpr auto operator==(const cntgs::BasicContiguousReference<IsConst, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTHROW_EQUALITY_COMPARABLE)
    {
        return reference_ == other;
    }

    [[nodiscard]] constexpr auto operator==(const BasicContiguousElement& other) const
        noexcept(ListTraits::IS_NOTHROW_EQUALITY_COMPARABLE)
    {
        return reference_ == other.reference_;
    }

    template <bool IsConst>
    [[nodiscard]] constexpr auto operator!=(const cntgs::BasicContiguousReference<IsConst, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTHROW_EQUALITY_COMPARABLE)
    {
        return !(reference_ == other);
    }

    [[nodiscard]] constexpr auto operator!=(const BasicContiguousElement& other) const
        noexcept(ListTraits::IS_NOTHROW_EQUALITY_COMPARABLE)
    {
        return !(reference_ == other.reference_);
    }

    template <bool IsConst>
    [[nodiscard]] constexpr auto operator<(const cntgs::BasicContiguousReference<IsConst, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return reference_ < other;
    }

    [[nodiscard]] constexpr auto operator<(const BasicContiguousElement& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return reference_ < other.reference_;
    }

    template <bool IsConst>
    [[nodiscard]] constexpr auto operator<=(const cntgs::BasicContiguousReference<IsConst, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return !(other < reference_);
    }

    [[nodiscard]] constexpr auto operator<=(const BasicContiguousElement& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return !(other.reference_ < reference_);
    }

    template <bool IsConst>
    [[nodiscard]] constexpr auto operator>(const cntgs::BasicContiguousReference<IsConst, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return other < reference_;
    }

    [[nodiscard]] constexpr auto operator>(const BasicContiguousElement& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return other.reference_ < reference_;
    }

    template <bool IsConst>
    [[nodiscard]] constexpr auto operator>=(const cntgs::BasicContiguousReference<IsConst, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return !(reference_ < other);
    }

    [[nodiscard]] constexpr auto operator>=(const BasicContiguousElement& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return !(reference_ < other.reference_);
    }

  private:
    template <class SourceReference>
    auto store_and_load(SourceReference& source, std::size_t memory_size) const
    {
        return store_and_load(source, memory_size, memory_begin());
    }

    template <class SourceReference>
    auto store_and_load(SourceReference& source, std::size_t memory_size, std::byte* target_memory) const
    {
        static constexpr auto USE_MOVE = !std::is_const_v<SourceReference> && !SourceReference::IS_CONST;
        std::memcpy(target_memory, source.data_begin(), memory_size);
        auto target =
            ElementTraits::template load_element_at<detail::ContiguousReferenceSizeGetter>(target_memory, source);
        ElementTraits::template construct_if_non_trivial<USE_MOVE>(source, target);
        return Reference{target};
    }

    template <class OtherAllocator>
    auto acquire_memory(BasicContiguousElement<OtherAllocator, Parameter...>& other,
                        [[maybe_unused]] const allocator_type& allocator) const
    {
        if constexpr (detail::ARE_EQUALITY_COMPARABLE<allocator_type, OtherAllocator>)
        {
            if constexpr (AllocatorTraits::is_always_equal::value)
            {
                return std::move(other.memory_);
            }
            else
            {
                if (allocator == other.memory_.get_allocator())
                {
                    return std::move(other.memory_);
                }
                return StorageType(other.memory_.size(), allocator);
            }
        }
        else
        {
            return StorageType(other.memory_.size(), allocator);
        }
    }

    template <class OtherAllocator>
    auto acquire_reference(BasicContiguousElement<OtherAllocator, Parameter...>& other,
                           [[maybe_unused]] const allocator_type& allocator) const
    {
        if constexpr (detail::ARE_EQUALITY_COMPARABLE<allocator_type, OtherAllocator>)
        {
            if constexpr (AllocatorTraits::is_always_equal::value)
            {
                return std::move(other.reference_);
            }
            else
            {
                if (allocator == other.memory_.get_allocator())
                {
                    return std::move(other.reference_);
                }
                return store_and_load(other.reference_, other.memory_.size());
            }
        }
        else
        {
            return store_and_load(other.reference_, other.memory_.size());
        }
    }

    auto memory_begin() const noexcept { return BasicContiguousElement::memory_begin(memory_); }

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
                reference_ = other.reference_;
                memory_.propagate_on_container_copy_assignment(other.memory_);
            }
            else
            {
                destruct();
                memory_ = other.memory_;
                store_and_construct_reference_inplace(other.reference_, other.memory_.size());
            }
        }
    }

    template <class OtherAllocator>
    constexpr void steal(BasicContiguousElement<OtherAllocator, Parameter...>&& other) noexcept
    {
        destruct();
        memory_ = std::move(other.memory_);
        reference_.tuple_ = std::move(other.reference_.tuple_);
    }

    template <class OtherAllocator>
    constexpr void move_assign(BasicContiguousElement<OtherAllocator, Parameter...>&& other)
    {
        if constexpr (AllocatorTraits::is_always_equal::value ||
                      AllocatorTraits::propagate_on_container_move_assignment::value)
        {
            steal(std::move(other));
        }
        else
        {
            if (get_allocator() == other.get_allocator())
            {
                steal(std::move(other));
            }
            else
            {
                if constexpr (ListTraits::IS_FIXED_SIZE_OR_PLAIN)
                {
                    reference_ = std::move(other.reference_);
                    memory_.propagate_on_container_move_assignment(other.memory_);
                }
                else
                {
                    const auto other_size_in_bytes = other.reference_.size_in_bytes();
                    if (other_size_in_bytes > memory_.size())
                    {
                        // allocate memory first because it might throw
                        StorageType new_memory{other.memory_.size(), get_allocator()};
                        destruct();
                        reference_.tuple_ = store_and_load(other.reference_, other_size_in_bytes,
                                                           BasicContiguousElement::memory_begin(new_memory))
                                                .tuple_;
                        memory_ = std::move(new_memory);
                    }
                    else
                    {
                        destruct();
                        store_and_construct_reference_inplace(other.reference_, other_size_in_bytes);
                    }
                }
            }
        }
    }

    template <class SourceReference>
    void store_and_construct_reference_inplace(SourceReference& other, std::size_t memory_size)
    {
        reference_.tuple_ = store_and_load(other, memory_size).tuple_;
    }

    void destruct() noexcept
    {
        if constexpr (!ListTraits::IS_TRIVIALLY_DESTRUCTIBLE)
        {
            if (memory_)
            {
                ElementTraits::destruct(reference_);
            }
        }
    }
};

template <std::size_t I, class Allocator, class... Parameter>
[[nodiscard]] constexpr decltype(auto) get(cntgs::BasicContiguousElement<Allocator, Parameter...>& element) noexcept
{
    return cntgs::get<I>(element.reference_);
}

template <std::size_t I, class Allocator, class... Parameter>
[[nodiscard]] constexpr decltype(auto) get(
    const cntgs::BasicContiguousElement<Allocator, Parameter...>& element) noexcept
{
    return detail::as_const(cntgs::get<I>(element.reference_));
}

template <std::size_t I, class Allocator, class... Parameter>
[[nodiscard]] constexpr decltype(auto) get(cntgs::BasicContiguousElement<Allocator, Parameter...>&& element) noexcept
{
    return std::move(cntgs::get<I>(element.reference_));
}

template <std::size_t I, class Allocator, class... Parameter>
[[nodiscard]] constexpr decltype(auto) get(
    const cntgs::BasicContiguousElement<Allocator, Parameter...>&& element) noexcept
{
    return detail::as_const(std::move(cntgs::get<I>(element.reference_)));
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

// #include "cntgs/detail/elementTraits.hpp"

// #include "cntgs/detail/parameterListTraits.hpp"

// #include "cntgs/detail/typeTraits.hpp"

// #include "cntgs/detail/utility.hpp"

#include <algorithm>
#include <cstring>
#include <vector>

namespace cntgs::detail
{
template <class Locator>
auto move_elements(std::size_t from, std::size_t to, std::byte* memory_begin, const Locator& locator) noexcept
{
    const auto target = locator.element_address(to, memory_begin);
    const auto source = locator.element_address(from, memory_begin);
    const auto count = static_cast<std::size_t>(locator.data_end() - source);
    std::memmove(target, source, count);
    return source - target;
}

inline std::byte* get_mixed_element_address(std::size_t index, std::byte* memory_begin,
                                            const std::size_t element_addresses[]) noexcept
{
    return memory_begin + element_addresses[index];
}

class IteratorMixedElementLocator;

template <class Allocator>
class BaseElementLocator
{
  protected:
    friend detail::IteratorMixedElementLocator;

    using ElementAddresses =
        std::vector<std::size_t, typename std::allocator_traits<Allocator>::template rebind_alloc<std::size_t>>;

    ElementAddresses element_addresses_;
    std::byte* last_element_{};

    BaseElementLocator() = default;

    explicit BaseElementLocator(ElementAddresses element_addresses, std::size_t new_max_element_count)
        : element_addresses_(std::move(element_addresses))
    {
        element_addresses_.reserve(new_max_element_count);
    }

    explicit BaseElementLocator(std::byte* last_element, std::size_t max_element_count,
                                const Allocator& allocator) noexcept
        : element_addresses_(allocator), last_element_(last_element)
    {
        element_addresses_.reserve(max_element_count);
    }

    friend void swap(BaseElementLocator& lhs, BaseElementLocator& rhs) noexcept
    {
        std::swap(lhs.element_addresses_, rhs.element_addresses_);
        std::swap(lhs.last_element_, rhs.last_element_);
    }

  public:
    static constexpr auto reserved_bytes(std::size_t) noexcept { return std::size_t{}; }

    bool empty(const std::byte*) const noexcept { return element_addresses_.empty(); }

    std::size_t memory_size() const noexcept
    {
        return element_addresses_.size() * sizeof(typename ElementAddresses::value_type);
    }

    std::size_t size(const std::byte*) const noexcept { return element_addresses_.size(); }

    std::byte* element_address(std::size_t index, std::byte* memory_begin) const noexcept
    {
        return detail::get_mixed_element_address(index, memory_begin, element_addresses_.data());
    }

    constexpr auto data_end() const noexcept { return last_element_; }

    void resize(std::size_t new_size, std::byte* memory_begin) noexcept
    {
        last_element_ = element_address(new_size, memory_begin);
        element_addresses_.resize(new_size);
    }

    void move_elements_forward(std::size_t from, std::size_t to, std::byte* memory_begin) noexcept
    {
        const auto diff = detail::move_elements(from, to, memory_begin, *this);
        std::transform(element_addresses_.begin() + from, element_addresses_.end(), element_addresses_.begin() + to,
                       [&](auto address)
                       {
                           return address - diff;
                       });
    }

    void make_room_for_last_element_at(std::size_t index, std::size_t size_of_element, std::byte* memory_begin) noexcept
    {
        const auto source = element_address(index, memory_begin);
        const auto target = source + size_of_element;
        const auto count = static_cast<std::size_t>(data_end() - source);
        std::memmove(target, source, count);
        element_addresses_.emplace_back(data_end() - memory_begin);
        const auto begin = element_addresses_.begin() + index + 1;
        std::transform(begin, element_addresses_.end(), begin,
                       [&](auto address)
                       {
                           return address + size_of_element;
                       });
    }
};

template <class Allocator, class... Parameter>
class ElementLocator : public BaseElementLocator<Allocator>
{
  private:
    using Base = BaseElementLocator<Allocator>;
    using ElementTraits = detail::ElementTraitsT<Parameter...>;
    using FixedSizesArray = typename detail::ParameterListTraits<Parameter...>::FixedSizesArray;

  public:
    ElementLocator() = default;

    ElementLocator(std::size_t max_element_count, std::byte* memory_begin, ElementSize,
                   const Allocator& allocator) noexcept
        : Base{memory_begin, max_element_count, allocator}
    {
    }

    template <class Locator>
    ElementLocator(Locator&& old_locator, std::byte* old_memory_begin, std::size_t new_max_element_count,
                   std::byte* new_memory_begin)
        : Base{std::forward<Locator>(old_locator).element_addresses_, new_max_element_count}
    {
        trivially_copy_into(old_locator.last_element_, old_memory_begin, new_memory_begin);
    }

    template <class... Args>
    auto emplace_back(std::byte* memory_begin, const FixedSizesArray& fixed_sizes, Args&&... args)
    {
        const auto last_element = ElementTraits::align_for_first_parameter(this->last_element_);
        const auto new_last_element = ElementTraits::emplace_at(last_element, fixed_sizes, std::forward<Args>(args)...);
        this->element_addresses_.emplace_back(last_element - memory_begin);
        this->last_element_ = new_last_element;
        return new_last_element;
    }

    template <class... Args>
    auto emplace_at(std::size_t index, std::byte* memory_begin, const FixedSizesArray& fixed_sizes, Args&&... args)
    {
        const auto element_addresses_begin = ElementTraits::emplace_at_aliased(
            memory_begin + this->element_addresses_[index], fixed_sizes, std::forward<Args>(args)...);
        this->element_addresses_[index + 1] = element_addresses_begin - memory_begin;
        return element_addresses_begin;
    }

    template <class... Args>
    static auto emplace_at(std::byte* address, const std::byte*, const FixedSizesArray& fixed_sizes, Args&&... args)
    {
        ElementTraits::emplace_at_aliased(address, fixed_sizes, std::forward<Args>(args)...);
    }

    void trivially_copy_into(std::byte* CNTGS_RESTRICT old_memory_begin,
                             std::byte* CNTGS_RESTRICT new_memory_begin) noexcept
    {
        trivially_copy_into(this->last_element_, old_memory_begin, new_memory_begin);
    }

    static constexpr std::size_t calculate_new_memory_size(std::size_t max_element_count,
                                                           std::size_t varying_size_bytes,
                                                           const FixedSizesArray& fixed_sizes) noexcept
    {
        return ElementTraits::calculate_needed_memory_size(max_element_count, varying_size_bytes,
                                                           ElementTraits::calculate_element_size(fixed_sizes));
    }

  private:
    void trivially_copy_into(std::byte* old_last_element, std::byte* CNTGS_RESTRICT old_memory_begin,
                             std::byte* CNTGS_RESTRICT new_memory_begin) noexcept
    {
        const auto new_start = new_memory_begin;
        const auto old_start = old_memory_begin;
        const auto size_diff = std::distance(new_memory_begin, new_start) - std::distance(old_memory_begin, old_start);
        const auto old_used_memory_size = std::distance(old_start, old_last_element);
        std::memcpy(new_start, old_start, old_used_memory_size);
        this->last_element_ = new_memory_begin + std::distance(old_memory_begin, old_last_element) + size_diff;
    }
};

class BaseAllFixedSizeElementLocator
{
  protected:
    std::size_t element_count_{};
    std::size_t stride_{};
    std::byte* start_{};

    BaseAllFixedSizeElementLocator() = default;

    constexpr BaseAllFixedSizeElementLocator(std::size_t element_count, std::size_t stride, std::byte* start) noexcept
        : element_count_(element_count), stride_(stride), start_(start)
    {
    }

  public:
    constexpr bool empty(const std::byte*) const noexcept { return element_count_ == std::size_t{}; }

    static constexpr std::size_t memory_size() noexcept { return {}; }

    constexpr std::size_t size(const std::byte*) const noexcept { return element_count_; }

    constexpr std::byte* element_address(std::size_t index, const std::byte*) const noexcept
    {
        return start_ + stride_ * index;
    }

    constexpr auto data_end() const noexcept { return start_ + stride_ * element_count_; }

    constexpr void resize(std::size_t new_size, const std::byte*) noexcept { element_count_ = new_size; }

    void move_elements_forward(std::size_t from, std::size_t to, const std::byte*) const noexcept
    {
        detail::move_elements(from, to, {}, *this);
    }

    void make_room_for_last_element_at(std::size_t from, std::size_t size_of_element, const std::byte*) const noexcept
    {
        const auto source = element_address(from, {});
        const auto target = source + size_of_element;
        const auto count = static_cast<std::size_t>(data_end() - source);
        std::memmove(target, source, count);
    }
};

template <class... Parameter>
class AllFixedSizeElementLocator : public BaseAllFixedSizeElementLocator
{
  private:
    using ElementTraits = detail::ElementTraitsT<Parameter...>;
    using FixedSizesArray = typename detail::ParameterListTraits<Parameter...>::FixedSizesArray;

  public:
    AllFixedSizeElementLocator() = default;

    template <class Allocator>
    constexpr AllFixedSizeElementLocator(std::size_t, std::byte* memory_begin, ElementSize element_stride,
                                         const Allocator&) noexcept
        : BaseAllFixedSizeElementLocator({}, element_stride.stride, memory_begin)
    {
    }

    AllFixedSizeElementLocator(const AllFixedSizeElementLocator& old_locator, const std::byte*, std::size_t,
                               std::byte* new_memory_begin) noexcept
        : BaseAllFixedSizeElementLocator(old_locator.element_count_, old_locator.stride_, {})
    {
        trivially_copy_into(old_locator, new_memory_begin);
    }

    template <class... Args>
    auto emplace_back(const std::byte*, const FixedSizesArray& fixed_sizes, Args&&... args)
    {
        const auto last_element = element_address(element_count_, {});
        const auto end = ElementTraits::emplace_at(last_element, fixed_sizes, std::forward<Args>(args)...);
        ++element_count_;
        return end;
    }

    template <class... Args>
    auto emplace_at(std::size_t index, const std::byte*, const FixedSizesArray& fixed_sizes, Args&&... args)
    {
        return ElementTraits::emplace_at_aliased(element_address(index, {}), fixed_sizes, std::forward<Args>(args)...);
    }

    void trivially_copy_into(const std::byte*, std::byte* new_memory_begin) noexcept
    {
        trivially_copy_into(*this, new_memory_begin);
    }

    constexpr std::size_t calculate_new_memory_size(std::size_t max_element_count, std::size_t varying_size_bytes,
                                                    const FixedSizesArray&) noexcept
    {
        return varying_size_bytes + stride_ * max_element_count;
    }

  private:
    void trivially_copy_into(const AllFixedSizeElementLocator& old_locator, std::byte* new_memory_begin) noexcept
    {
        std::memcpy(new_memory_begin, old_locator.start_, old_locator.element_count_ * old_locator.stride_);
        start_ = new_memory_begin;
    }
};

template <class Allocator, class... Parameter>
using ElementLocatorT =
    detail::ConditionalT<detail::ParameterListTraits<Parameter...>::IS_FIXED_SIZE_OR_PLAIN,
                         AllFixedSizeElementLocator<Parameter...>, ElementLocator<Allocator, Parameter...>>;

template <class Allocator, class... Parameter>
class ElementLocatorAndFixedSizes
    : private detail::EmptyBaseOptimization<typename detail::ParameterListTraits<Parameter...>::FixedSizesArray>
{
  private:
    using FixedSizesArray = typename detail::ParameterListTraits<Parameter...>::FixedSizesArray;
    using Locator = detail::ElementLocatorT<Allocator, Parameter...>;
    using Base = detail::EmptyBaseOptimization<FixedSizesArray>;

  public:
    Locator locator_;

    ElementLocatorAndFixedSizes() = default;

    constexpr ElementLocatorAndFixedSizes(const Locator& locator, const FixedSizesArray& fixed_sizes) noexcept
        : Base{fixed_sizes}, locator_(locator)
    {
    }

    constexpr ElementLocatorAndFixedSizes(std::size_t max_element_count, std::byte* memory,
                                          const FixedSizesArray& fixed_sizes, ElementSize element_size,
                                          const Allocator& allocator) noexcept
        : Base{fixed_sizes}, locator_(max_element_count, memory, element_size, allocator)
    {
    }

    constexpr auto operator->() noexcept { return std::addressof(locator_); }

    constexpr auto operator->() const noexcept { return std::addressof(locator_); }

    constexpr auto& operator*() noexcept { return locator_; }

    constexpr const auto& operator*() const noexcept { return locator_; }

    constexpr auto& fixed_sizes() noexcept { return Base::get(); }

    constexpr const auto& fixed_sizes() const noexcept { return Base::get(); }
};

class IteratorMixedElementLocator
{
  private:
    std::size_t* element_addresses_;

  public:
    template <class Allocator>
    explicit IteratorMixedElementLocator(BaseElementLocator<Allocator>& locator)
        : element_addresses_(locator.element_addresses_.data())
    {
    }

    auto element_address(std::size_t index, std::byte* memory_begin) const noexcept
    {
        return detail::get_mixed_element_address(index, memory_begin, element_addresses_);
    }
};

using IteratorAllFixedSizeElementLocator = BaseAllFixedSizeElementLocator;

template <bool IsAllFixedSize, class FixedSizesArray>
class IteratorElementLocatorAndFixedSizes : private detail::EmptyBaseOptimization<FixedSizesArray>
{
  private:
    using Locator =
        detail::ConditionalT<IsAllFixedSize, IteratorAllFixedSizeElementLocator, IteratorMixedElementLocator>;
    using Base = detail::EmptyBaseOptimization<FixedSizesArray>;

  public:
    Locator locator_;

    IteratorElementLocatorAndFixedSizes() = default;

    template <class... Parameter>
    constexpr IteratorElementLocatorAndFixedSizes(ElementLocatorAndFixedSizes<Parameter...>& locator) noexcept
        : Base{locator.fixed_sizes()}, locator_(locator.locator_)
    {
    }

    constexpr auto operator->() noexcept { return std::addressof(locator_); }

    constexpr auto operator->() const noexcept { return std::addressof(locator_); }

    constexpr auto& operator*() noexcept { return locator_; }

    constexpr const auto& operator*() const noexcept { return locator_; }

    constexpr auto& fixed_sizes() noexcept { return Base::get(); }

    constexpr const auto& fixed_sizes() const noexcept { return Base::get(); }
};
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
    using ElementLocatorAndFixedSizes = detail::IteratorElementLocatorAndFixedSizes<
        detail::ParameterListTraits<Parameter...>::IS_FIXED_SIZE_OR_PLAIN,
        typename detail::ParameterListTraits<Parameter...>::FixedSizesArray>;
    using SizeType = typename Vector::size_type;
    using StoragePointer = typename std::allocator_traits<
        typename std::allocator_traits<typename Vector::allocator_type>::template rebind_alloc<std::byte>>::pointer;

  public:
    using value_type = typename Vector::value_type;
    using reference = detail::ConditionalT<IsConst, typename Vector::const_reference, typename Vector::reference>;
    using pointer = detail::ArrowProxy<reference>;
    using difference_type = typename Vector::difference_type;
    using iterator_category = std::random_access_iterator_tag;

    ContiguousVectorIterator() = default;

    constexpr ContiguousVectorIterator(const Vector& vector, SizeType index) noexcept
        : i_(index), memory_(vector.memory_begin()), locator_(const_cast<Vector&>(vector).locator_)
    {
    }

    constexpr explicit ContiguousVectorIterator(const Vector& vector) noexcept
        : ContiguousVectorIterator(vector, SizeType{})
    {
    }

    template <bool OtherIsConst>
    /*implicit*/ constexpr ContiguousVectorIterator(
        const ContiguousVectorIterator<OtherIsConst, Options, Parameter...>& other) noexcept
        : i_(other.i_), memory_(other.memory_), locator_(other.locator_)
    {
    }

    ContiguousVectorIterator(const ContiguousVectorIterator&) = default;
    ContiguousVectorIterator(ContiguousVectorIterator&&) = default;

    template <bool OtherIsConst>
    constexpr ContiguousVectorIterator& operator=(
        const ContiguousVectorIterator<OtherIsConst, Options, Parameter...>& other) noexcept
    {
        i_ = other.i_;
        memory_ = other.memory_;
        locator_ = other.locator_;
        return *this;
    }

    ContiguousVectorIterator& operator=(const ContiguousVectorIterator&) = default;
    ContiguousVectorIterator& operator=(ContiguousVectorIterator&&) = default;

    [[nodiscard]] constexpr auto index() const noexcept { return i_; }

    [[nodiscard]] constexpr auto data() const noexcept -> detail::ConditionalT<IsConst, const std::byte*, std::byte*>
    {
        return locator_->element_address(i_, memory_);
    }

    [[nodiscard]] constexpr reference operator*() noexcept
    {
        return reference{locator_->element_address(i_, memory_), locator_.fixed_sizes()};
    }

    [[nodiscard]] constexpr reference operator*() const noexcept
    {
        return reference{locator_->element_address(i_, memory_), locator_.fixed_sizes()};
    }

    [[nodiscard]] constexpr pointer operator->() const noexcept { return {*(*this)}; }

    constexpr ContiguousVectorIterator& operator++() noexcept
    {
        ++i_;
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
        --i_;
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
        copy.i_ += diff;
        return copy;
    }

    [[nodiscard]] constexpr difference_type operator+(ContiguousVectorIterator it) const noexcept { return i_ + it.i_; }

    constexpr ContiguousVectorIterator& operator+=(difference_type diff) noexcept
    {
        i_ += diff;
        return *this;
    }

    [[nodiscard]] constexpr ContiguousVectorIterator operator-(difference_type diff) const noexcept
    {
        auto copy{*this};
        copy.i_ -= diff;
        return copy;
    }

    [[nodiscard]] constexpr difference_type operator-(ContiguousVectorIterator it) const noexcept { return i_ - it.i_; }

    constexpr ContiguousVectorIterator& operator-=(difference_type diff) noexcept
    {
        i_ -= diff;
        return *this;
    }

    [[nodiscard]] reference operator[](difference_type diff) const noexcept { return *(*this + diff); }

    [[nodiscard]] constexpr bool operator==(const ContiguousVectorIterator& other) const noexcept
    {
        return i_ == other.i_ && memory_ == other.memory_;
    }

    [[nodiscard]] constexpr bool operator!=(const ContiguousVectorIterator& other) const noexcept
    {
        return !(*this == other);
    }

    [[nodiscard]] constexpr bool operator<(const ContiguousVectorIterator& other) const noexcept
    {
        return i_ < other.i_ && memory_ == other.memory_;
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

    SizeType i_{};
    StoragePointer memory_;
    ElementLocatorAndFixedSizes locator_;
};
}  // namespace cntgs

#endif  // CNTGS_CNTGS_ITERATOR_HPP

// #include "cntgs/parameter.hpp"

// #include "cntgs/reference.hpp"

// #include "cntgs/span.hpp"

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
    using ListTraits = detail::ParameterListTraits<Parameter...>;
    using VectorTraits = detail::ContiguousVectorTraits<Parameter...>;
    using ElementTraits = detail::ElementTraitsT<Parameter...>;
    using Allocator = typename std::allocator_traits<typename ParsedOptions::Allocator>::template rebind_alloc<
        typename ElementTraits::StorageElementType>;
    using ElementLocator = detail::ElementLocatorT<Allocator, Parameter...>;
    using ElementLocatorAndFixedSizes = detail::ElementLocatorAndFixedSizes<Allocator, Parameter...>;
    using AllocatorTraits = std::allocator_traits<Allocator>;
    using StorageType = detail::AllocatorAwarePointer<Allocator>;
    using StorageElementType = typename ElementTraits::StorageElementType;
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

    size_type max_element_count_{};
    StorageType memory_{};
    ElementLocatorAndFixedSizes locator_;

    BasicContiguousVector() = default;

    template <bool IsMixed = IS_MIXED>
    BasicContiguousVector(size_type max_element_count, size_type varying_size_bytes, const FixedSizes& fixed_sizes,
                          const allocator_type& allocator = {}, std::enable_if_t<IsMixed>* = nullptr)
        : BasicContiguousVector(max_element_count, varying_size_bytes, fixed_sizes, allocator, 0)
    {
    }

    template <bool IsAllFixedSize = IS_ALL_FIXED_SIZE>
    constexpr BasicContiguousVector(size_type max_element_count, const FixedSizes& fixed_sizes,
                                    const allocator_type& allocator = {}, std::enable_if_t<IsAllFixedSize>* = nullptr)
        : BasicContiguousVector(max_element_count, size_type{}, fixed_sizes, allocator, 0)
    {
    }

    template <bool IsAllVaryingSize = IS_ALL_VARYING_SIZE>
    BasicContiguousVector(size_type max_element_count, size_type varying_size_bytes,
                          const allocator_type& allocator = {}, std::enable_if_t<IsAllVaryingSize>* = nullptr)
        : BasicContiguousVector(max_element_count, varying_size_bytes, FixedSizes{}, allocator, 0)
    {
    }

    template <bool IsNoneSpecial = IS_ALL_PLAIN>
    constexpr explicit BasicContiguousVector(size_type max_element_count, std::enable_if_t<IsNoneSpecial>* = nullptr)
        : BasicContiguousVector(max_element_count, size_type{}, FixedSizes{}, allocator_type{}, 0)
    {
    }

    template <bool IsNoneSpecial = IS_ALL_PLAIN>
    constexpr BasicContiguousVector(size_type max_element_count, const allocator_type& allocator,
                                    std::enable_if_t<IsNoneSpecial>* = nullptr)
        : BasicContiguousVector(max_element_count, size_type{}, FixedSizes{}, allocator)
    {
    }

    BasicContiguousVector(const BasicContiguousVector& other)
        : max_element_count_(other.max_element_count_), memory_(other.memory_), locator_(copy_construct_locator(other))
    {
    }

    BasicContiguousVector(BasicContiguousVector&&) = default;

    BasicContiguousVector& operator=(const BasicContiguousVector& other)
    {
        if (this != std::addressof(other))
        {
            copy_assign(other);
        }
        return *this;
    }

    constexpr BasicContiguousVector& operator=(BasicContiguousVector&& other) noexcept(
        AllocatorTraits::is_always_equal::value || AllocatorTraits::propagate_on_container_move_assignment::value)
    {
        if (this != std::addressof(other))
        {
            move_assign(std::move(other));
        }
        return *this;
    }

#if __cpp_constexpr_dynamic_alloc
    constexpr
#endif
        ~BasicContiguousVector() noexcept
    {
        destruct_if_owned();
    }

    template <class... Args>
    void emplace_back(Args&&... args)
    {
        emplace_back_impl(std::forward<Args>(args)...);
    }

    template <class... Args>
    iterator emplace(const_iterator position, Args&&... args)
    {
        auto it = make_iterator(position);
        const auto target_begin = it.data();
        const auto back_begin = data_end();
        const auto back_end = emplace_back_impl(std::forward<Args>(args)...);
        const auto byte_count = back_end - back_begin;
        make_room_for_last_element_at(it.index(), byte_count);
        std::memcpy(target_begin, back_end, byte_count);
        auto&& source = (*this)[size()];
        auto&& target = ElementTraits::load_element_at(target_begin, locator_.fixed_sizes());
        ElementTraits::template construct_if_non_trivial<true>(source, target);
        return std::next(begin(), it.index());
    }

    void pop_back() noexcept
    {
        ElementTraits::destruct(back());
        locator_->resize(size() - size_type{1}, memory_begin());
    }

    void reserve(size_type new_max_element_count, size_type new_varying_size_bytes = {})
    {
        if (max_element_count_ < new_max_element_count)
        {
            grow(new_max_element_count, new_varying_size_bytes);
        }
    }

    iterator erase(const_iterator position) noexcept(ListTraits::IS_NOTHROW_MOVE_CONSTRUCTIBLE)
    {
        auto it_position = make_iterator(position);
        const auto next_position = position.index() + 1;
        ElementTraits::destruct(*it_position);
        move_elements_forward(next_position, it_position.index());
        locator_->resize(size() - size_type{1}, memory_begin());
        return it_position;
    }

    iterator erase(const_iterator first, const_iterator last) noexcept(ListTraits::IS_NOTHROW_MOVE_CONSTRUCTIBLE)
    {
        const auto current_size = size();
        const auto it_first = make_iterator(first);
        const auto it_last = make_iterator(last);
        BasicContiguousVector::destruct(it_first, it_last);
        if (last.index() < current_size && first.index() != last.index())
        {
            move_elements_forward(last.index(), first.index());
        }
        locator_->resize(current_size - (last.index() - first.index()), memory_begin());
        return it_first;
    }

    void clear() noexcept
    {
        destruct();
        locator_->resize(0, memory_begin());
    }

    [[nodiscard]] reference operator[](size_type i) noexcept
    {
        return reference{locator_->element_address(i, memory_begin()), locator_.fixed_sizes()};
    }

    [[nodiscard]] const_reference operator[](size_type i) const noexcept
    {
        return const_reference{locator_->element_address(i, memory_begin()), locator_.fixed_sizes()};
    }

    [[nodiscard]] reference front() noexcept { return (*this)[{}]; }

    [[nodiscard]] const_reference front() const noexcept { return (*this)[{}]; }

    [[nodiscard]] reference back() noexcept { return (*this)[size() - size_type{1}]; }

    [[nodiscard]] const_reference back() const noexcept { return (*this)[size() - size_type{1}]; }

    template <std::size_t I>
    [[nodiscard]] constexpr size_type get_fixed_size() const noexcept
    {
        return detail::get<I>(locator_.fixed_sizes());
    }

    [[nodiscard]] constexpr bool empty() const noexcept { return locator_->empty(memory_begin()); }

    [[nodiscard]] constexpr std::byte* data() noexcept { return locator_->element_address({}, memory_begin()); }

    [[nodiscard]] constexpr const std::byte* data() const noexcept
    {
        return locator_->element_address({}, memory_begin());
    }

    [[nodiscard]] constexpr std::byte* data_begin() noexcept { return data(); }

    [[nodiscard]] constexpr const std::byte* data_begin() const noexcept { return data(); }

    [[nodiscard]] constexpr std::byte* data_end() noexcept { return locator_->data_end(); }

    [[nodiscard]] constexpr const std::byte* data_end() const noexcept { return locator_->data_end(); }

    [[nodiscard]] constexpr size_type size() const noexcept { return locator_->size(memory_begin()); }

    [[nodiscard]] constexpr size_type capacity() const noexcept { return max_element_count_; }

    [[nodiscard]] constexpr size_type memory_consumption() const noexcept
    {
        return memory_.size() * alignof(StorageElementType) + locator_->memory_size();
    }

    [[nodiscard]] constexpr iterator begin() noexcept { return iterator{*this}; }

    [[nodiscard]] constexpr const_iterator begin() const noexcept { return const_iterator{*this}; }

    [[nodiscard]] constexpr const_iterator cbegin() const noexcept { return begin(); }

    [[nodiscard]] constexpr iterator end() noexcept { return iterator{*this, size()}; }

    [[nodiscard]] constexpr const_iterator end() const noexcept { return const_iterator{*this, size()}; }

    [[nodiscard]] constexpr const_iterator cend() const noexcept { return end(); }

    [[nodiscard]] constexpr allocator_type get_allocator() const noexcept { return memory_.get_allocator(); }

    friend constexpr void swap(BasicContiguousVector& lhs, BasicContiguousVector& rhs) noexcept
    {
        using std::swap;
        swap(lhs.max_element_count_, rhs.max_element_count_);
        swap(lhs.memory_, rhs.memory_);
        swap(lhs.locator_, rhs.locator_);
    }

    template <class... TOption>
    [[nodiscard]] constexpr auto operator==(
        const cntgs::BasicContiguousVector<cntgs::Options<TOption...>, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTHROW_EQUALITY_COMPARABLE)
    {
        return equal(other);
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
        return lexicographical_compare(other);
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
    template <bool, class, class...>
    friend class cntgs::ContiguousVectorIterator;

    constexpr BasicContiguousVector(size_type max_element_count, size_type varying_size_bytes,
                                    const FixedSizes& fixed_sizes, const allocator_type& allocator, int)
        : BasicContiguousVector(max_element_count, varying_size_bytes, FixedSizesArray{fixed_sizes}, allocator,
                                ElementTraits::calculate_element_size(FixedSizesArray{fixed_sizes}))
    {
    }

    constexpr BasicContiguousVector(size_type max_element_count, size_type varying_size_bytes,
                                    const FixedSizesArray& fixed_sizes, const allocator_type& allocator,
                                    detail::ElementSize size)
        : max_element_count_(max_element_count),
          memory_(ElementTraits::template allocate_memory<StorageType>(
              ElementTraits::calculate_needed_memory_size(max_element_count, varying_size_bytes, size), allocator)),
          locator_(max_element_count, memory_begin(), fixed_sizes, size, allocator)
    {
    }

    std::byte* memory_begin() const noexcept { return reinterpret_cast<std::byte*>(memory_.get()); }

    template <class... Args>
    auto emplace_back_impl(Args&&... args)
    {
        return locator_->emplace_back(memory_begin(), locator_.fixed_sizes(), std::forward<Args>(args)...);
    }

    void grow(size_type new_max_element_count, size_type new_varying_size_bytes)
    {
        const auto new_memory_size =
            locator_->calculate_new_memory_size(new_max_element_count, new_varying_size_bytes, locator_.fixed_sizes());
        StorageType new_memory{new_memory_size, get_allocator()};
        BasicContiguousVector::insert_into<true>(*locator_, new_max_element_count,
                                                 reinterpret_cast<std::byte*>(new_memory.get()), *this);
        max_element_count_ = new_max_element_count;
        memory_.reset(std::move(new_memory));
    }

    template <bool IsDestruct = false, class Self = BasicContiguousVector>
    static void insert_into(ElementLocator& locator, size_type new_max_element_count, std::byte* new_memory, Self& from)
    {
        static constexpr auto USE_MOVE = !std::is_const_v<Self>;
        static constexpr auto IS_TRIVIAL =
            USE_MOVE ? ListTraits::IS_TRIVIALLY_MOVE_CONSTRUCTIBLE : ListTraits::IS_TRIVIALLY_COPY_CONSTRUCTIBLE;
        if constexpr (IS_TRIVIAL && (!IsDestruct || ListTraits::IS_TRIVIALLY_DESTRUCTIBLE))
        {
            locator.trivially_copy_into(from.memory_begin(), new_memory);
        }
        else
        {
            ElementLocator new_locator{detail::move_if<USE_MOVE>(locator), from.memory_begin(), new_max_element_count,
                                       new_memory};
            BasicContiguousVector::uninitialized_construct_if_non_trivial<USE_MOVE>(from, new_memory, new_locator);
            if constexpr (IsDestruct)
            {
                from.destruct();
            }
            locator = std::move(new_locator);
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
                auto&& target = ElementTraits::template load_element_at<detail::ContiguousReferenceSizeGetter>(
                    new_locator.element_address(i, new_memory), source);
                ElementTraits::template construct_if_non_trivial<UseMove>(source, target);
            }
        }
    }

    void move_elements_forward(std::size_t from, std::size_t to)
    {
        if constexpr (ListTraits::IS_TRIVIALLY_MOVE_CONSTRUCTIBLE && ListTraits::IS_TRIVIALLY_DESTRUCTIBLE)
        {
            locator_->move_elements_forward(from, to, memory_begin());
        }
        else
        {
            for (auto i = to; from != size(); ++i, (void)++from)
            {
                emplace_at(i, (*this)[from], ListTraits::make_index_sequence());
            }
        }
    }

    void make_room_for_last_element_at(std::size_t from, [[maybe_unused]] std::size_t bytes)
    {
        if constexpr (ListTraits::IS_TRIVIALLY_MOVE_CONSTRUCTIBLE && ListTraits::IS_TRIVIALLY_DESTRUCTIBLE)
        {
            locator_->make_room_for_last_element_at(from, bytes, memory_begin());
        }
        else
        {
            for (auto i = size(); i != from; --i)
            {
                emplace_at(i, (*this)[i - std::size_t{1}], ListTraits::make_index_sequence());
            }
        }
    }

    template <std::size_t... I>
    void emplace_at(std::size_t i, const reference& element, std::index_sequence<I...>)
    {
        locator_->emplace_at(i, memory_begin(), locator_.fixed_sizes(), std::move(cntgs::get<I>(element))...);
        ElementTraits::destruct(element);
    }

    constexpr void steal(BasicContiguousVector&& other) noexcept
    {
        destruct();
        max_element_count_ = other.max_element_count_;
        memory_ = std::move(other.memory_);
        locator_ = std::move(other.locator_);
    }

    constexpr void move_assign(BasicContiguousVector&& other)
    {
        if constexpr (AllocatorTraits::is_always_equal::value ||
                      AllocatorTraits::propagate_on_container_move_assignment::value)
        {
            steal(std::move(other));
        }
        else
        {
            if (get_allocator() == other.get_allocator())
            {
                steal(std::move(other));
            }
            else
            {
                auto other_locator = std::move(other.locator_);
                if (other.memory_consumption() > memory_consumption())
                {
                    // allocate memory first because it might throw
                    StorageType new_memory{other.memory_consumption(), get_allocator()};
                    destruct();
                    BasicContiguousVector::insert_into(*other_locator, other.max_element_count_,
                                                       reinterpret_cast<std::byte*>(new_memory.get()), other);
                    memory_ = std::move(new_memory);
                }
                else
                {
                    destruct();
                    BasicContiguousVector::insert_into(*other_locator, other.max_element_count_, memory_begin(), other);
                }
                max_element_count_ = other.max_element_count_;
                locator_ = std::move(other_locator);
            }
        }
    }

    auto copy_construct_locator(const BasicContiguousVector& other)
    {
        auto other_locator = other.locator_;
        BasicContiguousVector::insert_into(*other_locator, other.max_element_count_, memory_begin(), other);
        return other_locator;
    }

    void copy_assign(const BasicContiguousVector& other)
    {
        destruct();
        memory_ = other.memory_;
        auto other_locator = other.locator_;
        BasicContiguousVector::insert_into(*other_locator, other.max_element_count_, memory_begin(), other);
        max_element_count_ = other.max_element_count_;
        locator_ = other_locator;
    }

    template <class... TOption>
    constexpr auto equal(const cntgs::BasicContiguousVector<cntgs::Options<TOption...>, Parameter...>& other) const
    {
        if constexpr (ListTraits::IS_EQUALITY_MEMCMPABLE)
        {
            if (empty())
            {
                return other.empty();
            }
            if (other.empty())
            {
                return false;
            }
            return detail::trivial_equal(data_begin(), data_end(), other.data_begin(), other.data_end());
        }
        else
        {
            return std::equal(begin(), end(), other.begin());
        }
    }
    template <class... TOption>
    constexpr auto lexicographical_compare(
        const cntgs::BasicContiguousVector<cntgs::Options<TOption...>, Parameter...>& other) const
    {
        if constexpr (ListTraits::IS_LEXICOGRAPHICAL_MEMCMPABLE && ListTraits::IS_FIXED_SIZE_OR_PLAIN)
        {
            if (empty())
            {
                return !other.empty();
            }
            if (other.empty())
            {
                return false;
            }
            return detail::trivial_lexicographical_compare(data_begin(), data_end(), other.data_begin(),
                                                           other.data_end());
        }
        else
        {
            return std::lexicographical_compare(begin(), end(), other.begin(), other.end());
        }
    }

    constexpr iterator make_iterator(const const_iterator& it) noexcept { return {*this, it.index()}; }

    constexpr void destruct_if_owned() noexcept
    {
        if (memory_)
        {
            destruct();
        }
    }

    constexpr void destruct() noexcept { BasicContiguousVector::destruct(begin(), end()); }

    static constexpr void destruct([[maybe_unused]] iterator first, [[maybe_unused]] iterator last) noexcept
    {
        if constexpr (!ListTraits::IS_TRIVIALLY_DESTRUCTIBLE)
        {
            std::for_each(first, last, ElementTraits::destruct);
        }
    }
};
}  // namespace cntgs

#endif  // CNTGS_CNTGS_VECTOR_HPP

#endif  // CNTGS_CNTGS_CONTIGUOUS_HPP
