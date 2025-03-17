// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_CNTGS_VECTOR_HPP
#define CNTGS_CNTGS_VECTOR_HPP

#include "cntgs/detail/algorithm.hpp"
#include "cntgs/detail/allocator.hpp"
#include "cntgs/detail/array.hpp"
#include "cntgs/detail/elementLocator.hpp"
#include "cntgs/detail/forward.hpp"
#include "cntgs/detail/memory.hpp"
#include "cntgs/detail/optionsParser.hpp"
#include "cntgs/detail/parameterListTraits.hpp"
#include "cntgs/detail/parameterTraits.hpp"
#include "cntgs/detail/storage.hpp"
#include "cntgs/detail/utility.hpp"
#include "cntgs/detail/vectorTraits.hpp"
#include "cntgs/element.hpp"
#include "cntgs/iterator.hpp"
#include "cntgs/parameter.hpp"
#include "cntgs/reference.hpp"
#include "cntgs/span.hpp"

#include <algorithm>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace cntgs
{
/// Alias template for [cntgs::BasicContiguousVector]() that uses [std::allocator]()
// begin-snippet: contiguous-vector-definition
template <class... Parameter>
using ContiguousVector = cntgs::BasicContiguousVector<cntgs::Options<>, Parameter...>;
// end-snippet

/// Container that stores the value of each specified parameter contiguously.
///
/// \param Option Any of [cntgs::Allocator]() wrapped into [cntgs::Options]().
/// \param Parameter Any of [cntgs::VaryingSize](), [cntgs::FixedSize](), [cntgs::AlignAs]() or a plain user-defined or
/// built-in type. The underlying type of each parameter must satisfy
/// [Erasable](https://en.cppreference.com/w/cpp/named_req/Erasable).
template <class... Option, class... Parameter>
class BasicContiguousVector<cntgs::Options<Option...>, Parameter...>
{
  private:
    using Self = cntgs::BasicContiguousVector<cntgs::Options<Option...>, Parameter...>;
    using ParsedOptions = detail::OptionsParser<Option...>;
    using ListTraits = detail::ParameterListTraits<Parameter...>;
    using VectorTraits = detail::ContiguousVectorTraits<Parameter...>;
    using ElementTraits = detail::ElementTraitsT<Parameter...>;
    using StorageElementType = typename ElementTraits::StorageElementType;
    using Allocator =
        typename std::allocator_traits<typename ParsedOptions::Allocator>::template rebind_alloc<StorageElementType>;
    using ElementLocator = detail::ElementLocatorT<Parameter...>;
    using ElementLocatorAndFixedSizes = detail::ElementLocatorAndFixedSizes<Parameter...>;
    using AllocatorTraits = std::allocator_traits<Allocator>;
    using StorageType = detail::Storage<ElementTraits::FIRST_ELEMENT_HAS_OFFSET, Allocator>;
    using FixedSizes = typename ListTraits::FixedSizes;
    using FixedSizesArray = typename ListTraits::FixedSizesArray;

    static_assert(ListTraits::template ParameterTraitsAt<0>::TYPE != detail::ParameterType::VARYING_SIZE,
                  "VaryingSize must be preceded by a parameter that represents its size");

    static constexpr bool IS_MIXED = ListTraits::IS_MIXED;
    static constexpr bool IS_ALL_FIXED_SIZE = ListTraits::IS_ALL_FIXED_SIZE;
    static constexpr bool IS_ALL_VARYING_SIZE = ListTraits::IS_ALL_VARYING_SIZE;
    static constexpr bool IS_ALL_PLAIN = ListTraits::IS_ALL_PLAIN;

  public:
    /// Type that can create copies of [cntgs::BasicContiguousVector::reference]() and
    /// [cntgs::BasicContiguousVector::const_reference]()
    using value_type = cntgs::BasicContiguousElement<Allocator, Parameter...>;

    /// A [cntgs::ContiguousReference]()
    /// \exclude target
    using reference = typename VectorTraits::ReferenceType;

    /// A [cntgs::ContiguousConstReference]()
    /// \exclude target
    using const_reference = typename VectorTraits::ConstReferenceType;
    using iterator = cntgs::ContiguousVectorIterator<false, cntgs::Options<Option...>, Parameter...>;
    using const_iterator = cntgs::ContiguousVectorIterator<true, cntgs::Options<Option...>, Parameter...>;
    using difference_type = std::ptrdiff_t;
    using size_type = std::size_t;
    using allocator_type = Allocator;

    size_type max_element_count_{};
    StorageType memory_{};
    ElementLocatorAndFixedSizes locator_;

    BasicContiguousVector() = default;

    template <bool IsMixed = IS_MIXED>
    BasicContiguousVector(size_type max_element_count, size_type varying_size_bytes, const FixedSizes& fixed_sizes,
                          const allocator_type& allocator = {}, std::enable_if_t<IsMixed>* = nullptr)
        : BasicContiguousVector(max_element_count, varying_size_bytes, fixed_sizes, allocator, 0)
    {
    }

    template <bool IsAllFixedSize = IS_ALL_FIXED_SIZE>
    constexpr BasicContiguousVector(size_type max_element_count, const FixedSizes& fixed_sizes,
                                    const allocator_type& allocator = {}, std::enable_if_t<IsAllFixedSize>* = nullptr)
        : BasicContiguousVector(max_element_count, size_type{}, fixed_sizes, allocator, 0)
    {
    }

    template <bool IsAllVaryingSize = IS_ALL_VARYING_SIZE>
    BasicContiguousVector(size_type max_element_count, size_type varying_size_bytes,
                          const allocator_type& allocator = {}, std::enable_if_t<IsAllVaryingSize>* = nullptr)
        : BasicContiguousVector(max_element_count, varying_size_bytes, FixedSizes{}, allocator, 0)
    {
    }

    template <bool IsNoneSpecial = IS_ALL_PLAIN>
    constexpr explicit BasicContiguousVector(size_type max_element_count, std::enable_if_t<IsNoneSpecial>* = nullptr)
        : BasicContiguousVector(max_element_count, size_type{}, FixedSizes{}, allocator_type{}, 0)
    {
    }

    template <bool IsNoneSpecial = IS_ALL_PLAIN>
    constexpr BasicContiguousVector(size_type max_element_count, const allocator_type& allocator,
                                    std::enable_if_t<IsNoneSpecial>* = nullptr)
        : BasicContiguousVector(max_element_count, size_type{}, FixedSizes{}, allocator)
    {
    }

    BasicContiguousVector(const BasicContiguousVector& other)
        : max_element_count_(other.max_element_count_), memory_(other.memory_), locator_(copy_construct_locator(other))
    {
    }

    BasicContiguousVector(BasicContiguousVector&&) = default;

    BasicContiguousVector& operator=(const BasicContiguousVector& other)
    {
        if (this != std::addressof(other))
        {
            copy_assign(other);
        }
        return *this;
    }

    constexpr BasicContiguousVector& operator=(BasicContiguousVector&& other) noexcept(
        AllocatorTraits::is_always_equal::value || AllocatorTraits::propagate_on_container_move_assignment::value)
    {
        if (this != std::addressof(other))
        {
            move_assign(std::move(other));
        }
        return *this;
    }

    ~BasicContiguousVector() noexcept { destruct_if_owned(); }

    template <class... Args>
    void emplace_back(Args&&... args)
    {
        emplace_back_impl(static_cast<Args&&>(args)...);
    }

    template <class... Args>
    iterator emplace(const_iterator position, Args&&... args)
    {
        auto it = make_iterator(position);
        const auto target_begin = it.data();
        const auto back_begin = data_end();
        const auto back_end = emplace_back_impl(static_cast<Args&&>(args)...);
        const auto byte_count = back_end - back_begin;
        make_room_for_last_element_at(it.index(), byte_count);
        std::memcpy(target_begin, back_end, byte_count);
        auto&& source = (*this)[size()];
        auto&& target = ElementTraits::load_element_at(target_begin, locator_.fixed_sizes());
        ElementTraits::template construct_if_non_trivial<true>(source, target);
        return std::next(begin(), it.index());
    }

    void pop_back() noexcept
    {
        ElementTraits::destruct(back());
        locator_->resize(size() - size_type{1}, memory_begin());
    }

    void reserve(size_type new_max_element_count, size_type new_varying_size_bytes = {})
    {
        if (max_element_count_ < new_max_element_count)
        {
            grow(new_max_element_count, new_varying_size_bytes);
        }
    }

    iterator erase(const_iterator position) noexcept(ListTraits::IS_NOTHROW_MOVE_CONSTRUCTIBLE)
    {
        auto it_position = make_iterator(position);
        const auto next_position = position.index() + 1;
        ElementTraits::destruct(*it_position);
        move_elements_forward(next_position, it_position.index());
        locator_->resize(size() - size_type{1}, memory_begin());
        return it_position;
    }

    iterator erase(const_iterator first, const_iterator last) noexcept(ListTraits::IS_NOTHROW_MOVE_CONSTRUCTIBLE)
    {
        const auto current_size = size();
        const auto it_first = make_iterator(first);
        const auto it_last = make_iterator(last);
        BasicContiguousVector::destruct(it_first, it_last);
        if (last.index() < current_size && first.index() != last.index())
        {
            move_elements_forward(last.index(), first.index());
        }
        locator_->resize(current_size - (last.index() - first.index()), memory_begin());
        return it_first;
    }

    void clear() noexcept
    {
        destruct();
        locator_->resize(0, memory_begin());
    }

    [[nodiscard]] reference operator[](size_type i) noexcept
    {
        return reference{locator_->element_address(i, memory_begin()), locator_.fixed_sizes()};
    }

    [[nodiscard]] const_reference operator[](size_type i) const noexcept
    {
        return const_reference{locator_->element_address(i, memory_begin()), locator_.fixed_sizes()};
    }

    [[nodiscard]] reference front() noexcept { return (*this)[{}]; }

    [[nodiscard]] const_reference front() const noexcept { return (*this)[{}]; }

    [[nodiscard]] reference back() noexcept { return (*this)[size() - size_type{1}]; }

    [[nodiscard]] const_reference back() const noexcept { return (*this)[size() - size_type{1}]; }

    template <std::size_t I>
    [[nodiscard]] constexpr size_type get_fixed_size() const noexcept
    {
        return detail::get<I>(locator_.fixed_sizes());
    }

    [[nodiscard]] constexpr bool empty() const noexcept { return locator_->empty(memory_begin()); }

    [[nodiscard]] std::byte* allocation_begin() const noexcept { return memory_.allocation_begin(); }

    [[nodiscard]] constexpr std::byte* data() noexcept { return locator_->element_address({}, memory_begin()); }

    [[nodiscard]] constexpr const std::byte* data() const noexcept
    {
        return locator_->element_address({}, memory_begin());
    }

    [[nodiscard]] constexpr std::byte* data_begin() noexcept { return data(); }

    [[nodiscard]] constexpr const std::byte* data_begin() const noexcept { return data(); }

    [[nodiscard]] constexpr std::byte* data_end() noexcept
    {
        return const_cast<std::byte*>(static_cast<const BasicContiguousVector&>(*this).data_end());
    }

    [[nodiscard]] constexpr const std::byte* data_end() const noexcept
    {
        if (!empty())
        {
            return back().data_end();
        }
        return nullptr;
    }

    [[nodiscard]] constexpr size_type size() const noexcept { return locator_->size(memory_begin()); }

    [[nodiscard]] constexpr size_type capacity() const noexcept { return max_element_count_; }

    [[nodiscard]] constexpr size_type memory_consumption() const noexcept
    {
        return memory_.size() * alignof(StorageElementType);
    }

    [[nodiscard]] constexpr iterator begin() noexcept { return iterator{*this}; }

    [[nodiscard]] constexpr const_iterator begin() const noexcept { return const_iterator{*this}; }

    [[nodiscard]] constexpr const_iterator cbegin() const noexcept { return begin(); }

    [[nodiscard]] constexpr iterator end() noexcept { return iterator{*this, size()}; }

    [[nodiscard]] constexpr const_iterator end() const noexcept { return const_iterator{*this, size()}; }

    [[nodiscard]] constexpr const_iterator cend() const noexcept { return end(); }

    [[nodiscard]] constexpr allocator_type get_allocator() const noexcept { return memory_.get_allocator(); }

    friend constexpr void swap(BasicContiguousVector& lhs, BasicContiguousVector& rhs) noexcept
    {
        using std::swap;
        swap(lhs.max_element_count_, rhs.max_element_count_);
        swap(lhs.memory_, rhs.memory_);
        swap(lhs.locator_, rhs.locator_);
    }

    template <class... TOption>
    [[nodiscard]] constexpr auto operator==(
        const cntgs::BasicContiguousVector<cntgs::Options<TOption...>, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTHROW_EQUALITY_COMPARABLE)
    {
        return equal(other);
    }

    template <class... TOption>
    [[nodiscard]] constexpr auto operator!=(
        const cntgs::BasicContiguousVector<cntgs::Options<TOption...>, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTHROW_EQUALITY_COMPARABLE)
    {
        return !(*this == other);
    }

    template <class... TOption>
    [[nodiscard]] constexpr auto operator<(
        const cntgs::BasicContiguousVector<cntgs::Options<TOption...>, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return lexicographical_compare(other);
    }

    template <class... TOption>
    [[nodiscard]] constexpr auto operator<=(
        const cntgs::BasicContiguousVector<cntgs::Options<TOption...>, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return !(other < *this);
    }

    template <class... TOption>
    [[nodiscard]] constexpr auto operator>(
        const cntgs::BasicContiguousVector<cntgs::Options<TOption...>, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return other < *this;
    }

    template <class... TOption>
    [[nodiscard]] constexpr auto operator>=(
        const cntgs::BasicContiguousVector<cntgs::Options<TOption...>, Parameter...>& other) const
        noexcept(ListTraits::IS_NOTRHOW_LEXICOGRAPHICAL_COMPARABLE)
    {
        return !(*this < other);
    }

    // private API
  private:
    template <bool, class, class...>
    friend class cntgs::ContiguousVectorIterator;

    constexpr BasicContiguousVector(size_type max_element_count, size_type varying_size_bytes,
                                    const FixedSizes& fixed_sizes, const allocator_type& allocator, int)
        : BasicContiguousVector(max_element_count, varying_size_bytes, FixedSizesArray{fixed_sizes}, allocator,
                                ElementTraits::calculate_element_size(FixedSizesArray{fixed_sizes}))
    {
    }

    constexpr BasicContiguousVector(size_type max_element_count, size_type varying_size_bytes,
                                    const FixedSizesArray& fixed_sizes, const allocator_type& allocator,
                                    detail::ElementSize size)
        : max_element_count_(max_element_count),
          memory_(
              allocate_memory(ElementTraits::calculate_needed_memory_size(max_element_count, varying_size_bytes, size),
                              size.distance_to_first, allocator)),
          locator_(max_element_count, memory_begin(), fixed_sizes, size, allocator)
    {
    }

    [[nodiscard]] std::byte* memory_begin() const noexcept { return memory_.get(); }

    static StorageType allocate_memory(size_type size, size_type distance_to_first, const allocator_type& allocator)
    {
        auto storage = ElementTraits::template allocate_memory<StorageType>(size, allocator);
        storage.add_offset(distance_to_first);
        return storage;
    }

    template <class... Args>
    auto emplace_back_impl(Args&&... args)
    {
        return locator_->emplace_back(memory_begin(), locator_.fixed_sizes(), static_cast<Args&&>(args)...);
    }

    void grow(size_type new_max_element_count, size_type new_varying_size_bytes)
    {
        const auto new_memory_size = locator_->calculate_new_memory_size(new_max_element_count, new_varying_size_bytes,
                                                                         memory_.get_offset(), locator_.fixed_sizes());
        auto new_memory = allocate_memory(new_memory_size, memory_.get_offset(), get_allocator());
        ElementLocator other_locator{*locator_,        memory_begin(),        max_element_count_,
                                     new_memory.get(), new_max_element_count, get_allocator()};
        insert_into<true>(other_locator, new_max_element_count, new_memory, *this);
        max_element_count_ = new_max_element_count;
        *locator_ = std::move(other_locator);
        memory_.reset(std::move(new_memory));
    }

    template <bool IsDestruct = false, class Self = BasicContiguousVector>
    static void insert_into(ElementLocator& locator, size_type, StorageType& new_memory, Self& from)
    {
        static constexpr auto USE_MOVE = !std::is_const_v<Self>;
        static constexpr auto IS_TRIVIAL =
            USE_MOVE ? ListTraits::IS_TRIVIALLY_MOVE_CONSTRUCTIBLE : ListTraits::IS_TRIVIALLY_COPY_CONSTRUCTIBLE;
        auto* const mem = new_memory.get();
        if constexpr (IS_TRIVIAL && (!IsDestruct || ListTraits::IS_TRIVIALLY_DESTRUCTIBLE))
        {
            from.locator_->trivially_copy_into(from.memory_begin(), mem);
        }
        else
        {
            std::memcpy(mem, from.memory_begin(), from.data_end() - from.memory_begin());
            BasicContiguousVector::uninitialized_construct_if_non_trivial<USE_MOVE>(from, mem, locator);
            if constexpr (IsDestruct)
            {
                from.destruct();
            }
        }
    }

    template <bool UseMove, class Self>
    static void uninitialized_construct_if_non_trivial(Self& self, [[maybe_unused]] std::byte* new_memory,
                                                       [[maybe_unused]] ElementLocator& new_locator)
    {
        static constexpr auto IS_TRIVIAL =
            UseMove ? ListTraits::IS_TRIVIALLY_MOVE_CONSTRUCTIBLE : ListTraits::IS_TRIVIALLY_COPY_CONSTRUCTIBLE;
        if constexpr (!IS_TRIVIAL)
        {
            for (size_type i{}; i < self.size(); ++i)
            {
                auto&& source = self[i];
                auto&& target = ElementTraits::template load_element_at<detail::ContiguousReferenceSizeGetter>(
                    new_locator.element_address(i, new_memory), source);
                ElementTraits::template construct_if_non_trivial<UseMove>(source, target);
            }
        }
    }

    void move_elements_forward(std::size_t from, std::size_t to)
    {
        if constexpr (ListTraits::IS_TRIVIALLY_MOVE_CONSTRUCTIBLE && ListTraits::IS_TRIVIALLY_DESTRUCTIBLE)
        {
            locator_->move_elements_forward(from, to, memory_begin());
        }
        else
        {
            for (auto i = to; from != size(); ++i, (void)++from)
            {
                emplace_at(i, (*this)[from], ListTraits::make_index_sequence());
            }
        }
    }

    void make_room_for_last_element_at(std::size_t from, [[maybe_unused]] std::size_t bytes)
    {
        if constexpr (ListTraits::IS_TRIVIALLY_MOVE_CONSTRUCTIBLE && ListTraits::IS_TRIVIALLY_DESTRUCTIBLE)
        {
            locator_->make_room_for_last_element_at(from, bytes, memory_begin());
        }
        else
        {
            for (auto i = size(); i != from; --i)
            {
                emplace_at(i, (*this)[i - std::size_t{1}], ListTraits::make_index_sequence());
            }
        }
    }

    template <std::size_t... I>
    void emplace_at(std::size_t i, const reference& element, std::index_sequence<I...>)
    {
        locator_->emplace_at(i, memory_begin(), locator_.fixed_sizes(), std::move(cntgs::get<I>(element))...);
        ElementTraits::destruct(element);
    }

    constexpr void steal(BasicContiguousVector&& other) noexcept
    {
        destruct();
        max_element_count_ = other.max_element_count_;
        memory_ = std::move(other.memory_);
        locator_ = std::move(other.locator_);
    }

    constexpr void move_assign(BasicContiguousVector&& other)
    {
        if constexpr (AllocatorTraits::is_always_equal::value ||
                      AllocatorTraits::propagate_on_container_move_assignment::value)
        {
            steal(std::move(other));
        }
        else
        {
            if (get_allocator() == other.get_allocator())
            {
                steal(std::move(other));
            }
            else
            {
                if (other.memory_consumption() > memory_consumption())
                {
                    // allocate memory first because it might throw
                    StorageType new_memory{other.memory_consumption(), get_allocator()};
                    ElementLocatorAndFixedSizes other_locator{other.locator_,           other.memory_begin(),
                                                              other.max_element_count_, new_memory.get(),
                                                              other.max_element_count_, get_allocator()};
                    destruct();
                    BasicContiguousVector::insert_into(*other_locator, other.max_element_count_, new_memory, other);
                    memory_ = std::move(new_memory);
                    locator_ = std::move(other_locator);
                }
                else
                {
                    ElementLocatorAndFixedSizes other_locator{other.locator_,           other.memory_begin(),
                                                              other.max_element_count_, memory_begin(),
                                                              other.max_element_count_, get_allocator()};
                    destruct();
                    BasicContiguousVector::insert_into(*other_locator, other.max_element_count_, memory_, other);
                    locator_ = std::move(other_locator);
                }
                max_element_count_ = other.max_element_count_;
            }
        }
    }

    auto copy_construct_locator(const BasicContiguousVector& other)
    {
        ElementLocatorAndFixedSizes other_locator{other.locator_, other.memory_begin(),     other.max_element_count_,
                                                  memory_begin(), other.max_element_count_, get_allocator()};
        BasicContiguousVector::insert_into(*other_locator, other.max_element_count_, memory_, other);
        return other_locator;
    }

    void copy_assign(const BasicContiguousVector& other)
    {
        destruct();
        memory_ = other.memory_;
        ElementLocatorAndFixedSizes other_locator{other.locator_, other.memory_begin(),     other.max_element_count_,
                                                  memory_begin(), other.max_element_count_, get_allocator()};
        BasicContiguousVector::insert_into(*other_locator, other.max_element_count_, memory_, other);
        max_element_count_ = other.max_element_count_;
        locator_ = std::move(other_locator);
    }

    template <class... TOption>
    constexpr auto equal(const cntgs::BasicContiguousVector<cntgs::Options<TOption...>, Parameter...>& other) const
    {
        if constexpr (ListTraits::IS_EQUALITY_MEMCMPABLE)
        {
            if (empty())
            {
                return other.empty();
            }
            if (other.empty())
            {
                return false;
            }
            return detail::trivial_equal(data_begin(), data_end(), other.data_begin(), other.data_end());
        }
        else
        {
            return std::equal(begin(), end(), other.begin());
        }
    }
    template <class... TOption>
    constexpr auto lexicographical_compare(
        const cntgs::BasicContiguousVector<cntgs::Options<TOption...>, Parameter...>& other) const
    {
        if constexpr (ListTraits::IS_LEXICOGRAPHICAL_MEMCMPABLE && ListTraits::IS_FIXED_SIZE_OR_PLAIN)
        {
            if (empty())
            {
                return !other.empty();
            }
            if (other.empty())
            {
                return false;
            }
            return detail::trivial_lexicographical_compare(data_begin(), data_end(), other.data_begin(),
                                                           other.data_end());
        }
        else
        {
            return std::lexicographical_compare(begin(), end(), other.begin(), other.end());
        }
    }

    constexpr iterator make_iterator(const const_iterator& it) noexcept { return {*this, it.index()}; }

    constexpr void destruct_if_owned() noexcept
    {
        if (memory_)
        {
            destruct();
        }
    }

    constexpr void destruct() noexcept { BasicContiguousVector::destruct(begin(), end()); }

    static constexpr void destruct([[maybe_unused]] iterator first, [[maybe_unused]] iterator last) noexcept
    {
        if constexpr (!ListTraits::IS_TRIVIALLY_DESTRUCTIBLE)
        {
            std::for_each(first, last, ElementTraits::destruct);
        }
    }
};
}  // namespace cntgs

#endif  // CNTGS_CNTGS_VECTOR_HPP
