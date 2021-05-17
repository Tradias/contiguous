#pragma once

#include "cntgs/contiguous/detail/traits.h"
#include "cntgs/contiguous/detail/tuple.h"
#include "cntgs/contiguous/detail/vector.h"
#include "cntgs/contiguous/parameter.h"
#include "cntgs/contiguous/span.h"

#include <array>
#include <cstring>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

namespace cntgs
{
template <class... Types>
class ContiguousVector
{
  public:
    using Tuple = std::tuple<Types...>;
    using TupleProperties = detail::ContiguousTuplePropertiesT<Tuple>;
    using value_type = detail::ToContiguousTupleOfValueReturnTypes<Tuple>;
    using reference = detail::ToContiguousTupleOfReferenceReturnTypes<Tuple>;
    using difference_type = std::ptrdiff_t;
    using size_type = std::size_t;

    static constexpr auto TYPE_COUNT = sizeof...(Types);
    static constexpr auto SIZE_IN_MEMORY = TupleProperties::SIZE_IN_MEMORY;
    static constexpr auto CONTIGUOUS_COUNT = TupleProperties::CONTIGUOUS_COUNT;
    static constexpr auto CONTIGUOUS_FIXED_SIZE_COUNT = TupleProperties::CONTIGUOUS_FIXED_SIZE_COUNT;

    size_type memory_size{};
    size_type element_count{};
    std::unique_ptr<std::byte[]> memory{};
    std::byte* last_element{};
    std::byte** last_element_address{};
    std::array<size_type, CONTIGUOUS_FIXED_SIZE_COUNT> fixed_sizes{};

    ContiguousVector() = default;

    ContiguousVector(size_type element_count, size_type varying_size_bytes,
                     std::array<size_type, CONTIGUOUS_FIXED_SIZE_COUNT> fixed_sizes = {})
        : memory_size(SIZE_IN_MEMORY * element_count + varying_size_bytes +
                      detail::calculate_fixed_size_memory_consumption(fixed_sizes, detail::TypeList<Types...>{},
                                                                      std::make_index_sequence<TYPE_COUNT>{}) *
                          element_count +
                      detail::calculate_element_addresses_size(element_count)),
          element_count(element_count),
          memory(detail::make_unique_for_overwrite<std::byte[]>(memory_size)),
          last_element(memory.get() + detail::calculate_element_addresses_size(element_count)),
          last_element_address(reinterpret_cast<std::byte**>(memory.get())),
          fixed_sizes(fixed_sizes)
    {
    }

    auto element_addresses() const noexcept
    {
        const auto start = reinterpret_cast<std::byte**>(this->memory.get());
        return cntgs::Span{start, start + this->size()};
    }

    auto element_memory() const noexcept
    {
        return cntgs::Span{this->memory.get() + this->element_count * sizeof(std::byte*),
                           this->memory.get() + this->memory_size};
    }

    template <class... Args>
    void emplace_back(Args&&... args)
    {
        this->emplace_back_impl(std::make_index_sequence<sizeof...(Args)>{}, std::forward<Args>(args)...);
    }

    reference operator[](size_type i) { return this->subscript_operator(i); }

    template <std::size_t I>
    size_type get_fixed_size()
    {
        return std::get<I>(fixed_sizes);
    }

    bool empty() const noexcept { return last_element_address == reinterpret_cast<std::byte**>(this->memory.get()); }

    size_type size() const noexcept { return last_element_address - reinterpret_cast<std::byte**>(this->memory.get()); }

    constexpr size_type capacity() const noexcept { return element_count; }

    constexpr size_type memory_consumption() const noexcept { return memory_size; }

    constexpr bool operator==(const ContiguousVector& other) const noexcept { return false; }

    constexpr bool operator!=(const ContiguousVector& other) const noexcept { return !(*this == other); }

    // private API
  private:
    template <std::size_t... I, class... Args>
    auto emplace_back_impl(std::index_sequence<I...>, Args&&... args)
    {
        *last_element_address = last_element;
        ++last_element_address;
        ((last_element = detail::ContiguousTraits<std::tuple_element_t<I, Tuple>>::store_contiguously(
              std::forward<Args>(args), last_element,
              detail::FixedSizeGetter<std::tuple_element_t<I, Tuple>, Types...>::get<I>(fixed_sizes))),
         ...);
    }

    template <class... T, std::size_t... I>
    constexpr auto to_reference(const std::tuple<T...>& tuple_of_pointer, std::index_sequence<I...>) noexcept
    {
        return reference{detail::dereference(std::get<I>(tuple_of_pointer))...};
    }

    template <class... T>
    constexpr auto to_reference(const std::tuple<T...>& tuple_of_pointer) noexcept
    {
        return this->to_reference(tuple_of_pointer, std::make_index_sequence<sizeof...(T)>{});
    }

    template <class... T, class Function, std::size_t... I>
    constexpr auto for_each_impl(std::tuple<T...>& tuple, Function&& function, std::index_sequence<I...>)
    {
        return (function(std::get<I>(tuple),
                         detail::FixedSizeGetter<std::tuple_element_t<I, Tuple>, Types...>::get<I>(fixed_sizes),
                         detail::ContiguousTraits<std::tuple_element_t<I, Tuple>>{}),
                ...);
    }

    template <class T, class Function>
    constexpr auto for_each(T&& tuple, Function&& function)
    {
        return this->for_each_impl(std::forward<T>(tuple), std::forward<Function>(function),
                                   std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<T>>>{});
    }

    reference subscript_operator(size_type i)
    {
        detail::ToContiguousTupleOfPointerReturnTypes<Tuple> temp;
        auto* start = this->element_addresses()[i];
        this->for_each(temp, [&](auto& element, size_type size, auto traits) {
            std::tie(element, start) = decltype(traits)::from_address(start, size);
        });
        return this->to_reference(temp);
    }
};
}  // namespace cntgs