#pragma once

#include <algorithm>
#include <cstring>
#include <memory>
#include <vector>

struct Vector
{
    const uint32_t first_count;
    const uint32_t second_count;

    const uint32_t byte_size_per_node;
    const uint32_t second_offset;
    const uint32_t third_offset;
    uint32_t next_index;

    std::unique_ptr<char[]> ptr;
    char* memory;

    inline auto get_node(const uint32_t i) const { return memory + i * byte_size_per_node; }

    inline auto operator[](const uint32_t i) const { return reinterpret_cast<uint32_t*>(get_node(i) + second_offset); }

    void emplace_back(const uint32_t third, const char* first, const uint32_t* second)
    {
        const auto node_memory = get_node(next_index);
        next_index += 1;
        std::memcpy(node_memory, first, first_count);
        std::memcpy(node_memory + second_offset, second, second_count * 4);
        std::memcpy(node_memory + third_offset, &third, 4);
    }
};

auto a(Vector& vector, uint32_t& out, uint32_t i, const std::vector<char>& first, const std::vector<uint32_t>& second)
{
    vector.emplace_back(i, first.data(), second.data());
    auto&& firsts = vector[i];
    std::for_each_n(firsts, vector.second_count, [&](auto&& v) { out += v; });
}