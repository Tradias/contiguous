// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "cntgs/contiguous.hpp"

#include <benchmark/benchmark.h>

#include <memory_resource>
#include <random>
#include <vector>

static constexpr auto MSVC_VECTOR_CONSTRUCTION_OVERHEAD = 16;
static constexpr auto LEEWAY = 256;
static constexpr auto FLOAT_MAX = 100000.f;
static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_real_distribution<float> float_dist(0.f, FLOAT_MAX);

using FixedSizeVector = cntgs::ContiguousVector<cntgs::FixedSize<float>>;
using TwoFixedSizeVector = cntgs::ContiguousVector<cntgs::FixedSize<float>, float, cntgs::FixedSize<float>>;
using VaryingSizeVector = cntgs::ContiguousVector<cntgs::VaryingSize<float>>;
using TwoVaryingSizeVector = cntgs::ContiguousVector<cntgs::VaryingSize<float>, float, cntgs::VaryingSize<float>>;

template <std::size_t N, std::size_t K>
struct TwoArray
{
    std::array<float, N> a;
    float b;
    std::array<float, K> c;
};

struct TwoVector
{
    std::pmr::vector<float> a;
    float b;
    std::pmr::vector<float> c;
};

struct VectorVector
{
    std::unique_ptr<std::pmr::monotonic_buffer_resource> resource;
    std::pmr::vector<std::pmr::vector<float>> vector{resource.get()};

    explicit VectorVector(std::size_t elements, std::size_t floats)
        : resource(std::make_unique<std::pmr::monotonic_buffer_resource>(
              floats * sizeof(float) + elements * sizeof(std::pmr::vector<float>) +
              elements * MSVC_VECTOR_CONSTRUCTION_OVERHEAD + LEEWAY))
    {
    }
};

struct VectorTwoVector
{
    std::unique_ptr<std::pmr::monotonic_buffer_resource> resource;
    std::pmr::vector<TwoVector> vector{resource.get()};

    explicit VectorTwoVector(std::size_t elements, std::size_t floats)
        : resource(std::make_unique<std::pmr::monotonic_buffer_resource>(
              floats * sizeof(float) + elements * sizeof(TwoVector) + elements * MSVC_VECTOR_CONSTRUCTION_OVERHEAD * 2 +
              LEEWAY))
    {
    }
};

auto work(float e)
{
    auto v = std::sqrt(std::abs(e) + std::abs(e)) - std::abs(e) + std::sqrt(e);
    benchmark::DoNotOptimize(v);
}

template <class T, std::size_t N>
auto iterate(const std::vector<std::array<T, N>>& vector, size_t fixed_size)
{
    for (auto&& elem : vector)
    {
        for (size_t i = 0; i < fixed_size; i++)
        {
            work(elem[i]);
        }
    }
}

static auto iterate(const VectorVector& vector, size_t)
{
    for (auto&& elem : vector.vector)
    {
        for (auto&& e : elem)
        {
            work(e);
        }
    }
}

template <class T>
auto iterate(const cntgs::ContiguousVector<T>& vector, size_t)
{
    for (auto&& [elem] : vector)
    {
        for (auto&& e : elem)
        {
            work(e);
        }
    }
}

template <std::size_t N>
auto iterate(const std::vector<TwoArray<N, N>>& vector, size_t fixed_size)
{
    for (auto&& elem : vector)
    {
        for (size_t i = 0; i < fixed_size / 2; i++)
        {
            work(elem.a[i]);
        }
        work(elem.b);
        for (size_t i = 0; i < fixed_size / 2; i++)
        {
            work(elem.c[i]);
        }
    }
}

static auto iterate(const VectorTwoVector& vector, size_t)
{
    for (auto&& elem : vector.vector)
    {
        for (auto&& e : elem.a)
        {
            work(e);
        }
        work(elem.b);
        for (auto&& e : elem.c)
        {
            work(e);
        }
    }
}

