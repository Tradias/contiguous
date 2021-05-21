#include "cntgs/contiguous.h"
#include "utils/codeGenParser.h"

#include <doctest/doctest.h>

#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace test_contiguous
{
TEST_CASE("ContiguousTest: two varsize: empty()")
{
    using namespace std::literals;
    std::array functions{"reference1"sv, "contiguous1"sv};
    auto assemblies = cntgs::test::read({functions.data(), functions.size()});
    CHECK(assemblies[0].size() == assemblies[1].size());
}
}  // namespace test_contiguous
