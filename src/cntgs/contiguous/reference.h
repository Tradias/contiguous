#ifndef CNTGS_CONTIGUOUS_REFERENCE_H
#define CNTGS_CONTIGUOUS_REFERENCE_H

#include "cntgs/contiguous/detail/attributes.h"
#include "cntgs/contiguous/detail/elementTraits.h"
#include "cntgs/contiguous/detail/forward.h"
#include "cntgs/contiguous/detail/parameterListTraits.h"
#include "cntgs/contiguous/detail/tuple.h"
#include "cntgs/contiguous/detail/tupleQualifier.h"
#include "cntgs/contiguous/detail/typeUtils.h"

#include <cstddef>
#include <cstring>
#include <tuple>

namespace cntgs
{
template <detail::ContiguousReferenceQualifier Qualifier, class... Types>
class ContiguousReference
{
  private:
    using ListTraits = detail::ParameterListTraits<Types...>;
    using ElementTraits = detail::ElementTraitsT<Types...>;
    using PointerTuple = detail::ToTupleOfContiguousPointer<std::tuple<Types...>>;

  public:
    using Tuple = detail::ConditionalT<(detail::ContiguousReferenceQualifier::MUTABLE == Qualifier),
                                       detail::ToTupleOfContiguousReference<std::tuple<Types...>>,
                                       detail::ToTupleOfContiguousConstReference<std::tuple<Types...>>>;

    static constexpr auto IS_CONST = detail::ContiguousReferenceQualifier::CONST == Qualifier;

    Tuple tuple;

    ContiguousReference() = default;
    ~ContiguousReference() = default;

    ContiguousReference(const ContiguousReference&) = default;
    ContiguousReference(ContiguousReference&&) = default;

    explicit constexpr ContiguousReference(std::byte* CNTGS_RESTRICT address,
                                           const typename ListTraits::FixedSizes& fixed_sizes = {}) noexcept
        : ContiguousReference(ElementTraits::load_element_at(address, fixed_sizes))
    {
    }

    explicit constexpr ContiguousReference(const PointerTuple& tuple) noexcept
        : tuple(detail::convert_tuple_to<Tuple>(tuple))
    {
    }

    template <detail::ContiguousReferenceQualifier TQualifier>
    /*implicit*/ constexpr ContiguousReference(const cntgs::ContiguousReference<TQualifier, Types...>& other) noexcept
        : tuple(other.tuple)
    {
    }

    template <class Allocator>
    /*implicit*/ constexpr ContiguousReference(const cntgs::BasicContiguousElement<Allocator, Types...>& other) noexcept
        : tuple(other.reference)
    {
    }

    template <detail::ContiguousReferenceQualifier TQualifier>
    /*implicit*/ constexpr ContiguousReference(cntgs::ContiguousReference<TQualifier, Types...>&& other) noexcept
        : tuple(std::move(other.tuple))
    {
    }

    template <class Allocator>
    /*implicit*/ constexpr ContiguousReference(cntgs::BasicContiguousElement<Allocator, Types...>&& other) noexcept
        : tuple(std::move(other.reference))
    {
    }

    constexpr ContiguousReference& operator=(const ContiguousReference& other) noexcept(
        ListTraits::IS_NOTHROW_COPY_ASSIGNABLE)
    {
        this->assign(other);
        return *this;
    }

    template <detail::ContiguousReferenceQualifier TQualifier>
    constexpr ContiguousReference& operator=(const cntgs::ContiguousReference<TQualifier, Types...>& other) noexcept(
        ListTraits::IS_NOTHROW_COPY_ASSIGNABLE)
    {
        this->assign(other);
        return *this;
    }

    template <class Allocator>
    constexpr ContiguousReference& operator=(const cntgs::BasicContiguousElement<Allocator, Types...>& other) noexcept(
        ListTraits::IS_NOTHROW_COPY_ASSIGNABLE)
    {
        this->assign(other.reference);
        return *this;
    }

    constexpr ContiguousReference& operator=(ContiguousReference&& other) noexcept(
        ListTraits::IS_NOTHROW_MOVE_ASSIGNABLE)
    {
        this->assign(other);
        return *this;
    }

    template <detail::ContiguousReferenceQualifier TQualifier>
    constexpr ContiguousReference& operator=(cntgs::ContiguousReference<TQualifier, Types...>&& other) noexcept(
        ContiguousReference<TQualifier, Types...>::IS_CONST ? ListTraits::IS_NOTHROW_MOVE_ASSIGNABLE
                                                            : ListTraits::IS_NOTHROW_COPY_ASSIGNABLE)
    {
        this->assign(other);
        return *this;
    }

    template <class Allocator>
    constexpr ContiguousReference& operator=(cntgs::BasicContiguousElement<Allocator, Types...>&& other) noexcept(
        ListTraits::IS_NOTHROW_MOVE_ASSIGNABLE)
    {
        this->assign(other.reference);
        return *this;
    }

