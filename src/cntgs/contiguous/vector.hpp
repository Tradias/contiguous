#ifndef CNTGS_CONTIGUOUS_VECTOR_HPP
#define CNTGS_CONTIGUOUS_VECTOR_HPP

#include "cntgs/contiguous/detail/algorithm.hpp"
#include "cntgs/contiguous/detail/array.hpp"
#include "cntgs/contiguous/detail/elementLocator.hpp"
#include "cntgs/contiguous/detail/forward.hpp"
#include "cntgs/contiguous/detail/memory.hpp"
#include "cntgs/contiguous/detail/parameterListTraits.hpp"
#include "cntgs/contiguous/detail/parameterTraits.hpp"
#include "cntgs/contiguous/detail/tuple.hpp"
#include "cntgs/contiguous/detail/utility.hpp"
#include "cntgs/contiguous/detail/vectorTraits.hpp"
#include "cntgs/contiguous/iterator.hpp"
#include "cntgs/contiguous/parameter.hpp"
#include "cntgs/contiguous/reference.hpp"
#include "cntgs/contiguous/span.hpp"
#include "cntgs/contiguous/typeErasedVector.hpp"

#include <algorithm>
#include <array>
#include <cstring>
#include <memory>
#include <new>
#include <tuple>
#include <type_traits>
#include <utility>

namespace cntgs
{
/// Alias template for [cntgs::BasicContiguousVector]() that uses [std::allocator]()
template <class... Types>
using ContiguousVector = cntgs::BasicContiguousVector<std::allocator<std::byte>, Types...>;

/// Container that stores the value of each specified parameter contiguously.
///
/// \param Allocator An allocator that is used to acquire/release memory and to construct/destroy the elements in that
/// memory. The type must meet the requirements of [Allocator](https://en.cppreference.com/w/cpp/named_req/Allocator).
/// \param Types Any of [cntgs::VaryingSize](), [cntgs::FixedSize](), [cntgs::AlignAs]() or a plain user-defined or
/// built-in type. The underlying type of each parameter must meet the requirements of
/// [Erasable](https://en.cppreference.com/w/cpp/named_req/Erasable)
template <class Allocator, class... Types>
class BasicContiguousVector
{
  private:
    using Self = cntgs::BasicContiguousVector<Allocator, Types...>;
    using ListTraits = detail::ParameterListTraits<Types...>;
    using VectorTraits = detail::ContiguousVectorTraits<Types...>;
    using ElementLocator = detail::ElementLocatorT<Types...>;
    using ElementTraits = detail::ElementTraitsT<Types...>;
    using AllocatorTraits = std::allocator_traits<Allocator>;
    using StorageType =
        detail::MaybeOwnedAllocatorAwarePointer<typename AllocatorTraits::template rebind_alloc<std::byte>>;
    using FixedSizes = typename ListTraits::FixedSizes;

    static constexpr bool IS_MIXED = ListTraits::IS_MIXED;
    static constexpr bool IS_ALL_FIXED_SIZE = ListTraits::IS_ALL_FIXED_SIZE;
    static constexpr bool IS_ALL_VARYING_SIZE = ListTraits::IS_ALL_VARYING_SIZE;
    static constexpr bool IS_ALL_PLAIN = ListTraits::IS_ALL_PLAIN;

  public:
    /// Type that can create copies of [cntgs::BasicContiguousVector::reference]() and
    /// [cntgs::BasicContiguousVector::const_reference]()
    using value_type = cntgs::BasicContiguousElement<Allocator, Types...>;

    /// A [cntgs::BasicContiguousReference]() with [cntgs::ContiguousReferenceQualifier::MUTABLE]()
    /// \exclude target
    using reference = typename VectorTraits::ReferenceType;