template <class... T>
auto iterate(const cntgs::ContiguousVector<T...>& vector, size_t)
{
    for (auto&& [a, b, c] : vector)
    {
        for (auto&& e : a)
        {
            work(e);
        }
        work(b);
        for (auto&& e : c)
        {
            work(e);
        }
    }
}

template <class... T>
auto random_lookup(const cntgs::ContiguousVector<T...>& vector, const std::vector<size_t>& indices)
{
    for (auto&& j : indices)
    {
        for (auto&& elem : cntgs::get<0>(vector[j]))
        {
            work(elem);
        }
    }
}

static auto random_lookup(const VectorVector& vector, const std::vector<size_t>& indices)
{
    for (auto&& j : indices)
    {
        for (auto&& elem : vector.vector[j])
        {
            work(elem);
        }
    }
}

template <std::size_t N>
auto random_lookup(const std::vector<std::array<float, N>>& vector, const std::vector<size_t>& indices)
{
    for (auto&& j : indices)
    {
        for (auto&& elem : vector[j])
        {
            work(elem);
        }
    }
}

auto generate_single_element_input(std::size_t elements, std::size_t fixed_size)
{
    std::vector<std::vector<float>> input{elements};
    for (auto&& v : input)
    {
        v.resize(fixed_size);
        std::generate_n(v.begin(), fixed_size,
                        [&]
                        {
                            return float_dist(gen);
                        });
    }
    return input;
}

template <std::size_t N>
void fill_vector(std::vector<std::array<float, N>>& target, const std::vector<std::vector<float>>& source)
{
    target.resize(source.size());
    for (size_t i = 0; i < source.size(); i++)
    {
        std::copy(source[i].begin(), source[i].end(), target[i].begin());
    }
}

void fill_vector(VectorVector& target, const std::vector<std::vector<float>>& source)
{
    target.vector.reserve(source.size());
    for (size_t i = 0; i < source.size(); i++)
    {
        target.vector.emplace_back();
    }
    for (size_t i = 0; i < source.size(); i++)
    {
        target.vector[i].assign(source[i].begin(), source[i].end());
    }
}

template <class... T>
void fill_vector(cntgs::BasicContiguousVector<T...>& target, const std::vector<std::vector<float>>& source)
{
    for (const auto& v : source)
    {
        target.emplace_back(v);
    }
}

template <std::size_t N>
auto make_single_element_input_vectors(std::size_t elements, std::size_t fixed_size)
{
    auto input = generate_single_element_input(elements, fixed_size);
    std::vector<std::array<float, N>> array_vector;
    fill_vector(array_vector, input);
    VectorVector vector_vector{elements, elements * fixed_size};
    fill_vector(vector_vector, input);
    FixedSizeVector fixed_size_vector{input.size(), {fixed_size}};
    fill_vector(fixed_size_vector, input);
    VaryingSizeVector varying_size_vector{input.size(), input.size() * fixed_size * sizeof(float)};
    fill_vector(varying_size_vector, input);
    return std::tuple{std::move(array_vector), std::move(vector_vector), std::move(fixed_size_vector),
                      std::move(varying_size_vector)};
}

auto make_varying_size_input_vectors(std::size_t elements, std::size_t variance)
{
    std::uniform_int_distribution<std::size_t> size_t_dist(0, variance);
    std::vector<std::vector<float>> input{elements};
    size_t total_size = 0;
    for (auto&& v : input)
    {
        auto size = size_t_dist(gen);
        v.resize(size);
        total_size += v.size();
        std::generate(v.begin(), v.end(),
                      [&]
                      {
                          return float_dist(gen);
                      });
    }
    VectorVector vector_vector{elements, total_size};
    fill_vector(vector_vector, input);
    VaryingSizeVector varying_size_vector{input.size(), total_size * sizeof(float)};
    fill_vector(varying_size_vector, input);
    return std::tuple{std::move(vector_vector), std::move(varying_size_vector)};
}

