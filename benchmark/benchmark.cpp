#include "cntgs/contiguous.h"
#include "utils/string.h"

#define ANKERL_NANOBENCH_IMPLEMENT
#include <third-party/nanobench.h>

#include <memory_resource>
#include <random>
#include <vector>

using namespace cntgs::test;

using FixedSizeVector = cntgs::ContiguousVector<cntgs::FixedSize<float>>;
using VaryingSizeVector = cntgs::ContiguousVector<cntgs::VaryingSize<float>>;

static constexpr auto FLOAT_MAX = 100000.f;
static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_real_distribution<float> float_dist(0.f, FLOAT_MAX);

auto work(float e)
{
    auto v = std::abs(e) + std::sqrt(e);
    ankerl::nanobench::doNotOptimizeAway(v);
}

template <class Target>
void fill_vector(Target& target, const std::vector<std::vector<float>>& source)
{
    target.reserve(source.size());
    for (size_t i = 0; i < source.size(); i++)
    {
        target.emplace_back();
    }
    for (size_t i = 0; i < source.size(); i++)
    {
        target[i].assign(source[i].begin(), source[i].end());
    }
}

template <class... T>
auto iterate(const cntgs::ContiguousVector<T...>& vector, const size_t&)
{
    return [&] {
        for (auto&& [elem] : vector)
        {
            for (auto&& e : elem)
            {
                work(e);
            }
        }
    };
}

template <class... T>
auto iterate(const std::vector<T...>& vector, const size_t&)
{
    return [&] {
        for (auto&& elem : vector)
        {
            for (auto&& e : elem)
            {
                work(e);
            }
        }
    };
}

template <class T, std::size_t N>
auto iterate(const std::vector<std::array<T, N>>& vector, const size_t& fixed_size)
{
    return [&] {
        for (auto&& elem : vector)
        {
            for (size_t i = 0; i < fixed_size; i++)
            {
                work(elem[i]);
            }
        }
    };
}

template <class... T>
auto random_lookup(const cntgs::ContiguousVector<T...>& vector, const std::vector<size_t>& indices)
{
    return [&] {
        for (auto&& j : indices)
        {
            for (auto&& elem : cntgs::get<0>(vector[j]))
            {
                work(elem);
            }
        }
    };
}

struct VectorVector
{
    std::unique_ptr<std::pmr::monotonic_buffer_resource> resource;
    std::pmr::vector<std::pmr::vector<float>> vector{resource.get()};
};

template <std::size_t N>
auto make_single_element_input_vectors(std::size_t elements, std::size_t fixed_size)
{
    std::vector<std::vector<float>> input{elements};
    for (auto&& v : input)
    {
        v.resize(fixed_size);
        std::generate_n(v.begin(), fixed_size, [&] { return float_dist(gen); });
    }
    std::vector<std::array<float, N>> array_vector{input.size()};
    for (size_t i = 0; i < input.size(); i++)
    {
        std::copy(input[i].begin(), input[i].end(), array_vector[i].begin());
    }
    VectorVector vector_vector{std::make_unique<std::pmr::monotonic_buffer_resource>(
        elements * fixed_size * 16 * sizeof(float) * sizeof(std::pmr::vector<float>))};
    fill_vector(vector_vector.vector, input);
    FixedSizeVector fixed_size_vector{input.size(), {fixed_size}};
    for (size_t i = 0; i < input.size(); i++)
    {
        fixed_size_vector.emplace_back(input[i]);
    }
    VaryingSizeVector varying_size_vector{input.size(), input.size() * fixed_size * sizeof(float)};
    for (size_t i = 0; i < input.size(); i++)
    {
        varying_size_vector.emplace_back(input[i]);
    }
    input.clear();
    input.shrink_to_fit();
    return std::tuple{std::move(array_vector), std::move(vector_vector), std::move(fixed_size_vector),
                      std::move(varying_size_vector)};
}

template <std::size_t N>
void full_iteration(std::size_t elements, std::size_t fixed_size)
{
    auto [array_vector, vector_vector, fixed_size_vector, varying_size_vector] =
        make_single_element_input_vectors<N>(elements, fixed_size);
    ankerl::nanobench::Bench().run(
        format("full_iteration: std::array<float, {}> elements: {} fixed_size: {}", N, elements, fixed_size),
        iterate(array_vector, fixed_size));
    ankerl::nanobench::Bench().run(
        format("full_iteration: std::pmr::vector<std::pmr::vector<float>> elements: {} fixed_size: {}", elements,
               fixed_size),
        iterate(vector_vector.vector, fixed_size));
    ankerl::nanobench::Bench().run(
        format("full_iteration: ContiguousVector<FixedSize<float>> elements: {} fixed_size: {}", elements, fixed_size),
        iterate(fixed_size_vector, fixed_size));
    ankerl::nanobench::Bench().run(
        format("full_iteration: ContiguousVector<VaryingSize<float>> elements: {} fixed_size: {}", elements,
               fixed_size),
        iterate(varying_size_vector, fixed_size));
}

