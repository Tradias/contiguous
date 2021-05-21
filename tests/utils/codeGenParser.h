#pragma once

#include "cntgs/contiguous.h"

#include <string>
#include <string_view>
#include <vector>

namespace cntgs::test
{
std::vector<std::string> get_disassembly_of_functions(cntgs::Span<std::string_view> function_names);
}  // namespace cntgs::test
