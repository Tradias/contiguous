#pragma once

#include "cntgs/contiguous/detail/array.h"
#include "cntgs/contiguous/detail/elementLocator.h"
#include "cntgs/contiguous/detail/forward.h"
#include "cntgs/contiguous/detail/memory.h"
#include "cntgs/contiguous/detail/parameterListTraits.h"
#include "cntgs/contiguous/detail/parameterTraits.h"
#include "cntgs/contiguous/detail/tuple.h"
#include "cntgs/contiguous/detail/utility.h"
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
using ContiguousVector = cntgs::BasicContiguousVector<std::allocator<void>, Types...>;

template <class Allocator, class... Types>
class BasicContiguousVector
{
  private:
    using Self = cntgs::BasicContiguousVector<Allocator, Types...>;
    using ListTraits = detail::ParameterListTraits<Types...>;
    using VectorTraits = detail::ContiguousVectorTraits<Types...>;
    using ElementLocator = detail::ElementLocatorT<Types...>;
    using ElementTraits = detail::ElementTraitsT<Types...>;
    using StorageDeleter = detail::AllocatorDeleter<Allocator, true>;
    using StorageType = detail::MaybeOwnedPtr<std::byte[], StorageDeleter>;
    using FixedSizes = typename ListTraits::FixedSizes;

    static constexpr bool IS_MIXED = ListTraits::IS_MIXED;
    static constexpr bool IS_ALL_FIXED_SIZE = ListTraits::IS_ALL_FIXED_SIZE;
    static constexpr bool IS_ALL_VARYING_SIZE = ListTraits::IS_ALL_VARYING_SIZE;
    static constexpr bool IS_NONE_SPECIAL = ListTraits::IS_NONE_SPECIAL;

  public:
    using value_type = cntgs::BasicContiguousElement<Allocator, Types...>;
    using reference = typename VectorTraits::ReferenceReturnType;
    using const_reference = typename VectorTraits::ConstReferenceReturnType;
    using iterator = cntgs::ContiguousVectorIterator<Self>;
    using const_iterator = cntgs::ContiguousVectorIterator<std::add_const_t<Self>>;
    using difference_type = std::ptrdiff_t;
    using size_type = std::size_t;
    using allocator_type = Allocator;

    size_type max_element_count{};
    StorageType memory{};
    FixedSizes fixed_sizes{};
    ElementLocator locator;

    BasicContiguousVector() = default;

    explicit BasicContiguousVector(cntgs::TypeErasedVector&& vector) noexcept
        : BasicContiguousVector(vector, vector.memory, vector.is_memory_owned.value)
    {
        vector.is_memory_owned.value = false;
    }

    explicit BasicContiguousVector(const cntgs::TypeErasedVector& vector) noexcept
        : BasicContiguousVector(vector, vector.memory, false)
    {
    }

    template <bool IsMixed = IS_MIXED>
    BasicContiguousVector(size_type max_element_count, size_type varying_size_bytes, const FixedSizes& fixed_sizes,
                          allocator_type allocator = {}, std::enable_if_t<IsMixed>* = nullptr)
        : BasicContiguousVector(max_element_count, varying_size_bytes, fixed_sizes, std::move(allocator))
    {
    }

    template <bool IsAllFixedSize = IS_ALL_FIXED_SIZE>
    BasicContiguousVector(size_type max_element_count, const FixedSizes& fixed_sizes, allocator_type allocator = {},
                          std::enable_if_t<IsAllFixedSize>* = nullptr)
        : BasicContiguousVector(max_element_count, size_type{}, fixed_sizes, std::move(allocator))
    {
    }

    template <bool IsAllFixedSize = IS_ALL_FIXED_SIZE>
    BasicContiguousVector(size_type memory_size, std::byte* transferred_ownership, size_type max_element_count,
                          const FixedSizes& fixed_sizes, allocator_type allocator = {},
                          std::enable_if_t<IsAllFixedSize>* = nullptr)
        : BasicContiguousVector(
              StorageType{transferred_ownership, StorageDeleter{memory_size, std::move(allocator)}, true},
              max_element_count, fixed_sizes)
    {
    }

