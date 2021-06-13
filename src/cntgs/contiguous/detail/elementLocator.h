#pragma once

#include "cntgs/contiguous/detail/attributes.h"
#include "cntgs/contiguous/detail/forward.h"
#include "cntgs/contiguous/detail/math.h"
#include "cntgs/contiguous/detail/memory.h"
#include "cntgs/contiguous/detail/parameterTraits.h"
#include "cntgs/contiguous/detail/typeUtils.h"
#include "cntgs/contiguous/detail/vectorTraits.h"

#include <array>
#include <type_traits>

namespace cntgs::detail
{
template <std::size_t Alignment>
static constexpr auto alignment_offset([[maybe_unused]] std::size_t position) noexcept
{
    if constexpr (Alignment == 0)
    {
        return std::size_t{};
    }
    else
    {
        return detail::align(Alignment, position) - position;
    }
}

template <class, class...>
struct BaseElementLocator;

template <std::size_t... I, class... Types>
struct BaseElementLocator<std::index_sequence<I...>, Types...>
{
    using Traits = detail::ContiguousVectorTraits<Types...>;
    using SizeType = typename Traits::SizeType;

    template <class T>
    using FixedSizeGetter = typename Traits::template FixedSizeGetter<T>;

    template <std::size_t K>
    using TypeAt = std::tuple_element_t<K, std::tuple<Types...>>;

    template <class NeedsAlignmentSelector, std::size_t N, class... Args>
    CNTGS_RESTRICT_RETURN static std::byte* emplace_back(std::byte* CNTGS_RESTRICT last_element,
                                                         const std::array<SizeType, N>& fixed_sizes, Args&&... args)
    {
        ((last_element = detail::ParameterTraits<Types>::template store<NeedsAlignmentSelector::template VALUE<I>>(
              std::forward<Args>(args), last_element, FixedSizeGetter<Types>::template get<I>(fixed_sizes))),
         ...);
        return last_element;
    }

    template <class NeedsAlignmentSelector, std::size_t N>
    static auto load_element_at(std::byte* address, const std::array<SizeType, N>& fixed_sizes) noexcept
    {
        typename Traits::PointerReturnType result;
        ((std::tie(std::get<I>(result), address) =
              detail::ParameterTraits<Types>::template load<NeedsAlignmentSelector::template VALUE<I>>(
                  address, FixedSizeGetter<Types>::template get<I>(fixed_sizes))),
         ...);
        return result;
    }

    template <std::size_t N>
    static constexpr auto calculate_element_size(const std::array<SizeType, N>& fixed_sizes) noexcept
    {
        SizeType result{};
        ((result +=
          detail::ParameterTraits<Types>::aligned_size_in_memory(FixedSizeGetter<Types>::template get<I>(fixed_sizes)) +
          alignment_offset<detail::ParameterTraits<Types>::ALIGNMENT>(result)),
         ...);
        return result + alignment_offset<detail::ParameterTraits<TypeAt<0>>::ALIGNMENT>(result);
    }
};

template <class... Types>
using BaseElementLocatorT = detail::BaseElementLocator<std::make_index_sequence<sizeof...(Types)>, Types...>;

struct DefaultAlignmentSelector
{
    template <std::size_t>
    static constexpr auto VALUE = true;
};

template <class... Types>
class ElementLocator : public detail::BaseElementLocatorT<Types...>
{
  private:
    using Base = detail::BaseElementLocatorT<Types...>;
    using Traits = typename Base::Traits;
    using SizeType = typename Base::SizeType;
    using DifferenceType = std::ptrdiff_t;

    template <class T>
    using FixedSizeGetter = typename Base::template FixedSizeGetter<T>;

    static constexpr auto TYPE_COUNT = sizeof...(Types);
    static constexpr auto RESERVED_BYTES_PER_ELEMENT = sizeof(std::byte*);

    std::byte** last_element_address{};
    std::byte* last_element{};

  public:
    ElementLocator() = default;

