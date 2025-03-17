// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_UTILS_STRING_HPP
#define CNTGS_UTILS_STRING_HPP

#include <sstream>
#include <string>
#include <string_view>

namespace test
{
namespace detail
{
template <class Iterator>
auto to_string_view(Iterator begin, Iterator end)
{
    const auto* data = std::addressof(*begin);
    const auto size = static_cast<std::size_t>(std::distance(begin, end));
    return std::string_view{data, size};
}

template <class Iterator, class Arg>
void format_one(std::ostringstream& stream, std::string_view format, Iterator& iterator, Arg&& arg)
{
    const auto next = std::find(iterator, format.end(), '{');
    const auto next_string_view = detail::to_string_view(iterator, next);
    iterator = std::next(next, 2);
    stream << next_string_view << static_cast<Arg&&>(arg);
}
}  // namespace detail

size_t count_lines(const std::string& text);

template <class... Args>
auto format(std::string_view format, Args&&... args)
{
    std::ostringstream stream;
    auto iterator = format.begin();
    (detail::format_one(stream, format, iterator, static_cast<Args&&>(args)), ...);
    if (iterator != format.end())
    {
        stream << detail::to_string_view(iterator, format.end());
    }
    return std::move(stream).str();
}
}  // namespace test

#endif  // CNTGS_UTILS_STRING_HPP
