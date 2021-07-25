#include "cntgs/contiguous.h"
#include "reference.h"

#include <array>
#include <tuple>
#include <vector>

auto reference_two_fixed_lookup_and_accumulate(const ReferenceFixedSizeVector& vector, uint32_t& out, size_t index,
                                               const std::vector<char>& first, const std::vector<uint32_t>& second,
                                               uint32_t third)
{
    auto&& node = vector.get_node(index);
    auto&& seconds = vector.get_second(node);
    for (size_t i = 0; i < vector.second_count; i++)
    {
        out += seconds[i];
    }
    auto&& firsts = vector.get_first(node);
    for (size_t i = 0; i < vector.first_count; i++)
    {
        out += firsts[i];
    }
}

using CntgsFixedSizeVector = cntgs::ContiguousVector<cntgs::FixedSize<char>, cntgs::FixedSize<uint32_t>, uint32_t>;

auto contiguous_two_fixed_lookup_and_accumulate(const CntgsFixedSizeVector& vector, uint32_t& out, size_t index,
                                                const std::vector<char>& first, const std::vector<uint32_t>& second,
                                                uint32_t third)
{
    auto&& [firsts, seconds, c] = vector[index];
    for (auto&& v : seconds)
    {
        out += v;
    }
    for (auto&& v : firsts)
    {
        out += v;
    }
}

auto reference_two_fixed_emplace(ReferenceFixedSizeVector& vector, const std::vector<char>& first,
                                 const std::vector<uint32_t>& second, uint32_t third)
{
    vector.emplace_back(first.data(), second.data(), third);
}

auto contiguous_two_fixed_emplace(CntgsFixedSizeVector& vector, const std::vector<char>& first,
                                  const std::vector<uint32_t>& second, uint32_t third)
{
    vector.emplace_back(first.begin(), second.begin(), third);
}

auto reference_two_fixed_random_lookup(const ReferenceFixedSizeVector& vector, size_t i, size_t j, size_t k,
                                       char& first, uint32_t& second, uint32_t& third)
{
    auto&& a = vector.get_second(i);
    second = *a;
    auto&& b = vector.get_first(j);
    first = *b;
    auto&& c = vector.get_third(k);
    third = c;
}

auto contiguous_two_fixed_random_lookup(const CntgsFixedSizeVector& vector, size_t i, size_t j, size_t k, char& first,
                                        uint32_t& second, uint32_t& third)
{
    auto&& [a1, b1, c1] = vector[i];
    second = *b1.data();
    auto&& [a2, b2, c2] = vector[i];
    first = *a2.data();
    auto&& [a3, b3, c3] = vector[i];
    third = c3;
}

auto reference_two_fixed_aligned_lookup_and_accumulate(const ReferenceFixedSizeVector& vector, uint32_t& out,
                                                       size_t index, const std::vector<char>& first,
                                                       const std::vector<uint32_t>& second, uint32_t third)
{
    auto&& node = vector.get_node(index);
    auto&& seconds = vector.get_second(node);
    for (size_t i = 0; i < vector.second_count; i++)
    {
        out += seconds[i];
    }
    auto&& firsts = vector.get_first(node);
    for (size_t i = 0; i < vector.first_count; i++)
    {
        out += firsts[i];
    }
}

using CntgsFixedSizeAlignedVector =
    cntgs::ContiguousVector<cntgs::FixedSize<cntgs::AlignAs<char, 32>>, cntgs::FixedSize<uint32_t>, uint32_t>;

auto contiguous_two_fixed_aligned_lookup_and_accumulate(const CntgsFixedSizeAlignedVector& vector, uint32_t& out,
                                                        size_t index, const std::vector<char>& first,
                                                        const std::vector<uint32_t>& second, uint32_t third)
{
    auto&& [firsts, seconds, c] = vector[index];
    for (auto&& v : seconds)
    {
        out += v;
    }
    for (auto&& v : firsts)
    {
        out += v;
    }
}

auto reference_two_fixed_reserve_growth(ReferenceFixedSizeVector& vector, uint32_t new_capacity)
{
    vector.reserve_unaligned(new_capacity);
}

auto contiguous_two_fixed_reserve_growth(CntgsFixedSizeVector& vector, uint32_t new_capacity)
{
    vector.reserve(new_capacity);
}

auto std_vector_erase_at_the_end(
    std::vector<std::tuple<std::array<char, 15>, std::array<uint32_t, 15>, uint32_t>>& vector,
    std::vector<std::tuple<std::array<char, 15>, std::array<uint32_t, 15>, uint32_t>>::const_iterator begin)
{
    vector.erase(begin, vector.end());
}

auto contiguous_two_erase_at_the_end(CntgsFixedSizeVector& vector, CntgsFixedSizeVector::const_iterator begin)
{
    vector.erase(begin, vector.end());
}