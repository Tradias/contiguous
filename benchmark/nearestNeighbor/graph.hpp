// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_NEARESTNEIGHBOR_GRAPH_HPP
#define CNTGS_NEARESTNEIGHBOR_GRAPH_HPP

#include "nearestNeighbor/distance.hpp"

#include <cntgs/contiguous.hpp>

#include <array>
#include <cstdint>
#include <memory_resource>
#include <unordered_map>

namespace cntgs::bench
{
template <class Container>
struct GraphBase
{
    std::unordered_map<uint32_t, uint32_t> label_to_index;
    bench::L2Space feature_space;
    Container container;

    GraphBase(bench::L2Space feature_space, Container container)
        : feature_space(feature_space), container(std::move(container))
    {
    }

    auto get_internal_index(uint32_t external_label) const { return label_to_index.find(external_label)->second; }

    auto size() const { return label_to_index.size(); }
};

template <class Container>
struct Graph;

using FixedSizeContainer = cntgs::ContiguousVector<cntgs::FixedSize<char>, cntgs::FixedSize<uint32_t>, uint32_t>;

template <>
struct Graph<FixedSizeContainer> : GraphBase<FixedSizeContainer>
{
    using GraphBase<FixedSizeContainer>::GraphBase;

    static auto construct(size_t max_node_count, bench::L2Space feature_space, size_t edges_per_node)
    {
        return Graph<FixedSizeContainer>{
            feature_space, FixedSizeContainer{max_node_count, {feature_space.get_data_size(), edges_per_node}}};
    }

    auto get_external_label(size_t i) const { return cntgs::get<2>(this->container[i]); }

    auto feature_by_index(size_t i) const { return std::next(this->container.begin(), i).data(); }

    auto neighbors_by_index(size_t i) const { return cntgs::get<1>(this->container[i]); }
};

struct VectorContainer
{
    struct Data
    {
        using allocator_type = std::pmr::polymorphic_allocator<std::byte>;

        std::pmr::vector<char> features;
        std::pmr::vector<uint32_t> neighbor_indices;
        uint32_t external_label;

        explicit Data(const allocator_type& allocator)
            : features(allocator), neighbor_indices(allocator), external_label{}
        {
        }

        Data(Data&& other, const allocator_type& allocator) noexcept
            : features(std::move(other.features), allocator),
              neighbor_indices(std::move(other.neighbor_indices), allocator),
              external_label(other.external_label)
        {
        }

        allocator_type get_allocator() const noexcept { return features.get_allocator(); }
    };

    static constexpr auto MSVC_VECTOR_CONSTRUCTION_OVERHEAD = 16;
    static constexpr auto LEEWAY = 256;

    std::unique_ptr<std::pmr::monotonic_buffer_resource> resource;
    std::pmr::vector<Data> data;
    size_t last_insertion_index{};

    VectorContainer(size_t elements, size_t features, size_t neighbor_indices)
        : resource(std::make_unique<std::pmr::monotonic_buffer_resource>(
              elements * sizeof(Data) + features * sizeof(char) + neighbor_indices * sizeof(uint32_t) +
              elements * MSVC_VECTOR_CONSTRUCTION_OVERHEAD * 2 + LEEWAY)),
          data(elements, resource.get())
    {
    }

    template <class Features, class NeighborIndices>
    void emplace_back(Features&& features, NeighborIndices&& neighbor_indices, uint32_t external_label)
    {
        data[last_insertion_index].features.assign(features.begin(), features.end());
        data[last_insertion_index].neighbor_indices.assign(neighbor_indices.begin(), neighbor_indices.end());
        data[last_insertion_index].external_label = external_label;
        ++last_insertion_index;
    }
};

template <>
struct Graph<VectorContainer> : GraphBase<VectorContainer>
{
    using GraphBase<VectorContainer>::GraphBase;

    static auto construct(size_t max_node_count, bench::L2Space feature_space, size_t edges_per_node)
    {
        return Graph<VectorContainer>{feature_space,
                                      VectorContainer{max_node_count, feature_space.get_data_size(), edges_per_node}};
    }

    auto get_external_label(size_t i) const { return this->container.data[i].external_label; }

    auto feature_by_index(size_t i) const { return this->container.data[i].features.data(); }

    auto& neighbors_by_index(size_t i) const { return this->container.data[i].neighbor_indices; }
};

struct ArrayContainer
{
    struct Data
    {
        std::array<char, 512> features;
        std::array<uint32_t, 24> neighbor_indices;
        uint32_t external_label;
    };

    std::vector<Data> data;
    size_t last_insertion_index{};

    explicit ArrayContainer(size_t elements) : data(elements) {}

    template <class Features, class NeighborIndices>
    void emplace_back(Features&& features, NeighborIndices&& neighbor_indices, uint32_t external_label)
    {
        std::copy(features.begin(), features.end(), data[last_insertion_index].features.begin());
        std::copy(neighbor_indices.begin(), neighbor_indices.end(),
                  data[last_insertion_index].neighbor_indices.begin());
        data[last_insertion_index].external_label = external_label;
        ++last_insertion_index;
    }
};

template <>
struct Graph<ArrayContainer> : GraphBase<ArrayContainer>
{
    using GraphBase<ArrayContainer>::GraphBase;

    static auto construct(size_t max_node_count, bench::L2Space feature_space, size_t)
    {
        return Graph<ArrayContainer>{feature_space, ArrayContainer{max_node_count}};
    }

    auto get_external_label(size_t i) const { return this->container.data[i].external_label; }

    auto feature_by_index(size_t i) const { return this->container.data[i].features.data(); }

    auto& neighbors_by_index(size_t i) const { return this->container.data[i].neighbor_indices; }
};
}  // namespace cntgs::bench

#endif  // CNTGS_NEARESTNEIGHBOR_GRAPH_HPP
