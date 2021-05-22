#pragma once

#include <string>
#include <vector>

namespace cntgs::test
{
std::vector<std::string> get_disassembly_of_functions(const std::vector<std::string>& function_names);
}  // namespace cntgs::test
