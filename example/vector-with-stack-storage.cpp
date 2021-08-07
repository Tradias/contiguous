// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <cntgs/contiguous.hpp>

#include <algorithm>
#include <array>
#include <cassert>

int main()
{
    // begin-snippet: vector-with-stack-storage
    using Vector = cntgs::ContiguousVector<  //
        cntgs::FixedSize<float>, cntgs::FixedSize<int32_t>>;

    std::array<std::byte, 512> buffer;
    Vector vector{cntgs::Span{buffer.data(), buffer.size()}, 5, {2, 2}};

    // even with compile time initialization (C++20)
    static constinit std::array<std::byte, 512> BUFFER{};
    static constinit Vector VECTOR{cntgs::Span{BUFFER.data(), BUFFER.size()}, 5, {2, 2}};
    // end-snippet

    std::array firsts{1.f, 2.f};
    std::array seconds{1, 2};
    vector.emplace_back(firsts.begin(), seconds.data());
    VECTOR.emplace_back(firsts.begin(), seconds.data());

    auto&& [a, b] = vector[0];
    assert(std::equal(firsts.begin(), firsts.end(), a.begin(), a.end()));
    assert(std::equal(seconds.begin(), seconds.end(), b.begin(), b.end()));

    auto&& [a2, b2] = VECTOR[0];
    assert(std::equal(firsts.begin(), firsts.end(), a2.begin(), a2.end()));
    assert(std::equal(seconds.begin(), seconds.end(), b2.begin(), b2.end()));

    return 0;
}