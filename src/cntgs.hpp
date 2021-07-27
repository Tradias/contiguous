#ifndef CNTGS_CNTGS_CONTIGUOUS_HPP
#define CNTGS_CNTGS_CONTIGUOUS_HPP

// #include "cntgs/contiguous/element.hpp"
#ifndef CNTGS_CONTIGUOUS_ELEMENT_HPP
#define CNTGS_CONTIGUOUS_ELEMENT_HPP

// #include "cntgs/contiguous/detail/elementTraits.hpp"
#ifndef CNTGS_DETAIL_ELEMENTTRAITS_HPP
#define CNTGS_DETAIL_ELEMENTTRAITS_HPP

// #include "cntgs/contiguous/detail/algorithm.hpp"
#ifndef CNTGS_DETAIL_ALGORITHM_HPP
#define CNTGS_DETAIL_ALGORITHM_HPP

// #include "cntgs/contiguous/detail/memory.hpp"
#ifndef CNTGS_DETAIL_MEMORY_HPP
#define CNTGS_DETAIL_MEMORY_HPP

// #include "cntgs/contiguous/detail/attributes.hpp"
#ifndef CNTGS_DETAIL_ATTRIBUTES_HPP
#define CNTGS_DETAIL_ATTRIBUTES_HPP

#ifdef NDEBUG
#ifdef _MSC_VER
#define CNTGS_FORCE_INLINE __forceinline
#else
#define CNTGS_FORCE_INLINE __attribute__((always_inline)) inline
#endif
#else
#define CNTGS_FORCE_INLINE inline
#endif

#ifdef _MSC_VER
#define CNTGS_RESTRICT_RETURN __declspec(restrict)
#else
#define CNTGS_RESTRICT_RETURN __attribute__((malloc)) __attribute__((returns_nonnull))
#endif

#define CNTGS_RESTRICT __restrict

#endif  // CNTGS_DETAIL_ATTRIBUTES_HPP

// #include "cntgs/contiguous/detail/iteratorUtils.hpp"
#ifndef CNTGS_DETAIL_ITERATORUTILS_HPP
#define CNTGS_DETAIL_ITERATORUTILS_HPP

// #include "cntgs/contiguous/detail/typeUtils.hpp"
#ifndef CNTGS_DETAIL_TYPEUTILS_HPP
#define CNTGS_DETAIL_TYPEUTILS_HPP

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

#endif  // CNTGS_DETAIL_TYPEUTILS_HPP


#include <iterator>
#include <version>

namespace cntgs::detail
{
template <class T>
struct ArrowProxy
{
    T t;

    constexpr const T* operator->() const noexcept { return &t; }
};

template <class I>
inline constexpr auto CONTIGUOUS_ITERATOR_V =
    detail::DerivedFrom<typename std::iterator_traits<I>::iterator_category, std::random_access_iterator_tag>&&
        std::is_lvalue_reference_v<typename std::iterator_traits<I>::reference>&&
            std::is_same_v<typename std::iterator_traits<I>::value_type,
                           detail::RemoveCvrefT<typename std::iterator_traits<I>::reference>>&&
                std::is_same_v<decltype(std::declval<const I&>().operator->()),
                               std::add_pointer_t<typename std::iterator_traits<I>::reference>>;
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_ITERATORUTILS_HPP

// #include "cntgs/contiguous/detail/range.hpp"
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

// #include "cntgs/contiguous/detail/typeUtils.hpp"

// #include "cntgs/contiguous/detail/utility.hpp"
#ifndef CNTGS_DETAIL_UTILITY_HPP
#define CNTGS_DETAIL_UTILITY_HPP

// #include "cntgs/contiguous/span.hpp"
#ifndef CNTGS_CONTIGUOUS_SPAN_HPP
#define CNTGS_CONTIGUOUS_SPAN_HPP

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
    explicit constexpr Span(const Span<U>& other) noexcept : first(other.first), last(other.last)
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

#endif  // CNTGS_CONTIGUOUS_SPAN_HPP


#include <utility>

namespace cntgs::detail
{
template <class T>
struct MoveDefaultingValue
{
    T value;

    explicit constexpr MoveDefaultingValue(T value) noexcept : value(value) {}

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

template <class T, bool Inherit>
class EmptyBaseOptimization
{
  private:
    T value;

  public:
    EmptyBaseOptimization() = default;

    explicit constexpr EmptyBaseOptimization(const T& value) noexcept(std::is_nothrow_copy_constructible_v<T>)
        : value{value}
    {
    }

    explicit constexpr EmptyBaseOptimization(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>)
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

    explicit constexpr EmptyBaseOptimization(const T& value) noexcept(std::is_nothrow_copy_constructible_v<T>)
        : T{value}
    {
    }

    explicit constexpr EmptyBaseOptimization(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>)
        : T{std::move(value)}
    {
    }

    constexpr T& get() noexcept { return *this; }

    constexpr const T& get() const noexcept { return *this; }
};

template <class T>
using EmptyBaseOptimizationT = detail::EmptyBaseOptimization<T, (std::is_empty_v<T> && !std::is_final_v<T>)>;

template <class T>
constexpr const cntgs::Span<T>& dereference(const cntgs::Span<T>& value) noexcept
{
    return value;
}

template <class T>
constexpr cntgs::Span<T>& dereference(cntgs::Span<T>& value) noexcept
{
    return value;
}

template <class T>
constexpr decltype(auto) dereference(T& value) noexcept
{
    return *value;
}

template <class T>
constexpr auto as_const(const cntgs::Span<T>& value) noexcept
{
    return cntgs::Span<std::add_const_t<T>>{value};
}

template <class T>
constexpr auto as_const(cntgs::Span<T>& value) noexcept
{
    return cntgs::Span<std::add_const_t<T>>{value};
}

template <class T>
constexpr decltype(auto) as_const(T& value) noexcept
{
    return std::as_const(value);
}
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_UTILITY_HPP

// #include "cntgs/contiguous/span.hpp"


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
using TypeErasedAllocator = std::aligned_storage_t<32>;

template <std::size_t N>
struct alignas(N) AlignedByte
{
    std::byte byte;
};

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
    struct Impl : detail::EmptyBaseOptimizationT<Allocator>
    {
        using Base = detail::EmptyBaseOptimizationT<Allocator>;

        Pointer ptr{};
        std::size_t size{};

        Impl() = default;

        constexpr Impl(Pointer ptr, std::size_t size, const Allocator& allocator) noexcept
            : Base{allocator}, ptr(ptr), size(size)
        {
        }
    };

    Impl impl;

  public:
    AllocatorAwarePointer() = default;

    constexpr AllocatorAwarePointer(std::size_t size, Allocator allocator)
        : impl(AllocatorTraits::allocate(allocator, size), size, allocator)
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

    constexpr AllocatorAwarePointer(const AllocatorAwarePointer& other, Allocator allocator)
        : impl(AllocatorTraits::allocate(allocator, other.size()), other.size(), allocator)
    {
    }

    constexpr AllocatorAwarePointer(AllocatorAwarePointer&& other) noexcept
        : impl(std::exchange(other.get(), nullptr), other.size(), other.get_allocator())
    {
    }

    constexpr AllocatorAwarePointer(AllocatorAwarePointer&& other, const Allocator& allocator) noexcept
        : impl(std::exchange(other.get(), nullptr), other.size(), allocator)
    {
    }

    ~AllocatorAwarePointer() noexcept { this->deallocate(); }

    constexpr void propagate_on_container_copy_assignment(const AllocatorAwarePointer& other) noexcept
    {
        if constexpr (AllocatorTraits::propagate_on_container_copy_assignment::value)
        {
            this->get_allocator() = other.get_allocator();
        }
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
            if (this->size() < other.size())
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
            if constexpr (AllocatorTraits::propagate_on_container_move_assignment::value)
            {
                this->get_allocator() = std::move(other.get_allocator());
            }
            this->get() = std::exchange(other.get(), nullptr);
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

    explicit constexpr operator bool() const noexcept { return bool(this->get()); }

    constexpr auto release() noexcept { return std::exchange(this->impl.ptr, nullptr); }

    constexpr auto allocate() { return AllocatorTraits::allocate(this->get_allocator(), this->size()); }

    constexpr void deallocate() noexcept
    {
        if (this->get())
        {
            AllocatorTraits::deallocate(this->get_allocator(), this->get(), this->size());
        }
    }
};

template <class Allocator>
void swap(detail::AllocatorAwarePointer<Allocator>& lhs, detail::AllocatorAwarePointer<Allocator>& rhs) noexcept
{
    using std::swap;
    if constexpr (std::allocator_traits<Allocator>::propagate_on_container_swap::value)
    {
        swap(lhs.get_allocator(), rhs.get_allocator());
    }
    swap(lhs.get(), rhs.get());
    swap(lhs.size(), rhs.size());
}

template <class Allocator>
class MaybeOwnedAllocatorAwarePointer
{
  private:
    using StorageType = detail::AllocatorAwarePointer<Allocator>;

  public:
    using allocator_type = typename StorageType::allocator_type;
    using pointer = typename StorageType::pointer;
    using value_type = typename StorageType::value_type;

    StorageType ptr{};
    bool owned{};

    MaybeOwnedAllocatorAwarePointer() = default;

    constexpr MaybeOwnedAllocatorAwarePointer(pointer ptr, std::size_t size, bool is_owned,
                                              const allocator_type& allocator) noexcept
        : ptr(ptr, size, allocator), owned(is_owned)
    {
    }

    constexpr MaybeOwnedAllocatorAwarePointer(std::size_t size, const allocator_type& allocator) noexcept
        : ptr(size, allocator), owned(true)
    {
    }

    MaybeOwnedAllocatorAwarePointer(const MaybeOwnedAllocatorAwarePointer& other) = default;

    MaybeOwnedAllocatorAwarePointer(MaybeOwnedAllocatorAwarePointer&& other) = default;

    MaybeOwnedAllocatorAwarePointer& operator=(const MaybeOwnedAllocatorAwarePointer& other) = default;

    constexpr MaybeOwnedAllocatorAwarePointer& operator=(MaybeOwnedAllocatorAwarePointer&& other) noexcept
    {
        if (this != std::addressof(other))
        {
            this->release_ptr_if_not_owned();
            this->ptr = std::move(other.ptr);
            this->owned = other.owned;
        }
        return *this;
    }

    ~MaybeOwnedAllocatorAwarePointer() noexcept { this->release_ptr_if_not_owned(); }

    constexpr auto get() const noexcept { return this->ptr.get(); }

    constexpr auto size() const noexcept { return this->ptr.size(); }

    constexpr bool is_owned() const noexcept { return this->owned; }

    explicit constexpr operator bool() const noexcept { return bool(this->ptr); }

    constexpr auto release() noexcept { return this->ptr.release(); }

    constexpr auto get_allocator() const noexcept { return this->ptr.get_allocator(); }

    constexpr auto& get_impl() noexcept { return this->ptr; }

