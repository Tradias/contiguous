#pragma once

#include "cntgs/contiguous/detail/attributes.h"
#include "cntgs/contiguous/detail/elementLocator.h"
#include "cntgs/contiguous/detail/forward.h"
#include "cntgs/contiguous/detail/tuple.h"
#include "cntgs/contiguous/detail/tupleQualifier.h"
#include "cntgs/contiguous/detail/utility.h"
#include "cntgs/contiguous/detail/vector.h"
#include "cntgs/contiguous/detail/vectorTraits.h"

#include <cstddef>
#include <memory>
#include <tuple>

namespace cntgs
{
template <class... Types>
class ContiguousElement
{
  private:
    using Self = cntgs::ContiguousElement<Types...>;
    using Traits = detail::ContiguousVectorTraits<Types...>;
    using StorageType = std::unique_ptr<detail::AlignedByteT<Traits::MAX_ALIGNMENT>[]>;
    using StorageElementType = typename StorageType::element_type;
    using Tuple = typename Traits::ReferenceReturnType;
    using UnderlyingTuple = typename Tuple::Tuple;
    using Locator = detail::BaseElementLocatorT<Types...>;

    static constexpr auto TYPE_COUNT = sizeof...(Types);

  public:
    StorageType memory;
    Tuple tuple;

    ContiguousElement() = default;

    template <detail::ContiguousTupleQualifier Qualifier>
    /*implicit*/ ContiguousElement(const cntgs::ContiguousTuple<Qualifier, Types...>& other)
        : memory(detail::make_unique_for_overwrite<StorageElementType[]>(other.size_in_bytes())),
          tuple(this->store_and_load(other.tuple))
    {
    }

    template <detail::ContiguousTupleQualifier Qualifier>
    /*implicit*/ ContiguousElement(cntgs::ContiguousTuple<Qualifier, Types...>&& other)
        : memory(detail::make_unique_for_overwrite<StorageElementType[]>(other.size_in_bytes())),
          tuple(this->store_and_load(other.tuple))
    {
    }

    /*implicit*/ ContiguousElement(const ContiguousElement& other)
        : memory(detail::make_unique_for_overwrite<StorageElementType[]>(other.tuple.size_in_bytes())),
          tuple(this->store_and_load(other.tuple.tuple))
    {
    }

    ContiguousElement(ContiguousElement&&) = default;

    ContiguousElement& operator=(const ContiguousElement& other)
    {
        if (this != std::addressof(other))
        {
            this->destruct();
            const auto current_memory_size = this->tuple.size_in_bytes();
            const auto other_memory_size = other.tuple.size_in_bytes();
            if (current_memory_size < other_memory_size)
            {
                this->memory = detail::make_unique_for_overwrite<StorageElementType[]>(other_memory_size);
            }
            detail::construct_at(&this->tuple, this->store_and_load(other.tuple.tuple));
        }
        return *this;
    }

    ContiguousElement& operator=(ContiguousElement&& other) noexcept
    {
        if (this != std::addressof(other))
        {
            this->memory = std::move(other.memory);
            this->tuple.tuple.~UnderlyingTuple();
            detail::construct_at(&this->tuple.tuple, std::move(other.tuple.tuple));
        }
        return *this;
    }

    template <detail::ContiguousTupleQualifier Qualifier>
    constexpr ContiguousElement& operator=(const cntgs::ContiguousTuple<Qualifier, Types...>& other)
    {
        this->tuple = other;
        return *this;
    }

    template <detail::ContiguousTupleQualifier Qualifier>
    constexpr ContiguousElement& operator=(cntgs::ContiguousTuple<Qualifier, Types...>&& other)
    {
        this->tuple = std::move(other);
        return *this;
    }

    ~ContiguousElement() noexcept
    {
        if constexpr (!Traits::IS_TRIVIALLY_DESTRUCTIBLE)
        {
            if (this->memory)
            {
                this->destruct();
            }
        }
    }

  private:
    template <class Tuple>
    constexpr auto store_and_load(Tuple& source)
    {
        return this->store_and_load(source, this->memory_begin(), std::make_index_sequence<Self::TYPE_COUNT>{});
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

    template <class Tuple, std::size_t... I>
    static auto store_and_load(Tuple& source, std::byte* CNTGS_RESTRICT address, std::index_sequence<I...>)
    {
        typename Traits::PointerReturnType tuple_of_pointer;
        ((address = Self::template store_and_load_one<detail::IgnoreFirstAlignmentSelector::template VALUE<I>, Types>(
              std::get<I>(tuple_of_pointer), address, detail::extract<I>(source))),
         ...);
        return detail::convert_tuple_to<Tuple>(tuple_of_pointer);
    }

    template <bool NeedsAlignment, class Type, class Result, class Arg>
    CNTGS_RESTRICT_RETURN static std::byte* store_and_load_one(Result& CNTGS_RESTRICT result,
                                                               std::byte* CNTGS_RESTRICT address, Arg&& arg)
    {
        using ParameterTraits = detail::ParameterTraits<Type>;
        const auto fixed_size = Self::get_size(arg);
        const auto address_before_store = address;
        address = ParameterTraits::template store<NeedsAlignment>(std::forward<Arg>(arg), address, fixed_size);
        result = std::get<0>(ParameterTraits::template load<NeedsAlignment>(address_before_store, fixed_size));
        return address;
    }

    [[nodiscard]] auto memory_begin() const noexcept { return reinterpret_cast<std::byte*>(this->memory.get()); }

    void destruct() noexcept { this->destruct(std::make_index_sequence<Self::TYPE_COUNT>{}); }

    template <std::size_t... I>
    void destruct(std::index_sequence<I...>) noexcept
    {
        (detail::ParameterTraits<Types>::destroy(cntgs::get<I>(this->tuple)), ...);
    }
};

template <class... T>
constexpr void swap(cntgs::ContiguousElement<T...>& lhs, cntgs::ContiguousElement<T...>& rhs)
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
