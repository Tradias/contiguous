#pragma once

#include "cntgs/contiguous/detail/array.h"
#include "cntgs/contiguous/detail/elementLocator.h"
#include "cntgs/contiguous/detail/memory.h"
#include "cntgs/contiguous/detail/parameterTraits.h"
#include "cntgs/contiguous/detail/vector.h"
#include "cntgs/contiguous/detail/vectorTraits.h"
#include "cntgs/contiguous/iterator.h"
#include "cntgs/contiguous/parameter.h"
#include "cntgs/contiguous/span.h"
#include "cntgs/contiguous/tuple.h"
#include "cntgs/contiguous/typeErasedVector.h"

#include <array>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

namespace cntgs
{
template <class... Types>
class ContiguousVector
{
  private:
    using Self = cntgs::ContiguousVector<Types...>;
    using Traits = detail::VectorTraits<Self>;
    using ElementLocator = detail::ElementLocatorT<Types...>;
    using StorageType = typename Traits::StorageType;

    template <class T>
    using FixedSizeGetter = typename Traits::template FixedSizeGetter<T>;

    static constexpr auto TYPE_COUNT = sizeof...(Types);
    static constexpr bool IS_MIXED = Traits::IS_MIXED;
    static constexpr bool IS_ALL_FIXED_SIZE = Traits::IS_ALL_FIXED_SIZE;
    static constexpr bool IS_ALL_VARYING_SIZE = Traits::IS_ALL_VARYING_SIZE;
    static constexpr bool IS_NONE_SPECIAL = Traits::IS_NONE_SPECIAL;

  public:
    using value_type = typename Traits::ValueReturnType;
    using reference = typename Traits::ReferenceReturnType;
    using const_reference = typename Traits::ConstReferenceReturnType;
    using iterator = cntgs::ContiguousVectorIterator<Self>;
    using const_iterator = cntgs::ContiguousVectorIterator<std::add_const_t<Self>>;
    using difference_type = typename Traits::DifferenceType;
    using size_type = typename Traits::SizeType;

    size_type memory_size{};
    size_type max_element_count{};
    StorageType memory{};
    std::array<size_type, Traits::CONTIGUOUS_FIXED_SIZE_COUNT> fixed_sizes{};
    ElementLocator locator;

    ContiguousVector() = default;

    explicit ContiguousVector(cntgs::TypeErasedVector&& vector) noexcept
        : ContiguousVector(vector, {std::move(vector.memory)})
    {
    }

    explicit ContiguousVector(const cntgs::TypeErasedVector& vector) noexcept
        : ContiguousVector(vector, cntgs::Span<std::byte>{vector.memory.get(), vector.memory_size})
    {
    }

    template <bool IsMixed = IS_MIXED>
    ContiguousVector(size_type max_element_count, size_type varying_size_bytes,
                     const std::array<size_type, Traits::CONTIGUOUS_FIXED_SIZE_COUNT>& fixed_sizes,
                     std::enable_if_t<IsMixed>* = nullptr)
        : ContiguousVector({}, {}, max_element_count, varying_size_bytes, fixed_sizes)
    {
    }

    template <bool IsAllFixedSize = IS_ALL_FIXED_SIZE>
    ContiguousVector(size_type max_element_count,
                     const std::array<size_type, Traits::CONTIGUOUS_FIXED_SIZE_COUNT>& fixed_sizes,
                     std::enable_if_t<IsAllFixedSize>* = nullptr)
        : ContiguousVector({}, {}, max_element_count, {}, fixed_sizes)
    {
    }

    template <bool IsAllFixedSize = IS_ALL_FIXED_SIZE>
    ContiguousVector(size_type memory_size, std::unique_ptr<std::byte[]>&& transferred_ownership,
                     size_type max_element_count,
                     const std::array<size_type, Traits::CONTIGUOUS_FIXED_SIZE_COUNT>& fixed_sizes,
                     std::enable_if_t<IsAllFixedSize>* = nullptr)
        : ContiguousVector(memory_size, {std::move(transferred_ownership)}, max_element_count, {}, fixed_sizes)
    {
    }

    template <bool IsAllFixedSize = IS_ALL_FIXED_SIZE>
    ContiguousVector(cntgs::Span<std::byte> mutable_view, size_type max_element_count,
                     const std::array<size_type, Traits::CONTIGUOUS_FIXED_SIZE_COUNT>& fixed_sizes,
                     std::enable_if_t<IsAllFixedSize>* = nullptr)
        : ContiguousVector(mutable_view.size(), {mutable_view}, max_element_count, {}, fixed_sizes)
    {
    }

    template <bool IsAllVaryingSize = IS_ALL_VARYING_SIZE>
    ContiguousVector(size_type max_element_count, size_type varying_size_bytes,
                     std::enable_if_t<IsAllVaryingSize>* = nullptr)
        : ContiguousVector({}, {}, max_element_count, varying_size_bytes, {})
    {
    }

    template <bool IsNoneSpecial = IS_NONE_SPECIAL>
    ContiguousVector(size_type max_element_count, std::enable_if_t<IsNoneSpecial>* = nullptr)
        : ContiguousVector({}, {}, max_element_count, {}, {})
    {
    }

    template <bool IsNoneSpecial = IS_NONE_SPECIAL>
    ContiguousVector(size_type memory_size, std::unique_ptr<std::byte[]>&& transferred_ownership,
                     size_type max_element_count, std::enable_if_t<IsNoneSpecial>* = nullptr)
        : ContiguousVector(memory_size, {std::move(transferred_ownership)}, max_element_count, {}, {})
    {
    }