    /// A [cntgs::BasicContiguousReference]() with [cntgs::ContiguousReferenceQualifier::CONST]()
    /// \exclude target
    using const_reference = typename VectorTraits::ConstReferenceType;
    using iterator = cntgs::ContiguousVectorIterator<false, Allocator, Types...>;
    using const_iterator = cntgs::ContiguousVectorIterator<true, Allocator, Types...>;
    using difference_type = std::ptrdiff_t;
    using size_type = std::size_t;
    using allocator_type = Allocator;

    size_type max_element_count{};
    StorageType memory{};
    FixedSizes fixed_sizes{};
    ElementLocator locator;

    BasicContiguousVector() = default;

    explicit BasicContiguousVector(cntgs::TypeErasedVector&& vector) noexcept
        : BasicContiguousVector(vector, std::exchange(vector.is_memory_owned.value, false))
    {
    }

    explicit BasicContiguousVector(const cntgs::TypeErasedVector& vector) noexcept
        : BasicContiguousVector(vector, false)
    {
    }

    template <bool IsMixed = IS_MIXED>
    BasicContiguousVector(size_type max_element_count, size_type varying_size_bytes, const FixedSizes& fixed_sizes,
                          const allocator_type& allocator = {}, std::enable_if_t<IsMixed>* = nullptr)
        : BasicContiguousVector(max_element_count, varying_size_bytes, fixed_sizes, allocator)
    {
    }

    template <bool IsAllFixedSize = IS_ALL_FIXED_SIZE>
    BasicContiguousVector(size_type max_element_count, const FixedSizes& fixed_sizes,
                          const allocator_type& allocator = {}, std::enable_if_t<IsAllFixedSize>* = nullptr)
        : BasicContiguousVector(max_element_count, size_type{}, fixed_sizes, allocator)
    {
    }

    template <bool IsAllFixedSize = IS_ALL_FIXED_SIZE>
    BasicContiguousVector(std::byte* transferred_ownership, size_type memory_size, size_type max_element_count,
                          const FixedSizes& fixed_sizes, const allocator_type& allocator = {},
                          std::enable_if_t<IsAllFixedSize>* = nullptr)
        : BasicContiguousVector(transferred_ownership, memory_size, true, max_element_count, fixed_sizes, allocator)
    {
    }

    template <bool IsAllFixedSize = IS_ALL_FIXED_SIZE>
    BasicContiguousVector(cntgs::Span<std::byte> mutable_view, size_type max_element_count,
                          const FixedSizes& fixed_sizes, const allocator_type& allocator = {},
                          std::enable_if_t<IsAllFixedSize>* = nullptr)
        : BasicContiguousVector(mutable_view.data(), mutable_view.size(), false, max_element_count, fixed_sizes,
                                allocator)
    {
    }

    template <bool IsAllVaryingSize = IS_ALL_VARYING_SIZE>
    BasicContiguousVector(size_type max_element_count, size_type varying_size_bytes,
                          const allocator_type& allocator = {}, std::enable_if_t<IsAllVaryingSize>* = nullptr)
        : BasicContiguousVector(max_element_count, varying_size_bytes, FixedSizes{}, allocator)
    {
    }

    template <bool IsNoneSpecial = IS_ALL_PLAIN>
    BasicContiguousVector(size_type max_element_count, const allocator_type& allocator = {},
                          std::enable_if_t<IsNoneSpecial>* = nullptr)
        : BasicContiguousVector(max_element_count, size_type{}, FixedSizes{}, allocator)
    {
    }

    template <bool IsNoneSpecial = IS_ALL_PLAIN>
    BasicContiguousVector(std::byte* transferred_ownership, size_type memory_size, size_type max_element_count,
                          const allocator_type& allocator = {}, std::enable_if_t<IsNoneSpecial>* = nullptr)
        : BasicContiguousVector(transferred_ownership, memory_size, true, max_element_count, FixedSizes{}, allocator)
    {
    }