template <std::size_t N>
void random_lookup(std::size_t elements, std::size_t fixed_size)
{
    auto [array_vector, vector_vector, fixed_size_vector, varying_size_vector] =
        make_single_element_input_vectors<N>(elements, fixed_size);
    std::uniform_int_distribution<size_t> size_t_dist(0, elements - 1);
    std::vector<size_t> indices(1000000);
    std::generate(indices.begin(), indices.end(), [&] { return size_t_dist(gen); });
    ankerl::nanobench::Bench().run(
        format("random_lookup: std::array<float, {}> elements: {} fixed_size: {}", N, elements, fixed_size), [&] {
            for (auto&& j : indices)
            {
                for (auto&& elem : array_vector[j])
                {
                    work(elem);
                }
            }
        });
    ankerl::nanobench::Bench().run(
        format("random_lookup: std::pmr::vector<std::pmr::vector<float>> elements: {} fixed_size: {}", elements,
               fixed_size),
        [&] {
            for (auto&& j : indices)
            {
                for (auto&& elem : vector_vector.vector[j])
                {
                    work(elem);
                }
            }
        });
    ankerl::nanobench::Bench().run(
        format("random_lookup: ContiguousVector<FixedSize<float>> elements: {} fixed_size: {}", elements, fixed_size),
        random_lookup(fixed_size_vector, indices));
    ankerl::nanobench::Bench().run(
        format("random_lookup: ContiguousVector<VaryingSize<float>> elements: {} fixed_size: {}", elements, fixed_size),
        random_lookup(fixed_size_vector, indices));
}

auto make_varying_since_input_vectors(std::size_t elements, uint32_t variance)
{
    std::uniform_int_distribution<uint32_t> int_dist(0, variance);
    std::vector<std::vector<float>> input{elements};
    size_t total_size = 0;
    for (auto&& v : input)
    {
        auto size = int_dist(gen);
        v.resize(size);
        total_size += v.size();
        std::generate(v.begin(), v.end(), [&] { return float_dist(gen); });
    }
    VectorVector vector_vector{std::make_unique<std::pmr::monotonic_buffer_resource>(
        total_size * sizeof(float) + elements * 16 * sizeof(std::pmr::vector<float>))};
    fill_vector(vector_vector.vector, input);
    VaryingSizeVector varying_size_vector{input.size(), total_size * sizeof(float)};
    for (size_t i = 0; i < input.size(); i++)
    {
        varying_size_vector.emplace_back(input[i]);
    }
    input.clear();
    input.shrink_to_fit();
    return std::tuple{std::move(vector_vector), std::move(varying_size_vector)};
}

void full_iteration_varying(std::size_t elements, std::uint32_t variance)
{
    auto [vector_vector, varying_size_vector] = make_varying_since_input_vectors(elements, variance);
    size_t dummy;
    ankerl::nanobench::Bench().run(
        format("full_iteration: std::pmr::vector<std::pmr::vector<float>> elements: {} variance: 0-{}", elements,
               variance),
        iterate(vector_vector.vector, dummy));
    ankerl::nanobench::Bench().run(
        format("full_iteration: ContiguousVector<VaryingSize<float>> elements: {} variance: 0-{}", elements, variance),
        iterate(varying_size_vector, dummy));
}

void random_lookup_varying(std::size_t elements, std::uint32_t variance)
{
    auto [vector_vector, varying_size_vector] = make_varying_since_input_vectors(elements, variance);
    std::uniform_int_distribution<size_t> size_t_dist(0, elements - 1);
    std::vector<size_t> indices(1000000);
    std::generate(indices.begin(), indices.end(), [&] { return size_t_dist(gen); });
    ankerl::nanobench::Bench().run(
        format("random_lookup: std::pmr::vector<std::pmr::vector<float>> elements: {} variance: 0-{}", elements,
               variance),
        [&] {
            for (auto&& j : indices)
            {
                for (auto&& elem : vector_vector.vector[j])
                {
                    work(elem);
                }
            }
        });
    ankerl::nanobench::Bench().run(
        format("random_lookup: ContiguousVector<VaryingSize<float>> elements: {} variance: 0-{}", elements, variance),
        random_lookup(varying_size_vector, indices));
}

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

