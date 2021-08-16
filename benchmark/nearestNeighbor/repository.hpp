// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_NEARESTNEIGHBOR_REPOSITORY_HPP
#define CNTGS_NEARESTNEIGHBOR_REPOSITORY_HPP

#include "nearestNeighbor/load.hpp"

#include <cassert>
#include <cstddef>
#include <cstring>
#include <memory>

namespace cntgs::bench
{
struct StaticFeatureRepository
{
    size_t dims;
    size_t size;
    std::unique_ptr<float[]> contiguous_features;

    StaticFeatureRepository(std::unique_ptr<float[]> contiguous_features, size_t dims, size_t size)
        : contiguous_features{std::move(contiguous_features)}, dims{dims}, size{size}
    {
    }

    const float* get_feature(uint32_t node_id) const { return &contiguous_features[node_id * dims]; }
};

auto fvecs_read(const std::filesystem::path& fname, size_t& d_out, size_t& n_out)
{
    auto ifstream = open_file_stream(fname);
    const auto file_size = std::filesystem::file_size(fname);

    int dims;
    ifstream.read(reinterpret_cast<char*>(&dims), sizeof(int));
    assert((dims > 0 && dims < 1000000) || !"unreasonable dimension");
    assert(file_size % ((dims + 1) * 4) == 0 || !"weird file size");
    size_t n = file_size / ((dims + 1) * 4);

    d_out = dims;
    n_out = n;

    auto x = std::make_unique<float[]>(n * (dims + 1));
    ifstream.seekg(0);
    ifstream.read(reinterpret_cast<char*>(x.get()), n * (dims + 1) * sizeof(float));
    if (!ifstream)
    {
        assert(ifstream.gcount() == static_cast<int>(n * (dims + 1)) || !"could not read whole file");
    }

    // shift array to remove row headers
    for (size_t i = 0; i < n; i++)
    {
        std::memmove(&x[i * dims], &x[1 + i * (dims + 1)], dims * sizeof(float));
    }

    return x;
}

StaticFeatureRepository load_static_repository(const std::filesystem::path& path_repository)
{
    size_t dims;
    size_t count;
    auto contiguous_features = fvecs_read(path_repository, dims, count);
    return {std::move(contiguous_features), dims, count};
}
}  // namespace cntgs::bench

#endif  // CNTGS_NEARESTNEIGHBOR_REPOSITORY_HPP