  private:
    constexpr void release_ptr_if_not_owned() noexcept
    {
        if (!this->owned)
        {
            (void)this->ptr.release();
        }
    }
};

template <class T>
auto copy_using_memcpy(const T* CNTGS_RESTRICT source, std::byte* CNTGS_RESTRICT target, std::size_t size) noexcept
{
    std::memcpy(target, source, size * sizeof(T));
    return target + size * sizeof(T);
}

template <bool IgnoreAliasing, class TargetType, class Range>
auto uninitialized_range_construct(Range&& CNTGS_RESTRICT range, TargetType* CNTGS_RESTRICT address)
{
    using RangeValueType = typename std::iterator_traits<decltype(std::begin(range))>::value_type;
    if constexpr (IgnoreAliasing && detail::HasDataAndSize<std::decay_t<Range>>{} &&
                  detail::MEMCPY_COMPATIBLE<TargetType, RangeValueType>)
    {
        return detail::copy_using_memcpy(std::data(range), reinterpret_cast<std::byte*>(address), std::size(range));
    }
    else
    {
        if constexpr (!std::is_lvalue_reference_v<Range>)
        {
            return reinterpret_cast<std::byte*>(std::uninitialized_move(std::begin(range), std::end(range), address));
        }
        else
        {
            return reinterpret_cast<std::byte*>(std::uninitialized_copy(std::begin(range), std::end(range), address));
        }
    }
}

template <bool IgnoreAliasing, class TargetType, class Range>
auto uninitialized_construct(Range&& CNTGS_RESTRICT range, TargetType* CNTGS_RESTRICT address, std::size_t)
    -> std::enable_if_t<detail::IsRange<Range>::value, std::byte*>
{
    return uninitialized_range_construct<IgnoreAliasing>(std::forward<Range>(range), address);
}

template <bool IgnoreAliasing, class TargetType, class Iterator>
auto uninitialized_construct(const Iterator& CNTGS_RESTRICT iterator, TargetType* CNTGS_RESTRICT address,
                             std::size_t size) -> std::enable_if_t<!detail::IsRange<Iterator>::value, std::byte*>
{
    using IteratorValueType = typename std::iterator_traits<Iterator>::value_type;
    if constexpr (IgnoreAliasing && std::is_pointer_v<Iterator> &&
                  detail::MEMCPY_COMPATIBLE<TargetType, IteratorValueType>)
    {
        return detail::copy_using_memcpy(iterator, reinterpret_cast<std::byte*>(address), size);
    }
    else if constexpr (IgnoreAliasing && detail::CONTIGUOUS_ITERATOR_V<Iterator> &&
                       detail::MEMCPY_COMPATIBLE<TargetType, IteratorValueType>)
    {
        return detail::copy_using_memcpy(iterator.operator->(), reinterpret_cast<std::byte*>(address), size);
    }
    else
    {
        return reinterpret_cast<std::byte*>(std::uninitialized_copy_n(iterator, size, address));
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
    return static_cast<T*>(__builtin_assume_aligned(ptr, Alignment));
}
#endif

[[nodiscard]] constexpr auto align(std::size_t alignment, std::uintptr_t position) noexcept
{
    return (position - 1u + alignment) & (alignment * std::numeric_limits<std::size_t>::max());
}

template <std::size_t Alignment>
[[nodiscard]] constexpr auto align(std::uintptr_t position) noexcept
{
    if constexpr (Alignment == 1)
    {
        return position;
    }
    else
    {
        return detail::align(Alignment, position);
    }
}

template <std::size_t Alignment>
[[nodiscard]] void* align(void* ptr) noexcept
{
    const auto uintptr = reinterpret_cast<std::uintptr_t>(ptr);
    const auto aligned = detail::align<Alignment>(uintptr);
    return detail::assume_aligned<Alignment>(reinterpret_cast<void*>(aligned));
}

template <bool NeedsAlignment, std::size_t Alignment>
[[nodiscard]] void* align_if(void* ptr) noexcept
{
    if constexpr (NeedsAlignment)
    {
        ptr = detail::align<Alignment>(ptr);
    }
    return detail::assume_aligned<Alignment>(ptr);
}

template <class T>
auto type_erase_allocator(T&& allocator) noexcept
{
    detail::TypeErasedAllocator result;
    detail::construct_at(reinterpret_cast<detail::RemoveCvrefT<T>*>(&result), std::forward<T>(allocator));
    return result;
}
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_MEMORY_HPP


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

// #include "cntgs/contiguous/detail/attributes.hpp"

// #include "cntgs/contiguous/detail/forward.hpp"
#ifndef CNTGS_DETAIL_FORWARD_HPP
#define CNTGS_DETAIL_FORWARD_HPP

// #include "cntgs/contiguous/referenceQualifier.hpp"
#ifndef CNTGS_CONTIGUOUS_REFERENCEQUALIFIER_HPP
#define CNTGS_CONTIGUOUS_REFERENCEQUALIFIER_HPP

namespace cntgs
{
enum class ContiguousReferenceQualifier
{
    MUTABLE,
    CONST
};
}  // namespace cntgs

#endif  // CNTGS_CONTIGUOUS_REFERENCEQUALIFIER_HPP


#include <cstddef>

namespace cntgs
{
template <class T>
struct VaryingSize;

template <class T>
struct FixedSize;

template <class T, std::size_t Alignment = 1>
struct AlignAs;

template <class Allocator, class... T>
class BasicContiguousVector;

template <bool IsConst, class Allocator, class... Types>
class ContiguousVectorIterator;

class TypeErasedVector;

template <cntgs::ContiguousReferenceQualifier Qualifier, class... Types>
class BasicContiguousReference;

template <class Allocator, class... Types>
class BasicContiguousElement;
}  // namespace cntgs

#endif  // CNTGS_DETAIL_FORWARD_HPP

// #include "cntgs/contiguous/detail/memory.hpp"

// #include "cntgs/contiguous/detail/parameterListTraits.hpp"
#ifndef CNTGS_DETAIL_PARAMETERLISTTRAITS_HPP
#define CNTGS_DETAIL_PARAMETERLISTTRAITS_HPP

// #include "cntgs/contiguous/detail/parameterTraits.hpp"
#ifndef CNTGS_DETAIL_PARAMETERTRAITS_HPP
#define CNTGS_DETAIL_PARAMETERTRAITS_HPP

// #include "cntgs/contiguous/detail/attributes.hpp"

// #include "cntgs/contiguous/detail/memory.hpp"

// #include "cntgs/contiguous/detail/parameterType.hpp"
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

// #include "cntgs/contiguous/detail/typeUtils.hpp"

// #include "cntgs/contiguous/parameter.hpp"
#ifndef CNTGS_CONTIGUOUS_PARAMETER_HPP
#define CNTGS_CONTIGUOUS_PARAMETER_HPP

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
}  // namespace cntgs

#endif  // CNTGS_CONTIGUOUS_PARAMETER_HPP

// #include "cntgs/contiguous/span.hpp"


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

