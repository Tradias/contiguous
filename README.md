# contiguous

[![Coverage](https://sonarcloud.io/api/project_badges/measure?project=Tradias_contiguous&metric=coverage)](https://sonarcloud.io/dashboard?id=Tradias_contiguous)

C++ library for storing objects of different types contiguously. Header-only, zero dependencies, no exceptions, no rtti, C++17

# Motivation

Data locality can greatly improve the performance of an algorithm due to better CPU cache utilization. If the size of the data needed for one step of the algorithm is 
known at compile time then data locality can easily be achieved by storing elements in a vector of structs. But if the size is known only at runtime then you might 
find yourself writing rather hard to read and maintain `memcpy` with offset code like [this](https://github.com/Neiko2002/deglib/blob/fd297878adfe98eb001c452bdbe387285d27b7ff/deglib/include/graph/sizebounded_graph.h#L263) for storing data and [reinterpret_cast'ing code for retrieving it](https://github.com/Neiko2002/deglib/blob/fd297878adfe98eb001c452bdbe387285d27b7ff/deglib/include/graph/sizebounded_graph.h#L173). And what if you want to add new data in the middle, 
increase the memory size or store non-trivial types? This library aims to provide a generalized abstraction for such tasks through a templated container type.

# Usage

The library can be used as a single header file, through `add_subdirectory` or through `find_package` in a CMake project.

## Single header

Simply copy the <a href='/src/cntgs.hpp'>single header</a> into your project and include it to use the library. You can also play with the library on [compiler-explorer](https://godbolt.org/z/7GjKe8rME).

## As a subdirectory

Clone the repository into a subdirectory of your CMake project. Then add it and link it to your target.

```cmake
add_subdirectory(/path/to/repository/root)
target_link_libraries(your_app PUBLIC cntgs::cntgs)
```

## As a CMake package

Clone the repository and install the library

```shell
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/desired/installation/directory ..
cmake --build . --target install
```

Locate it and link it to your target.

```cmake
# Make sure to set CMAKE_PREFIX_PATH to /desired/installation/directory
find_package(cntgs)
target_link_libraries(your_app PUBLIC cntgs::cntgs)
```

# Documentation

A full API reference documentation is still work-in-progress. Meanwhile, the following examples should help you get started.

## Specialize a ContiguousVector

The main workhorse of this library is the `cntgs::ContiguousVector`, a container modelled after [SequenceContainer](https://en.cppreference.com/w/cpp/named_req/SequenceContainer). Start by defining a container capable of storing elements in your desired layout.

<!-- snippet: contiguous-vector-definition -->
<a id='snippet-contiguous-vector-definition'></a>
```hpp
template <class... Parameter>
using ContiguousVector = cntgs::BasicContiguousVector<cntgs::Options<>, Parameter...>;
```
<sup><a href='/src/cntgs/vector.hpp#L34-L37' title='Snippet source file'>snippet source</a> | <a href='#snippet-contiguous-vector-definition' title='Start of snippet'>anchor</a></sup>
<!-- endSnippet -->

Each parameter must be a built-in or user-deinfed type, optionally wrapped into a parameter decorator. The vector stores objects of those types within one element in the order they are specified.

### Store a fixed number of objects per element

To store objects within one element of the `cntgs::ContiguousVector` in the following layout, where `f` denotes an object of type `float`, `u` denotes an object of type `uint32_t` and `|` the end of one element:

```
|fffuuuuuf|fffuuuuuf|fffuuuuuf|
```

<!-- snippet: fixed-vector-definition -->
<a id='snippet-fixed-vector-definition'></a>
```cpp
using Vector = cntgs::ContiguousVector<cntgs::FixedSize<float>,     //
                                       cntgs::FixedSize<uint32_t>,  //
                                       float>;
```
<sup><a href='/example/fixed-vector.cpp#L13-L17' title='Snippet source file'>snippet source</a> | <a href='#snippet-fixed-vector-definition' title='Start of snippet'>anchor</a></sup>
<!-- endSnippet -->

The library employs special optimizations when all parameter are either plain types or `cntgs::FixedSize` (optionally wrapped into `cntgs::AlignAs`, see below).

### Store a varying number of objects per element

To store objects within one element of the `cntgs::ContiguousVector` in the following layout, where `f` denotes an object of type `float`, `i` denotes an object of type `int32_t` and `|` the end of one element:

```
|iiiif|iif|iiif|iiiiiif|
```

Define the `cntgs::ContiguousVector` as follows:

<!-- snippet: varying-vector-definition -->
<a id='snippet-varying-vector-definition'></a>
```cpp
using Vector = cntgs::ContiguousVector<cntgs::VaryingSize<int32_t>,  //
                                       float>;
```
<sup><a href='/example/varying-vector.cpp#L13-L16' title='Snippet source file'>snippet source</a> | <a href='#snippet-varying-vector-definition' title='Start of snippet'>anchor</a></sup>
<!-- endSnippet -->

## Construct a ContiguousVector

The exact signature of the constructor of a `cntgs::ContiguousVector` depends on the provided template parameter. For a vector that in comprised of only plain types and `cntgs::FixedSize` parameter the constructor takes the initial capacity and the number of objects per elements for each `cntgs::FixedSize` parameter as arguments.

<!-- snippet: fixed-vector-construction -->
<a id='snippet-fixed-vector-construction'></a>
```cpp
cntgs::ContiguousVector<cntgs::FixedSize<float>,     //
                        cntgs::FixedSize<uint32_t>,  //
                        float>
    vector{initial_capacity, {first_object_count, second_object_count}};
```
<sup><a href='/example/fixed-vector.cpp#L23-L28' title='Snippet source file'>snippet source</a> | <a href='#snippet-fixed-vector-construction' title='Start of snippet'>anchor</a></sup>
<!-- endSnippet -->

For `cntgs::ContiguousVector` with `cntgs::VaryingSize` parameter the constructor takes the total number of bytes that all objects of `cntgs::VaryingSize` parameter will need as an argument, in addition to the initial capacity.

<!-- snippet: varying-vector-construction -->
<a id='snippet-varying-vector-construction'></a>
```cpp
cntgs::ContiguousVector<cntgs::VaryingSize<int32_t>,  //
                        float>                        //
    vector{initial_capacity, varying_object_count * sizeof(int32_t)};
```
<sup><a href='/example/varying-vector.cpp#L21-L25' title='Snippet source file'>snippet source</a> | <a href='#snippet-varying-vector-construction' title='Start of snippet'>anchor</a></sup>
<!-- endSnippet -->

`cntgs::ContiguousVector`s that have both `cntgs::FixedSize` and `cntgs::VaryingSize` parameter, first take the total number of bytes of all objects of `cntgs::VaryingSize` followed by the objects per elements for each `cntgs::FixedSize` parameter.

<!-- snippet: mixed-vector-construction -->
<a id='snippet-mixed-vector-construction'></a>
```cpp
cntgs::ContiguousVector<cntgs::FixedSize<std::unique_ptr<int32_t>>,  //
                        cntgs::Varying<std::unique_ptr<uint32_t>>>
    vector{initial_capacity, varying_object_count * sizeof(std::unique_ptr<uint32_t>), {fixed_object_count}};
```
<sup><a href='/example/mixed-vector.cpp#L18-L22' title='Snippet source file'>snippet source</a> | <a href='#snippet-mixed-vector-construction' title='Start of snippet'>anchor</a></sup>
<!-- endSnippet -->

## Emplace elements into a ContiguousVector

Similar to the constructor, the exact signature of `emplace_back` also depends on the parameter of the `cntgs::ContiguousVector`. The objects of each parameter are constructed from one of the arguments provided to the function. For `cntgs::FixedSize` parameter the argument can either be a range of an iterator. As an example for the vector defined above, assuming that the size of the first parameter is no less than three and the size of the second parameter no more than five.

<!-- snippet: fixed-vector-emplace_back -->
<a id='snippet-fixed-vector-emplace_back'></a>
```cpp
std::vector first{1.f, 2.f, 3.f};
std::vector second{10u, 20u, 30u, 40u, 50u};
vector.emplace_back(first, second.begin(), 0.f);
```
<sup><a href='/example/fixed-vector.cpp#L33-L37' title='Snippet source file'>snippet source</a> | <a href='#snippet-fixed-vector-emplace_back' title='Start of snippet'>anchor</a></sup>
<!-- endSnippet -->

For `cntgs::VaryingSize` parameter the argument must be a range.

<!-- snippet: varying-vector-emplace_back -->
<a id='snippet-varying-vector-emplace_back'></a>
```cpp
vector.emplace_back(std::vector{1, 2}, 10.f);
vector.emplace_back(std::vector{3, 4, 5}, 20.f);
```
<sup><a href='/example/varying-vector.cpp#L30-L33' title='Snippet source file'>snippet source</a> | <a href='#snippet-varying-vector-emplace_back' title='Start of snippet'>anchor</a></sup>
<!-- endSnippet -->

To emplace move-only types either pass the range as an rvalue or wrap its iterator into a [std::move_iterator](https://en.cppreference.com/w/cpp/iterator/move_iterator).

<!-- snippet: mixed-vector-emplace_back -->
<a id='snippet-mixed-vector-emplace_back'></a>
```cpp
std::vector first{std::make_unique<int32_t>(1u), std::make_unique<int32_t>(2u)};
std::vector second{std::make_unique<int32_t>(10), std::make_unique<int32_t>(20)};
vector.emplace_back(std::make_move_iterator(first.begin()), std::move(second));
```
<sup><a href='/example/mixed-vector.cpp#L27-L31' title='Snippet source file'>snippet source</a> | <a href='#snippet-mixed-vector-emplace_back' title='Start of snippet'>anchor</a></sup>
<!-- endSnippet -->

## Retrieving elements

Elements can be retrieved through the subscript operator, `front()` or `back()` of the `cntgs::ContiguousVector` or by de-referencing its iterator. In either case a proxy reference is returned which is similar to a tuple of references.

<!-- snippet: varying-vector-subscript -->
<a id='snippet-varying-vector-subscript'></a>
```cpp
auto&& [varying_int, the_float] = vector[0];
assert((std::is_same_v<cntgs::Span<int32_t>, decltype(varying_int)>));
assert((std::is_same_v<float&, decltype(the_float)>));
```
<sup><a href='/example/varying-vector.cpp#L37-L41' title='Snippet source file'>snippet source</a> | <a href='#snippet-varying-vector-subscript' title='Start of snippet'>anchor</a></sup>
<!-- endSnippet -->

The objects of an individual parameter of one element can also be accessed using `cntgs::get`.

<!-- snippet: mixed-vector-get -->
<a id='snippet-mixed-vector-get'></a>
```cpp
auto&& objects_of_first_parameter = cntgs::get<0>(vector.front());
```
<sup><a href='/example/mixed-vector.cpp#L33-L35' title='Snippet source file'>snippet source</a> | <a href='#snippet-mixed-vector-get' title='Start of snippet'>anchor</a></sup>
<!-- endSnippet -->

## Additional ContiguousVector member functions

The `cntgs::ContiguousVector` has additional member functions that behave very similar to their stl counterpart like `pop_back`, `reserve`, `erase`, `clear`, `size`, `capacity`, `empty`, `data`, `get_allocator`, `operator=` and `operator<=>`. See the [source file](/src/cntgs/vector.hpp) for more details.

## Allocator support

The allocator used by the `cntgs::ContiguousVector` can be changed to any allocator that fulfills the standard [allocator requirements](https://en.cppreference.com/w/cpp/named_req/Allocator). The actual allocator used by the vector and returned by `vector.get_allocator()` will be rebound to [std::byte](https://en.cppreference.com/w/cpp/types/byte).

<!-- snippet: pmr-vector-definition -->
<a id='snippet-pmr-vector-definition'></a>
```cpp
using Vector = cntgs::BasicContiguousVector<                                       //
    cntgs::Options<cntgs::Allocator<std::pmr::polymorphic_allocator<std::byte>>>,  //
    cntgs::FixedSize<uint32_t>, float>;
```
<sup><a href='/example/pmr-vector.cpp#L13-L17' title='Snippet source file'>snippet source</a> | <a href='#snippet-pmr-vector-definition' title='Start of snippet'>anchor</a></sup>
<!-- endSnippet -->

At construction pass the allocator as the last argument.

<!-- snippet: pmr-vector-construction -->
<a id='snippet-pmr-vector-construction'></a>
```cpp
std::pmr::monotonic_buffer_resource resource;
Vector vector{initial_capacity, {fixed_object_count}, &resource};
```
<sup><a href='/example/pmr-vector.cpp#L22-L25' title='Snippet source file'>snippet source</a> | <a href='#snippet-pmr-vector-construction' title='Start of snippet'>anchor</a></sup>
<!-- endSnippet -->

## Alignment

The default alignment of objects stored in a `cntgs::ContiguousVector` is one. Some types require an alignment larger than that. On most processors however, misaligned memory access works correctly and has basically no performance penalty. On some processors, like RISC, unaligned memory access is not supported at all.

If you need aligned objects because you want to load them into a SIMD register or because you have measured a performance penalty then specify the desired alignment with the `cntgs::AlignAs` parameter decorator.

Each element in the following `cntgs::ContiguousVector` is comprised of a fixed number of 32-byte aligned floats, followed by a variable number of 8-byte aligned integers, followed by one 8-byte aligned integer.

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