    template <bool IsNoneSpecial = IS_ALL_PLAIN>
    BasicContiguousVector(cntgs::Span<std::byte> mutable_view, size_type max_element_count,
                          const allocator_type& allocator = {}, std::enable_if_t<IsNoneSpecial>* = nullptr)
        : BasicContiguousVector(mutable_view.data(), mutable_view.size(), false, max_element_count, FixedSizes{},
                                allocator)
    {
    }

    BasicContiguousVector(const BasicContiguousVector& other)
        : max_element_count(other.max_element_count),
          memory(other.memory),
          fixed_sizes(other.fixed_sizes),
          locator(this->copy_construct_locator(other))
    {
    }

    BasicContiguousVector(BasicContiguousVector&&) = default;

    BasicContiguousVector& operator=(const BasicContiguousVector& other)
    {
        if (this != std::addressof(other))
        {
            this->copy_assign(other);
        }
        return *this;
    }

    BasicContiguousVector& operator=(BasicContiguousVector&& other)
    {
        if (this != std::addressof(other))
        {
            this->move_assign(std::move(other));
        }
        return *this;
    }

    ~BasicContiguousVector() noexcept { this->destruct_if_owned(); }

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

    iterator erase(const_iterator position) noexcept(ListTraits::IS_NOTHROW_MOVE_CONSTRUCTIBLE)
    {
        iterator it_position{*this, position.index()};
        const auto next_position = position.index() + 1;
        ElementTraits::destruct(*it_position);
        this->move_elements_forward_to(it_position.index(), next_position);
        this->locator.resize(this->size() - size_type{1}, this->memory.get());
        return it_position;
    }

    iterator erase(const_iterator first, const_iterator last) noexcept(ListTraits::IS_NOTHROW_MOVE_CONSTRUCTIBLE)
    {
        const auto current_size = this->size();
        iterator it_first{*this, first.index()};
        iterator it_last{*this, last.index()};
        this->destruct(it_first, it_last);
        if (last.index() < current_size && first.index() != last.index())
        {
            this->move_elements_forward_to(first.index(), last.index());
        }
        this->locator.resize(current_size - (last.index() - first.index()), this->memory.get());
        return it_first;
    }

    void clear() noexcept
    {
        this->destruct();
        this->locator.resize(0, this->memory.get());
    }

    [[nodiscard]] reference operator[](size_type i) noexcept
    {
        return reference{this->locator.element_address(i, this->memory.get()), this->fixed_sizes};
    }

    [[nodiscard]] const_reference operator[](size_type i) const noexcept
    {
        return const_reference{this->locator.element_address(i, this->memory.get()), this->fixed_sizes};
    }

    [[nodiscard]] reference front() noexcept { return (*this)[{}]; }

    [[nodiscard]] const_reference front() const noexcept { return (*this)[{}]; }

    [[nodiscard]] reference back() noexcept { return (*this)[this->size() - size_type{1}]; }

    [[nodiscard]] const_reference back() const noexcept { return (*this)[this->size() - size_type{1}]; }

    template <std::size_t I>
    [[nodiscard]] constexpr size_type get_fixed_size() const noexcept
    {
        return std::get<I>(this->fixed_sizes);
    }

    [[nodiscard]] constexpr bool empty() const noexcept { return this->locator.empty(this->memory.get()); }

    [[nodiscard]] constexpr std::byte* data() noexcept { return this->locator.element_address({}, this->memory.get()); }

    [[nodiscard]] constexpr const std::byte* data() const noexcept
    {
        return this->locator.element_address({}, this->memory.get());
    }

    [[nodiscard]] constexpr std::byte* data_begin() noexcept { return this->data(); }

    [[nodiscard]] constexpr const std::byte* data_begin() const noexcept { return this->data(); }

    [[nodiscard]] constexpr std::byte* data_end() noexcept { return this->locator.data_end(); }

    [[nodiscard]] constexpr const std::byte* data_end() const noexcept { return this->locator.data_end(); }

    [[nodiscard]] constexpr size_type size() const noexcept { return this->locator.size(this->memory.get()); }

    [[nodiscard]] constexpr size_type capacity() const noexcept { return this->max_element_count; }

