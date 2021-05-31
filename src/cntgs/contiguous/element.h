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
    using Traits = detail::ContiguousVectorTraits<Types...>;
    using StorageType = std::unique_ptr<detail::AlignedByte<Traits::MAX_ALIGNMENT>[]>;
    using StorageElementType = typename StorageType::element_type;
    using Tuple = typename Traits::ReferenceReturnType;
    using Locator = detail::BaseElementLocatorT<Types...>;

    static constexpr auto TYPE_COUNT = sizeof...(Types);
    static constexpr auto INVERSE_FIXED_SIZE_INDICES = detail::calculate_inverse_fixed_size_indices(
        detail::TypeList<Types...>{}, std::make_index_sequence<TYPE_COUNT>{});
    static constexpr auto CONTIGUOUS_FIXED_SIZE_COUNT = Traits::CONTIGUOUS_FIXED_SIZE_COUNT;

  public:
    StorageType memory;
    Tuple tuple;

    ContiguousElement() = default;

    template <detail::ContiguousTupleQualifier Qualifier>
    constexpr ContiguousElement(const cntgs::ContiguousTuple<Qualifier, Types...>& other)
        : memory(detail::make_unique_for_overwrite<StorageElementType[]>(other.size_in_bytes())),
          tuple(this->create_tuple(other.tuple))
    {
    }

    template <detail::ContiguousTupleQualifier Qualifier>
    constexpr ContiguousElement(cntgs::ContiguousTuple<Qualifier, Types...>&& other)
        : memory(detail::make_unique_for_overwrite<StorageElementType[]>(other.size_in_bytes())),
          tuple(this->create_tuple(std::move(other.tuple)))
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

    ~ContiguousElement()
    {
        if constexpr (!Traits::IS_TRIVIALLY_DESTRUCTIBLE)
        {
            if (memory)
            {
                destruct(std::make_index_sequence<sizeof...(Types)>{});
            }
        }
    }

    template <class Tuple>
    constexpr auto create_tuple(Tuple&& tuple) noexcept
    {
        const auto fixed_sizes = create_fixed_sizes(tuple, std::make_index_sequence<CONTIGUOUS_FIXED_SIZE_COUNT>{});
        this->emplace_back(fixed_sizes, std::forward<Tuple>(tuple), std::make_index_sequence<TYPE_COUNT>{});
        auto tuple_of_pointer =
            Locator::template load_element_at<detail::IgnoreFirstAlignmentSelector>(this->memory_begin(), fixed_sizes);
        return detail::convert_tuple_to<Tuple>(tuple_of_pointer);
    }

    template <std::size_t... I, class... T>
    static constexpr auto create_fixed_sizes(const std::tuple<T...>& tuple, std::index_sequence<I...>) noexcept
    {
        return std::array<std::size_t, CONTIGUOUS_FIXED_SIZE_COUNT>{
            std::get<std::get<I>(INVERSE_FIXED_SIZE_INDICES)>(tuple).size()...};
    }

    template <std::size_t N, class Tuple, std::size_t... I>
    constexpr auto emplace_back(const std::array<std::size_t, N>& fixed_sizes, Tuple&& tuple,
                                std::index_sequence<I...>) noexcept
    {
        Locator::template emplace_back<detail::IgnoreFirstAlignmentSelector>(this->memory_begin(), fixed_sizes,
                                                                             extract<I>(tuple)...);
    }

    template <std::size_t I, class Tuple>
    static constexpr decltype(auto) extract(const Tuple& tuple) noexcept
    {
        return std::get<I>(tuple);
    }

    template <std::size_t I, class Tuple>
    static constexpr decltype(auto) extract(Tuple& tuple) noexcept
    {
        return std::move(std::get<I>(tuple));
    }

    constexpr auto memory_begin() const noexcept { return reinterpret_cast<std::byte*>(memory.get()); }

    template <std::size_t... I>
    void destruct(std::index_sequence<I...>)
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