auto make_indices(std::size_t size)
{
    std::vector<size_t> indices;
    indices.resize(1000000);
    std::uniform_int_distribution<size_t> size_t_dist(0, size - 1);
    std::generate(indices.begin(), indices.end(),
                  [&]
                  {
                      return size_t_dist(gen);
                  });
    return indices;
}

template <std::size_t N>
void fill_two_vector(std::vector<TwoArray<N, N>>& target, const std::vector<std::vector<float>>& source)
{
    target.resize(source.size());
    for (size_t i = 0; i < source.size(); i++)
    {
        std::copy(source[i].begin(), source[i].begin() + source[i].size() / 2, target[i].a.begin());
        target[i].b = source[i].front();
        std::copy(source[i].begin() + source[i].size() / 2, source[i].end(), target[i].c.begin());
    }
}

static void fill_two_vector(VectorTwoVector& target, const std::vector<std::vector<float>>& source)
{
    auto& vector = target.vector;
    vector.reserve(source.size());
    for (size_t i = 0; i < source.size(); i++)
    {
        vector.emplace_back();
    }
    for (size_t i = 0; i < source.size(); i++)
    {
        vector[i].a.assign(source[i].begin(), source[i].begin() + source[i].size() / 2);
        vector[i].b = source[i].front();
        vector[i].c.assign(source[i].begin() + source[i].size() / 2, source[i].end());
    }
}

template <class... T>
void fill_two_vector(cntgs::ContiguousVector<T...>& target, const std::vector<std::vector<float>>& source)
{
    for (auto&& v : source)
    {
        target.emplace_back(cntgs::Span{v.data(), v.data() + v.size() / 2}, v.front(),
                            cntgs::Span{v.data() + v.size() / 2, v.data() + v.size()});
    }
}

template <std::size_t N>
auto make_two_element_input_vectors(std::size_t elements, std::size_t fixed_size)
{
    auto input = generate_single_element_input(elements, fixed_size);
    std::vector<TwoArray<N, N>> array_vector;
    fill_two_vector(array_vector, input);
    VectorTwoVector vector_vector{elements, elements * fixed_size};
    fill_two_vector(vector_vector, input);
    TwoFixedSizeVector fixed_size_vector{input.size(), {fixed_size / 2, fixed_size / 2}};
    fill_two_vector(fixed_size_vector, input);
    TwoVaryingSizeVector varying_size_vector{input.size(), input.size() * fixed_size * sizeof(float)};
    fill_two_vector(varying_size_vector, input);
    return std::tuple{std::move(array_vector), std::move(vector_vector), std::move(fixed_size_vector),
                      std::move(varying_size_vector)};
}

static std::vector<int64_t> INPUT_SIZES{100000, 250000, 500000};

template <std::size_t I, std::size_t N>
struct SingleElementInputVectors
{
    using Input = std::remove_reference_t<decltype(std::get<I>(
        make_single_element_input_vectors<N>(std::size_t{}, std::size_t{})))>;

    std::size_t fixed_size;
    Input input;

    SingleElementInputVectors(benchmark::State& state)
        : fixed_size(state.range(1)),
          input(
              [&]
              {
                  gen.seed();
                  return std::get<I>(make_single_element_input_vectors<N>(state.range(0), fixed_size));
              }())
    {
        state.counters["elements"] = static_cast<double>(state.range(0));
        state.counters["fixed size"] = static_cast<double>(fixed_size);
    }
};

template <std::size_t I, std::size_t N>
void BM_full_iteration(benchmark::State& state)
{
    auto [fixed_size, input] = SingleElementInputVectors<I, N>{state};
    for (auto _ : state)
    {
        iterate(input, fixed_size);
    }
}

BENCHMARK_TEMPLATE(BM_full_iteration, 0, 15)
    ->Name("full iteration: std::array<float, 15>")
    ->ArgsProduct({INPUT_SIZES, {15}});