    [[nodiscard]] constexpr size_type memory_consumption() const noexcept { return this->memory.size(); }

    [[nodiscard]] constexpr iterator begin() noexcept { return iterator{*this}; }

    [[nodiscard]] constexpr const_iterator begin() const noexcept { return const_iterator{*this}; }

    [[nodiscard]] constexpr const_iterator cbegin() const noexcept { return this->begin(); }

    [[nodiscard]] constexpr iterator end() noexcept { return iterator{*this, this->size()}; }

    [[nodiscard]] constexpr const_iterator end() const noexcept { return const_iterator{*this, this->size()}; }

    [[nodiscard]] constexpr const_iterator cend() const noexcept { return this->end(); }

    [[nodiscard]] constexpr allocator_type get_allocator() const noexcept { return this->memory.get_allocator(); }

    template <class TAllocator>
    [[nodiscard]] constexpr auto operator==(const cntgs::BasicContiguousVector<TAllocator, Types...>& other) const
    {
        return this->equal(other);
    }

    template <class TAllocator>
    [[nodiscard]] constexpr auto operator!=(const cntgs::BasicContiguousVector<TAllocator, Types...>& other) const
    {
        return !(*this == other);
    }

    template <class TAllocator>
    [[nodiscard]] constexpr auto operator<(const cntgs::BasicContiguousVector<TAllocator, Types...>& other) const
    {
        return this->lexicographical_compare(other);
    }

    template <class TAllocator>
    [[nodiscard]] constexpr auto operator<=(const cntgs::BasicContiguousVector<TAllocator, Types...>& other) const
    {
        return !(other < *this);
    }

    template <class TAllocator>
    [[nodiscard]] constexpr auto operator>(const cntgs::BasicContiguousVector<TAllocator, Types...>& other) const
    {
        return other < *this;
    }

    template <class TAllocator>
    [[nodiscard]] constexpr auto operator>=(const cntgs::BasicContiguousVector<TAllocator, Types...>& other) const
    {
        return !(*this < other);
    }

    // private API
  private:
    BasicContiguousVector(std::byte* memory, size_type memory_size, bool is_memory_owned, size_type max_element_count,
                          const FixedSizes& fixed_sizes, const allocator_type& allocator)
        : max_element_count(max_element_count),
          memory(memory, memory_size, is_memory_owned, allocator),
          fixed_sizes(fixed_sizes),
          locator(max_element_count, this->memory.get(), fixed_sizes)
    {
    }

    BasicContiguousVector(size_type max_element_count, size_type varying_size_bytes, const FixedSizes& fixed_sizes,
                          const allocator_type& allocator)
        : max_element_count(max_element_count),
          memory(Self::calculate_needed_memory_size(max_element_count, varying_size_bytes, fixed_sizes), allocator),
          fixed_sizes(fixed_sizes),
          locator(max_element_count, this->memory.get(), fixed_sizes)
    {
    }

    BasicContiguousVector(const cntgs::TypeErasedVector& vector, bool is_memory_owned) noexcept
        : max_element_count(vector.max_element_count),
          memory(vector.memory, vector.memory_size, is_memory_owned,
                 *std::launder(reinterpret_cast<const allocator_type*>(&vector.allocator))),
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
        const auto new_memory_size =
            this->calculate_needed_memory_size(new_max_element_count, new_varying_size_bytes, this->fixed_sizes);
        StorageType new_memory{new_memory_size, this->get_allocator()};
        this->template insert_into<true, true>(new_max_element_count, new_memory, this->locator);
        this->max_element_count = new_max_element_count;
        this->memory.get_impl().get() = new_memory.release();
        this->memory.get_impl().size() = new_memory_size;
    }

