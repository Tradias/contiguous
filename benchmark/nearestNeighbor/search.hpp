// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_NEARESTNEIGHBOR_SEARCH_HPP
#define CNTGS_NEARESTNEIGHBOR_SEARCH_HPP

#include "nearestNeighbor/graph.hpp"

#include <cstdint>
#include <limits>
#include <queue>

namespace cntgs::bench
{
struct ObjectDistance
{
    uint32_t internal_index;
    float distance;

    ObjectDistance() = default;

    ObjectDistance(const uint32_t internal_index, const float distance)
        : internal_index(internal_index), distance(distance)
    {
    }

    bool operator==(const ObjectDistance& o) const
    {
        return (distance == o.distance) && (internal_index == o.internal_index);
    }

    bool operator<(const ObjectDistance& o) const
    {
        if (distance == o.distance)
        {
            return internal_index < o.internal_index;
        }
        else
        {
            return distance < o.distance;
        }
    }

    bool operator>(const ObjectDistance& o) const
    {
        if (distance == o.distance)
        {
            return internal_index > o.internal_index;
        }
        else
        {
            return distance > o.distance;
        }
    }
};

template <class Container>
auto yahoo_search(const bench::Graph<Container>& graph, const std::vector<uint32_t>& entry_node_indizies,
                  const std::byte* query, const float eps, const int k)
{
    using ResultSet = std::priority_queue<ObjectDistance, std::vector<ObjectDistance>, std::less<ObjectDistance>>;
    using UncheckedSet = std::priority_queue<ObjectDistance, std::vector<ObjectDistance>, std::greater<ObjectDistance>>;

    const auto dist_func = graph.feature_space.get_dist_func();
    const auto dist_func_param = graph.feature_space.get_dist_func_param();

    // set of checked node ids
    auto checked_ids = std::vector<bool>(graph.size());

    // items to traverse next
    auto next_nodes = UncheckedSet();

    auto results = ResultSet();

    // copy the initial entry nodes and their distances to the query into the three containers
    for (auto&& index : entry_node_indizies)
    {
        checked_ids[index] = true;

        const auto feature = reinterpret_cast<const float*>(graph.feature_by_index(index));
        const auto distance = dist_func(query, feature, dist_func_param);
        next_nodes.emplace(index, distance);
        results.emplace(index, distance);
    }

    // search radius
    auto r = std::numeric_limits<float>::max();

    // iterate as long as good elements are in the next_nodes queue
    auto good_neighbors = std::array<uint32_t, 256>();
    while (next_nodes.empty() == false)
    {
        // next node to check
        const auto next_node = next_nodes.top();
        next_nodes.pop();

        // max distance reached
        if (next_node.distance > r * (1 + eps))
        {
            break;
        }

        size_t good_neighbor_count = 0;
        const auto neighbor_indices = graph.neighbors_by_index(next_node.internal_index);
        for (auto&& neighbor_index : neighbor_indices)
        {
            if (checked_ids[neighbor_index] == false)
            {
                checked_ids[neighbor_index] = true;
                good_neighbors[good_neighbor_count++] = neighbor_index;
            }
        }

        if (good_neighbor_count == 0) continue;

        // bench::prefetch(reinterpret_cast<const char*>(graph.feature_by_index(good_neighbors[0])));
        for (size_t i = 0; i < good_neighbor_count; i++)
        {
            // bench::prefetch(reinterpret_cast<const char*>(
            //     graph.feature_by_index(good_neighbors[std::min(i + 1, good_neighbor_count - 1)])));

            const auto neighbor_index = good_neighbors[i];
            const auto neighbor_feature_vector = graph.feature_by_index(neighbor_index);
            const auto neighbor_distance = dist_func(query, neighbor_feature_vector, dist_func_param);

            // check the neighborhood of this node later, if its good enough
            if (neighbor_distance <= r * (1 + eps))
            {
                next_nodes.emplace(neighbor_index, neighbor_distance);

                // remember the node, if its better than the worst in the result list
                if (neighbor_distance < r)
                {
                    results.emplace(neighbor_index, neighbor_distance);

                    // update the search radius
                    if (results.size() > k)
                    {
                        results.pop();
                        r = results.top().distance;
                    }
                }
            }
        }
    }

    return results;
}
}  // namespace cntgs::bench

#endif  // CNTGS_NEARESTNEIGHBOR_SEARCH_HPP
