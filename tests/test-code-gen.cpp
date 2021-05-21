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
    CAPTURE(disassemblies[0]);
    CAPTURE(disassemblies[1]);
    const auto reference_line_count = cntgs::test::count_lines(disassemblies[0]);
    const auto contiguous_line_count = cntgs::test::count_lines(disassemblies[1]);
    CAPTURE(reference_line_count);
    CAPTURE(contiguous_line_count);
    CHECK_GE(reference_line_count + size_deviation, contiguous_line_count);
}

TEST_CASE("CodeGenTest: two FixedSize lookup and accumulate")
{
    check_code_gen_sizes("reference_two_fixed_lookup_and_accumulate", "contiguous_two_fixed_lookup_and_accumulate");
}

TEST_CASE("CodeGenTest: two FixedSize emplace")
{
    check_code_gen_sizes("reference_two_fixed_emplace", "contiguous_two_fixed_emplace");
}
}  // namespace test_contiguous
