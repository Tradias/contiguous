#pragma once

#include <cstddef>
#include <memory>

namespace cntgs::test
{
template <class T, class Allocator = std::allocator<T>>
struct AllocationGuard
{
    T* ptr;
    std::size_t size;
    bool perform_deallocation{true};

    explicit constexpr AllocationGuard(std::size_t size = 1) : ptr(Allocator{}.allocate(size)), size(size) {}

    constexpr auto release() noexcept
    {
        perform_deallocation = false;
        return ptr;
    }

    ~AllocationGuard() noexcept
    {
        if (perform_deallocation)
        {
            Allocator{}.deallocate(ptr, size);
        }
    }
};
}  // namespace cntgs::test