BENCHMARK_TEMPLATE(BM_full_iteration, 0, 30)
    ->Name("full iteration: std::array<float, 30>")
    ->ArgsProduct({INPUT_SIZES, {15, 30}});

BENCHMARK_TEMPLATE(BM_full_iteration, 0, 45)
    ->Name("full iteration: std::array<float, 45>")
    ->ArgsProduct({INPUT_SIZES, {15, 30, 45}});

BENCHMARK_TEMPLATE(BM_full_iteration, 1, 45)
    ->Name("full iteration: std::pmr::vector<std::pmr::vector<float>>")
    ->ArgsProduct({INPUT_SIZES, {15, 30, 45}});

BENCHMARK_TEMPLATE(BM_full_iteration, 2, 24)
    ->Name("full iteration: ContiguousVector<FixedSize<float>>")
    ->ArgsProduct({INPUT_SIZES, {15, 30, 45}});

BENCHMARK_TEMPLATE(BM_full_iteration, 3, 24)
    ->Name("full iteration: ContiguousVector<VaryingSize<float>>")
    ->ArgsProduct({INPUT_SIZES, {15, 30, 45}});

template <std::size_t I, std::size_t N>
void BM_random_lookup(benchmark::State& state)
{
    auto [fixed_size, input] = SingleElementInputVectors<I, N>{state};
    auto indices = make_indices(state.range(0));
    for (auto _ : state)
    {
        random_lookup(input, indices);
    }
}

BENCHMARK_TEMPLATE(BM_random_lookup, 0, 15)
    ->Name("random lookup: std::array<float, 15>")
    ->ArgsProduct({INPUT_SIZES, {15}});

BENCHMARK_TEMPLATE(BM_random_lookup, 0, 30)
    ->Name("random lookup: std::array<float, 30>")
    ->ArgsProduct({INPUT_SIZES, {15, 30}});

BENCHMARK_TEMPLATE(BM_random_lookup, 0, 45)
    ->Name("random lookup: std::array<float, 45>")
    ->ArgsProduct({INPUT_SIZES, {15, 30, 45}});

BENCHMARK_TEMPLATE(BM_random_lookup, 1, 45)
    ->Name("random lookup: std::pmr::vector<std::pmr::vector<float>>")
    ->ArgsProduct({INPUT_SIZES, {15, 30, 45}});

BENCHMARK_TEMPLATE(BM_random_lookup, 2, 45)
    ->Name("random lookup: ContiguousVector<FixedSize<float>>")
    ->ArgsProduct({INPUT_SIZES, {15, 30, 45}});

BENCHMARK_TEMPLATE(BM_random_lookup, 3, 45)
    ->Name("random lookup: ContiguousVector<VaryingSize<float>>")
    ->ArgsProduct({INPUT_SIZES, {15, 30, 45}});

template <std::size_t I>
struct VaryingSizeInputVectors
{
    using Input =
        std::remove_reference_t<decltype(std::get<I>(make_varying_size_input_vectors(std::size_t{}, std::size_t{})))>;

    Input input;

    VaryingSizeInputVectors(benchmark::State& state)
        : input(
              [&]
              {
                  gen.seed();
                  return std::get<I>(make_varying_size_input_vectors(state.range(0), state.range(1)));
              }())
    {
        state.counters["elements"] = static_cast<double>(state.range(0));
        state.counters["variance"] = static_cast<double>(state.range(1));
    }
};

static std::vector<int64_t> VARYING_ITERATION_INPUT_SIZES{50000, 100000, 250000};
static std::vector<int64_t> VARYING_ITERATION_VARIANCES{10, 100, 1000};

template <std::size_t I>
void BM_full_varying_iteration(benchmark::State& state)
{
    auto [input] = VaryingSizeInputVectors<0>{state};
    for (auto _ : state)
    {
        iterate(input, {});
    }
}

