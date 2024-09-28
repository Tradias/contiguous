// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <cntgs/contiguous.hpp>

#include <cassert>
#include <memory>
#include <vector>

template <class T>
void check_alignment(T* ptr, std::size_t alignment)
{
    auto* void_ptr = static_cast<void*>(ptr);
    auto size = std::numeric_limits<size_t>::max();
    std::align(alignment, sizeof(T), void_ptr, size);
    assert(void_ptr == ptr);
}

template <class T>
void check_alignment(cntgs::Span<T>& span, std::size_t alignment)
{
    check_alignment(std::data(span), alignment);
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
    check_alignment(a, 32);
    check_alignment(b, 8);
    check_alignment(std::addressof(c), 8);

    return 0;
}