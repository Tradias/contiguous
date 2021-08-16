// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "nearestNeighbor/graph.hpp"
#include "nearestNeighbor/load.hpp"
#include "nearestNeighbor/repository.hpp"
#include "nearestNeighbor/search.hpp"

#include <benchmark/benchmark.h>

namespace cntgs::bench
{
template <class Container>
void BM_nearest_neighbor(benchmark::State& state)
{
    std::filesystem::path data_path{CNTGS_BENCHMARK_GRAPH_DATA_DIR};
    auto repository = bench::load_static_repository(data_path / "SIFT1M" / "sift_query.fvecs");
    auto graph = bench::load_graph<Container>(
        data_path / "k24nns_128D_L2_Path10_Rnd3+3_AddK20Eps0.2_ImproveK20Eps0.02_ImproveExtK12-1StepEps0.02.deg");
    const std::vector<uint32_t> entry_node_indices{graph.get_internal_index(0)};
    const auto search_radius_epsilon = 0.01f;
    const auto search_results = 100;
    for (auto _ : state)
    {
        for (int i = 0; i < repository.size; i++)
        {
            auto query = reinterpret_cast<const std::byte*>(repository.get_feature(i));
            auto result_queue =
                bench::yahoo_search(graph, entry_node_indices, query, search_radius_epsilon, search_results);
            benchmark::DoNotOptimize(result_queue);
        }
    }
}

static constexpr auto ITERATION = 7;

BENCHMARK_TEMPLATE(BM_nearest_neighbor, bench::FixedSizeContainer)
    ->Name("nearest neighbor cntgs")
    ->Iterations(ITERATION);

BENCHMARK_TEMPLATE(BM_nearest_neighbor, bench::VectorContainer)
    ->Name("nearest neighbor pmr::vector")
    ->Iterations(ITERATION);

BENCHMARK_TEMPLATE(BM_nearest_neighbor, bench::ArrayContainer)->Name("nearest neighbor array")->Iterations(ITERATION);
}  // namespace cntgs::bench
