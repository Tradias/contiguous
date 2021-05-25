#include "cntgs/contiguous.h"
#include "utils/string.h"

#define ANKERL_NANOBENCH_IMPLEMENT
#include <../third-party/nanobench.h>

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

template <std::size_t N>
void array_vs_cntgs(std::size_t elements, std::size_t fixed_size)
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
    std::pmr::monotonic_buffer_resource resource{elements * 16 * sizeof(float) * sizeof(std::pmr::vector<float>)};
    std::pmr::vector<std::pmr::vector<float>> vector_vector{&resource};
    fill_vector(vector_vector, input);
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
    ankerl::nanobench::Bench().minEpochIterations(5).run(
        format("std::array<{}> elements: {} fixed_size: {}", N, elements, fixed_size), [&] {
            for (auto&& elem : array_vector)
            {
                for (size_t i = 0; i < fixed_size; i++)
                {
                    work(elem[i]);
                }
            }
        });
    ankerl::nanobench::Bench().minEpochIterations(5).run(
        format("std::vector<std::vector> elements: {} fixed_size: {}", elements, fixed_size), [&] {
            for (auto&& elem : vector_vector)
            {
                for (auto&& e : elem)
                {
                    work(e);
                }
            }
        });
    ankerl::nanobench::Bench().minEpochIterations(5).run(
        format("cntgs::FixedSize elements: {} fixed_size: {}", elements, fixed_size), [&] {
            for (auto&& [elem] : fixed_size_vector)
            {
                for (auto&& e : elem)
                {
                    work(e);
                }
            }
        });
    ankerl::nanobench::Bench().minEpochIterations(5).run(
        format("cntgs::VaryingSize elements: {} fixed_size: {}", elements, fixed_size), [&] {
            for (auto&& [elem] : varying_size_vector)
            {
                for (auto&& e : elem)
                {
                    work(e);
                }
            }
        });
}

void vector_vs_cntgs(std::size_t elements, uint32_t variance)
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
    std::pmr::monotonic_buffer_resource resource{total_size * sizeof(float) +
                                                 elements * 16 * sizeof(std::pmr::vector<float>)};
    std::pmr::vector<std::pmr::vector<float>> vector_vector{&resource};
    fill_vector(vector_vector, input);
    VaryingSizeVector varying_size_vector{input.size(), total_size * sizeof(float)};
    for (size_t i = 0; i < input.size(); i++)
    {
        varying_size_vector.emplace_back(input[i]);
    }
    input.clear();
    input.shrink_to_fit();
    ankerl::nanobench::Bench().run(format("std::vector<std::vector> elements: {} variance: 0-{}", elements, variance),
                                   [&] {
                                       for (auto&& elem : vector_vector)
                                       {
                                           for (auto&& e : elem)
                                           {
                                               work(e);
                                           }
                                       }
                                   });
    ankerl::nanobench::Bench().run(format("cntgs::VaryingSize elements: {} variance: 0-{}", elements, variance), [&] {
        for (auto&& [elem] : varying_size_vector)
        {
            for (auto&& e : elem)
            {
                work(e);
            }
        }
    });
}

int main()
{
    auto random = static_cast<uint32_t>(float_dist(gen) / FLOAT_MAX * 3);
    array_vs_cntgs<15>(500000, random + 12);
    array_vs_cntgs<30>(500000, random + 12);
    array_vs_cntgs<45>(500000, random + 12);
    array_vs_cntgs<100>(50000, random + 97);
    array_vs_cntgs<200>(50000, random + 97);
    vector_vs_cntgs(500000, random + 10);
    vector_vs_cntgs(100000, random + 100);
    vector_vs_cntgs(50000, random + 1000);
}