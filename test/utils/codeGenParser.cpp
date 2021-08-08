#include "utils/codeGenParser.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <limits>
#include <string>
#include <vector>

namespace cntgs::test
{
namespace
{
auto find_function_name_in_current_line(std::string_view line, const std::vector<std::string>& function_names)
{
    auto next_index = 0;
    for (size_t i = 0; i < function_names.size(); i++)
    {
        if (std::search(line.begin(), line.end(), function_names[i].begin(), function_names[i].end()) != line.end())
        {
            return next_index;
        }
        else
        {
            ++next_index;
        }
    }
    return std::numeric_limits<int>::max();
}
}  // namespace

std::vector<std::string> get_disassembly_of_functions(const std::filesystem::path& disassembly_file,
                                                      const std::vector<std::string>& function_names)
{
    static constexpr size_t MAX_LINE_SIZE{1500};
    std::vector<std::string> result{function_names.size()};
    std::ifstream stream{disassembly_file};
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
            current_index = find_function_name_in_current_line(line, function_names);
        }
        else
        {
            std::string_view line_chars{line.data()};
            if (line_chars.empty() || line_chars.find_first_of("?") == 0)
            {
                std::swap(current_string, result[current_index]);
                current_index = find_function_name_in_current_line(line_chars, function_names);
            }
            else
            {
                current_string.append("\n").append(std::string_view(line.data()));
            }
        }
        line.assign(MAX_LINE_SIZE, '\0');
    }
    if (!current_string.empty() && current_index != std::numeric_limits<int>::max())
    {
        result[current_index] = std::move(current_string);
    }
    return result;
}
}  // namespace cntgs::test