#pragma once

#include "cntgs/contiguous/detail/elementLocator.h"
#include "cntgs/contiguous/detail/forward.h"
#include "cntgs/contiguous/detail/tuple.h"
#include "cntgs/contiguous/detail/tupleQualifier.h"
#include "cntgs/contiguous/detail/utility.h"
#include "cntgs/contiguous/detail/vector.h"
#include "cntgs/contiguous/detail/vectorTraits.h"

#include <cstddef>
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
    using Locator = detail::BaseElementLocatorT<Types...>;

    static constexpr auto TYPE_COUNT = sizeof...(Types);
    static constexpr auto INVERSE_FIXED_SIZE_INDICES = detail::calculate_inverse_fixed_size_indices(
        detail::TypeList<Types...>{}, std::make_index_sequence<Self::TYPE_COUNT>{});
    static constexpr auto CONTIGUOUS_FIXED_SIZE_COUNT = Traits::CONTIGUOUS_FIXED_SIZE_COUNT;

  public:
    StorageType memory;
    Tuple tuple;

    ContiguousElement() = default;

    template <detail::ContiguousTupleQualifier Qualifier>
    constexpr ContiguousElement(const cntgs::ContiguousTuple<Qualifier, Types...>& other)
        : memory(detail::make_unique_for_overwrite<StorageElementType[]>(other.size_in_bytes())),
          tuple(this->store_and_load(other.tuple))
    {
    }

    template <detail::ContiguousTupleQualifier Qualifier>
    constexpr ContiguousElement(cntgs::ContiguousTuple<Qualifier, Types...>&& other)
        : memory(detail::make_unique_for_overwrite<StorageElementType[]>(other.size_in_bytes())),
          tuple(this->store_and_load(std::move(other.tuple)))
    {
    }

    ContiguousElement(const ContiguousElement&) = delete;
    ContiguousElement(ContiguousElement&&) = default;

    ContiguousElement& operator=(const ContiguousElement&) = delete;
    ContiguousElement& operator=(ContiguousElement&&) = default;

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

    ~ContiguousElement() noexcept(Traits::IS_NOTHROW_DESTRUCTIBLE)
    {
        if constexpr (!Traits::IS_TRIVIALLY_DESTRUCTIBLE)
        {
            if (this->memory)
            {
                this->destruct(std::make_index_sequence<Self::TYPE_COUNT>{});
            }
        }
    }

  private:
    template <class Tuple>
    constexpr auto store_and_load(Tuple&& tuple)
    {
        return this->store_and_load(std::forward<Tuple>(tuple), this->memory_begin(),
                                    std::make_index_sequence<Self::TYPE_COUNT>{});
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
    static auto store_and_load(Tuple&& tuple, std::byte* address, std::index_sequence<I...>)
    {
        typename Traits::PointerReturnType tuple_of_pointer;
        ((address = Self::template store_and_load_one<detail::IgnoreFirstAlignmentSelector::template VALUE<I>, Types>(
              std::get<I>(tuple_of_pointer), address, detail::extract<I>(tuple))),
         ...);
        return detail::convert_tuple_to<Tuple>(tuple_of_pointer);
    }

    template <bool NeedsAlignment, class Type, class Result, class Arg>
    static auto store_and_load_one(Result& result, std::byte* address, Arg&& arg)
    {
        using ParameterTraits = detail::ParameterTraits<Type>;
        const auto fixed_size = Self::get_size(arg);
        const auto address_before_store = address;
        address = ParameterTraits::template store<NeedsAlignment>(std::forward<Arg>(arg), address, fixed_size);
        result = std::get<0>(ParameterTraits::template load<NeedsAlignment>(address_before_store, fixed_size));
        return address;
    }

    constexpr auto memory_begin() const noexcept { return reinterpret_cast<std::byte*>(this->memory.get()); }

    template <std::size_t... I>
    void destruct(std::index_sequence<I...>) noexcept(Traits::IS_NOTHROW_DESTRUCTIBLE)
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