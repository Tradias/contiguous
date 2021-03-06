// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_DETAIL_PARAMETERTYPE_HPP
#define CNTGS_DETAIL_PARAMETERTYPE_HPP

namespace cntgs::detail
{
enum class ParameterType
{
    PLAIN,
    FIXED_SIZE,
    VARYING_SIZE
};
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_PARAMETERTYPE_HPP
