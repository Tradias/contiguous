#include "cntgs/contiguous.hpp"
#include "utils/codeGenParser.hpp"
#include "utils/string.hpp"

#include <doctest/doctest.h>

#include <array>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

namespace test_contiguous
{
TEST_SUITE_BEGIN(CNTGS_TEST_CPP_VERSION);

auto get_disassembly_of_functions(const std::vector<std::string>& function_names)
{
    return cntgs::test::get_disassembly_of_functions(CNTGS_CODE_GEN_DISASSEMBLY_FILE, function_names);
}

void check_code_gen_sizes(const std::string& reference, const std::string& contiguous, size_t size_deviation = 0)
{
    auto disassemblies = get_disassembly_of_functions({reference, contiguous});
    CAPTURE(disassemblies[0]);
    CAPTURE(disassemblies[1]);
    const auto reference_line_count = cntgs::test::count_lines(disassemblies[0]);
    const auto contiguous_line_count = cntgs::test::count_lines(disassemblies[1]);
    CAPTURE(reference_line_count);
    CAPTURE(contiguous_line_count);
    REQUIRE((reference_line_count > 0 && contiguous_line_count > 0));
    CHECK_GE(reference_line_count + size_deviation, contiguous_line_count);
}

#ifndef __clang__
TEST_CASE("CodeGenTest: two FixedSize lookup and accumulate")
{
    check_code_gen_sizes("reference_two_fixed_lookup_and_accumulate", "contiguous_two_fixed_lookup_and_accumulate");
}

TEST_CASE("CodeGenTest: two FixedSize aligned lookup and accumulate")
{
    check_code_gen_sizes("reference_two_fixed_aligned_lookup_and_accumulate",
                         "contiguous_two_fixed_aligned_lookup_and_accumulate");
}
#endif

TEST_CASE("CodeGenTest: two FixedSize emplace")
{
    check_code_gen_sizes("reference_two_fixed_emplace", "contiguous_two_fixed_emplace", 2);
}

TEST_CASE("CodeGenTest: two FixedSize random lookup")
{
    check_code_gen_sizes("reference_two_fixed_random_lookup", "contiguous_two_fixed_random_lookup", 1);
}

TEST_CASE("CodeGenTest: two FixedSize reserve growth")
{
    check_code_gen_sizes("reference_two_fixed_reserve_growth", "contiguous_two_fixed_reserve_growth", 10);
}

TEST_CASE("CodeGenTest: two FixedSize erase(iterator, end) vs. std::vector")
{
    check_code_gen_sizes("std_vector_erase_at_the_end", "contiguous_two_erase_at_the_end", 1);
}

TEST_CASE("CodeGenTest: two FixedSize erase(iterator) vs. std::vector")
{
    check_code_gen_sizes("reference_two_fixed_erase_within", "contiguous_two_fixed_erase_within");
}

TEST_SUITE_END();
}  // namespace test_contiguous
