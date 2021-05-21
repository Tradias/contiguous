#include "utils/string.h"

#include <sstream>
#include <string>

namespace cntgs::test
{
size_t count_lines(const std::string& text)
{
    std::istringstream stream;
    stream.str(text);
    return std::count(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>(), '\n');
}
}  // namespace cntgs::test
