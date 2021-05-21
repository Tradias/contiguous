#include "cntgs/contiguous.h"
#include "reference.h"

auto reference_two_fixed_lookup_and_accumulate(ReferenceFixedSizeVector& vector, uint32_t& out, size_t i,
                                               const std::vector<char>& first, const std::vector<uint32_t>& second,
                                               uint32_t third)
{
    auto&& node = vector.get_node(i);
    auto&& seconds = vector.get_second(node);
    std::for_each_n(seconds, vector.second_count, [&](auto&& v) { out += v; });
    auto&& firsts = vector.get_first(node);
    std::for_each_n(firsts, vector.first_count, [&](auto&& v) { out += v; });
}

using CntgsFixedSizeVector = cntgs::ContiguousVector<cntgs::FixedSize<char>, cntgs::FixedSize<uint32_t>, uint32_t>;

auto contiguous_two_fixed_lookup_and_accumulate(CntgsFixedSizeVector& vector, uint32_t& out, size_t i,
                                                const std::vector<char>& first, const std::vector<uint32_t>& second,
                                                uint32_t third)
{
    auto&& [firsts, seconds, c] = vector[i];
    std::for_each(seconds.begin(), seconds.end(), [&](auto&& v) { out += v; });
    std::for_each(firsts.begin(), firsts.end(), [&](auto&& v) { out += v; });
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

auto reference_two_fixed_random_lookup(ReferenceFixedSizeVector& vector, size_t i, size_t j, size_t k, char& first,
                                       uint32_t& second, uint32_t& third)
{
    auto&& a = vector.get_second(i);
    second = *a;
    auto&& b = vector.get_first(j);
    first = *b;
    auto&& c = vector.get_third(k);
    third = c;
}