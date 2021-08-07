// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <cntgs/contiguous.hpp>

#include <cassert>
#include <memory>
#include <vector>

template <std::size_t Alignment, class T>
void check_alignment(T* ptr)
{
    auto* void_ptr = static_cast<void*>(ptr);
    auto size = std::numeric_limits<size_t>::max();
    std::align(Alignment, 4, void_ptr, size);
    assert(void_ptr == ptr);
}

template <std::size_t Alignment, class T>
void check_alignment(cntgs::Span<T>& span)
{
    check_alignment<Alignment>(std::data(span));
}

int main()
{
    // begin-snippet: vector-with-alignment
    using Vector = cntgs::ContiguousVector<              //
        cntgs::FixedSize<cntgs::AlignAs<float, 32>>,     //
        cntgs::VaryingSize<cntgs::AlignAs<int32_t, 8>>,  //
        cntgs::AlignAs<int32_t, 8>>;
    // end-snippet

    Vector vector{1, 2 * sizeof(int32_t), {2}};

    vector.emplace_back(std::vector{1.f, 2.f}, std::vector{1, 2}, 5);

    auto&& [a, b, c] = vector[0];
    check_alignment<32>(a);
    check_alignment<8>(b);
    check_alignment<8>(std::addressof(c));

    return 0;
}