#pragma once

#include "cntgs/contiguous/detail/attributes.h"
#include "cntgs/contiguous/detail/elementLocator.h"
#include "cntgs/contiguous/detail/forward.h"
#include "cntgs/contiguous/detail/parameterListTraits.h"
#include "cntgs/contiguous/detail/tuple.h"
#include "cntgs/contiguous/detail/tupleQualifier.h"
#include "cntgs/contiguous/detail/utility.h"
#include "cntgs/contiguous/detail/vectorTraits.h"
#include "cntgs/contiguous/tuple.h"

#include <cstddef>
#include <cstring>
#include <memory>
#include <tuple>

namespace cntgs
{
template <class... Types>
class ContiguousElement
{
  private:
    using Self = cntgs::ContiguousElement<Types...>;
    using ListTraits = detail::ParameterListTraits<Types...>;
    using VectorTraits = detail::ContiguousVectorTraits<Types...>;
    using StorageType = std::unique_ptr<detail::AlignedByteT<ListTraits::MAX_ALIGNMENT>[]>;
    using StorageElementType = typename StorageType::element_type;
    using Tuple = typename VectorTraits::ReferenceReturnType;
    using UnderlyingTuple = typename Tuple::Tuple;
    using Locator = detail::BaseElementLocatorT<Types...>;

  public:
    StorageType memory;
    Tuple tuple;

    ContiguousElement() = default;

    template <detail::ContiguousTupleQualifier Qualifier>
    /*implicit*/ ContiguousElement(const cntgs::ContiguousTuple<Qualifier, Types...>& other)
        : memory(detail::make_unique_for_overwrite<StorageElementType[]>(other.size_in_bytes())),
          tuple(this->store_and_load(other, other.size_in_bytes()))
    {
    }

    template <detail::ContiguousTupleQualifier Qualifier>
    /*implicit*/ ContiguousElement(cntgs::ContiguousTuple<Qualifier, Types...>&& other)
        : memory(detail::make_unique_for_overwrite<StorageElementType[]>(other.size_in_bytes())),
          tuple(this->store_and_load(other, other.size_in_bytes()))
    {
    }

    /*implicit*/ ContiguousElement(const ContiguousElement& other)
        : memory(detail::make_unique_for_overwrite<StorageElementType[]>(other.tuple.size_in_bytes())),
          tuple(this->store_and_load(other.tuple, other.tuple.size_in_bytes()))
    {
    }

    ContiguousElement(ContiguousElement&&) = default;

    ContiguousElement& operator=(const ContiguousElement& other) noexcept(ListTraits::IS_NOTHROW_COPY_ASSIGNABLE)
    {
        this->tuple = other.tuple;
        return *this;
    }

    ContiguousElement& operator=(ContiguousElement&& other) noexcept(ListTraits::IS_NOTHROW_MOVE_ASSIGNABLE)
    {
        this->tuple = std::move(other.tuple);
        return *this;
    }

    template <detail::ContiguousTupleQualifier Qualifier>
    constexpr ContiguousElement& operator=(const cntgs::ContiguousTuple<Qualifier, Types...>& other) noexcept(
        ListTraits::IS_NOTHROW_COPY_ASSIGNABLE)
    {
        this->tuple = other;
        return *this;
    }

    template <detail::ContiguousTupleQualifier Qualifier>
    constexpr ContiguousElement& operator=(cntgs::ContiguousTuple<Qualifier, Types...>&& other) noexcept(
        ContiguousTuple<Qualifier, Types...>::IS_CONST ? ListTraits::IS_NOTHROW_MOVE_ASSIGNABLE
                                                       : ListTraits::IS_NOTHROW_COPY_ASSIGNABLE)
    {
        this->tuple = std::move(other);
        return *this;
    }

    ~ContiguousElement() noexcept
    {
        if constexpr (!ListTraits::IS_TRIVIALLY_DESTRUCTIBLE)
        {
            if (this->memory)
            {
                this->destruct();
            }
        }
    }

  private:
    template <class Tuple>
    auto store_and_load(Tuple& source, std::size_t memory_size)
    {
        static constexpr auto USE_MOVE = !std::is_const_v<Tuple> && !Tuple::IS_CONST;
        std::memcpy(this->memory_begin(), source.start_address(), memory_size);
        auto target = this->load(this->memory_begin(), source.tuple, ListTraits::make_index_sequence());
        Locator::template construct_if_non_trivial<USE_MOVE>(source, target);
        return detail::convert_tuple_to<Tuple>(target);
    }

    template <class T>
    static constexpr std::size_t get_size(const cntgs::Span<T>& span) noexcept
    {
        return span.size();
    }

    template <class T>
    static constexpr std::size_t get_size(const T&) noexcept
    {
        return std::size_t{};
    }

    template <std::size_t... I>
    static auto load(std::byte* CNTGS_RESTRICT address, const UnderlyingTuple& tuple,
                     std::index_sequence<I...>) noexcept
    {
        typename VectorTraits::PointerReturnType result;
        ((std::tie(std::get<I>(result), address) =
              detail::ParameterTraits<Types>::template load<detail::IgnoreFirstAlignmentSelector::template VALUE<I>>(
                  address, get_size(std::get<I>(tuple)))),
         ...);
        return result;
    }

    [[nodiscard]] auto memory_begin() const noexcept { return reinterpret_cast<std::byte*>(this->memory.get()); }

    void destruct() noexcept { this->destruct(ListTraits::make_index_sequence()); }

    template <std::size_t... I>
    void destruct(std::index_sequence<I...>) noexcept
    {
        (detail::ParameterTraits<Types>::destroy(cntgs::get<I>(this->tuple)), ...);
    }
};

template <class... T>
constexpr void swap(cntgs::ContiguousElement<T...>& lhs, cntgs::ContiguousElement<T...>& rhs) noexcept
{
    std::swap(lhs.memory, rhs.memory);
    std::swap(lhs.tuple.tuple, rhs.tuple.tuple);
}

template <std::size_t I, class... Types>
[[nodiscard]] constexpr decltype(auto) get(cntgs::ContiguousElement<Types...>& element) noexcept
{
    return cntgs::get<I>(element.tuple);
}

template <std::size_t I, class... Types>
[[nodiscard]] constexpr decltype(auto) get(const cntgs::ContiguousElement<Types...>& element) noexcept
{
    return detail::as_const(cntgs::get<I>(element.tuple));
}

template <std::size_t I, class... Types>
[[nodiscard]] constexpr decltype(auto) get(cntgs::ContiguousElement<Types...>&& element) noexcept
{
    return cntgs::get<I>(std::move(element.tuple));
}

template <std::size_t I, class... Types>
[[nodiscard]] constexpr decltype(auto) get(const cntgs::ContiguousElement<Types...>&& element) noexcept
{
    return cntgs::get<I>(std::move(element.tuple));
}
}  // namespace cntgs

namespace std
{
template <std::size_t I, class... Types>
struct tuple_element<I, ::cntgs::ContiguousElement<Types...>>
    : std::tuple_element<I, typename ::cntgs::ContiguousElement<Types...>::Tuple>
{
};

template <class... Types>
struct tuple_size<::cntgs::ContiguousElement<Types...>> : std::integral_constant<std::size_t, sizeof...(Types)>
{
};
}  // namespace std