    template <bool IsNoneSpecial = IS_NONE_SPECIAL>
    ContiguousVector(cntgs::Span<std::byte> mutable_view, size_type max_element_count,
                     std::enable_if_t<IsNoneSpecial>* = nullptr)
        : ContiguousVector(mutable_view.size(), {mutable_view}, max_element_count, {}, {})
    {
    }

    ContiguousVector(const ContiguousVector&) = default;
    ContiguousVector(ContiguousVector&&) = default;
    ContiguousVector& operator=(const ContiguousVector&) = default;
    ContiguousVector& operator=(ContiguousVector&&) = default;

    ~ContiguousVector() { destruct(); }

    template <class... Args>
    void emplace_back(Args&&... args)
    {
        this->locator.emplace_back(this->fixed_sizes, std::forward<Args>(args)...);
    }

    reference operator[](size_type i) noexcept { return this->subscript_operator(i); }

    const_reference operator[](size_type i) const noexcept { return this->subscript_operator(i); }

    template <std::size_t I>
    size_type get_fixed_size() const noexcept
    {
        return std::get<I>(this->fixed_sizes);
    }

    bool empty() const noexcept { return this->locator.empty(this->memory.get()); }

    size_type size() const noexcept { return this->locator.size(this->memory.get()); }

    constexpr size_type capacity() const noexcept { return this->max_element_count; }

    constexpr size_type memory_consumption() const noexcept { return this->memory_size; }

    constexpr iterator begin() noexcept { return {*this}; }

    constexpr const_iterator begin() const noexcept { return {*this}; }

    constexpr iterator end() noexcept { return {*this, this->size()}; }

    constexpr const_iterator end() const noexcept { return {*this, this->size()}; }

    // private API
  private:
    ContiguousVector(size_type memory_size, StorageType&& storage, size_type max_element_count,
                     size_type varying_size_bytes,
                     const std::array<size_type, Traits::CONTIGUOUS_FIXED_SIZE_COUNT>& fixed_sizes)
        : memory_size(memory_size > 0
                          ? memory_size
                          : this->calculate_needed_memory_size(max_element_count, varying_size_bytes, fixed_sizes)),
          max_element_count(max_element_count),
          memory(detail::acquire_or_create_new(std::move(storage), this->memory_size)),
          fixed_sizes(fixed_sizes),
          locator(max_element_count, this->memory.get(), fixed_sizes)
    {
    }

    ContiguousVector(const cntgs::TypeErasedVector& vector, StorageType&& storage) noexcept
        : memory_size(vector.memory_size),
          max_element_count(vector.max_element_count),
          memory(std::move(storage)),
          fixed_sizes(detail::convert_array_to_size<Traits::CONTIGUOUS_FIXED_SIZE_COUNT>(vector.fixed_sizes)),
          locator(*reinterpret_cast<const ElementLocator*>(std::launder(vector.locator.data())))
    {
    }

    static constexpr auto calculate_needed_memory_size(
        size_type max_element_count, size_type varying_size_bytes,
        const std::array<size_type, Traits::CONTIGUOUS_FIXED_SIZE_COUNT>& fixed_sizes) noexcept
    {
        constexpr auto ALIGNMENT_OVERHEAD = Traits::MAX_ALIGNMENT > 0 ? Traits::MAX_ALIGNMENT - 1 : size_type{};
        return varying_size_bytes + ElementLocator::calculate_element_size(fixed_sizes) * max_element_count +
               ElementLocator::reserved_bytes(max_element_count) + ALIGNMENT_OVERHEAD;
    }

    template <class Result, class... T, std::size_t... I>
    static constexpr auto convert_tuple_to(
        const cntgs::ContiguousTuple<detail::ContiguousTupleQualifier::POINTER, T...>& tuple_of_pointer,
        std::index_sequence<I...>) noexcept
    {
        return Result{detail::dereference(cntgs::get<I>(tuple_of_pointer))...};
    }

    template <class Result, class... T>
    static constexpr auto convert_tuple_to(
        const cntgs::ContiguousTuple<detail::ContiguousTupleQualifier::POINTER, T...>& tuple_of_pointer) noexcept
    {
        return convert_tuple_to<Result>(tuple_of_pointer, std::make_index_sequence<sizeof...(T)>{});
    }

    auto load_element_at(size_type i) const noexcept
    {
        return this->locator.load_element_at(i, this->memory.get(), fixed_sizes);
    }

    auto subscript_operator(size_type i) noexcept
    {
        const auto tuple = load_element_at(i);
        return this->convert_tuple_to<reference>(tuple);
    }

    auto subscript_operator(size_type i) const noexcept
    {
        const auto tuple = load_element_at(i);
        return this->convert_tuple_to<const_reference>(tuple);
    }

    template <std::size_t... I>
    void destruct(std::index_sequence<I...>)
    {
        for (auto&& element : *this)
        {
            (detail::ParameterTraits<Types>::destroy(cntgs::get<I>(element)), ...);
        }
    }

    void destruct()
    {
        if constexpr (!Traits::IS_TRIVIALLY_DESTRUCTIBLE)
        {
            if (memory && memory.is_owned)
            {
                destruct(std::make_index_sequence<TYPE_COUNT>{});
            }
        }
    }
};

template <class... Types>
auto type_erase(cntgs::ContiguousVector<Types...>&& vector)
{
    return cntgs::TypeErasedVector{
        vector.memory_size, vector.max_element_count, std::move(vector.memory),
        detail::convert_array_to_size<detail::ContiguousVectorTraits<>::MAX_FIXED_SIZE_VECTOR_PARAMETER>(
            vector.fixed_sizes),
        detail::type_erase_element_locator(std::move(vector.locator))};
}
}  // namespace cntgs