    ElementLocator(SizeType max_element_count, std::byte* memory_begin,
                   const std::array<SizeType, Traits::CONTIGUOUS_FIXED_SIZE_COUNT>&) noexcept
        : last_element_address(reinterpret_cast<std::byte**>(memory_begin)),
          last_element(reinterpret_cast<std::byte*>(
              detail::align<Traits::MAX_ALIGNMENT>(memory_begin + ElementLocator::reserved_bytes(max_element_count))))
    {
    }

    ElementLocator(SizeType max_element_count, std::byte* memory_begin, ElementLocator& other,
                   SizeType other_max_element_count, std::byte* other_memory_begin) noexcept
    {
        this->copy_from(max_element_count, memory_begin, other, other_max_element_count, other_memory_begin);
    }

    static constexpr auto reserved_bytes(SizeType element_count) noexcept
    {
        return element_count * RESERVED_BYTES_PER_ELEMENT;
    }

    bool empty(std::byte* memory_begin) const noexcept
    {
        return this->last_element_address == reinterpret_cast<std::byte**>(memory_begin);
    }

    SizeType size(std::byte* memory_begin) const noexcept
    {
        return this->last_element_address - reinterpret_cast<std::byte**>(memory_begin);
    }

    template <std::size_t N, class... Args>
    void emplace_back(const std::array<SizeType, N>& fixed_sizes, Args&&... args)
    {
        *this->last_element_address = last_element;
        ++this->last_element_address;
        last_element = Base::template emplace_back<detail::DefaultAlignmentSelector>(last_element, fixed_sizes,
                                                                                     std::forward<Args>(args)...);
    }

    template <std::size_t N>
    auto load_element_at(SizeType i, std::byte* memory_begin, const std::array<SizeType, N>& fixed_sizes) const noexcept
    {
        return Base::template load_element_at<detail::DefaultAlignmentSelector>(this->at(memory_begin, i), fixed_sizes);
    }

    auto at(std::byte* memory_begin, SizeType index) const noexcept
    {
        const auto element_addresses_begin = reinterpret_cast<std::byte**>(memory_begin);
        return element_addresses_begin[index];
    }

    void copy_from(SizeType new_max_element_count, std::byte* new_memory_begin, SizeType old_max_element_count,
                   std::byte* old_memory_begin) noexcept
    {
        this->copy_from(new_max_element_count, new_memory_begin, *this, old_max_element_count, old_memory_begin);
    }

  private:
    void copy_from(SizeType new_max_element_count, std::byte* new_memory_begin, ElementLocator& old_locator,
                   SizeType old_max_element_count, std::byte* old_memory_begin) noexcept
    {
        auto new_last_element_address = reinterpret_cast<std::byte**>(new_memory_begin);
        const auto size_diff =
            static_cast<DifferenceType>(new_max_element_count) - static_cast<DifferenceType>(old_max_element_count);
        auto old_begin = reinterpret_cast<std::byte**>(old_memory_begin);
        while (old_begin != old_locator.last_element_address)
        {
            *new_last_element_address =
                new_memory_begin + std::distance(old_memory_begin, *old_begin) + size_diff * RESERVED_BYTES_PER_ELEMENT;
            ++new_last_element_address;
            ++old_begin;
        }
        const auto other_used_memory_size = std::distance(old_memory_begin, old_locator.last_element);
        const auto other_reserved_bytes = ElementLocator::reserved_bytes(old_max_element_count);
        std::memcpy(new_memory_begin + ElementLocator::reserved_bytes(new_max_element_count),
                    old_memory_begin + other_reserved_bytes, other_used_memory_size - other_reserved_bytes);
        this->last_element_address = new_last_element_address;
        this->last_element = new_memory_begin + other_used_memory_size + size_diff * RESERVED_BYTES_PER_ELEMENT;
    }
};

struct IgnoreFirstAlignmentSelector
{
    template <std::size_t I>
    static constexpr auto VALUE = I != 0;
};

template <class... Types>
class AllFixedSizeElementLocator : public detail::BaseElementLocatorT<Types...>
{
  private:
    using Base = detail::BaseElementLocatorT<Types...>;
    using Traits = typename Base::Traits;
    using SizeType = typename Base::SizeType;

