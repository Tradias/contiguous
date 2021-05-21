#pragma once

#include <iterator>
#include <type_traits>
#include <utility>

namespace cntgs::detail
{
template <class, class = std::void_t<>>
struct HasDataAndSize : std::false_type
{
};

template <class T>
struct HasDataAndSize<T, std::void_t<decltype(std::data(std::declval<T&>())), decltype(std::size(std::declval<T&>()))>>
    : std::true_type
{
};

template <class, class = std::void_t<>>
struct IsRange : std::false_type
{
};

template <class T>
struct IsRange<T, std::void_t<decltype(std::begin(std::declval<T&>())), decltype(std::end(std::declval<T&>()))>>
    : std::true_type
{
};
}  // namespace cntgs::detail