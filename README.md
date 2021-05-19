# contiguous

A C++ library for storing structs in contiguous memory. Header-only, zero dependencies, no exceptions, no rtti, C++17

# Installation

```cmake
find_package(cntgs)
target_link_libraries(your_library PUBLIC cntgs::cntgs)

OR 

add_subdirectory(/path/to/repository/root)
```

# Usage

Storing elements in the Form

```
[varying number of int16_t, one float, varying number of int16_t, one float, ...]
```

```c++
#include <cntgs/contiguous.h>

using Vector = cntgs::ContiguousVector<cntgs::VaryingSize<int16_t>, float>;

const auto element_count = 5;
const auto total_varying_element_count = 20;

Vector vector{element_count, total_varying_element_count * sizeof(int16_t)};
assert(element_count == vector.capacity());

vector.emplace_back(<container to int16_t>, <float>);
assert(1 == vector.size());

size_t index = 10;
auto&& [varying, the_float] = vector[index];
// Types are:
// varying: Span<int16_t>
// the_float: float&
```

Storing elements in the Form

```
[fixed number of floats, fixed number of uint32_t, one uint32_t, fixed number of floats, fixed number of uint32_t, one uint32_t, ...]
```

```c++
#include <cntgs/contiguous.h>

using Vector = cntgs::ContiguousVector<cntgs::FixedSize<float>, cntgs::FixedSize<uint32_t>, uint32_t>;

const auto element_count = 2;
const auto first_fixed_size_element_count = 10;
const auto second_fixed_size_element_count = 12;

Vector vector{element_count, {first_fixed_size_element_count, second_fixed_size_element_count}};
assert(2 == vector.capacity());

vector.emplace_back(<container or iterator to floats>, <container or iterator to uint32_t>, <uint32_t>);
assert(1 == vector.size());

size_t index = 10;
auto&& [first_fixed_size, second_fixed_size, uinteger] = vector[index];
// Types are:
// first_fixed_size: Span<float>
// second_fixed_size: Span<uint32_t>
// uinteger: uint32_t&

assert(first_fixed_size_element_count == vector.get_fixed_size<0>());
assert(second_fixed_size_element_count == vector.get_fixed_size<1>());
```

Type erase a ContiguousVector

```c++
#include <cntgs/contiguous.h>

using Vector = cntgs::ContiguousVector<...>;

Vector vector;

auto type_erased_vector = cntgs::type_erase(std::move(vector));
// Type is cntgs::TypeErasedVector

Vector restored{std::move(type_erased_vector)};
```