    template <bool IsAllFixedSize = IS_ALL_FIXED_SIZE>
    BasicContiguousVector(cntgs::Span<std::byte> mutable_view, size_type max_element_count,
                          const FixedSizes& fixed_sizes, allocator_type allocator = {},
                          std::enable_if_t<IsAllFixedSize>* = nullptr)
        : BasicContiguousVector(
              StorageType{mutable_view.data(), StorageDeleter{mutable_view.size(), std::move(allocator)}},
              max_element_count, fixed_sizes)
    {
    }

    template <bool IsAllVaryingSize = IS_ALL_VARYING_SIZE>
    BasicContiguousVector(size_type max_element_count, size_type varying_size_bytes, allocator_type allocator = {},
                          std::enable_if_t<IsAllVaryingSize>* = nullptr)
        : BasicContiguousVector(max_element_count, varying_size_bytes, FixedSizes{}, std::move(allocator))
    {
    }

    template <bool IsNoneSpecial = IS_NONE_SPECIAL>
    BasicContiguousVector(size_type max_element_count, allocator_type allocator = {},
                          std::enable_if_t<IsNoneSpecial>* = nullptr)
        : BasicContiguousVector(max_element_count, size_type{}, FixedSizes{}, std::move(allocator))
    {
    }

    template <bool IsNoneSpecial = IS_NONE_SPECIAL>
    BasicContiguousVector(size_type memory_size, std::byte* transferred_ownership, size_type max_element_count,
                          allocator_type allocator = {}, std::enable_if_t<IsNoneSpecial>* = nullptr)
        : BasicContiguousVector(
              StorageType{transferred_ownership, StorageDeleter{memory_size, std::move(allocator)}, true},
              max_element_count, FixedSizes{})
    {
    }

    template <bool IsNoneSpecial = IS_NONE_SPECIAL>
    BasicContiguousVector(cntgs::Span<std::byte> mutable_view, size_type max_element_count,
                          allocator_type allocator = {}, std::enable_if_t<IsNoneSpecial>* = nullptr)
        : BasicContiguousVector(
              StorageType{mutable_view.data(), StorageDeleter{mutable_view.size(), std::move(allocator)}},
              max_element_count, FixedSizes{})
    {
    }

    BasicContiguousVector(const BasicContiguousVector&) = delete;
    BasicContiguousVector(BasicContiguousVector&&) = default;
    BasicContiguousVector& operator=(const BasicContiguousVector&) = delete;
    BasicContiguousVector& operator=(BasicContiguousVector&&) = default;

    ~BasicContiguousVector() noexcept { this->destruct(); }

    template <class... Args>
    void emplace_back(Args&&... args)
    {
        this->locator.emplace_back(this->fixed_sizes, std::forward<Args>(args)...);
    }

    void pop_back() noexcept
    {
        ElementTraits::destruct(this->back());
        this->locator.resize(this->size() - size_type{1}, this->memory.get());
    }

    void reserve(size_type new_max_element_count, size_type new_varying_size_bytes = {})
    {
        if (this->max_element_count < new_max_element_count)
        {
            this->grow(new_max_element_count, new_varying_size_bytes);
        }
    }

    void erase(const_iterator position) noexcept(ListTraits::IS_NOTHROW_MOVE_CONSTRUCTIBLE)
    {
        this->erase(position, position + 1);
    }

    void erase(const_iterator first, const_iterator last) noexcept(ListTraits::IS_NOTHROW_MOVE_CONSTRUCTIBLE)
    {
        iterator it_first{*this, first.index()};
        iterator it_last{*this, last.index()};
        this->destruct(it_first, it_last);
        this->move_elements_forward_to(it_first, it_last);
        this->locator.resize(this->size() - std::distance(first, last), this->memory.get());
    }

    reference operator[](size_type i) noexcept
    {
        return reference{this->locator.element_address(i, this->memory.get()), fixed_sizes};
    }