    template <bool UseMove, bool IsDestruct = false>
    void insert_into(size_type new_max_element_count, StorageType& new_memory, ElementLocator& locator)
    {
        static constexpr auto IS_TRIVIAL =
            UseMove ? ListTraits::IS_TRIVIALLY_MOVE_CONSTRUCTIBLE : ListTraits::IS_TRIVIALLY_COPY_CONSTRUCTIBLE;
        if constexpr (IS_TRIVIAL && ListTraits::IS_TRIVIALLY_DESTRUCTIBLE)
        {
            locator.copy_into(this->max_element_count, this->memory.get(), new_max_element_count, new_memory.get());
        }
        else
        {
            ElementLocator new_locator{locator, this->max_element_count, this->memory.get(), new_max_element_count,
                                       new_memory.get()};
            this->template uninitialized_construct_if_non_trivial<UseMove>(new_memory.get(), new_locator);
            if constexpr (IsDestruct)
            {
                this->destruct();
            }
            locator = new_locator;
        }
    }

    template <bool UseMove>
    void uninitialized_construct_if_non_trivial([[maybe_unused]] std::byte* new_memory,
                                                [[maybe_unused]] ElementLocator& new_locator)
    {
        static constexpr auto IS_TRIVIAL =
            UseMove ? ListTraits::IS_TRIVIALLY_MOVE_CONSTRUCTIBLE : ListTraits::IS_TRIVIALLY_COPY_CONSTRUCTIBLE;
        if constexpr (!IS_TRIVIAL)
        {
            for (size_type i{}; i < this->size(); ++i)
            {
                auto&& source = (*this)[i];
                auto&& target =
                    ElementTraits::load_element_at(new_locator.element_address(i, new_memory), this->fixed_sizes);
                ElementTraits::template construct_if_non_trivial<UseMove>(source, target);
            }
        }
    }

    void move_elements_forward_to(const iterator& position, [[maybe_unused]] std::size_t from,
                                  [[maybe_unused]] std::byte* from_data_begin)
    {
        if constexpr (ListTraits::IS_TRIVIALLY_MOVE_CONSTRUCTIBLE && ListTraits::IS_TRIVIALLY_DESTRUCTIBLE &&
                      ListTraits::IS_FIXED_SIZE_OR_PLAIN)
        {
            const auto target = position->data_begin();
            std::memmove(target, from_data_begin, (this->memory.get() + this->memory_consumption()) - from_data_begin);
        }
        else
        {
            for (auto i = position.index(); from != this->size(); ++i, (void)++from)
            {
                this->emplace_at(i, (*this)[from], ListTraits::make_index_sequence());
            }
        }
    }

    void move_elements_forward_to(std::size_t where, std::size_t from)
    {
        if constexpr (ListTraits::IS_TRIVIALLY_MOVE_CONSTRUCTIBLE && ListTraits::IS_TRIVIALLY_DESTRUCTIBLE)
        {
            this->locator.move_elements_forward_to(where, from, this->memory.get());
        }
        else
        {
            for (auto i = where; from != this->size(); ++i, (void)++from)
            {
                this->emplace_at(i, (*this)[from], ListTraits::make_index_sequence());
            }
        }
    }

    template <std::size_t... I>
    void emplace_at(std::size_t i, const reference& element, std::index_sequence<I...>)
    {
        this->locator.emplace_at(i, this->memory.get(), this->fixed_sizes, std::move(cntgs::get<I>(element))...);
        ElementTraits::destruct(element);
    }

    void steal(BasicContiguousVector&& other)
    {
        this->destruct();
        this->max_element_count = other.max_element_count;
        this->memory = std::move(other.memory);
        this->fixed_sizes = other.fixed_sizes;
        this->locator = other.locator;
    }