BENCHMARK_TEMPLATE(BM_full_varying_iteration, 0)
    ->Name("full varying iteration: std::pmr::vector<std::pmr::vector<float>>")
    ->ArgsProduct({VARYING_ITERATION_INPUT_SIZES, VARYING_ITERATION_VARIANCES});

BENCHMARK_TEMPLATE(BM_full_varying_iteration, 1)
    ->Name("full varying iteration: ContiguousVector<VaryingSize<float>>")
    ->ArgsProduct({VARYING_ITERATION_INPUT_SIZES, VARYING_ITERATION_VARIANCES});

template <std::size_t I>
void BM_random_varying_lookup(benchmark::State& state)
{
    auto [input] = VaryingSizeInputVectors<0>{state};
    auto indices = make_indices(state.range(0));
    for (auto _ : state)
    {
        random_lookup(input, indices);
    }
}

BENCHMARK_TEMPLATE(BM_random_varying_lookup, 0)
    ->Name("random varying lookup: std::pmr::vector<std::pmr::vector<float>>")
    ->ArgsProduct({VARYING_ITERATION_INPUT_SIZES, VARYING_ITERATION_VARIANCES});

BENCHMARK_TEMPLATE(BM_random_varying_lookup, 1)
    ->Name("random varying lookup: ContiguousVector<VaryingSize<float>>")
    ->ArgsProduct({VARYING_ITERATION_INPUT_SIZES, VARYING_ITERATION_VARIANCES});

//---- Multi element

template <std::size_t I, std::size_t N>
struct TwoElementInputVectors
{
    using Input =
        std::remove_reference_t<decltype(std::get<I>(make_two_element_input_vectors<N>(std::size_t{}, std::size_t{})))>;

    std::size_t fixed_size;
    Input input;

    TwoElementInputVectors(benchmark::State& state)
        : fixed_size(state.range(1)),
          input(
              [&]
              {
                  gen.seed();
                  return std::get<I>(make_two_element_input_vectors<N>(state.range(0), fixed_size));
              }())
    {
        state.counters["elements"] = static_cast<double>(state.range(0));
        state.counters["fixed size"] = static_cast<double>(fixed_size);
    }
};

template <std::size_t I, std::size_t N>
void BM_full_iteration_two(benchmark::State& state)
{
    auto [fixed_size, input] = TwoElementInputVectors<I, N>{state};
    for (auto _ : state)
    {
        iterate(input, fixed_size);
    }
}

BENCHMARK_TEMPLATE(BM_full_iteration_two, 0, 15)
    ->Name("full iteration2: std::vector<struct(std::array<float, 15>,float,std::array<float, 15>)>")
    ->ArgsProduct({INPUT_SIZES, {20, 25, 30}});

BENCHMARK_TEMPLATE(BM_full_iteration_two, 0, 45)
    ->Name("full iteration2: std::vector<struct(std::array<float, 45>,float,std::array<float, 45>)>")
    ->ArgsProduct({INPUT_SIZES, {70, 80, 90}});

BENCHMARK_TEMPLATE(BM_full_iteration_two, 1, 45)
    ->Name("full iteration2: std::pmr::vector<struct(std::pmr::vector<float>,float,std::pmr::vector<float>)>")
    ->ArgsProduct({INPUT_SIZES, {20, 25, 30, 70, 80, 90}});

BENCHMARK_TEMPLATE(BM_full_iteration_two, 2, 45)
    ->Name("full iteration2: ContiguousVector<FixedSize<float>,float,FixedSize<float>>")
    ->ArgsProduct({INPUT_SIZES, {20, 25, 30, 70, 80, 90}});

BENCHMARK_TEMPLATE(BM_full_iteration_two, 3, 45)
    ->Name("full iteration2: ContiguousVector<VaryingSize<float>,float,VaryingSize<float>>")
    ->ArgsProduct({INPUT_SIZES, {20, 25, 30, 70, 80, 90}});