    const_reference operator[](size_type i) const noexcept
    {
        return const_reference{this->locator.element_address(i, this->memory.get()), fixed_sizes};
    }

    reference front() noexcept { return (*this)[{}]; }

    const_reference front() const noexcept { return (*this)[{}]; }

    reference back() noexcept { return (*this)[this->size() - size_type{1}]; }

    const_reference back() const noexcept { return (*this)[this->size() - size_type{1}]; }

    template <std::size_t I>
    [[nodiscard]] constexpr size_type get_fixed_size() const noexcept
    {
        return std::get<I>(this->fixed_sizes);
    }

    [[nodiscard]] bool empty() const noexcept { return this->locator.empty(this->memory.get()); }

    [[nodiscard]] std::byte* data() const noexcept { return this->locator.element_address({}, this->memory.get()); }

    [[nodiscard]] size_type size() const noexcept { return this->locator.size(this->memory.get()); }

    [[nodiscard]] constexpr size_type capacity() const noexcept { return this->max_element_count; }

    [[nodiscard]] constexpr size_type memory_consumption() const noexcept { return this->memory.get_deleter().size(); }

    [[nodiscard]] constexpr iterator begin() noexcept { return iterator{*this}; }

    [[nodiscard]] constexpr const_iterator begin() const noexcept { return const_iterator{*this}; }

    [[nodiscard]] constexpr const_iterator cbegin() const noexcept { return this->begin(); }

    [[nodiscard]] constexpr iterator end() noexcept { return iterator{*this, this->size()}; }

    [[nodiscard]] constexpr const_iterator end() const noexcept { return const_iterator{*this, this->size()}; }

    [[nodiscard]] constexpr const_iterator cend() const noexcept { return this->end(); }

    [[nodiscard]] allocator_type get_allocator() const noexcept { return this->memory.get_deleter().get_allocator(); }

    // private API
  private:
    BasicContiguousVector(StorageType&& storage, size_type max_element_count, const FixedSizes& fixed_sizes)
        : max_element_count(max_element_count),
          memory(std::move(storage)),
          fixed_sizes(fixed_sizes),
          locator(max_element_count, this->memory.get(), fixed_sizes)
    {
    }

    BasicContiguousVector(size_type max_element_count, size_type varying_size_bytes, const FixedSizes& fixed_sizes,
                          allocator_type allocator)
        : max_element_count(max_element_count),
          memory(detail::allocate_unique_for_overwrite<typename StorageType::element_type[]>(
              Self::calculate_needed_memory_size(max_element_count, varying_size_bytes, fixed_sizes),
              std::move(allocator))),
          fixed_sizes(fixed_sizes),
          locator(max_element_count, this->memory.get(), fixed_sizes)
    {
    }

    BasicContiguousVector(const cntgs::TypeErasedVector& vector, std::byte* memory, bool is_memory_owned) noexcept
        : max_element_count(vector.max_element_count),
          memory(memory, *reinterpret_cast<const StorageDeleter*>(&vector.deleter), is_memory_owned),
          fixed_sizes(detail::convert_array_to_size<ListTraits::CONTIGUOUS_FIXED_SIZE_COUNT>(vector.fixed_sizes)),
          locator(*std::launder(reinterpret_cast<const ElementLocator*>(&vector.locator)))
    {
    }

    [[nodiscard]] static constexpr auto calculate_needed_memory_size(size_type max_element_count,
                                                                     size_type varying_size_bytes,
                                                                     const FixedSizes& fixed_sizes) noexcept
    {
        constexpr auto ALIGNMENT_OVERHEAD = ListTraits::template ParameterTraitsAt<0>::ALIGNMENT - 1;
        return varying_size_bytes + ElementTraits::calculate_element_size(fixed_sizes) * max_element_count +
               ElementLocator::reserved_bytes(max_element_count) + ALIGNMENT_OVERHEAD;
    }