    void move_assign(BasicContiguousVector&& other)
    {
        if constexpr (AllocatorTraits::is_always_equal::value ||
                      AllocatorTraits::propagate_on_container_move_assignment::value)
        {
            this->steal(std::move(other));
        }
        else
        {
            if (this->get_allocator() == other.get_allocator())
            {
                this->steal(std::move(other));
            }
            else
            {
                auto other_locator = other.locator;
                if (other.memory_consumption() > this->memory_consumption())
                {
                    // allocate memory first because it may throw
                    StorageType new_memory{other.memory_consumption(), this->get_allocator()};
                    this->destruct();
                    other.template insert_into<true>(other.max_element_count, new_memory, other_locator);
                    this->memory = std::move(new_memory);
                }
                else
                {
                    this->destruct();
                    other.template insert_into<true>(other.max_element_count, this->memory, other_locator);
                }
                this->max_element_count = other.max_element_count;
                this->fixed_sizes = other.fixed_sizes;
                this->locator = other_locator;
            }
        }
    }

    auto copy_construct_locator(const BasicContiguousVector& other)
    {
        auto other_locator = other.locator;
        const_cast<BasicContiguousVector&>(other).template insert_into<false>(other.max_element_count, this->memory,
                                                                              other_locator);
        return other_locator;
    }

    void copy_assign(const BasicContiguousVector& other)
    {
        this->destruct();
        this->memory = other.memory;
        auto other_locator = other.locator;
        const_cast<BasicContiguousVector&>(other).template insert_into<false>(other.max_element_count, this->memory,
                                                                              other_locator);
        this->max_element_count = other.max_element_count;
        this->fixed_sizes = other.fixed_sizes;
        this->locator = other_locator;
    }

    template <class TAllocator>
    constexpr auto equal(const cntgs::BasicContiguousVector<TAllocator, Types...>& other) const
    {
        if constexpr (ListTraits::IS_EQUALITY_MEMCMPABLE)
        {
            if (this->empty())
            {
                return other.empty();
            }
            if (other.empty())
            {
                return false;
            }
            return detail::trivial_equal(this->data_begin(), this->data_end(), other.data_begin(), other.data_end());
        }
        else
        {
            return std::equal(this->begin(), this->end(), other.begin());
        }
    }
    template <class TAllocator>
    constexpr auto lexicographical_compare(const cntgs::BasicContiguousVector<TAllocator, Types...>& other) const
    {
        if constexpr (ListTraits::IS_LEXICOGRAPHICAL_MEMCMPABLE && ListTraits::IS_FIXED_SIZE_OR_PLAIN)
        {
            if (this->empty())
            {
                return !other.empty();
            }
            if (other.empty())
            {
                return false;
            }
            return detail::trivial_lexicographical_compare(this->data_begin(), this->data_end(), other.data_begin(),
                                                           other.data_end());
        }
        else
        {
            return std::lexicographical_compare(this->begin(), this->end(), other.begin(), other.end());
        }
    }

    void destruct_if_owned() noexcept
    {
        if (this->memory && this->memory.is_owned())
        {
            this->destruct();
        }
    }

    void destruct() noexcept { this->destruct(this->begin(), this->end()); }

    void destruct([[maybe_unused]] iterator first, [[maybe_unused]] iterator last) noexcept
    {
        if constexpr (!ListTraits::IS_TRIVIALLY_DESTRUCTIBLE)
        {
            std::for_each(first, last, ElementTraits::destruct);
        }
    }
};

template <class Allocator, class... T>
auto type_erase(cntgs::BasicContiguousVector<Allocator, T...>&& vector) noexcept
{
    return cntgs::TypeErasedVector{
        vector.memory.size(),
        vector.max_element_count,
        vector.memory.release(),
        vector.memory.is_owned(),
        detail::type_erase_allocator(vector.get_allocator()),
        detail::convert_array_to_size<detail::MAX_FIXED_SIZE_VECTOR_PARAMETER>(vector.fixed_sizes),
        detail::type_erase_element_locator(vector.locator),
        []([[maybe_unused]] cntgs::TypeErasedVector& erased)
        {
            cntgs::BasicContiguousVector<Allocator, T...>{std::move(erased)};
        }};
}
}  // namespace cntgs

#endif  // CNTGS_CONTIGUOUS_VECTOR_HPP
