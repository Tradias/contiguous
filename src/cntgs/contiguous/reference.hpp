#ifndef CNTGS_CONTIGUOUS_REFERENCE_HPP
#define CNTGS_CONTIGUOUS_REFERENCE_HPP

#include "cntgs/contiguous/detail/attributes.hpp"
#include "cntgs/contiguous/detail/elementTraits.hpp"
#include "cntgs/contiguous/detail/forward.hpp"
#include "cntgs/contiguous/detail/parameterListTraits.hpp"
#include "cntgs/contiguous/detail/tuple.hpp"
#include "cntgs/contiguous/detail/typeUtils.hpp"

#include <cstddef>
#include <cstring>
#include <tuple>

namespace cntgs
{
template <class... Types>
using ContiguousReference = cntgs::BasicContiguousReference<false, Types...>;

template <class... Types>
using ContiguousConstReference = cntgs::BasicContiguousReference<true, Types...>;

/// Reference type
template <bool IsConst, class... Types>
class BasicContiguousReference
{
  private:
    using ListTraits = detail::ParameterListTraits<Types...>;
    using ElementTraits = detail::ElementTraitsT<Types...>;
    using PointerTuple = detail::ToTupleOfContiguousPointer<std::tuple<Types...>>;

  public:
    using Tuple = detail::ConditionalT<IsConst, detail::ToTupleOfContiguousConstReference<std::tuple<Types...>>,
                                       detail::ToTupleOfContiguousReference<std::tuple<Types...>>>;

    static constexpr auto IS_CONST = IsConst;

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

    template <bool OtherIsConst>
    /*implicit*/ constexpr BasicContiguousReference(
        const cntgs::BasicContiguousReference<OtherIsConst, Types...>& other) noexcept
        : tuple(other.tuple)
    {
    }

    template <class Allocator>
    /*implicit*/ constexpr BasicContiguousReference(
        const cntgs::BasicContiguousElement<Allocator, Types...>& other) noexcept
        : BasicContiguousReference(other.reference)
    {
    }

    template <class Allocator>
    /*implicit*/ constexpr BasicContiguousReference(cntgs::BasicContiguousElement<Allocator, Types...>& other) noexcept
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
    constexpr BasicContiguousReference& operator=(const cntgs::BasicContiguousReference<OtherIsConst, Types...>&
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

    template <bool OtherIsConst>
    constexpr BasicContiguousReference& operator=(cntgs::BasicContiguousReference<OtherIsConst, Types...>&&
                                                      other) noexcept(OtherIsConst
                                                                          ? ListTraits::IS_NOTHROW_COPY_ASSIGNABLE
                                                                          : ListTraits::IS_NOTHROW_MOVE_ASSIGNABLE)
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

    template <bool OtherIsConst>
    [[nodiscard]] constexpr auto operator==(const cntgs::BasicContiguousReference<OtherIsConst, Types...>& other) const
    {
        return ElementTraits::equal(*this, other);
    }

    template <class Allocator>
    [[nodiscard]] constexpr auto operator==(const cntgs::BasicContiguousElement<Allocator, Types...>& other) const
    {
        return *this == other.reference;
    }

    template <bool OtherIsConst>
    [[nodiscard]] constexpr auto operator!=(const cntgs::BasicContiguousReference<OtherIsConst, Types...>& other) const
    {
        return !(*this == other);
    }

    template <class Allocator>
    [[nodiscard]] constexpr auto operator!=(const cntgs::BasicContiguousElement<Allocator, Types...>& other) const
    {
        return !(*this == other.reference);
    }

    template <bool OtherIsConst>
    [[nodiscard]] constexpr auto operator<(const cntgs::BasicContiguousReference<OtherIsConst, Types...>& other) const
    {
        return ElementTraits::lexicographical_compare(*this, other);
    }

    template <class Allocator>
    [[nodiscard]] constexpr auto operator<(const cntgs::BasicContiguousElement<Allocator, Types...>& other) const
    {
        return *this < other.reference;
    }

    template <bool OtherIsConst>
    [[nodiscard]] constexpr auto operator<=(const cntgs::BasicContiguousReference<OtherIsConst, Types...>& other) const
    {
        return !(other < *this);
    }

    template <class Allocator>
    [[nodiscard]] constexpr auto operator<=(const cntgs::BasicContiguousElement<Allocator, Types...>& other) const
    {
        return !(other.reference < *this);
    }

    template <bool OtherIsConst>
    [[nodiscard]] constexpr auto operator>(const cntgs::BasicContiguousReference<OtherIsConst, Types...>& other) const
    {
        return other < *this;
    }

    template <class Allocator>
    [[nodiscard]] constexpr auto operator>(const cntgs::BasicContiguousElement<Allocator, Types...>& other) const
    {
        return other.reference < *this;
    }

    template <bool OtherIsConst>
    [[nodiscard]] constexpr auto operator>=(const cntgs::BasicContiguousReference<OtherIsConst, Types...>& other) const
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

template <bool IsConst, class... T>
constexpr void swap(const cntgs::BasicContiguousReference<IsConst, T...>& lhs,
                    const cntgs::BasicContiguousReference<IsConst, T...>&
                        rhs) noexcept(detail::ParameterListTraits<T...>::IS_NOTHROW_SWAPPABLE)
{
    lhs.swap(rhs);
}

template <std::size_t I, bool IsConst, class... Types>
[[nodiscard]] constexpr decltype(auto) get(cntgs::BasicContiguousReference<IsConst, Types...>& reference) noexcept
{
    return std::get<I>(reference.tuple);
}

template <std::size_t I, bool IsConst, class... Types>
[[nodiscard]] constexpr decltype(auto) get(const cntgs::BasicContiguousReference<IsConst, Types...>& reference) noexcept
{
    return std::get<I>(reference.tuple);
}

template <std::size_t I, bool IsConst, class... Types>
[[nodiscard]] constexpr decltype(auto) get(cntgs::BasicContiguousReference<IsConst, Types...>&& reference) noexcept
{
    return std::get<I>(std::move(reference.tuple));
}

template <std::size_t I, bool IsConst, class... Types>
[[nodiscard]] constexpr decltype(auto) get(
    const cntgs::BasicContiguousReference<IsConst, Types...>&& reference) noexcept
{
    return std::get<I>(std::move(reference.tuple));
}
}  // namespace cntgs

namespace std
{
template <std::size_t I, bool IsConst, class... Types>
struct tuple_element<I, ::cntgs::BasicContiguousReference<IsConst, Types...>>
    : std::tuple_element<I, typename ::cntgs::BasicContiguousReference<IsConst, Types...>::Tuple>
{
};

template <bool IsConst, class... Types>
struct tuple_size<::cntgs::BasicContiguousReference<IsConst, Types...>>
    : std::integral_constant<std::size_t, sizeof...(Types)>
{
};
}  // namespace std

#endif  // CNTGS_CONTIGUOUS_REFERENCE_HPP