    template <class T>
    using FixedSizeGetter = typename Base::template FixedSizeGetter<T>;

    static constexpr auto TYPE_COUNT = sizeof...(Types);

    SizeType element_count{};
    SizeType stride{};
    std::byte* start{};

  public:
    AllFixedSizeElementLocator() = default;

    AllFixedSizeElementLocator(SizeType, std::byte* memory_begin,
                               const std::array<SizeType, Traits::CONTIGUOUS_FIXED_SIZE_COUNT>& fixed_sizes) noexcept
        : stride(Base::calculate_element_size(fixed_sizes)),
          start(reinterpret_cast<std::byte*>(detail::align<Traits::MAX_ALIGNMENT>(memory_begin)))
    {
    }

    AllFixedSizeElementLocator(SizeType, std::byte* memory_begin, AllFixedSizeElementLocator& other, SizeType,
                               std::byte*) noexcept
        : element_count(other.element_count), stride(other.stride)
    {
        this->copy_from(memory_begin, other);
    }

    static constexpr auto reserved_bytes(SizeType) noexcept { return SizeType{}; }

    constexpr bool empty(std::byte*) const noexcept { return this->element_count == SizeType{}; }

    constexpr SizeType size(std::byte*) const noexcept { return this->element_count; }

    template <std::size_t N, class... Args>
    void emplace_back(const std::array<SizeType, N>& fixed_sizes, Args&&... args)
    {
        auto last_element = this->at(element_count);
        ++this->element_count;
        Base::template emplace_back<detail::IgnoreFirstAlignmentSelector>(last_element, fixed_sizes,
                                                                          std::forward<Args>(args)...);
    }

    template <std::size_t N>
    auto load_element_at(SizeType i, std::byte*, const std::array<SizeType, N>& fixed_sizes) const noexcept
    {
        return Base::template load_element_at<detail::IgnoreFirstAlignmentSelector>(this->at(i), fixed_sizes);
    }

    constexpr auto at(SizeType index) const noexcept { return start + this->stride * index; }

    void copy_from(SizeType, std::byte* new_memory_begin, SizeType, std::byte*) noexcept
    {
        this->copy_from(new_memory_begin, *this);
    }

  private:
    void copy_from(std::byte* new_memory_begin, AllFixedSizeElementLocator& old) noexcept
    {
        const auto new_start = reinterpret_cast<std::byte*>(detail::align<Traits::MAX_ALIGNMENT>(new_memory_begin));
        std::memcpy(new_start, old.start, old.element_count * old.stride);
        this->start = new_start;
    }
};

template <class... Types>
using ElementLocatorT =
    std::conditional_t<detail::ContiguousVectorTraits<Types...>::CONTIGUOUS_FIXED_SIZE_COUNT ==
                           detail::ContiguousVectorTraits<Types...>::CONTIGUOUS_COUNT,
                       detail::AllFixedSizeElementLocator<Types...>, detail::ElementLocator<Types...>>;

using TypeErasedElementLocator = std::aligned_storage_t<
    detail::MAX_SIZE_T_OF<sizeof(detail::ElementLocator<>), sizeof(detail::AllFixedSizeElementLocator<>)>,
    detail::MAX_SIZE_T_OF<alignof(detail::ElementLocator<>), alignof(detail::AllFixedSizeElementLocator<>)>>;

template <class T>
auto type_erase_element_locator(T&& locator) noexcept
{
    TypeErasedElementLocator result;
    detail::construct_at(reinterpret_cast<detail::RemoveCvrefT<T>*>(&result), std::forward<T>(locator));
    return result;
}
}  // namespace cntgs::detail