# contiguous

[![Coverage](https://sonarcloud.io/api/project_badges/measure?project=Tradias_contiguous&metric=coverage)](https://sonarcloud.io/dashboard?id=Tradias_contiguous)

A C++ library for storing structs in contiguous memory. Header-only, zero dependencies, no exceptions, no rtti, C++17

# Usage

```cmake
find_package(cntgs)
target_link_libraries(your_library PUBLIC cntgs::cntgs)

# OR 

add_subdirectory(/path/to/repository/root)
target_link_libraries(your_library PUBLIC cntgs::cntgs)
```

# Examples

## Store varying number of objects per element

```
[varying number of int, one float, varying number of int, one float, ...]
```

<!-- snippet: varying-vector -->
<a id='snippet-varying-vector'></a>
```cpp
const auto initial_capacity = 2;
const auto varying_object_count = 5;

using Vector = cntgs::ContiguousVector<cntgs::VaryingSize<int32_t>,  //
                                       float>;
Vector vector{initial_capacity, varying_object_count * sizeof(int32_t)};

assert(initial_capacity == vector.capacity());

vector.emplace_back(std::array{1, 2}, 10.f);
vector.emplace_back(std::array{3, 4, 5}, 20.f);

assert(2 == vector.size());

auto&& [varying_int, the_float] = vector[0];

assert((std::is_same_v<cntgs::Span<int32_t>, decltype(varying_int)>));
assert((std::is_same_v<float&, decltype(the_float)>));
```
<sup><a href='/example/varying-vector.cpp#L13-L32' title='Snippet source file'>snippet source</a> | <a href='#snippet-varying-vector' title='Start of snippet'>anchor</a></sup>
<!-- endSnippet -->

## Store fixed number of objects per element

```
[fixed number of floats, fixed number of uint32_t, one float, fixed number of floats, fixed number of uint32_t, one float, ...]
```

<!-- snippet: fixed-vector -->
<a id='snippet-fixed-vector'></a>
```cpp
const auto first_object_count = 3;
const auto second_object_count = 5;

using Vector = cntgs::ContiguousVector<cntgs::FixedSize<float>,     //
                                       cntgs::FixedSize<uint32_t>,  //
                                       float>;
Vector vector{1, {first_object_count, second_object_count}};

std::array first{1.f, 2.f, 3.f};
std::array second{10u, 20u, 30u, 40u, 50u};
vector.emplace_back(first, second.begin(), 0.f);

auto&& [firsts, seconds, the_uint] = vector[0];

assert(8 == std::addressof(the_uint) - firsts.data());

assert(first_object_count == vector.get_fixed_size<0>());
assert(second_object_count == vector.get_fixed_size<1>());
```
<sup><a href='/example/fixed-vector.cpp#L13-L32' title='Snippet source file'>snippet source</a> | <a href='#snippet-fixed-vector' title='Start of snippet'>anchor</a></sup>
<!-- endSnippet -->

## Type erase a ContiguousVector

<!-- snippet: type-erased-vector -->
<a id='snippet-type-erased-vector'></a>
```cpp
using Vector = cntgs::ContiguousVector<cntgs::FixedSize<float>,  //
                                       cntgs::VaryingSize<uint32_t>>;
Vector vector{1, 2 * sizeof(uint32_t), {1}};
fill_vector(vector);

cntgs::TypeErasedVector type_erased_vector{std::move(vector)};

Vector restored{std::move(type_erased_vector)};
```
<sup><a href='/example/type-erased-vector.cpp#L18-L27' title='Snippet source file'>snippet source</a> | <a href='#snippet-type-erased-vector' title='Start of snippet'>anchor</a></sup>
<!-- endSnippet -->

## Specify alignment

Note that the default alignment of types stored by a ContiguousVector is 1. Misaligned memory access is not supported by all processors (e.g. RISC).
On most processors however, misaligned access works correctly with basically no performance penalty.

Each element in the following ContiguousVector is comprised of a fixed number of 32-byte aligned floats, followed by a variable number of 8-byte aligned integers, followed by one 8-byte aligned integer.

<!-- snippet: vector-with-alignment -->
<a id='snippet-vector-with-alignment'></a>
```cpp
using Vector = cntgs::ContiguousVector<              //
    cntgs::FixedSize<cntgs::AlignAs<float, 32>>,     //
    cntgs::VaryingSize<cntgs::AlignAs<int32_t, 8>>,  //
    cntgs::AlignAs<int32_t, 8>>;
```
<sup><a href='/example/vector-with-alignment.cpp#L29-L34' title='Snippet source file'>snippet source</a> | <a href='#snippet-vector-with-alignment' title='Start of snippet'>anchor</a></sup>
<!-- endSnippet -->
