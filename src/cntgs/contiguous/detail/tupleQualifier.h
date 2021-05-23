#pragma once

namespace cntgs::detail
{
enum class ContiguousTupleQualifier
{
    NONE,
    REFERENCE,
    CONST_REFERENCE,
    POINTER
};
}