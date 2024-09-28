// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_UTILS_TESTMEMORYRESOURCE_HPP
#define CNTGS_UTILS_TESTMEMORYRESOURCE_HPP

#include "utils/doctest.hpp"

#include <algorithm>
#include <array>
#include <memory_resource>

namespace cntgs::test
{
struct TestMemoryResource;

template <class T = std::byte>
class TestAllocator
{
  public:
    using value_type = T;

    TestAllocator() = default;

    constexpr explicit TestAllocator(TestMemoryResource& tracked) noexcept : tracked(&tracked) {}

    template <class U>
    constexpr TestAllocator(const test::TestAllocator<U>& other) noexcept : tracked(other.tracked)
    {
    }

    [[nodiscard]] T* allocate(std::size_t n);

    void deallocate(T* p, std::size_t n);

    template <class U>
    friend constexpr bool operator==(const TestAllocator& lhs, const test::TestAllocator<U>& rhs) noexcept
    {
        return lhs.tracked == rhs.tracked;
    }

    template <class U>
    friend constexpr bool operator!=(const TestAllocator& lhs, const test::TestAllocator<U>& rhs) noexcept
    {
        return lhs.tracked != rhs.tracked;
    }

  private:
    template <class>
    friend class test::TestAllocator;

    TestMemoryResource* tracked{};
};

struct TestMemoryResource
{
    std::size_t bytes_allocated{};
    std::size_t bytes_deallocated{};

    auto get_allocator() { return TestAllocator(*this); }
};

template <class T>
inline T* TestAllocator<T>::allocate(std::size_t n)
{
    if (tracked)
    {
        tracked->bytes_allocated += n * sizeof(T);
    }
    return std::allocator<T>{}.allocate(n);
}

template <class T>
inline void TestAllocator<T>::deallocate(T* p, std::size_t n)
{
    if (tracked)
    {
        tracked->bytes_deallocated += n * sizeof(T);
    }
    std::allocator<T>{}.deallocate(p, n);
}

struct TestPmrMemoryResource
{
    using allocator_type = std::pmr::polymorphic_allocator<std::byte>;

    std::array<std::byte, 256> buffer{};
    std::pmr::monotonic_buffer_resource resource{buffer.data(), buffer.size()};

    auto get_allocator() noexcept { return std::pmr::polymorphic_allocator<std::byte>{&resource}; }

    auto check_was_used(const std::pmr::polymorphic_allocator<std::byte>& allocator)
    {
        CHECK_EQ(&resource, allocator.resource());
        CHECK(std::any_of(buffer.begin(), buffer.end(),
                          [](auto&& byte)
                          {
                              return byte != std::byte{};
                          }));
    }

    auto check_was_not_used(const std::pmr::polymorphic_allocator<std::byte>& allocator)
    {
        CHECK_EQ(&resource, allocator.resource());
        CHECK(std::all_of(buffer.begin(), buffer.end(),
                          [](auto&& byte)
                          {
                              return byte == std::byte{};
                          }));
    }
};
}  // namespace cntgs::test

#endif  // CNTGS_UTILS_TESTMEMORYRESOURCE_HPP
