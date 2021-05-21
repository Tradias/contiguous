#include "cntgs/contiguous.h"
#include "utils/codeGenParser.h"
#include "utils/string.h"

#include <doctest/doctest.h>

#include <array>
#include <ostream>
#include <string_view>

namespace test_contiguous
{
void check_code_gen_sizes(std::string_view reference, std::string_view contiguous, size_t size_deviation = 0)
{
    std::array functions{reference, contiguous};
    auto disassemblies = cntgs::test::get_disassembly_of_functions({functions.data(), functions.size()});
    CAPTURE(reference << disassemblies[0]);
    CAPTURE(contiguous << disassemblies[1]);
    CHECK_GE(cntgs::test::count_lines(disassemblies[0]) + size_deviation, cntgs::test::count_lines(disassemblies[1]));
}

TEST_CASE("CodeGenTest: two FixedSize emplace and accumulate")
{
    check_code_gen_sizes("reference_two_fixed_emplace_and_accumulate", "contiguous_two_fixed_emplace_and_accumulate");
}
}  // namespace test_contiguous
