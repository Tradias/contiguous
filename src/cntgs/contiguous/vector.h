#pragma once

#include "cntgs/contiguous/detail/parameterTraits.h"
#include "cntgs/contiguous/detail/vector.h"
#include "cntgs/contiguous/detail/vectorTraits.h"
#include "cntgs/contiguous/iterator.h"
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
    using Self = cntgs::ContiguousVector<Types...>;
    using Traits = detail::ContiguousVectorTraits<Self>;
    using value_type = typename Traits::ValueReturnType;
    using reference = typename Traits::ReferenceReturnType;
    using const_reference = typename Traits::ConstReferenceReturnType;
    using iterator = cntgs::ContiguousVectorIterator<Self>;
    using const_iterator = cntgs::ContiguousVectorIterator<std::add_const_t<Self>>;
    using difference_type = std::ptrdiff_t;
    using size_type = std::size_t;

    static constexpr auto TYPE_COUNT = sizeof...(Types);

    size_type memory_size{};
    size_type element_count{};
    std::unique_ptr<std::byte[]> memory{};
    std::byte* last_element{};
    std::byte** last_element_address{};
    std::array<size_type, Traits::CONTIGUOUS_FIXED_SIZE_COUNT> fixed_sizes{};

    ContiguousVector() = default;

    ContiguousVector(size_type element_count, size_type varying_size_bytes,
                     std::array<size_type, Traits::CONTIGUOUS_FIXED_SIZE_COUNT> fixed_sizes = {})
        : memory_size(this->calculate_needed_memory_size(element_count, varying_size_bytes, fixed_sizes)),
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

    reference operator[](size_type i) noexcept { return this->subscript_operator(i); }

    const_reference operator[](size_type i) const noexcept { return this->subscript_operator(i); }

    template <std::size_t I>
    size_type get_fixed_size() const noexcept
    {
        return std::get<I>(this->fixed_sizes);
    }

    bool empty() const noexcept
    {
        return this->last_element_address == reinterpret_cast<std::byte**>(this->memory.get());
    }

    size_type size() const noexcept
    {
        return this->last_element_address - reinterpret_cast<std::byte**>(this->memory.get());
    }

    constexpr size_type capacity() const noexcept { return this->element_count; }

    constexpr size_type memory_consumption() const noexcept { return this->memory_size; }

    constexpr iterator begin() noexcept { return {*this}; }

    constexpr const_iterator begin() const noexcept { return {*this}; }

    constexpr iterator end() noexcept { return {*this, this->size()}; }

    constexpr const_iterator end() const noexcept { return {*this, this->size()}; }

    // private API
  private:
    template <size_t... I>
    static constexpr auto calculate_fixed_size_memory_consumption(
        const std::array<std::size_t, Traits::CONTIGUOUS_FIXED_SIZE_COUNT>& fixed_sizes,
        std::index_sequence<I...>) noexcept
    {
        return ((detail::ParameterTraits<Types>::VALUE_BYTES * Traits::FixedSizeGetter<Types>::get<I>(fixed_sizes)) +
                ...);
    }

    static constexpr auto calculate_needed_memory_size(
        size_type element_count, size_type varying_size_bytes,
        const std::array<std::size_t, Traits::CONTIGUOUS_FIXED_SIZE_COUNT>& fixed_sizes) noexcept
    {
        return Traits::SIZE_IN_MEMORY * element_count + varying_size_bytes +
               calculate_fixed_size_memory_consumption(fixed_sizes, std::make_index_sequence<TYPE_COUNT>{}) *
                   element_count +
               detail::calculate_element_addresses_size(element_count);
    }

    template <std::size_t... I, class... Args>
    auto emplace_back_impl(std::index_sequence<I...>, Args&&... args)
    {
        *this->last_element_address = this->last_element;
        ++this->last_element_address;
        ((last_element = detail::ParameterTraits<Traits::TypeAt<I>>::store_contiguously(
              std::forward<Args>(args), this->last_element,
              Traits::FixedSizeGetter<Traits::TypeAt<I>>::get<I>(fixed_sizes))),
         ...);
    }

    template <class Result, class... T, std::size_t... I>
    constexpr auto convert_tuple_to(const std::tuple<T...>& tuple_of_pointer, std::index_sequence<I...>) const noexcept
    {
        return Result{detail::dereference(std::get<I>(tuple_of_pointer))...};
    }

    template <class Result, class... T>
    constexpr auto convert_tuple_to(const std::tuple<T...>& tuple_of_pointer) const noexcept
    {
        return this->convert_tuple_to<Result>(tuple_of_pointer, std::make_index_sequence<sizeof...(T)>{});
    }

    template <class... T, class Function, std::size_t... I>
    constexpr auto for_each_impl(std::tuple<T...>& tuple, Function&& function, std::index_sequence<I...>) const noexcept
    {
        return (function(std::get<I>(tuple), Traits::FixedSizeGetter<Traits::TypeAt<I>>::get<I>(fixed_sizes),
                         detail::ParameterTraits<Traits::TypeAt<I>>{}),
                ...);
    }

    template <class T, class Function>
    constexpr auto for_each(T&& tuple, Function&& function) const noexcept
    {
        return this->for_each_impl(std::forward<T>(tuple), std::forward<Function>(function),
                                   std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<T>>>{});
    }

    auto get_tuple_pointers_at(size_type i) const noexcept
    {
        typename Traits::PointerReturnType result;
        auto* start = this->element_addresses()[i];
        this->for_each(result, [&](auto& element, size_type size, auto traits) {
            std::tie(element, start) = decltype(traits)::from_address(start, size);
        });
        return result;
    }

    auto subscript_operator(size_type i) noexcept
    {
        const auto tuple = get_tuple_pointers_at(i);
        return this->convert_tuple_to<reference>(tuple);
    }

    auto subscript_operator(size_type i) const noexcept
    {
        const auto tuple = get_tuple_pointers_at(i);
        return this->convert_tuple_to<const_reference>(tuple);
    }
};
}  // namespace cntgs