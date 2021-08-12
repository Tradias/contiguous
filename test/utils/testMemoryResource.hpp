// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_UTILS_TESTMEMORYRESOURCE_HPP
#define CNTGS_UTILS_TESTMEMORYRESOURCE_HPP

#include <doctest/doctest.h>

#include <array>
#include <memory_resource>

namespace cntgs::test
{
struct TestMemoryResource
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
