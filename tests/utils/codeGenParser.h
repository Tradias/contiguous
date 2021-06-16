#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace cntgs::test
{
std::vector<std::string> get_disassembly_of_functions(const std::filesystem::path& disassembly_file,
                                                      const std::vector<std::string>& function_names);
}  // namespace cntgs::test