    constexpr void swap(const ContiguousReference& other) const noexcept(ListTraits::IS_NOTHROW_SWAPPABLE)
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

    template <detail::ContiguousReferenceQualifier TQualifier>
    constexpr auto operator==(const cntgs::ContiguousReference<TQualifier, Types...>& other) const
    {
        return ElementTraits::equal(*this, other);
    }

    template <class Allocator>
    constexpr auto operator==(const cntgs::BasicContiguousElement<Allocator, Types...>& other) const
    {
        return *this == other.reference;
    }

    template <detail::ContiguousReferenceQualifier TQualifier>
    constexpr auto operator!=(const cntgs::ContiguousReference<TQualifier, Types...>& other) const
    {
        return !(*this == other);
    }

    template <class Allocator>
    constexpr auto operator!=(const cntgs::BasicContiguousElement<Allocator, Types...>& other) const
    {
        return !(*this == other.reference);
    }

    template <detail::ContiguousReferenceQualifier TQualifier>
    constexpr auto operator<(const cntgs::ContiguousReference<TQualifier, Types...>& other) const
    {
        return ElementTraits::lexicographical_compare(*this, other);
    }

    template <class Allocator>
    constexpr auto operator<(const cntgs::BasicContiguousElement<Allocator, Types...>& other) const
    {
        return *this < other.reference;
    }

    template <detail::ContiguousReferenceQualifier TQualifier>
    constexpr auto operator<=(const cntgs::ContiguousReference<TQualifier, Types...>& other) const
    {
        return !(other < *this);
    }

    template <class Allocator>
    constexpr auto operator<=(const cntgs::BasicContiguousElement<Allocator, Types...>& other) const
    {
        return !(other.reference < *this);
    }

    template <detail::ContiguousReferenceQualifier TQualifier>
    constexpr auto operator>(const cntgs::ContiguousReference<TQualifier, Types...>& other) const
    {
        return other < *this;
    }

    template <class Allocator>
    constexpr auto operator>(const cntgs::BasicContiguousElement<Allocator, Types...>& other) const
    {
        return other.reference < *this;
    }

    template <detail::ContiguousReferenceQualifier TQualifier>
    constexpr auto operator>=(const cntgs::ContiguousReference<TQualifier, Types...>& other) const
    {
        return !(*this < other);
    }

    template <class Allocator>
    constexpr auto operator>=(const cntgs::BasicContiguousElement<Allocator, Types...>& other) const
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

template <detail::ContiguousReferenceQualifier Qualifier, class... T>
constexpr void swap(const cntgs::ContiguousReference<Qualifier, T...>& lhs,
                    const cntgs::ContiguousReference<Qualifier, T...>&
                        rhs) noexcept(detail::ParameterListTraits<T...>::IS_NOTHROW_SWAPPABLE)
{
    lhs.swap(rhs);
}

template <std::size_t I, detail::ContiguousReferenceQualifier Qualifier, class... Types>
[[nodiscard]] constexpr decltype(auto) get(cntgs::ContiguousReference<Qualifier, Types...>& reference) noexcept
{
    return std::get<I>(reference.tuple);
}

template <std::size_t I, detail::ContiguousReferenceQualifier Qualifier, class... Types>
[[nodiscard]] constexpr decltype(auto) get(const cntgs::ContiguousReference<Qualifier, Types...>& reference) noexcept
{
    return std::get<I>(reference.tuple);
}

template <std::size_t I, detail::ContiguousReferenceQualifier Qualifier, class... Types>
[[nodiscard]] constexpr decltype(auto) get(cntgs::ContiguousReference<Qualifier, Types...>&& reference) noexcept
{
    return std::get<I>(std::move(reference.tuple));
}

template <std::size_t I, detail::ContiguousReferenceQualifier Qualifier, class... Types>
[[nodiscard]] constexpr decltype(auto) get(const cntgs::ContiguousReference<Qualifier, Types...>&& reference) noexcept
{
    return std::get<I>(std::move(reference.tuple));
}
}  // namespace cntgs

namespace std
{
template <std::size_t I, ::cntgs::detail::ContiguousReferenceQualifier Qualifier, class... Types>
struct tuple_element<I, ::cntgs::ContiguousReference<Qualifier, Types...>>
    : std::tuple_element<I, typename ::cntgs::ContiguousReference<Qualifier, Types...>::Tuple>
{
};

template <::cntgs::detail::ContiguousReferenceQualifier Qualifier, class... Types>
struct tuple_size<::cntgs::ContiguousReference<Qualifier, Types...>>
    : std::integral_constant<std::size_t, sizeof...(Types)>
{
};
}  // namespace std

#endif  // CNTGS_CONTIGUOUS_REFERENCE_H
