// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_NEARESTNEIGHBOR_LOAD_HPP
#define CNTGS_NEARESTNEIGHBOR_LOAD_HPP

#include "nearestNeighbor/distance.hpp"
#include "nearestNeighbor/graph.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

namespace cntgs::bench
{
auto open_file_stream(const std::filesystem::path& path)
{
    auto ifstream = std::ifstream(path, std::ios::binary);
    if (!ifstream)
    {
        std::cerr << "Could not open " << path << "\n";
        abort();
    }
    return ifstream;
}

template <class Container>
auto load(uint32_t max_node_count, uint8_t edges_per_node, bench::L2Space feature_space, std::ifstream& ifstream)
{
    auto graph = bench::Graph<Container>::construct(max_node_count, feature_space, edges_per_node);
    graph.label_to_index.reserve(max_node_count);
    std::vector<char> features;
    std::vector<uint32_t> neighbor_indices;
    for (uint32_t i = 0; i < max_node_count; i++)
    {
        features.resize(feature_space.get_data_size());
        ifstream.read(features.data(), features.size());

        neighbor_indices.resize(edges_per_node);
        ifstream.read(reinterpret_cast<char*>(neighbor_indices.data()), neighbor_indices.size() * sizeof(uint32_t));

        ifstream.ignore(uint32_t(edges_per_node) * sizeof(float));  // skip the weights

        uint32_t external_label;
        ifstream.read(reinterpret_cast<char*>(&external_label), sizeof(uint32_t));
        
        graph.container.emplace_back(features, neighbor_indices, external_label);
        graph.label_to_index.emplace(graph.get_external_label(i), i);
    }
    return graph;
}

template <class Container>
auto load_graph(const std::filesystem::path& path_graph)
{
    auto ifstream = open_file_stream(path_graph);

    // create feature space
    uint8_t data_type;
    ifstream.read(reinterpret_cast<char*>(&data_type), sizeof(data_type));
    uint16_t dim;
    ifstream.read(reinterpret_cast<char*>(&dim), sizeof(dim));
    auto feature_space = bench::L2Space(dim);

    // create the graph
    uint32_t size;
    ifstream.read(reinterpret_cast<char*>(&size), sizeof(size));
    uint8_t edges_per_node;
    ifstream.read(reinterpret_cast<char*>(&edges_per_node), sizeof(edges_per_node));

    return bench::load<Container>(size, edges_per_node, feature_space, ifstream);
}
}  // namespace cntgs::bench

#endif  // CNTGS_NEARESTNEIGHBOR_LOAD_HPP