    void grow(size_type new_max_element_count, size_type new_varying_size_bytes)
    {
        using StorageElementType = typename StorageType::element_type;
        const auto new_memory_size =
            this->calculate_needed_memory_size(new_max_element_count, new_varying_size_bytes, this->fixed_sizes);
        auto new_memory =
            detail::allocate_unique_for_overwrite<StorageElementType[]>(new_memory_size, this->get_allocator());
        if constexpr (ListTraits::IS_TRIVIALLY_MOVE_CONSTRUCTIBLE && ListTraits::IS_TRIVIALLY_DESTRUCTIBLE)
        {
            this->locator.copy_from(new_max_element_count, new_memory.get(), this->max_element_count,
                                    this->memory.get());
        }
        else
        {
            ElementLocator new_locator{new_max_element_count, new_memory.get(), this->locator, this->max_element_count,
                                       this->memory.get()};
            this->uninitialized_move(new_memory.get(), new_locator, ListTraits::make_index_sequence());
            this->locator = std::move(new_locator);
        }
        this->max_element_count = new_max_element_count;
        this->memory = StorageType{std::move(new_memory)};
    }

    template <std::size_t... I>
    void uninitialized_move([[maybe_unused]] std::byte* new_memory, [[maybe_unused]] ElementLocator& new_locator,
                            std::index_sequence<I...>)
    {
        if constexpr (!ListTraits::IS_TRIVIALLY_MOVE_CONSTRUCTIBLE)
        {
            for (size_type i{}; i < this->size(); ++i)
            {
                auto&& source = (*this)[i];
                auto&& target =
                    ElementTraits::load_element_at(new_locator.element_address(i, new_memory), this->fixed_sizes);
                ElementTraits::template construct_if_non_trivial<true>(source, target);
            }
        }
        this->destruct();
    }

    void move_elements_forward_to(iterator position, iterator from)
    {
        if (this->end() == from)
        {
            return;
        }
        if constexpr (ListTraits::IS_TRIVIALLY_MOVE_CONSTRUCTIBLE && ListTraits::IS_TRIVIALLY_DESTRUCTIBLE &&
                      (ListTraits::IS_ALL_FIXED_SIZE || ListTraits::IS_NONE_SPECIAL))
        {
            const auto source = from->start_address();
            std::memmove(position->start_address(), source, (this->memory.get() + this->memory_consumption()) - source);
        }
        else
        {
            for (auto i = static_cast<size_type>(std::distance(this->begin(), position)); from != this->end();
                 ++i, ++from)
            {
                this->emplace_at(i, *from, ListTraits::make_index_sequence());
            }
        }
    }

    template <std::size_t... I>
    void emplace_at(std::size_t i, const reference& element, std::index_sequence<I...>)
    {
        this->locator.emplace_at(i, this->memory.get(), this->fixed_sizes, std::move(cntgs::get<I>(element))...);
        ElementTraits::destruct(element);
    }

    void destruct() noexcept { this->destruct(this->begin(), this->end()); }

    void destruct([[maybe_unused]] iterator first, [[maybe_unused]] iterator last) noexcept
    {
        if constexpr (!ListTraits::IS_TRIVIALLY_DESTRUCTIBLE)
        {
            if (this->memory && this->memory.is_owned())
            {
                std::for_each(first, last,
                              [](const auto& element)
                              {
                                  ElementTraits::destruct(element);
                              });
            }
        }
    }
};

template <class Allocator, class... T>
auto type_erase(cntgs::BasicContiguousVector<Allocator, T...>&& vector) noexcept
{
    return cntgs::TypeErasedVector{
        vector.memory_consumption(),
        vector.max_element_count,
        vector.memory.release(),
        vector.memory.is_owned(),
        detail::type_erase_deleter(std::move(vector.memory.get_deleter())),
        detail::convert_array_to_size<detail::MAX_FIXED_SIZE_VECTOR_PARAMETER>(vector.fixed_sizes),
        detail::type_erase_element_locator(std::move(vector.locator)),
        []([[maybe_unused]] cntgs::TypeErasedVector& erased)
        {
            cntgs::BasicContiguousVector<Allocator, T...>{std::move(erased)};
        }};
}
}  // namespace cntgs
