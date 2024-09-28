// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_UTILS_CODEGENPARSER_HPP
#define CNTGS_UTILS_CODEGENPARSER_HPP

#include <filesystem>
#include <string>
#include <vector>

namespace test
{
std::vector<std::string> get_disassembly_of_functions(const std::filesystem::path& disassembly_file,
                                                      const std::vector<std::string>& function_names);
}  // namespace test

#endif  // CNTGS_UTILS_CODEGENPARSER_HPP
