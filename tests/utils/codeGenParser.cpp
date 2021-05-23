#include "utils/codeGenParser.h"

#include <algorithm>
#include <fstream>
#include <limits>
#include <string>
#include <vector>

namespace cntgs::test
{
std::vector<std::string> get_disassembly_of_functions(const std::vector<std::string>& function_names)
{
    static constexpr size_t MAX_LINE_SIZE{1500};
    std::vector<std::string> result{function_names.size()};
    std::ifstream stream{CNTGS_CODE_GEN_DISASSEMBLY_FILE};
    if (!stream)
    {
        return result;
    }
    std::string line(MAX_LINE_SIZE, '\0');
    auto current_index = std::numeric_limits<int>::max();
    std::string current_string;
    while (stream.getline(line.data(), line.size()))
    {
        if (current_index == std::numeric_limits<int>::max())
        {
            auto next_index = 0;
            for (size_t i = 0; i < function_names.size(); i++)
            {
                if (std::search(line.begin(), line.end(), function_names[i].begin(), function_names[i].end()) !=
                    line.end())
                {
                    current_index = next_index;
                }
                else
                {
                    ++next_index;
                }
            }
        }
        else
        {
            if (std::string_view(line.data()).empty())
            {
                std::swap(current_string, result[current_index]);
                current_index = std::numeric_limits<int>::max();
            }
            else
            {
                current_string.append("\n").append(std::string_view(line.data()));
            }
        }
        line.assign(MAX_LINE_SIZE, '\0');
    }
    if (!current_string.empty())
    {
        result[current_index] = std::move(current_string);
    }
    return result;
}
}  // namespace cntgs::test