    template <bool NeedsAlignment>
    static auto load(std::byte* address, std::size_t) noexcept
    {
        address = static_cast<std::byte*>(detail::align_if<NeedsAlignment, ALIGNMENT>(address));
        auto result = std::launder(reinterpret_cast<PointerType>(address));
        return std::pair{result, address + VALUE_BYTES};
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

    static auto data_begin(const cntgs::Span<std::add_const_t<T>>& value) noexcept
    {
        return reinterpret_cast<const std::byte*>(Self::begin(value));
    }

    static auto data_begin(const cntgs::Span<T>& value) noexcept
    {
        return reinterpret_cast<std::byte*>(Self::begin(value));
    }

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

    template <bool NeedsAlignment>
    static auto load(std::byte* address, std::size_t) noexcept
    {
        const auto size = *reinterpret_cast<std::size_t*>(address);
        address += MEMORY_OVERHEAD;
        const auto first_byte = static_cast<std::byte*>(detail::align_if<NeedsAlignment, ALIGNMENT>(address));
        const auto first = std::launder(reinterpret_cast<IteratorType>(first_byte));
        const auto last = std::launder(reinterpret_cast<IteratorType>(first_byte + size));
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
        *size = new_address - reinterpret_cast<std::byte*>(aligned_address);
        return new_address;
    }

    static constexpr auto aligned_size_in_memory(std::size_t) noexcept { return ALIGNED_SIZE_IN_MEMORY; }
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

    template <bool NeedsAlignment>
    static auto load(std::byte* address, std::size_t size) noexcept
    {
        const auto first =
            std::launder(static_cast<IteratorType>(detail::align_if<NeedsAlignment, ALIGNMENT>(address)));
        const auto last = first + size;
        return std::pair{PointerType{first, last}, reinterpret_cast<std::byte*>(last)};
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

// #include "cntgs/contiguous/detail/parameterType.hpp"

// #include "cntgs/contiguous/detail/typeUtils.hpp"


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

    static constexpr auto CONTIGUOUS_COUNT =
        (std::size_t{} + ... + (detail::ParameterTraits<Types>::TYPE != detail::ParameterType::PLAIN));
    static constexpr auto CONTIGUOUS_FIXED_SIZE_COUNT =
        (std::size_t{} + ... + (detail::ParameterTraits<Types>::TYPE == detail::ParameterType::FIXED_SIZE));

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

    static constexpr bool IS_MIXED =
        CONTIGUOUS_FIXED_SIZE_COUNT != 0 && CONTIGUOUS_FIXED_SIZE_COUNT != CONTIGUOUS_COUNT;
    static constexpr bool IS_ALL_FIXED_SIZE =
        CONTIGUOUS_FIXED_SIZE_COUNT != 0 && CONTIGUOUS_FIXED_SIZE_COUNT == CONTIGUOUS_COUNT;
    static constexpr bool IS_ALL_VARYING_SIZE = CONTIGUOUS_FIXED_SIZE_COUNT == 0 && CONTIGUOUS_COUNT != 0;
    static constexpr bool IS_ALL_PLAIN = CONTIGUOUS_COUNT == 0;
    static constexpr bool IS_FIXED_SIZE_OR_PLAIN = IS_ALL_FIXED_SIZE || IS_ALL_PLAIN;

    using FixedSizes = std::array<std::size_t, CONTIGUOUS_FIXED_SIZE_COUNT>;

    static_assert(detail::MAX_FIXED_SIZE_VECTOR_PARAMETER > CONTIGUOUS_FIXED_SIZE_COUNT,
                  "Maximum number of FixedSize vector parameter exceeded. Define CNTGS_MAX_FIXED_SIZE_VECTOR_PARAMETER "
                  "to a higher limit.");

    static constexpr auto make_index_sequence() noexcept { return std::make_index_sequence<sizeof...(Types)>{}; }
};
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_PARAMETERLISTTRAITS_HPP

// #include "cntgs/contiguous/detail/parameterTraits.hpp"

// #include "cntgs/contiguous/detail/sizeGetter.hpp"
#ifndef CNTGS_DETAIL_SIZEGETTER_HPP
#define CNTGS_DETAIL_SIZEGETTER_HPP

// #include "cntgs/contiguous/detail/parameterTraits.hpp"

// #include "cntgs/contiguous/detail/parameterType.hpp"

// #include "cntgs/contiguous/detail/typeUtils.hpp"


#include <array>
#include <cstddef>
#include <tuple>

namespace cntgs::detail
{
template <class... Types>
struct FixedSizeGetter
{
    template <std::size_t... I>
    static constexpr auto calculate_fixed_size_indices(std::index_sequence<I...>) noexcept
    {
        std::array<std::size_t, sizeof...(Types)> fixed_size_indices{};
        [[maybe_unused]] std::size_t index{};
        (
            [&]
            {
                if constexpr (detail::ParameterTraits<Types>::TYPE == detail::ParameterType::FIXED_SIZE)
                {
                    std::get<I>(fixed_size_indices) = index;
                    ++index;
                }
            }(),
            ...);
        return fixed_size_indices;
    }

    static constexpr auto FIXED_SIZE_INDICES =
        calculate_fixed_size_indices(std::make_index_sequence<sizeof...(Types)>{});

    template <class Type, std::size_t I, std::size_t N>
    static constexpr auto get([[maybe_unused]] const std::array<std::size_t, N>& fixed_sizes) noexcept
    {
        if constexpr (detail::ParameterType::FIXED_SIZE == detail::ParameterTraits<Type>::TYPE)
        {
            return std::get<std::get<I>(FIXED_SIZE_INDICES)>(fixed_sizes);
        }
        else
        {
            return std::size_t{};
        }
    }
};

struct ContiguousReferenceSizeGetter
{
    template <class Type, std::size_t I, class... U>
    static constexpr auto get([[maybe_unused]] const std::tuple<U...>& tuple) noexcept
    {
        if constexpr (detail::ParameterType::FIXED_SIZE == detail::ParameterTraits<Type>::TYPE)
        {
            return std::get<I>(tuple).size();
        }
        else
        {
            return std::size_t{};
        }
    }
};
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_SIZEGETTER_HPP

// #include "cntgs/contiguous/detail/typeUtils.hpp"

// #include "cntgs/contiguous/detail/vectorTraits.hpp"
#ifndef CNTGS_DETAIL_VECTORTRAITS_HPP
#define CNTGS_DETAIL_VECTORTRAITS_HPP

// #include "cntgs/contiguous/detail/forward.hpp"

// #include "cntgs/contiguous/detail/memory.hpp"

// #include "cntgs/contiguous/detail/tuple.hpp"
#ifndef CNTGS_DETAIL_TUPLE_HPP
#define CNTGS_DETAIL_TUPLE_HPP

// #include "cntgs/contiguous/detail/forward.hpp"

// #include "cntgs/contiguous/detail/parameterTraits.hpp"

// #include "cntgs/contiguous/detail/utility.hpp"


#include <tuple>
#include <utility>

namespace cntgs::detail
{
template <template <class> class U, class T>
struct TransformTuple
{
};

template <template <class> class Transformer, class... T>
struct TransformTuple<Transformer, std::tuple<T...>>
{
    using Type = std::tuple<Transformer<T>...>;
};

template <class T>
using ToContiguousReference = typename detail::ParameterTraits<T>::ReferenceType;

template <class T>
using ToTupleOfContiguousReference = typename detail::TransformTuple<ToContiguousReference, T>::Type;

template <class T>
using ToContiguousConstReference = typename detail::ParameterTraits<T>::ConstReferenceType;

template <class T>
using ToTupleOfContiguousConstReference = typename detail::TransformTuple<ToContiguousConstReference, T>::Type;

template <class T>
using ToContiguousPointer = typename detail::ParameterTraits<T>::PointerType;

template <class T>
using ToTupleOfContiguousPointer = typename detail::TransformTuple<ToContiguousPointer, T>::Type;

template <class Result, class... T, std::size_t... I>
constexpr auto convert_tuple_to(const std::tuple<T...>& tuple_of_pointer, std::index_sequence<I...>) noexcept
{
    return Result{detail::dereference(std::get<I>(tuple_of_pointer))...};
}

template <class Result, class... T>
constexpr auto convert_tuple_to(const std::tuple<T...>& tuple_of_pointer) noexcept
{
    return detail::convert_tuple_to<Result>(tuple_of_pointer, std::make_index_sequence<sizeof...(T)>{});
}
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_TUPLE_HPP


#include <tuple>

namespace cntgs::detail
{
template <class... Types>
struct ContiguousVectorTraits
{
    using ReferenceType = cntgs::BasicContiguousReference<cntgs::ContiguousReferenceQualifier::MUTABLE, Types...>;
    using ConstReferenceType = cntgs::BasicContiguousReference<cntgs::ContiguousReferenceQualifier::CONST, Types...>;
    using PointerType = detail::ToTupleOfContiguousPointer<std::tuple<Types...>>;
};
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_VECTORTRAITS_HPP


#include <array>
#include <cstddef>
#include <limits>
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
constexpr auto alignment_offset([[maybe_unused]] std::size_t position) noexcept
{
    if constexpr (Alignment == 1)
    {
        return std::size_t{};
    }
    else
    {
        return detail::align(Alignment, position) - position;
    }
}

template <bool UseMove, class Type, class Source, class Target>
constexpr void construct_one_if_non_trivial([[maybe_unused]] Source& source, [[maybe_unused]] const Target& target)
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

template <std::size_t... I, class... Types>
class ElementTraits<std::index_sequence<I...>, Types...>
{
  private:
    using Self = ElementTraits<std::index_sequence<I...>, Types...>;
    using ListTraits = detail::ParameterListTraits<Types...>;
    using FixedSizes = typename ListTraits::FixedSizes;
    using ContiguousPointer = typename detail::ContiguousVectorTraits<Types...>::PointerType;
    using ContiguousReference = typename detail::ContiguousVectorTraits<Types...>::ReferenceType;
    using AlignmentNeeds =
        detail::ConditionalT<(ListTraits::CONTIGUOUS_FIXED_SIZE_COUNT == ListTraits::CONTIGUOUS_COUNT),
                             detail::IgnoreFirstAlignmentNeeds, detail::DefaultAlignmentNeeds>;
    using FixedSizeGetter = detail::FixedSizeGetter<Types...>;

    static constexpr auto SKIP = std::numeric_limits<std::size_t>::max();
    static constexpr auto MANUAL = SKIP - 1;

    template <template <class> class Predicate>
    static constexpr auto calculate_consecutive_indices() noexcept
    {
        std::array<std::size_t, sizeof...(Types)> consecutive_indices{((void)I, SKIP)...};
        [[maybe_unused]] std::size_t index{};
        (
            [&]
            {
                if constexpr (Predicate<typename detail::ParameterTraits<Types>::ValueType>::value)
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

    template <std::size_t K, std::size_t L, cntgs::ContiguousReferenceQualifier LhsQualifier,
              cntgs::ContiguousReferenceQualifier RhsQualifier>
    static constexpr auto get_data_begin_and_end(
        const cntgs::BasicContiguousReference<LhsQualifier, Types...>& lhs,
        const cntgs::BasicContiguousReference<RhsQualifier, Types...>& rhs) noexcept
    {
        return std::tuple{ParameterTraitsAt<K>::data_begin(std::get<K>(lhs.tuple)),
                          ParameterTraitsAt<L>::data_end(std::get<L>(lhs.tuple)),
                          ParameterTraitsAt<K>::data_begin(std::get<K>(rhs.tuple))};
    }

    template <bool IgnoreAliasing, class... Args>
    static std::byte* emplace_at(std::byte* CNTGS_RESTRICT address, const FixedSizes& fixed_sizes, Args&&... args)
    {
        ((address = detail::ParameterTraits<Types>::template store<AlignmentNeeds::template VALUE<I>, IgnoreAliasing>(
              std::forward<Args>(args), address, FixedSizeGetter::template get<Types, I>(fixed_sizes))),
         ...);
        return address;
    }

  public:
    template <std::size_t K>
    using ParameterTraitsAt = typename ListTraits::template ParameterTraitsAt<K>;

    template <class... Args>
    CNTGS_RESTRICT_RETURN static std::byte* emplace_at(std::byte* CNTGS_RESTRICT address, const FixedSizes& fixed_sizes,
                                                       Args&&... args)
    {
        return Self::template emplace_at<true>(address, fixed_sizes, std::forward<Args>(args)...);
    }

    template <class... Args>
    static std::byte* emplace_at_aliased(std::byte* CNTGS_RESTRICT address, const FixedSizes& fixed_sizes,
                                         Args&&... args)
    {
        return Self::template emplace_at<false>(address, fixed_sizes, std::forward<Args>(args)...);
    }

    template <class AlignmentNeedsType = AlignmentNeeds, class FixedSizeGetterType = FixedSizeGetter,
              class FixedSizesType = FixedSizes>
    static auto load_element_at(std::byte* CNTGS_RESTRICT address, const FixedSizesType& fixed_sizes) noexcept
    {
        ContiguousPointer result;
        ((std::tie(std::get<I>(result), address) =
              detail::ParameterTraits<Types>::template load<AlignmentNeedsType::template VALUE<I>>(
                  address, FixedSizeGetterType::template get<Types, I>(fixed_sizes))),
         ...);
        return result;
    }

    static constexpr auto calculate_element_size(const FixedSizes& fixed_sizes) noexcept
    {
        std::size_t result{};
        if constexpr (ListTraits::IS_FIXED_SIZE_OR_PLAIN)
        {
            ((result += detail::ParameterTraits<Types>::guaranteed_size_in_memory(
                            FixedSizeGetter::template get<Types, I>(fixed_sizes)) +
                        detail::alignment_offset<detail::ParameterTraits<Types>::ALIGNMENT>(result)),
             ...);
        }
        else
        {
            ((result += detail::ParameterTraits<Types>::aligned_size_in_memory(
                            FixedSizeGetter::template get<Types, I>(fixed_sizes)) +
                        detail::alignment_offset<detail::ParameterTraits<Types>::ALIGNMENT>(result)),
             ...);
        }
        return result + detail::alignment_offset<ParameterTraitsAt<0>::ALIGNMENT>(result);
    }

    template <bool UseMove, cntgs::ContiguousReferenceQualifier Qualifier>
    static constexpr void construct_if_non_trivial(const cntgs::BasicContiguousReference<Qualifier, Types...>& source,
                                                   const ContiguousPointer& target)
    {
        (detail::construct_one_if_non_trivial<UseMove, Types>(std::get<I>(source.tuple), std::get<I>(target)), ...);
    }

    template <bool UseMove, std::size_t K, cntgs::ContiguousReferenceQualifier LhsQualifier>
    static void assign_one(const cntgs::BasicContiguousReference<LhsQualifier, Types...>& source,
                           const ContiguousReference& target)
    {
        static constexpr auto INDEX = std::get<K>(std::get<UseMove>(CONSECUTIVE_TRIVIALLY_ASSIGNABLE_INDICES));
        if constexpr (INDEX == MANUAL)
        {
            if constexpr (UseMove)
            {
                ParameterTraitsAt<K>::move(std::get<K>(source.tuple), std::get<K>(target.tuple));
            }
            else
            {
                ParameterTraitsAt<K>::copy(std::get<K>(source.tuple), std::get<K>(target.tuple));
            }
        }
        else if constexpr (INDEX != SKIP)
        {
            const auto [source_start, source_end, target_start] =
                Self::template get_data_begin_and_end<K, INDEX>(source, target);
            std::memmove(target_start, source_start, source_end - source_start);
        }
    }

    template <bool UseMove, cntgs::ContiguousReferenceQualifier LhsQualifier>
    static void assign(const cntgs::BasicContiguousReference<LhsQualifier, Types...>& source,
                       const ContiguousReference& target)
    {
        (Self::template assign_one<UseMove, I>(source, target), ...);
    }

    template <std::size_t K>
    static void swap_one(const ContiguousReference& lhs, const ContiguousReference& rhs)
    {
        static constexpr auto INDEX = std::get<K>(CONSECUTIVE_TRIVIALLY_SWAPPABLE_INDICES);
        if constexpr (INDEX == MANUAL)
        {
            ParameterTraitsAt<K>::swap(std::get<K>(lhs.tuple), std::get<K>(rhs.tuple));
        }
        else if constexpr (INDEX != SKIP)
        {
            const auto [lhs_start, lhs_end, rhs_start] = Self::template get_data_begin_and_end<K, INDEX>(lhs, rhs);
            detail::trivial_swap_ranges(lhs_start, lhs_end, rhs_start);
        }
    }

    static void swap(const ContiguousReference& lhs, const ContiguousReference& rhs)
    {
        (Self::template swap_one<I>(lhs, rhs), ...);
    }

    template <std::size_t K, cntgs::ContiguousReferenceQualifier LhsQualifier,
              cntgs::ContiguousReferenceQualifier RhsQualifier>
    static constexpr auto equal_one(const cntgs::BasicContiguousReference<LhsQualifier, Types...>& lhs,
                                    const cntgs::BasicContiguousReference<RhsQualifier, Types...>& rhs)
    {
        constexpr auto INDEX = std::get<K>(CONSECUTIVE_EQUALITY_MEMCMPABLE_INDICES);
        if constexpr (INDEX == MANUAL)
        {
            return ParameterTraitsAt<K>::equal(std::get<K>(lhs.tuple), std::get<K>(rhs.tuple));
        }
        else if constexpr (INDEX != SKIP)
        {
            const auto [lhs_start, lhs_end, rhs_start] = Self::template get_data_begin_and_end<K, INDEX>(lhs, rhs);
            const auto rhs_end = ParameterTraitsAt<INDEX>::data_end(std::get<INDEX>(rhs.tuple));
            return detail::trivial_equal(lhs_start, lhs_end, rhs_start, rhs_end);
        }
        else
        {
            return true;
        }
    }

    template <cntgs::ContiguousReferenceQualifier LhsQualifier, cntgs::ContiguousReferenceQualifier RhsQualifier>
    static constexpr auto equal(const cntgs::BasicContiguousReference<LhsQualifier, Types...>& lhs,
                                const cntgs::BasicContiguousReference<RhsQualifier, Types...>& rhs)
    {
        return (Self::template equal_one<I>(lhs, rhs) && ...);
    }

    template <std::size_t K, cntgs::ContiguousReferenceQualifier LhsQualifier,
              cntgs::ContiguousReferenceQualifier RhsQualifier>
    static constexpr auto lexicographical_compare_one(
        const cntgs::BasicContiguousReference<LhsQualifier, Types...>& lhs,
        const cntgs::BasicContiguousReference<RhsQualifier, Types...>& rhs)
    {
        constexpr auto INDEX = std::get<K>(CONSECUTIVE_LEXICOGRAPHICAL_MEMCMPABLE_INDICES);
        if constexpr (INDEX == MANUAL)
        {
            return ParameterTraitsAt<K>::lexicographical_compare(std::get<K>(lhs.tuple), std::get<K>(rhs.tuple));
        }
        else if constexpr (INDEX != SKIP)
        {
            const auto [lhs_start, lhs_end, rhs_start] = Self::template get_data_begin_and_end<K, INDEX>(lhs, rhs);
            const auto rhs_end = ParameterTraitsAt<INDEX>::data_end(std::get<INDEX>(rhs.tuple));
            return detail::trivial_lexicographical_compare(lhs_start, lhs_end, rhs_start, rhs_end);
        }
        else
        {
            return true;
        }
    }

    template <cntgs::ContiguousReferenceQualifier LhsQualifier, cntgs::ContiguousReferenceQualifier RhsQualifier>
    static constexpr auto lexicographical_compare(const cntgs::BasicContiguousReference<LhsQualifier, Types...>& lhs,
                                                  const cntgs::BasicContiguousReference<RhsQualifier, Types...>& rhs)
    {
        return (Self::template lexicographical_compare_one<I>(lhs, rhs) && ...);
    }

    static void destruct(const ContiguousReference& reference) noexcept
    {
        (detail::ParameterTraits<Types>::destroy(std::get<I>(reference.tuple)), ...);
    }
};

template <class... Types>
using ElementTraitsT = detail::ElementTraits<std::make_index_sequence<sizeof...(Types)>, Types...>;
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_ELEMENTTRAITS_HPP

// #include "cntgs/contiguous/detail/forward.hpp"

// #include "cntgs/contiguous/detail/memory.hpp"

// #include "cntgs/contiguous/detail/parameterListTraits.hpp"

// #include "cntgs/contiguous/detail/sizeGetter.hpp"

// #include "cntgs/contiguous/detail/utility.hpp"

// #include "cntgs/contiguous/detail/vectorTraits.hpp"

// #include "cntgs/contiguous/reference.hpp"
#ifndef CNTGS_CONTIGUOUS_REFERENCE_HPP
#define CNTGS_CONTIGUOUS_REFERENCE_HPP

// #include "cntgs/contiguous/detail/attributes.hpp"

// #include "cntgs/contiguous/detail/elementTraits.hpp"

// #include "cntgs/contiguous/detail/forward.hpp"

// #include "cntgs/contiguous/detail/parameterListTraits.hpp"

// #include "cntgs/contiguous/detail/tuple.hpp"

// #include "cntgs/contiguous/detail/typeUtils.hpp"

// #include "cntgs/contiguous/referenceQualifier.hpp"


#include <cstddef>
#include <cstring>
#include <tuple>

namespace cntgs
{
template <class... Types>
using ContiguousReference = cntgs::BasicContiguousReference<cntgs::ContiguousReferenceQualifier::MUTABLE, Types...>;

template <class... Types>
using ContiguousConstReference = cntgs::BasicContiguousReference<cntgs::ContiguousReferenceQualifier::CONST, Types...>;

/// Reference type
template <cntgs::ContiguousReferenceQualifier Qualifier, class... Types>
class BasicContiguousReference
{
  private:
    using ListTraits = detail::ParameterListTraits<Types...>;
    using ElementTraits = detail::ElementTraitsT<Types...>;
    using PointerTuple = detail::ToTupleOfContiguousPointer<std::tuple<Types...>>;

  public:
    using Tuple = detail::ConditionalT<(cntgs::ContiguousReferenceQualifier::MUTABLE == Qualifier),
                                       detail::ToTupleOfContiguousReference<std::tuple<Types...>>,
                                       detail::ToTupleOfContiguousConstReference<std::tuple<Types...>>>;

    static constexpr auto IS_CONST = cntgs::ContiguousReferenceQualifier::CONST == Qualifier;

    Tuple tuple;

    BasicContiguousReference() = default;
    ~BasicContiguousReference() = default;

    BasicContiguousReference(const BasicContiguousReference&) = default;
    BasicContiguousReference(BasicContiguousReference&&) = default;

    explicit constexpr BasicContiguousReference(std::byte* CNTGS_RESTRICT address,
                                                const typename ListTraits::FixedSizes& fixed_sizes = {}) noexcept
        : BasicContiguousReference(ElementTraits::load_element_at(address, fixed_sizes))
    {
    }

    explicit constexpr BasicContiguousReference(const PointerTuple& tuple) noexcept
        : tuple(detail::convert_tuple_to<Tuple>(tuple))
    {
    }

    template <cntgs::ContiguousReferenceQualifier TQualifier>
    /*implicit*/ constexpr BasicContiguousReference(
        const cntgs::BasicContiguousReference<TQualifier, Types...>& other) noexcept
        : tuple(other.tuple)
    {
    }

    template <class Allocator>
    /*implicit*/ constexpr BasicContiguousReference(
        const cntgs::BasicContiguousElement<Allocator, Types...>& other) noexcept
        : tuple(other.reference)
    {
    }

    template <cntgs::ContiguousReferenceQualifier TQualifier>
    /*implicit*/ constexpr BasicContiguousReference(
        cntgs::BasicContiguousReference<TQualifier, Types...>&& other) noexcept
        : tuple(std::move(other.tuple))
    {
    }

    template <class Allocator>
    /*implicit*/ constexpr BasicContiguousReference(cntgs::BasicContiguousElement<Allocator, Types...>&& other) noexcept
        : tuple(std::move(other.reference))
    {
    }

    constexpr BasicContiguousReference& operator=(const BasicContiguousReference& other) noexcept(
        ListTraits::IS_NOTHROW_COPY_ASSIGNABLE)
    {
        this->assign(other);
        return *this;
    }

    template <cntgs::ContiguousReferenceQualifier TQualifier>
    constexpr BasicContiguousReference& operator=(const cntgs::BasicContiguousReference<TQualifier, Types...>&
                                                      other) noexcept(ListTraits::IS_NOTHROW_COPY_ASSIGNABLE)
    {
        this->assign(other);
        return *this;
    }

    template <class Allocator>
    constexpr BasicContiguousReference& operator=(const cntgs::BasicContiguousElement<Allocator, Types...>&
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

    template <cntgs::ContiguousReferenceQualifier TQualifier>
    constexpr BasicContiguousReference&
    operator=(cntgs::BasicContiguousReference<TQualifier, Types...>&& other) noexcept(
        cntgs::BasicContiguousReference<TQualifier, Types...>::IS_CONST ? ListTraits::IS_NOTHROW_MOVE_ASSIGNABLE
                                                                        : ListTraits::IS_NOTHROW_COPY_ASSIGNABLE)
    {
        this->assign(other);
        return *this;
    }

    template <class Allocator>
    constexpr BasicContiguousReference& operator=(cntgs::BasicContiguousElement<Allocator, Types...>&& other) noexcept(
        ListTraits::IS_NOTHROW_MOVE_ASSIGNABLE)
    {
        this->assign(other.reference);
        return *this;
    }

    constexpr void swap(const BasicContiguousReference& other) const noexcept(ListTraits::IS_NOTHROW_SWAPPABLE)
    {
        ElementTraits::swap(other, *this);
    }

    [[nodiscard]] constexpr auto size_in_bytes() const noexcept { return this->data_end() - this->data_begin(); }

    [[nodiscard]] constexpr auto data_begin() const noexcept
    {
        return ElementTraits::template ParameterTraitsAt<0>::data_begin(std::get<0>(this->tuple));
    }

    [[nodiscard]] constexpr auto data_end() const noexcept
    {
        return ElementTraits::template ParameterTraitsAt<sizeof...(Types) - 1>::data_end(
            std::get<sizeof...(Types) - 1>(this->tuple));
    }

    template <cntgs::ContiguousReferenceQualifier TQualifier>
    [[nodiscard]] constexpr auto operator==(const cntgs::BasicContiguousReference<TQualifier, Types...>& other) const
    {
        return ElementTraits::equal(*this, other);
    }

    template <class Allocator>
    [[nodiscard]] constexpr auto operator==(const cntgs::BasicContiguousElement<Allocator, Types...>& other) const
    {
        return *this == other.reference;
    }

    template <cntgs::ContiguousReferenceQualifier TQualifier>
    [[nodiscard]] constexpr auto operator!=(const cntgs::BasicContiguousReference<TQualifier, Types...>& other) const
    {
        return !(*this == other);
    }

    template <class Allocator>
    [[nodiscard]] constexpr auto operator!=(const cntgs::BasicContiguousElement<Allocator, Types...>& other) const
    {
        return !(*this == other.reference);
    }

    template <cntgs::ContiguousReferenceQualifier TQualifier>
    [[nodiscard]] constexpr auto operator<(const cntgs::BasicContiguousReference<TQualifier, Types...>& other) const
    {
        return ElementTraits::lexicographical_compare(*this, other);
    }

    template <class Allocator>
    [[nodiscard]] constexpr auto operator<(const cntgs::BasicContiguousElement<Allocator, Types...>& other) const
    {
        return *this < other.reference;
    }

    template <cntgs::ContiguousReferenceQualifier TQualifier>
    [[nodiscard]] constexpr auto operator<=(const cntgs::BasicContiguousReference<TQualifier, Types...>& other) const
    {
        return !(other < *this);
    }

    template <class Allocator>
    [[nodiscard]] constexpr auto operator<=(const cntgs::BasicContiguousElement<Allocator, Types...>& other) const
    {
        return !(other.reference < *this);
    }

    template <cntgs::ContiguousReferenceQualifier TQualifier>
    [[nodiscard]] constexpr auto operator>(const cntgs::BasicContiguousReference<TQualifier, Types...>& other) const
    {
        return other < *this;
    }

    template <class Allocator>
    [[nodiscard]] constexpr auto operator>(const cntgs::BasicContiguousElement<Allocator, Types...>& other) const
    {
        return other.reference < *this;
    }

    template <cntgs::ContiguousReferenceQualifier TQualifier>
    [[nodiscard]] constexpr auto operator>=(const cntgs::BasicContiguousReference<TQualifier, Types...>& other) const
    {
        return !(*this < other);
    }

    template <class Allocator>
    [[nodiscard]] constexpr auto operator>=(const cntgs::BasicContiguousElement<Allocator, Types...>& other) const
    {
        return !(*this < other.reference);
    }

  private:
    template <class Reference>
    void assign(Reference& other) const
    {
        static constexpr auto USE_MOVE = !std::is_const_v<Reference> && !Reference::IS_CONST;
        ElementTraits::template assign<USE_MOVE>(other, *this);
    }
};

template <cntgs::ContiguousReferenceQualifier Qualifier, class... T>
constexpr void swap(const cntgs::BasicContiguousReference<Qualifier, T...>& lhs,
                    const cntgs::BasicContiguousReference<Qualifier, T...>&
                        rhs) noexcept(detail::ParameterListTraits<T...>::IS_NOTHROW_SWAPPABLE)
{
    lhs.swap(rhs);
}

template <std::size_t I, cntgs::ContiguousReferenceQualifier Qualifier, class... Types>
[[nodiscard]] constexpr decltype(auto) get(cntgs::BasicContiguousReference<Qualifier, Types...>& reference) noexcept
{
    return std::get<I>(reference.tuple);
}

template <std::size_t I, cntgs::ContiguousReferenceQualifier Qualifier, class... Types>
[[nodiscard]] constexpr decltype(auto) get(
    const cntgs::BasicContiguousReference<Qualifier, Types...>& reference) noexcept
{
    return std::get<I>(reference.tuple);
}

template <std::size_t I, cntgs::ContiguousReferenceQualifier Qualifier, class... Types>
[[nodiscard]] constexpr decltype(auto) get(cntgs::BasicContiguousReference<Qualifier, Types...>&& reference) noexcept
{
    return std::get<I>(std::move(reference.tuple));
}

template <std::size_t I, cntgs::ContiguousReferenceQualifier Qualifier, class... Types>
[[nodiscard]] constexpr decltype(auto) get(
    const cntgs::BasicContiguousReference<Qualifier, Types...>&& reference) noexcept
{
    return std::get<I>(std::move(reference.tuple));
}
}  // namespace cntgs

namespace std
{
template <std::size_t I, ::cntgs::ContiguousReferenceQualifier Qualifier, class... Types>
struct tuple_element<I, ::cntgs::BasicContiguousReference<Qualifier, Types...>>
    : std::tuple_element<I, typename ::cntgs::BasicContiguousReference<Qualifier, Types...>::Tuple>
{
};

template <::cntgs::ContiguousReferenceQualifier Qualifier, class... Types>
struct tuple_size<::cntgs::BasicContiguousReference<Qualifier, Types...>>
    : std::integral_constant<std::size_t, sizeof...(Types)>
{
};
}  // namespace std

#endif  // CNTGS_CONTIGUOUS_REFERENCE_HPP

// #include "cntgs/contiguous/referenceQualifier.hpp"


#include <cstddef>
#include <cstring>
#include <memory>
#include <tuple>

namespace cntgs
{
template <class... Types>
using ContiguousElement = cntgs::BasicContiguousElement<std::allocator<std::byte>, Types...>;

template <class Allocator, class... Types>
class BasicContiguousElement
{
  private:
    using ListTraits = detail::ParameterListTraits<Types...>;
    using VectorTraits = detail::ContiguousVectorTraits<Types...>;
    using ElementTraits = detail::ElementTraitsT<Types...>;
    using AllocatorTraits = std::allocator_traits<Allocator>;
    using StorageElementType = detail::AlignedByte<ElementTraits::template ParameterTraitsAt<0>::ALIGNMENT>;
    using StorageType = detail::AllocatorAwarePointer<
        typename std::allocator_traits<Allocator>::template rebind_alloc<StorageElementType>>;
    using Reference = typename VectorTraits::ReferenceType;

  public:
    using allocator_type = Allocator;

    StorageType memory;
    Reference reference;

    BasicContiguousElement() = default;

    template <cntgs::ContiguousReferenceQualifier Qualifier>
    /*implicit*/ BasicContiguousElement(const cntgs::BasicContiguousReference<Qualifier, Types...>& other,
                                        const allocator_type& allocator = {})
        : memory(other.size_in_bytes(), allocator), reference(this->store_and_load(other, other.size_in_bytes()))
    {
    }

    template <cntgs::ContiguousReferenceQualifier Qualifier>
    /*implicit*/ BasicContiguousElement(cntgs::BasicContiguousReference<Qualifier, Types...>&& other,
                                        const allocator_type& allocator = {})
        : memory(other.size_in_bytes(), allocator), reference(this->store_and_load(other, other.size_in_bytes()))
    {
    }

    /*implicit*/ BasicContiguousElement(const BasicContiguousElement& other)
        : memory(other.memory), reference(this->store_and_load(other.reference, other.reference.size_in_bytes()))
    {
    }

    template <class TAllocator>
    /*implicit*/ BasicContiguousElement(const BasicContiguousElement<TAllocator, Types...>& other)
        : memory(other.memory), reference(this->store_and_load(other.reference, other.reference.size_in_bytes()))
    {
    }

    template <class TAllocator>
    BasicContiguousElement(const BasicContiguousElement<TAllocator, Types...>& other, const allocator_type& allocator)
        : memory(other.reference.size_in_bytes(), allocator),
          reference(this->store_and_load(other.reference, other.reference.size_in_bytes()))
    {
    }

    BasicContiguousElement(BasicContiguousElement&&) = default;

    template <class TAllocator>
    BasicContiguousElement(BasicContiguousElement<TAllocator, Types...>&& other)
        : memory(std::move(other.memory)), reference(std::move(other.reference))
    {
    }

    template <class TAllocator>
    BasicContiguousElement(BasicContiguousElement<TAllocator, Types...>&& other, const allocator_type& allocator)
        : memory(this->acquire_memory(other, allocator)), reference(this->acquire_reference(other, allocator))
    {
    }

    ~BasicContiguousElement() noexcept
    {
        if constexpr (!ListTraits::IS_TRIVIALLY_DESTRUCTIBLE)
        {
            if (this->memory)
            {
                ElementTraits::destruct(this->reference);
            }
        }
    }

    BasicContiguousElement& operator=(const BasicContiguousElement& other) noexcept(
        ListTraits::IS_NOTHROW_COPY_ASSIGNABLE)
    {
        this->reference = other.reference;
        return *this;
    }

    template <class TAllocator>
    BasicContiguousElement& operator=(const BasicContiguousElement<TAllocator, Types...>& other) noexcept(
        ListTraits::IS_NOTHROW_COPY_ASSIGNABLE)
    {
        this->reference = other.reference;
        return *this;
    }

    BasicContiguousElement& operator=(BasicContiguousElement&& other) noexcept(ListTraits::IS_NOTHROW_MOVE_ASSIGNABLE)
    {
        this->reference = std::move(other.reference);
        return *this;
    }

    template <class TAllocator>
    BasicContiguousElement& operator=(BasicContiguousElement<TAllocator, Types...>&& other) noexcept(
        ListTraits::IS_NOTHROW_MOVE_ASSIGNABLE)
    {
        this->reference = std::move(other.reference);
        return *this;
    }

    template <cntgs::ContiguousReferenceQualifier Qualifier>
    constexpr BasicContiguousElement& operator=(const cntgs::BasicContiguousReference<Qualifier, Types...>&
                                                    other) noexcept(ListTraits::IS_NOTHROW_COPY_ASSIGNABLE)
    {
        this->reference = other;
        return *this;
    }

    template <cntgs::ContiguousReferenceQualifier Qualifier>
    constexpr BasicContiguousElement& operator=(cntgs::BasicContiguousReference<Qualifier, Types...>&& other) noexcept(
        cntgs::BasicContiguousReference<Qualifier, Types...>::IS_CONST ? ListTraits::IS_NOTHROW_MOVE_ASSIGNABLE
                                                                       : ListTraits::IS_NOTHROW_COPY_ASSIGNABLE)
    {
        this->reference = std::move(other);
        return *this;
    }

    [[nodiscard]] constexpr allocator_type get_allocator() const noexcept { return this->memory.get_allocator(); }

    template <cntgs::ContiguousReferenceQualifier TQualifier>
    [[nodiscard]] constexpr auto operator==(const cntgs::BasicContiguousReference<TQualifier, Types...>& other) const
    {
        return this->reference == other;
    }

    [[nodiscard]] constexpr auto operator==(const BasicContiguousElement& other) const
    {
        return this->reference == other.reference;
    }

    template <cntgs::ContiguousReferenceQualifier TQualifier>
    [[nodiscard]] constexpr auto operator!=(const cntgs::BasicContiguousReference<TQualifier, Types...>& other) const
    {
        return !(this->reference == other);
    }

    [[nodiscard]] constexpr auto operator!=(const BasicContiguousElement& other) const
    {
        return !(this->reference == other.reference);
    }

    template <cntgs::ContiguousReferenceQualifier TQualifier>
    [[nodiscard]] constexpr auto operator<(const cntgs::BasicContiguousReference<TQualifier, Types...>& other) const
    {
        return this->reference < other;
    }

    [[nodiscard]] constexpr auto operator<(const BasicContiguousElement& other) const
    {
        return this->reference < other.reference;
    }

    template <cntgs::ContiguousReferenceQualifier TQualifier>
    [[nodiscard]] constexpr auto operator<=(const cntgs::BasicContiguousReference<TQualifier, Types...>& other) const
    {
        return !(other < this->reference);
    }

    [[nodiscard]] constexpr auto operator<=(const BasicContiguousElement& other) const
    {
        return !(other.reference < this->reference);
    }

    template <cntgs::ContiguousReferenceQualifier TQualifier>
    [[nodiscard]] constexpr auto operator>(const cntgs::BasicContiguousReference<TQualifier, Types...>& other) const
    {
        return other < this->reference;
    }

    [[nodiscard]] constexpr auto operator>(const BasicContiguousElement& other) const
    {
        return other.reference < this->reference;
    }

    template <cntgs::ContiguousReferenceQualifier TQualifier>
    [[nodiscard]] constexpr auto operator>=(const cntgs::BasicContiguousReference<TQualifier, Types...>& other) const
    {
        return !(this->reference < other);
    }

    [[nodiscard]] constexpr auto operator>=(const BasicContiguousElement& other) const
    {
        return !(this->reference < other.reference);
    }

  private:
    template <class SourceReference>
    auto store_and_load(SourceReference& source, std::size_t memory_size) const
    {
        static constexpr auto USE_MOVE = !std::is_const_v<SourceReference> && !SourceReference::IS_CONST;
        const auto begin = this->memory_begin();
        std::memcpy(begin, source.data_begin(), memory_size);
        auto target =
            ElementTraits::template load_element_at<detail::IgnoreFirstAlignmentNeeds,
                                                    detail::ContiguousReferenceSizeGetter>(begin, source.tuple);
        ElementTraits::template construct_if_non_trivial<USE_MOVE>(source, target);
        return Reference{target};
    }

    template <class TAllocator>
    auto acquire_memory(BasicContiguousElement<TAllocator, Types...>& other, const allocator_type& allocator) const
    {
        if (allocator == other.memory.get_allocator())
        {
            return std::move(other.memory);
        }
        return StorageType(other.reference.size_in_bytes(), allocator);
    }

    template <class TAllocator>
    auto acquire_reference(BasicContiguousElement<TAllocator, Types...>& other, const allocator_type& allocator) const
    {
        if (allocator == other.memory.get_allocator())
        {
            return std::move(other.reference);
        }
        return this->store_and_load(other.reference, other.reference.size_in_bytes());
    }

    auto memory_begin() const noexcept
    {
        return detail::assume_aligned<ElementTraits::template ParameterTraitsAt<0>::ALIGNMENT>(
            reinterpret_cast<std::byte*>(this->memory.get()));
    }
};

template <class Allocator, class... T>
constexpr void swap(cntgs::BasicContiguousElement<Allocator, T...>& lhs,
                    cntgs::BasicContiguousElement<Allocator, T...>& rhs) noexcept
{
    using std::swap;
    swap(lhs.memory, rhs.memory);
    auto temp{lhs.reference.tuple};
    detail::construct_at(&lhs.reference.tuple, rhs.reference.tuple);
    detail::construct_at(&rhs.reference.tuple, temp);
}

template <std::size_t I, class Allocator, class... Types>
[[nodiscard]] constexpr decltype(auto) get(cntgs::BasicContiguousElement<Allocator, Types...>& element) noexcept
{
    return cntgs::get<I>(element.reference);
}

template <std::size_t I, class Allocator, class... Types>
[[nodiscard]] constexpr decltype(auto) get(const cntgs::BasicContiguousElement<Allocator, Types...>& element) noexcept
{
    return detail::as_const(cntgs::get<I>(element.reference));
}

template <std::size_t I, class Allocator, class... Types>
[[nodiscard]] constexpr decltype(auto) get(cntgs::BasicContiguousElement<Allocator, Types...>&& element) noexcept
{
    return cntgs::get<I>(std::move(element.reference));
}

template <std::size_t I, class Allocator, class... Types>
[[nodiscard]] constexpr decltype(auto) get(const cntgs::BasicContiguousElement<Allocator, Types...>&& element) noexcept
{
    return detail::as_const(cntgs::get<I>(std::move(element.reference)));
}
}  // namespace cntgs

namespace std
{
template <std::size_t I, class Allocator, class... Types>
struct tuple_element<I, ::cntgs::BasicContiguousElement<Allocator, Types...>>
    : std::tuple_element<I, decltype(::cntgs::BasicContiguousElement<Allocator, Types...>::reference)>
{
};

template <class Allocator, class... Types>
struct tuple_size<::cntgs::BasicContiguousElement<Allocator, Types...>>
    : std::integral_constant<std::size_t, sizeof...(Types)>
{
};
}  // namespace std

#endif  // CNTGS_CONTIGUOUS_ELEMENT_HPP

// #include "cntgs/contiguous/iterator.hpp"
#ifndef CNTGS_CONTIGUOUS_ITERATOR_HPP
#define CNTGS_CONTIGUOUS_ITERATOR_HPP

// #include "cntgs/contiguous/detail/elementLocator.hpp"
#ifndef CNTGS_DETAIL_ELEMENTLOCATOR_HPP
#define CNTGS_DETAIL_ELEMENTLOCATOR_HPP

// #include "cntgs/contiguous/detail/elementTraits.hpp"

// #include "cntgs/contiguous/detail/memory.hpp"

// #include "cntgs/contiguous/detail/parameterListTraits.hpp"

// #include "cntgs/contiguous/detail/typeUtils.hpp"


#include <algorithm>
#include <array>
#include <type_traits>

namespace cntgs::detail
{
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
};

template <bool IsAllFixedSize, class... Types>
class ElementLocator : public BaseElementLocator
{
  private:
    using Self = ElementLocator<IsAllFixedSize, Types...>;
    using ElementTraits = detail::ElementTraitsT<Types...>;
    using FixedSizes = typename detail::ParameterListTraits<Types...>::FixedSizes;

  public:
    ElementLocator() = default;

    ElementLocator(std::size_t max_element_count, std::byte* memory_begin, const FixedSizes&) noexcept
        : BaseElementLocator(reinterpret_cast<std::byte**>(memory_begin),
                             Self::calculate_element_start(max_element_count, memory_begin))
    {
    }

    ElementLocator(ElementLocator& old_locator, std::size_t old_max_element_count, std::byte* old_memory_begin,
                   std::size_t new_max_element_count, std::byte* new_memory_begin) noexcept
    {
        this->copy_into(old_locator, old_max_element_count, old_memory_begin, new_max_element_count, new_memory_begin);
    }

    template <class... Args>
    void emplace_back(const FixedSizes& fixed_sizes, Args&&... args)
    {
        *this->last_element_address = last_element;
        ++this->last_element_address;
        last_element = ElementTraits::emplace_at(last_element, fixed_sizes, std::forward<Args>(args)...);
    }

    template <class... Args>
    static void emplace_at(std::size_t index, std::byte* memory_begin, const FixedSizes& fixed_sizes, Args&&... args)
    {
        const auto element_addresses_begin = reinterpret_cast<std::byte**>(memory_begin);
        element_addresses_begin[index + 1] =
            ElementTraits::emplace_at_aliased(element_addresses_begin[index], fixed_sizes, std::forward<Args>(args)...);
    }

    void copy_into(std::size_t old_max_element_count, std::byte* old_memory_begin, std::size_t new_max_element_count,
                   std::byte* new_memory_begin) noexcept
    {
        this->copy_into(*this, old_max_element_count, old_memory_begin, new_max_element_count, new_memory_begin);
    }

  private:
    void copy_into(ElementLocator& old_locator, std::size_t old_max_element_count, std::byte* old_memory_begin,
                   std::size_t new_max_element_count, std::byte* new_memory_begin) noexcept
    {
        const auto new_start = Self::calculate_element_start(new_max_element_count, new_memory_begin);
        const auto old_start = Self::calculate_element_start(old_max_element_count, old_memory_begin);
        const auto size_diff = std::distance(new_memory_begin, new_start) - std::distance(old_memory_begin, old_start);
        const auto new_last_element_address = Self::copy_element_addresses(new_memory_begin, old_memory_begin,
                                                                           old_locator.last_element_address, size_diff);
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
        return static_cast<std::byte*>(detail::align<ElementTraits::template ParameterTraitsAt<0>::ALIGNMENT>(
            memory_begin + BaseElementLocator::reserved_bytes(max_element_count)));
    }
};

class BaseAllFixedSizeElementLocator
{
  protected:
    std::size_t element_count{};
    std::size_t stride{};
    std::byte* start{};

    BaseAllFixedSizeElementLocator() = default;

    BaseAllFixedSizeElementLocator(std::size_t element_count, std::size_t stride, std::byte* start)
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
};

template <class... Types>
class ElementLocator<true, Types...> : public BaseAllFixedSizeElementLocator
{
  private:
    using Self = detail::ElementLocator<true, Types...>;
    using ElementTraits = detail::ElementTraitsT<Types...>;
    using FixedSizes = typename detail::ParameterListTraits<Types...>::FixedSizes;

  public:
    ElementLocator() = default;

    ElementLocator(std::size_t, std::byte* memory_begin, const FixedSizes& fixed_sizes) noexcept
        : BaseAllFixedSizeElementLocator({}, ElementTraits::calculate_element_size(fixed_sizes),
                                         Self::calculate_element_start(memory_begin))
    {
    }

    ElementLocator(ElementLocator& old_locator, std::size_t, const std::byte*, std::size_t,
                   std::byte* new_memory_begin) noexcept
        : BaseAllFixedSizeElementLocator(old_locator.element_count, old_locator.stride, {})
    {
        this->copy_into(old_locator, new_memory_begin);
    }

    template <class... Args>
    void emplace_back(const FixedSizes& fixed_sizes, Args&&... args)
    {
        auto last_element = this->element_address(this->element_count, {});
        ++this->element_count;
        ElementTraits::emplace_at(last_element, fixed_sizes, std::forward<Args>(args)...);
    }

    template <class... Args>
    void emplace_at(std::size_t index, const std::byte*, const FixedSizes& fixed_sizes, Args&&... args)
    {
        ElementTraits::emplace_at_aliased(this->element_address(index, {}), fixed_sizes, std::forward<Args>(args)...);
    }

    void copy_into(std::size_t, const std::byte*, std::size_t, std::byte* new_memory_begin) noexcept
    {
        this->copy_into(*this, new_memory_begin);
    }

  private:
    void copy_into(const ElementLocator& old_locator, std::byte* new_memory_begin) noexcept
    {
        const auto new_start = Self::calculate_element_start(new_memory_begin);
        std::memcpy(new_start, old_locator.start, old_locator.element_count * old_locator.stride);
        this->start = new_start;
    }

    static constexpr auto calculate_element_start(std::byte* memory_begin) noexcept
    {
        return static_cast<std::byte*>(
            detail::align<ElementTraits::template ParameterTraitsAt<0>::ALIGNMENT>(memory_begin));
    }
};

template <class... Types>
using ElementLocatorT = detail::ElementLocator<(detail::ParameterListTraits<Types...>::CONTIGUOUS_FIXED_SIZE_COUNT ==
                                                detail::ParameterListTraits<Types...>::CONTIGUOUS_COUNT),
                                               Types...>;

using TypeErasedElementLocator =
    std::aligned_storage_t<std::max(sizeof(detail::ElementLocator<false>), sizeof(detail::ElementLocator<true>)),
                           std::max(alignof(detail::ElementLocator<false>), alignof(detail::ElementLocator<true>))>;

template <class T>
auto type_erase_element_locator(T&& locator) noexcept
{
    detail::TypeErasedElementLocator result;
    detail::construct_at(reinterpret_cast<detail::RemoveCvrefT<T>*>(&result), std::forward<T>(locator));
    return result;
}
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_ELEMENTLOCATOR_HPP

// #include "cntgs/contiguous/detail/forward.hpp"

// #include "cntgs/contiguous/detail/iteratorUtils.hpp"

// #include "cntgs/contiguous/detail/typeUtils.hpp"


#include <iterator>

namespace cntgs
{
template <bool IsConst, class Allocator, class... Types>
class ContiguousVectorIterator
{
  private:
    using Vector = cntgs::BasicContiguousVector<Allocator, Types...>;
    using ListTraits = detail::ParameterListTraits<Types...>;
    using ElementLocator = detail::ElementLocatorT<Types...>;

  public:
    using value_type = typename Vector::value_type;
    using reference = detail::ConditionalT<IsConst, typename Vector::const_reference, typename Vector::reference>;
    using pointer = detail::ArrowProxy<reference>;
    using difference_type = typename Vector::difference_type;
    using iterator_category = std::random_access_iterator_tag;

    ContiguousVectorIterator() = default;

    constexpr ContiguousVectorIterator(const Vector& vector, typename Vector::size_type index) noexcept
        : i(index), memory(vector.memory.get()), fixed_sizes(vector.fixed_sizes), locator(vector.locator)
    {
    }

    explicit constexpr ContiguousVectorIterator(const Vector& vector) noexcept : ContiguousVectorIterator(vector, {}) {}

    template <bool OtherIsConst>
    /*implicit*/ constexpr ContiguousVectorIterator(
        const ContiguousVectorIterator<OtherIsConst, Allocator, Types...>& other) noexcept
        : i(other.i), memory(memory), fixed_sizes(fixed_sizes), locator(locator)
    {
    }

    ContiguousVectorIterator(const ContiguousVectorIterator&) = default;
    ContiguousVectorIterator(ContiguousVectorIterator&&) = default;

    template <bool OtherIsConst>
    constexpr ContiguousVectorIterator& operator=(
        const ContiguousVectorIterator<OtherIsConst, Allocator, Types...>& other) noexcept
    {
        this->i = other.i;
        this->memory = memory;
        this->fixed_sizes = fixed_sizes;
        this->locator = locator;
        return *this;
    }

    ContiguousVectorIterator& operator=(const ContiguousVectorIterator&) = default;
    ContiguousVectorIterator& operator=(ContiguousVectorIterator&&) = default;

    [[nodiscard]] constexpr auto index() const noexcept { return this->i; }

    [[nodiscard]] constexpr reference operator*() const noexcept
    {
        return reference{this->locator.element_address(i, this->memory), this->fixed_sizes};
    }

    [[nodiscard]] constexpr reference operator*() noexcept
    {
        return reference{this->locator.element_address(i, this->memory), this->fixed_sizes};
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
    template <bool, class, class...>
    friend class ContiguousVectorIterator;

    typename Vector::size_type i{};
    std::byte* memory;
    typename ListTraits::FixedSizes fixed_sizes;
    ElementLocator locator;
};
}  // namespace cntgs

#endif  // CNTGS_CONTIGUOUS_ITERATOR_HPP

// #include "cntgs/contiguous/parameter.hpp"

// #include "cntgs/contiguous/reference.hpp"

// #include "cntgs/contiguous/span.hpp"

// #include "cntgs/contiguous/typeErasedVector.hpp"
#ifndef CNTGS_CONTIGUOUS_TYPEERASEDVECTOR_HPP
#define CNTGS_CONTIGUOUS_TYPEERASEDVECTOR_HPP

// #include "cntgs/contiguous/detail/elementLocator.hpp"

// #include "cntgs/contiguous/detail/memory.hpp"

// #include "cntgs/contiguous/detail/utility.hpp"

// #include "cntgs/contiguous/detail/vectorTraits.hpp"


#include <memory>

namespace cntgs
{
class TypeErasedVector
{
  public:
    std::size_t memory_size;
    std::size_t max_element_count;
    std::byte* memory;
    detail::MoveDefaultingValue<bool> is_memory_owned;
    std::array<std::size_t, detail::MAX_FIXED_SIZE_VECTOR_PARAMETER> fixed_sizes;
    detail::TypeErasedElementLocator locator;
    void (*destructor)(cntgs::TypeErasedVector&);
    detail::TypeErasedAllocator allocator;

    TypeErasedVector(std::size_t memory_size, std::size_t max_element_count, std::byte* memory, bool is_memory_owned,
                     detail::TypeErasedAllocator allocator,
                     const std::array<std::size_t, detail::MAX_FIXED_SIZE_VECTOR_PARAMETER>& fixed_sizes,
                     detail::TypeErasedElementLocator locator, void (*destructor)(cntgs::TypeErasedVector&)) noexcept
        : memory_size(memory_size),
          max_element_count(max_element_count),
          memory(memory),
          is_memory_owned(is_memory_owned),
          fixed_sizes(fixed_sizes),
          locator(locator),
          destructor(destructor),
          allocator(allocator)
    {
    }

    TypeErasedVector(const TypeErasedVector&) = delete;
    TypeErasedVector(TypeErasedVector&&) = default;

    TypeErasedVector& operator=(const TypeErasedVector&) = delete;
    TypeErasedVector& operator=(TypeErasedVector&&) = default;

    ~TypeErasedVector() noexcept { destructor(*this); }
};
}  // namespace cntgs

#endif  // CNTGS_CONTIGUOUS_TYPEERASEDVECTOR_HPP

// #include "cntgs/contiguous/vector.hpp"
#ifndef CNTGS_CONTIGUOUS_VECTOR_HPP
#define CNTGS_CONTIGUOUS_VECTOR_HPP

// #include "cntgs/contiguous/detail/algorithm.hpp"

// #include "cntgs/contiguous/detail/array.hpp"
#ifndef CNTGS_DETAIL_ARRAY_HPP
#define CNTGS_DETAIL_ARRAY_HPP

#include <algorithm>
#include <array>
#include <cstddef>
#include <utility>

namespace cntgs::detail
{
template <std::size_t N, class T, std::size_t K, std::size_t... I>
constexpr auto convert_array_to_size(const std::array<T, K>& array, std::index_sequence<I...>)
{
    return std::array<T, N>{std::get<I>(array)...};
}

template <std::size_t N, class T, std::size_t K>
constexpr auto convert_array_to_size(const std::array<T, K>& array)
{
    return detail::convert_array_to_size<N>(array, std::make_index_sequence<std::min(N, K)>{});
}
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_ARRAY_HPP

// #include "cntgs/contiguous/detail/elementLocator.hpp"

// #include "cntgs/contiguous/detail/forward.hpp"

// #include "cntgs/contiguous/detail/memory.hpp"

// #include "cntgs/contiguous/detail/parameterListTraits.hpp"

// #include "cntgs/contiguous/detail/parameterTraits.hpp"

// #include "cntgs/contiguous/detail/tuple.hpp"

// #include "cntgs/contiguous/detail/utility.hpp"

// #include "cntgs/contiguous/detail/vectorTraits.hpp"

// #include "cntgs/contiguous/iterator.hpp"

// #include "cntgs/contiguous/parameter.hpp"

// #include "cntgs/contiguous/reference.hpp"

// #include "cntgs/contiguous/span.hpp"

// #include "cntgs/contiguous/typeErasedVector.hpp"


#include <algorithm>
#include <array>
#include <cstring>
#include <memory>
#include <new>
#include <tuple>
#include <type_traits>
#include <utility>

namespace cntgs
{
/// Alias template for [cntgs::BasicContiguousVector]() that uses [std::allocator]()
template <class... Types>
using ContiguousVector = cntgs::BasicContiguousVector<std::allocator<std::byte>, Types...>;

/// Container that stores the value of each specified parameter contiguously.
///
/// \param Allocator An allocator that is used to acquire/release memory and to construct/destroy the elements in that
/// memory. The type must meet the requirements of [Allocator](https://en.cppreference.com/w/cpp/named_req/Allocator).
/// \param Types Any of [cntgs::VaryingSize](), [cntgs::FixedSize](), [cntgs::AlignAs]() or a plain user-defined or
/// built-in type. The underlying type of each parameter must meet the requirements of
/// [Erasable](https://en.cppreference.com/w/cpp/named_req/Erasable)
template <class Allocator, class... Types>
class BasicContiguousVector
{
  private:
    using Self = cntgs::BasicContiguousVector<Allocator, Types...>;
    using ListTraits = detail::ParameterListTraits<Types...>;
    using VectorTraits = detail::ContiguousVectorTraits<Types...>;
    using ElementLocator = detail::ElementLocatorT<Types...>;
    using ElementTraits = detail::ElementTraitsT<Types...>;
    using AllocatorTraits = std::allocator_traits<Allocator>;
    using StorageType =
        detail::MaybeOwnedAllocatorAwarePointer<typename AllocatorTraits::template rebind_alloc<std::byte>>;
    using FixedSizes = typename ListTraits::FixedSizes;

    static constexpr bool IS_MIXED = ListTraits::IS_MIXED;
    static constexpr bool IS_ALL_FIXED_SIZE = ListTraits::IS_ALL_FIXED_SIZE;
    static constexpr bool IS_ALL_VARYING_SIZE = ListTraits::IS_ALL_VARYING_SIZE;
    static constexpr bool IS_ALL_PLAIN = ListTraits::IS_ALL_PLAIN;

  public:
    /// Type that can create copies of [cntgs::BasicContiguousVector::reference]() and
    /// [cntgs::BasicContiguousVector::const_reference]()
    using value_type = cntgs::BasicContiguousElement<Allocator, Types...>;

    /// A [cntgs::BasicContiguousReference]() with [cntgs::ContiguousReferenceQualifier::MUTABLE]()
    /// \exclude target
    using reference = typename VectorTraits::ReferenceType;

    /// A [cntgs::BasicContiguousReference]() with [cntgs::ContiguousReferenceQualifier::CONST]()
    /// \exclude target
    using const_reference = typename VectorTraits::ConstReferenceType;
    using iterator = cntgs::ContiguousVectorIterator<false, Allocator, Types...>;
    using const_iterator = cntgs::ContiguousVectorIterator<true, Allocator, Types...>;
    using difference_type = std::ptrdiff_t;
    using size_type = std::size_t;
    using allocator_type = Allocator;

    size_type max_element_count{};
    StorageType memory{};
    FixedSizes fixed_sizes{};
    ElementLocator locator;

    BasicContiguousVector() = default;

    explicit BasicContiguousVector(cntgs::TypeErasedVector&& vector) noexcept
        : BasicContiguousVector(vector, std::exchange(vector.is_memory_owned.value, false))
    {
    }

    explicit BasicContiguousVector(const cntgs::TypeErasedVector& vector) noexcept
        : BasicContiguousVector(vector, false)
    {
    }

    template <bool IsMixed = IS_MIXED>
    BasicContiguousVector(size_type max_element_count, size_type varying_size_bytes, const FixedSizes& fixed_sizes,
                          const allocator_type& allocator = {}, std::enable_if_t<IsMixed>* = nullptr)
        : BasicContiguousVector(max_element_count, varying_size_bytes, fixed_sizes, allocator)
    {
    }

    template <bool IsAllFixedSize = IS_ALL_FIXED_SIZE>
    BasicContiguousVector(size_type max_element_count, const FixedSizes& fixed_sizes,
                          const allocator_type& allocator = {}, std::enable_if_t<IsAllFixedSize>* = nullptr)
        : BasicContiguousVector(max_element_count, size_type{}, fixed_sizes, allocator)
    {
    }

    template <bool IsAllFixedSize = IS_ALL_FIXED_SIZE>
    BasicContiguousVector(std::byte* transferred_ownership, size_type memory_size, size_type max_element_count,
                          const FixedSizes& fixed_sizes, const allocator_type& allocator = {},
                          std::enable_if_t<IsAllFixedSize>* = nullptr)
        : BasicContiguousVector(transferred_ownership, memory_size, true, max_element_count, fixed_sizes, allocator)
    {
    }

    template <bool IsAllFixedSize = IS_ALL_FIXED_SIZE>
    BasicContiguousVector(cntgs::Span<std::byte> mutable_view, size_type max_element_count,
                          const FixedSizes& fixed_sizes, const allocator_type& allocator = {},
                          std::enable_if_t<IsAllFixedSize>* = nullptr)
        : BasicContiguousVector(mutable_view.data(), mutable_view.size(), false, max_element_count, fixed_sizes,
                                allocator)
    {
    }

    template <bool IsAllVaryingSize = IS_ALL_VARYING_SIZE>
    BasicContiguousVector(size_type max_element_count, size_type varying_size_bytes,
                          const allocator_type& allocator = {}, std::enable_if_t<IsAllVaryingSize>* = nullptr)
        : BasicContiguousVector(max_element_count, varying_size_bytes, FixedSizes{}, allocator)
    {
    }

    template <bool IsNoneSpecial = IS_ALL_PLAIN>
    BasicContiguousVector(size_type max_element_count, const allocator_type& allocator = {},
                          std::enable_if_t<IsNoneSpecial>* = nullptr)
        : BasicContiguousVector(max_element_count, size_type{}, FixedSizes{}, allocator)
    {
    }

    template <bool IsNoneSpecial = IS_ALL_PLAIN>
    BasicContiguousVector(std::byte* transferred_ownership, size_type memory_size, size_type max_element_count,
                          const allocator_type& allocator = {}, std::enable_if_t<IsNoneSpecial>* = nullptr)
        : BasicContiguousVector(transferred_ownership, memory_size, true, max_element_count, FixedSizes{}, allocator)
    {
    }

    template <bool IsNoneSpecial = IS_ALL_PLAIN>
    BasicContiguousVector(cntgs::Span<std::byte> mutable_view, size_type max_element_count,
                          const allocator_type& allocator = {}, std::enable_if_t<IsNoneSpecial>* = nullptr)
        : BasicContiguousVector(mutable_view.data(), mutable_view.size(), false, max_element_count, FixedSizes{},
                                allocator)
    {
    }

    BasicContiguousVector(const BasicContiguousVector& other)
        : max_element_count(other.max_element_count),
          memory(other.memory),
          fixed_sizes(other.fixed_sizes),
          locator(this->copy_construct_locator(other))
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

    BasicContiguousVector& operator=(BasicContiguousVector&& other)
    {
        if (this != std::addressof(other))
        {
            this->move_assign(std::move(other));
        }
        return *this;
    }

    ~BasicContiguousVector() noexcept { this->destruct_if_owned(); }

    template <class... Args>
    void emplace_back(Args&&... args)
    {
        this->locator.emplace_back(this->fixed_sizes, std::forward<Args>(args)...);
    }

    void pop_back() noexcept
    {
        ElementTraits::destruct(this->back());
        this->locator.resize(this->size() - size_type{1}, this->memory.get());
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
        const auto position_data_end = it_position->data_end();
        const auto next_position = position.index() + 1;
        ElementTraits::destruct(*it_position);
        this->move_elements_forward_to(it_position, next_position, position_data_end);
        this->locator.resize(this->size() - size_type{1}, this->memory.get());
        return it_position;
    }

    iterator erase(const_iterator first, const_iterator last) noexcept(ListTraits::IS_NOTHROW_MOVE_CONSTRUCTIBLE)
    {
        const auto current_size = this->size();
        iterator it_first{*this, first.index()};
        iterator it_last{*this, last.index()};
        this->destruct(it_first, it_last);
        if (last.index() < current_size && first.index() != last.index())
        {
            this->move_elements_forward_to(it_first, last.index(), it_last->data_begin());
        }
        this->locator.resize(current_size - std::distance(it_first, it_last), this->memory.get());
        return it_first;
    }

    void clear() noexcept
    {
        this->destruct();
        this->locator.resize(0, this->memory.get());
    }

    [[nodiscard]] reference operator[](size_type i) noexcept
    {
        return reference{this->locator.element_address(i, this->memory.get()), this->fixed_sizes};
    }

    [[nodiscard]] const_reference operator[](size_type i) const noexcept
    {
        return const_reference{this->locator.element_address(i, this->memory.get()), this->fixed_sizes};
    }

    [[nodiscard]] reference front() noexcept { return (*this)[{}]; }

    [[nodiscard]] const_reference front() const noexcept { return (*this)[{}]; }

    [[nodiscard]] reference back() noexcept { return (*this)[this->size() - size_type{1}]; }

    [[nodiscard]] const_reference back() const noexcept { return (*this)[this->size() - size_type{1}]; }

    template <std::size_t I>
    [[nodiscard]] constexpr size_type get_fixed_size() const noexcept
    {
        return std::get<I>(this->fixed_sizes);
    }

    [[nodiscard]] constexpr bool empty() const noexcept { return this->locator.empty(this->memory.get()); }

    [[nodiscard]] constexpr std::byte* data() noexcept { return this->locator.element_address({}, this->memory.get()); }

    [[nodiscard]] constexpr const std::byte* data() const noexcept
    {
        return this->locator.element_address({}, this->memory.get());
    }

    [[nodiscard]] constexpr std::byte* data_begin() noexcept { return this->data(); }

    [[nodiscard]] constexpr const std::byte* data_begin() const noexcept { return this->data(); }

    [[nodiscard]] constexpr std::byte* data_end() noexcept { return this->locator.data_end(); }

    [[nodiscard]] constexpr const std::byte* data_end() const noexcept { return this->locator.data_end(); }

    [[nodiscard]] constexpr size_type size() const noexcept { return this->locator.size(this->memory.get()); }

    [[nodiscard]] constexpr size_type capacity() const noexcept { return this->max_element_count; }

    [[nodiscard]] constexpr size_type memory_consumption() const noexcept { return this->memory.size(); }

    [[nodiscard]] constexpr iterator begin() noexcept { return iterator{*this}; }

    [[nodiscard]] constexpr const_iterator begin() const noexcept { return const_iterator{*this}; }

    [[nodiscard]] constexpr const_iterator cbegin() const noexcept { return this->begin(); }

    [[nodiscard]] constexpr iterator end() noexcept { return iterator{*this, this->size()}; }

    [[nodiscard]] constexpr const_iterator end() const noexcept { return const_iterator{*this, this->size()}; }

    [[nodiscard]] constexpr const_iterator cend() const noexcept { return this->end(); }

    [[nodiscard]] constexpr allocator_type get_allocator() const noexcept { return this->memory.get_allocator(); }

    template <class TAllocator>
    [[nodiscard]] constexpr auto operator==(const cntgs::BasicContiguousVector<TAllocator, Types...>& other) const
    {
        return this->equal(other);
    }

    template <class TAllocator>
    [[nodiscard]] constexpr auto operator!=(const cntgs::BasicContiguousVector<TAllocator, Types...>& other) const
    {
        return !(*this == other);
    }

    template <class TAllocator>
    [[nodiscard]] constexpr auto operator<(const cntgs::BasicContiguousVector<TAllocator, Types...>& other) const
    {
        return this->lexicographical_compare(other);
    }

    template <class TAllocator>
    [[nodiscard]] constexpr auto operator<=(const cntgs::BasicContiguousVector<TAllocator, Types...>& other) const
    {
        return !(other < *this);
    }

    template <class TAllocator>
    [[nodiscard]] constexpr auto operator>(const cntgs::BasicContiguousVector<TAllocator, Types...>& other) const
    {
        return other < *this;
    }

    template <class TAllocator>
    [[nodiscard]] constexpr auto operator>=(const cntgs::BasicContiguousVector<TAllocator, Types...>& other) const
    {
        return !(*this < other);
    }

    // private API
  private:
    BasicContiguousVector(std::byte* memory, size_type memory_size, bool is_memory_owned, size_type max_element_count,
                          const FixedSizes& fixed_sizes, const allocator_type& allocator)
        : max_element_count(max_element_count),
          memory(memory, memory_size, is_memory_owned, allocator),
          fixed_sizes(fixed_sizes),
          locator(max_element_count, this->memory.get(), fixed_sizes)
    {
    }

    BasicContiguousVector(size_type max_element_count, size_type varying_size_bytes, const FixedSizes& fixed_sizes,
                          const allocator_type& allocator)
        : max_element_count(max_element_count),
          memory(Self::calculate_needed_memory_size(max_element_count, varying_size_bytes, fixed_sizes), allocator),
          fixed_sizes(fixed_sizes),
          locator(max_element_count, this->memory.get(), fixed_sizes)
    {
    }

    BasicContiguousVector(const cntgs::TypeErasedVector& vector, bool is_memory_owned) noexcept
        : max_element_count(vector.max_element_count),
          memory(vector.memory, vector.memory_size, is_memory_owned,
                 *std::launder(reinterpret_cast<const allocator_type*>(&vector.allocator))),
          fixed_sizes(detail::convert_array_to_size<ListTraits::CONTIGUOUS_FIXED_SIZE_COUNT>(vector.fixed_sizes)),
          locator(*std::launder(reinterpret_cast<const ElementLocator*>(&vector.locator)))
    {
    }

    [[nodiscard]] static constexpr auto calculate_needed_memory_size(size_type max_element_count,
                                                                     size_type varying_size_bytes,
                                                                     const FixedSizes& fixed_sizes) noexcept
    {
        constexpr auto ALIGNMENT_OVERHEAD = ListTraits::template ParameterTraitsAt<0>::ALIGNMENT - 1;
        return varying_size_bytes + ElementTraits::calculate_element_size(fixed_sizes) * max_element_count +
               ElementLocator::reserved_bytes(max_element_count) + ALIGNMENT_OVERHEAD;
    }

    void grow(size_type new_max_element_count, size_type new_varying_size_bytes)
    {
        const auto new_memory_size =
            this->calculate_needed_memory_size(new_max_element_count, new_varying_size_bytes, this->fixed_sizes);
        StorageType new_memory{new_memory_size, this->get_allocator()};
        this->template insert_into<true, true>(new_max_element_count, new_memory, this->locator);
        this->max_element_count = new_max_element_count;
        this->memory.get_impl().get() = new_memory.release();
        this->memory.get_impl().size() = new_memory_size;
    }

    template <bool UseMove, bool IsDestruct = false>
    void insert_into(size_type new_max_element_count, StorageType& new_memory, ElementLocator& locator)
    {
        static constexpr auto IS_TRIVIAL =
            UseMove ? ListTraits::IS_TRIVIALLY_MOVE_CONSTRUCTIBLE : ListTraits::IS_TRIVIALLY_COPY_CONSTRUCTIBLE;
        if constexpr (IS_TRIVIAL && ListTraits::IS_TRIVIALLY_DESTRUCTIBLE)
        {
            locator.copy_into(this->max_element_count, this->memory.get(), new_max_element_count, new_memory.get());
        }
        else
        {
            ElementLocator new_locator{locator, this->max_element_count, this->memory.get(), new_max_element_count,
                                       new_memory.get()};
            this->template uninitialized_construct_if_non_trivial<UseMove>(new_memory.get(), new_locator);
            if constexpr (IsDestruct)
            {
                this->destruct();
            }
            locator = new_locator;
        }
    }

    template <bool UseMove>
    void uninitialized_construct_if_non_trivial([[maybe_unused]] std::byte* new_memory,
                                                [[maybe_unused]] ElementLocator& new_locator)
    {
        static constexpr auto IS_TRIVIAL =
            UseMove ? ListTraits::IS_TRIVIALLY_MOVE_CONSTRUCTIBLE : ListTraits::IS_TRIVIALLY_COPY_CONSTRUCTIBLE;
        if constexpr (!IS_TRIVIAL)
        {
            for (size_type i{}; i < this->size(); ++i)
            {
                auto&& source = (*this)[i];
                auto&& target =
                    ElementTraits::load_element_at(new_locator.element_address(i, new_memory), this->fixed_sizes);
                ElementTraits::template construct_if_non_trivial<UseMove>(source, target);
            }
        }
    }

    void move_elements_forward_to(const iterator& position, [[maybe_unused]] std::size_t from,
                                  [[maybe_unused]] std::byte* from_data_begin)
    {
        if constexpr (ListTraits::IS_TRIVIALLY_MOVE_CONSTRUCTIBLE && ListTraits::IS_TRIVIALLY_DESTRUCTIBLE &&
                      ListTraits::IS_FIXED_SIZE_OR_PLAIN)
        {
            const auto target = position->data_begin();
            std::memmove(target, from_data_begin, (this->memory.get() + this->memory_consumption()) - from_data_begin);
        }
        else
        {
            for (auto i = position.index(); from != this->size(); ++i, (void)++from)
            {
                this->emplace_at(i, (*this)[from], ListTraits::make_index_sequence());
            }
        }
    }

    template <std::size_t... I>
    void emplace_at(std::size_t i, const reference& element, std::index_sequence<I...>)
    {
        this->locator.emplace_at(i, this->memory.get(), this->fixed_sizes, std::move(cntgs::get<I>(element))...);
        ElementTraits::destruct(element);
    }

    void steal(BasicContiguousVector&& other)
    {
        this->destruct();
        this->max_element_count = other.max_element_count;
        this->memory = std::move(other.memory);
        this->fixed_sizes = other.fixed_sizes;
        this->locator = other.locator;
    }

    void move_assign(BasicContiguousVector&& other)
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
                    // allocate memory first because it may throw
                    StorageType new_memory{other.memory_consumption(), this->get_allocator()};
                    this->destruct();
                    other.template insert_into<true>(other.max_element_count, new_memory, other_locator);
                    this->memory = std::move(new_memory);
                }
                else
                {
                    this->destruct();
                    other.template insert_into<true>(other.max_element_count, this->memory, other_locator);
                }
                this->max_element_count = other.max_element_count;
                this->fixed_sizes = other.fixed_sizes;
                this->locator = other_locator;
            }
        }
    }

    auto copy_construct_locator(const BasicContiguousVector& other)
    {
        auto other_locator = other.locator;
        const_cast<BasicContiguousVector&>(other).template insert_into<false>(other.max_element_count, this->memory,
                                                                              other_locator);
        return other_locator;
    }

    void copy_assign(const BasicContiguousVector& other)
    {
        this->destruct();
        this->memory = other.memory;
        auto other_locator = other.locator;
        const_cast<BasicContiguousVector&>(other).template insert_into<false>(other.max_element_count, this->memory,
                                                                              other_locator);
        this->max_element_count = other.max_element_count;
        this->fixed_sizes = other.fixed_sizes;
        this->locator = other_locator;
    }

    template <class TAllocator>
    constexpr auto equal(const cntgs::BasicContiguousVector<TAllocator, Types...>& other) const
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
    template <class TAllocator>
    constexpr auto lexicographical_compare(const cntgs::BasicContiguousVector<TAllocator, Types...>& other) const
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

    void destruct_if_owned() noexcept
    {
        if (this->memory && this->memory.is_owned())
        {
            this->destruct();
        }
    }

    void destruct() noexcept { this->destruct(this->begin(), this->end()); }

    void destruct([[maybe_unused]] iterator first, [[maybe_unused]] iterator last) noexcept
    {
        if constexpr (!ListTraits::IS_TRIVIALLY_DESTRUCTIBLE)
        {
            std::for_each(first, last, ElementTraits::destruct);
        }
    }
};

template <class Allocator, class... T>
auto type_erase(cntgs::BasicContiguousVector<Allocator, T...>&& vector) noexcept
{
    return cntgs::TypeErasedVector{
        vector.memory.size(),
        vector.max_element_count,
        vector.memory.release(),
        vector.memory.is_owned(),
        detail::type_erase_allocator(vector.get_allocator()),
        detail::convert_array_to_size<detail::MAX_FIXED_SIZE_VECTOR_PARAMETER>(vector.fixed_sizes),
        detail::type_erase_element_locator(vector.locator),
        []([[maybe_unused]] cntgs::TypeErasedVector& erased)
        {
            cntgs::BasicContiguousVector<Allocator, T...>{std::move(erased)};
        }};
}
}  // namespace cntgs

#endif // CNTGS_CONTIGUOUS_VECTOR_HPP


#endif  // CNTGS_CNTGS_CONTIGUOUS_HPP
