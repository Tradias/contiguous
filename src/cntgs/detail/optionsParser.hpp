// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_DETAIL_OPTIONSPARSER_HPP
#define CNTGS_DETAIL_OPTIONSPARSER_HPP

#include "cntgs/detail/typeTraits.hpp"
#include "cntgs/parameter.hpp"

#include <memory>
#include <type_traits>

namespace cntgs::detail
{
template <class = void>
struct AllocatorOptionParser : std::false_type
{
    using Allocator = std::allocator<std::byte>;
};

template <class T>
struct AllocatorOptionParser<cntgs::Allocator<T>> : std::true_type
{
    using Allocator = T;
};

template <class... Option>
struct OptionsParser
{
    template <template <class = void> class Parser>
    using Parse =
        detail::ConditionalT<std::disjunction<Parser<Option>...>::value, std::disjunction<Parser<Option>...>, Parser<>>;

    using Allocator = typename Parse<detail::AllocatorOptionParser>::Allocator;
};
}  // namespace cntgs::detail

#endif  // CNTGS_DETAIL_OPTIONSPARSER_HPP