template <std::size_t N, std::size_t K>
void full_iteration_two(std::size_t elements, std::size_t fixed_size)
{
    std::vector<std::vector<float>> input{elements};
    for (auto&& v : input)
    {
        v.resize(fixed_size);
        std::generate_n(v.begin(), fixed_size, [&] { return float_dist(gen); });
    }
    std::vector<TwoArray<N, K>> array_vector{input.size()};
    for (size_t i = 0; i < input.size(); i++)
    {
        std::copy(input[i].begin(), input[i].begin() + input[i].size() / 2, array_vector[i].a.begin());
        array_vector[i].b = input[i].front();
        std::copy(input[i].begin() + input[i].size() / 2, input[i].end(), array_vector[i].c.begin());
    }
    std::pmr::monotonic_buffer_resource resource{elements * sizeof(TwoVector) + fixed_size * sizeof(float)};
    std::pmr::vector<TwoVector> vector_vector{&resource};
    vector_vector.reserve(input.size());
    for (size_t i = 0; i < input.size(); i++)
    {
        vector_vector.emplace_back();
    }
    for (size_t i = 0; i < input.size(); i++)
    {
        vector_vector[i].a.assign(input[i].begin(), input[i].begin() + input[i].size() / 2);
        vector_vector[i].b = input[i].front();
        vector_vector[i].c.assign(input[i].begin() + input[i].size() / 2, input[i].end());
    }
    cntgs::ContiguousVector<cntgs::FixedSize<float>, float, cntgs::FixedSize<float>> fixed_size_vector{
        input.size(), {fixed_size / 2, fixed_size / 2}};
    for (size_t i = 0; i < input.size(); i++)
    {
        fixed_size_vector.emplace_back(input[i].begin(), input[i].front(), input[i].begin() + input[i].size() / 2);
    }
    cntgs::ContiguousVector<cntgs::VaryingSize<float>, float, cntgs::VaryingSize<float>> varying_size_vector{
        input.size(), input.size() * fixed_size * sizeof(float)};
    for (size_t i = 0; i < input.size(); i++)
    {
        std::vector<float> firsts{input[i].begin(), input[i].begin() + input[i].size() / 2};
        std::vector<float> seconds{input[i].begin() + input[i].size() / 2, input[i].end()};
        varying_size_vector.emplace_back(firsts, input[i].front(), seconds);
    }
    input.clear();
    input.shrink_to_fit();
    ankerl::nanobench::Bench().run(format("full_iteration: std::vector<struct(std::array<float, "
                                          "{}>,float,std::array<float, {}>)> elements: {} fixed_size: {}",
                                          N, K, elements, fixed_size),
                                   [&] {
                                       for (auto&& elem : array_vector)
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
                                   });
    ankerl::nanobench::Bench().run(format("full_iteration: std::pmr::vector<struct(std::pmr::vector<float>,float,"
                                          "std::pmr::vector<float>)> elements: {} fixed_size: {}",
                                          elements, fixed_size),
                                   [&] {
                                       for (auto&& elem : vector_vector)
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
                                   });
    ankerl::nanobench::Bench().run(
        format("full_iteration: ContiguousVector<FixedSize<float>,float,FixedSize<float>> elements: {} fixed_size: {}",
               elements, fixed_size),
        [&] {
            for (auto&& [a, b, c] : fixed_size_vector)
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
        });
    ankerl::nanobench::Bench().run(
        format(
            "full_iteration: ContiguousVector<VaryingSize<float>,float,VaryingSize<float>> elements: {} fixed_size: {}",
            elements, fixed_size),
        [&] {
            for (auto&& [a, b, c] : fixed_size_vector)
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
        });
}

int main()
{
    auto random = static_cast<uint32_t>(float_dist(gen) / FLOAT_MAX * 2);
    full_iteration<15>(500000, random + 12);
    full_iteration<30>(500000, random + 12);
    full_iteration<45>(500000, random + 12);
    full_iteration<100>(50000, random + 97);
    full_iteration<200>(50000, random + 97);
    random_lookup<15>(500000, random + 13);
    random_lookup<30>(500000, random + 13);
    random_lookup<45>(500000, random + 13);
    random_lookup<100>(50000, random + 98);
    random_lookup<200>(50000, random + 98);
    full_iteration_varying(500000, random + 10);
    full_iteration_varying(100000, random + 100);
    full_iteration_varying(50000, random + 1000);
    random_lookup_varying(500000, random + 10);
    random_lookup_varying(100000, random + 100);
    random_lookup_varying(50000, random + 1000);
    auto multiple_of_two = random % 2u == 1u ? random - 1u : random;
    full_iteration_two<15, 15>(500000, multiple_of_two + 28);
    full_iteration_two<30, 30>(500000, multiple_of_two + 28);
    full_iteration_two<45, 45>(500000, multiple_of_two + 28);
    full_iteration_two<100, 100>(50000, multiple_of_two + 98);
    full_iteration_two<200, 200>(50000, multiple_of_two + 98);
}