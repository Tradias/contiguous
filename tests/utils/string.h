#pragma once

#include <sstream>
#include <string>
#include <string_view>

namespace cntgs::test
{
namespace detail
{
template <class Iterator, class Arg>
decltype(auto) format_one(std::ostringstream& stream, std::string_view format, Iterator& iterator, Arg&& arg)
{
    auto next = std::find(iterator, format.end(), '{');
    auto* data = std::addressof(*iterator);
    auto size = static_cast<std::size_t>(std::distance(iterator, next));
    iterator = std::next(next, 2);
    stream << std::string_view{data, size} << std::forward<Arg>(arg);
    return stream;
}
}  // namespace detail

size_t count_lines(const std::string& text);

template <class... Args>
auto format(std::string_view format, Args&&... args)
{
    std::ostringstream stream;
    auto iterator = format.begin();
    (detail::format_one(stream, format, iterator, std::forward<Args>(args)), ...);
    return std::move(stream).str();
}
}  // namespace cntgs::test
