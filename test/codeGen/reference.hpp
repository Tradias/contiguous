// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_CODEGEN_REFERENCE_HPP
#define CNTGS_CODEGEN_REFERENCE_HPP

#include <algorithm>
#include <cstring>
#include <memory>
#include <vector>

struct ReferenceFixedSizeVector
{
    const uint32_t first_count;
    const uint32_t second_count;

    const uint32_t byte_size_per_node;
    const uint32_t second_offset;
    const uint32_t third_offset;
    uint32_t next_index;
    uint32_t capacity;

    char* memory;

    inline auto get_node(const uint32_t i) const { return memory + i * byte_size_per_node; }

    inline auto get_first(const uint32_t i) const { return get_node(i); }

    inline auto get_first(const char* node) const { return node; }

    inline auto get_second(const uint32_t i) const { return reinterpret_cast<uint32_t*>(get_node(i) + second_offset); }

    inline auto get_second(const char* node) const { return reinterpret_cast<const uint32_t*>(node + second_offset); }

    inline auto get_third(const uint32_t internal_idx) const
    {
        return *reinterpret_cast<const uint32_t*>(get_node(internal_idx) + third_offset);
    }

    void emplace_back(const char* first, const uint32_t* second, const uint32_t third)
    {
        const auto node_memory = get_node(next_index);
        next_index += 1;
        std::memcpy(node_memory, first, first_count);
        std::memcpy(node_memory + second_offset, second, second_count * 4);
        std::memcpy(node_memory + third_offset, &third, 4);
    }

    void reserve_unaligned(uint32_t new_max_element_count)
    {
        if (new_max_element_count <= capacity)
        {
            return;
        }
        using Traits = std::allocator_traits<std::allocator<char>>;
        Traits::allocator_type alloc{};
        const auto new_capacity = new_max_element_count * byte_size_per_node;
        auto new_memory = Traits::allocate(alloc, new_capacity);
        std::memcpy(new_memory, memory, capacity * byte_size_per_node);
        Traits::deallocate(alloc, memory, capacity);
        capacity = new_capacity;
        memory = new_memory;
    }

    void erase(uint32_t i)
    {
        const auto source = get_node(i + 1);
        const auto target = get_node(i);
        std::memmove(target, source, get_node(next_index) - source);
        next_index -= 1;
    }
};

#endif  // CNTGS_CODEGEN_REFERENCE_HPP
