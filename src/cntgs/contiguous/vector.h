#pragma once

#include "cntgs/contiguous/detail/array.h"
#include "cntgs/contiguous/detail/elementLocator.h"
#include "cntgs/contiguous/detail/memory.h"
#include "cntgs/contiguous/detail/parameterTraits.h"
#include "cntgs/contiguous/detail/tuple.h"
#include "cntgs/contiguous/detail/utility.h"
#include "cntgs/contiguous/detail/vector.h"
#include "cntgs/contiguous/detail/vectorTraits.h"
#include "cntgs/contiguous/iterator.h"
#include "cntgs/contiguous/parameter.h"
#include "cntgs/contiguous/span.h"
#include "cntgs/contiguous/tuple.h"
#include "cntgs/contiguous/typeErasedVector.h"

#include <algorithm>
#include <array>
#include <memory>
#include <new>
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

    static constexpr std::size_t GROWTH_FACTOR = 2;
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

    ~ContiguousVector() noexcept(Traits::IS_NOTHROW_DESTRUCTIBLE) { this->destruct(); }

    template <class... Args>
    void emplace_back(Args&&... args)
    {
        this->locator.emplace_back(this->fixed_sizes, std::forward<Args>(args)...);
    }

    void reserve(size_type new_max_element_count, size_type new_varying_size_bytes = {})
    {
        if (this->max_element_count < new_max_element_count)
        {
            this->grow(new_max_element_count, new_varying_size_bytes);
        }
    }

    void shrink(size_type new_max_element_count) { this->shrink_and_destruct(new_max_element_count); }

    reference operator[](size_type i) noexcept { return this->subscript_operator(i); }

    const_reference operator[](size_type i) const noexcept { return this->subscript_operator(i); }

    template <std::size_t I>
    [[nodiscard]] size_type get_fixed_size() const noexcept
    {
        return std::get<I>(this->fixed_sizes);
    }

    [[nodiscard]] bool empty() const noexcept { return this->locator.empty(this->memory.get()); }

    [[nodiscard]] size_type size() const noexcept { return this->locator.size(this->memory.get()); }

    [[nodiscard]] constexpr size_type capacity() const noexcept { return this->max_element_count; }

    [[nodiscard]] constexpr size_type memory_consumption() const noexcept { return this->memory_size; }

    [[nodiscard]] constexpr iterator begin() noexcept { return iterator{*this}; }

    [[nodiscard]] constexpr const_iterator begin() const noexcept { return const_iterator{*this}; }

    [[nodiscard]] constexpr iterator end() noexcept { return iterator{*this, this->size()}; }

    [[nodiscard]] constexpr const_iterator end() const noexcept { return const_iterator{*this, this->size()}; }

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
          locator(*std::launder(reinterpret_cast<const ElementLocator*>(&vector.locator)))
    {
    }

    [[nodiscard]] static constexpr auto calculate_needed_memory_size(
        size_type max_element_count, size_type varying_size_bytes,
        const std::array<size_type, Traits::CONTIGUOUS_FIXED_SIZE_COUNT>& fixed_sizes) noexcept
    {
        constexpr auto ALIGNMENT_OVERHEAD = Traits::MAX_ALIGNMENT > 0 ? Traits::MAX_ALIGNMENT - 1 : size_type{};
        return varying_size_bytes + ElementLocator::calculate_element_size(fixed_sizes) * max_element_count +
               ElementLocator::reserved_bytes(max_element_count) + ALIGNMENT_OVERHEAD;
    }

    template <class ReturnType>
    auto subscript_operator(size_type i, std::byte* memory_begin, const ElementLocator& element_locator) const noexcept
    {
        const auto tuple_of_pointer = element_locator.load_element_at(i, memory_begin, this->fixed_sizes);
        return detail::convert_tuple_to<ReturnType>(tuple_of_pointer);
    }

    auto subscript_operator(size_type i) noexcept
    {
        return this->subscript_operator<reference>(i, this->memory.get(), this->locator);
    }

    [[nodiscard]] auto subscript_operator(size_type i) const noexcept
    {
        return this->subscript_operator<const_reference>(i, this->memory.get(), this->locator);
    }

    void grow(size_type new_max_element_count, size_type new_varying_size_bytes)
    {
        using StorageElementType = typename StorageType::element_type;
        const auto new_memory_size =
            this->calculate_needed_memory_size(new_max_element_count, new_varying_size_bytes, this->fixed_sizes);
        auto new_memory = detail::make_unique_for_overwrite<StorageElementType[]>(new_memory_size);
        if constexpr (Traits::IS_TRIVIALLY_MOVE_CONSTRUCTIBLE && Traits::IS_TRIVIALLY_DESTRUCTIBLE)
        {
            this->locator.copy_from(new_max_element_count, new_memory.get(), this->max_element_count,
                                    this->memory.get());
        }
        else
        {
            ElementLocator new_locator{new_max_element_count, new_memory.get(), this->locator, this->max_element_count,
                                       this->memory.get()};
            this->uninitialized_move(new_memory.get(), new_locator, std::make_index_sequence<sizeof...(Types)>{});
            this->locator = std::move(new_locator);
        }
        this->memory_size = new_memory_size;
        this->max_element_count = new_max_element_count;
        this->memory = std::move(new_memory);
    }

    template <std::size_t... I>
    void uninitialized_move([[maybe_unused]] std::byte* new_memory, [[maybe_unused]] ElementLocator& new_locator,
                            std::index_sequence<I...>)
    {
        if constexpr (!Traits::IS_TRIVIALLY_MOVE_CONSTRUCTIBLE)
        {
            for (size_type i{}; i < this->size(); ++i)
            {
                auto&& source = this->subscript_operator(i);
                auto&& target = new_locator.load_element_at(i, new_memory, this->fixed_sizes);
                (this->template uninitialized_move<Types>(cntgs::get<I>(source), std::get<I>(target)), ...);
            }
        }
        this->destruct();
    }

    template <class Type, class Source, class Target>
    void uninitialized_move([[maybe_unused]] Source&& source, [[maybe_unused]] Target&& target)
    {
        if constexpr (!std::is_trivially_move_constructible_v<typename detail::ParameterTraits<Type>::value_type>)
        {
            detail::ParameterTraits<Type>::uninitialized_move(source, target);
        }
    }

    void grow()
    {
        const auto new_max_element_count =
            std::max(size_type{1} + this->max_element_count, this->max_element_count * Self::GROWTH_FACTOR);
        this->grow(new_max_element_count, {});
    }

    void shrink_and_destruct(size_type new_max_element_count)
    {
        this->destruct(this->begin() + std::min(new_max_element_count, this->size()), this->end());
        this->max_element_count = new_max_element_count;
        this->locator.resize(new_max_element_count, this->memory.get());
    }

    void destruct() noexcept(Traits::IS_NOTHROW_DESTRUCTIBLE) { this->destruct(this->begin(), this->end()); }

    void destruct([[maybe_unused]] iterator first,
                  [[maybe_unused]] iterator last) noexcept(Traits::IS_NOTHROW_DESTRUCTIBLE)
    {
        if constexpr (!Traits::IS_TRIVIALLY_DESTRUCTIBLE)
        {
            if (this->memory && this->memory.is_owned)
            {
                destruct(first, last, std::make_index_sequence<sizeof...(Types)>{});
            }
        }
    }

    template <std::size_t... I>
    static void destruct(iterator first, iterator last,
                         std::index_sequence<I...>) noexcept(Traits::IS_NOTHROW_DESTRUCTIBLE)
    {
        std::for_each(first, last,
                      [](auto&& element) { (detail::ParameterTraits<Types>::destroy(cntgs::get<I>(element)), ...); });
    }
};

template <class... Types>
auto type_erase(cntgs::ContiguousVector<Types...>&& vector)
{
    return cntgs::TypeErasedVector{
        vector.memory_size, vector.max_element_count, std::move(vector.memory),
        detail::convert_array_to_size<detail::MAX_FIXED_SIZE_VECTOR_PARAMETER>(vector.fixed_sizes),
        detail::type_erase_element_locator(std::move(vector.locator))};
}
}  // namespace cntgs