#include "cntgs/contiguous.h"
#include "utils/reference.h"

auto reference1(ReferenceFixedSizeVector& vector, uint32_t& out, size_t i, uint32_t first,
                const std::vector<char>& second, const std::vector<uint32_t>& third)
{
    vector.emplace_back(first, second.data(), third.data());
    auto&& seconds = vector.get_second(i);
    std::for_each_n(seconds, vector.second_count, [&](auto&& v) { out += v; });
}

using CntgsFixedSizeVector = cntgs::ContiguousVector<uint32_t, cntgs::FixedSize<char>, cntgs::FixedSize<uint32_t>>;

auto contiguous1(CntgsFixedSizeVector& vector, uint32_t& out, size_t i, uint32_t first, const std::vector<char>& second,
                 const std::vector<uint32_t>& third)
{
    vector.emplace_back(first, second.data(), third.data());
    auto&& [a, seconds, c] = vector[i];
    std::for_each(seconds.begin(), seconds.end(), [&](auto&& v) { out += v; });
}