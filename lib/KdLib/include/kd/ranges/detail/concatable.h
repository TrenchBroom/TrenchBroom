/*
 Copyright (C) 2025 Kristian Duske

 Permission is hereby granted, free of charge, to any person obtaining a copy of this
 software and associated documentation files (the "Software"), to deal in the Software
 without restriction, including without limitation the rights to use, copy, modify, merge,
 publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include <concepts>
#include <iterator>
#include <ranges>
#include <type_traits>

namespace kdl::ranges::detail
{

template <typename... Rs>
using concat_reference_t = std::common_reference_t<std::ranges::range_reference_t<Rs>...>;

template <typename... Rs>
using concat_value_t = std::common_type_t<std::ranges::range_value_t<Rs>...>;

template <typename... Rs>
using concat_rvalue_reference_t =
  std::common_reference_t<std::ranges::range_rvalue_reference_t<Rs>...>;

template <typename Ref, typename RRef, typename It>
concept concat_indirectly_readable_impl = requires(const It it) {
  { *it } -> std::convertible_to<Ref>;
  { std::ranges::iter_move(it) } -> std::convertible_to<RRef>;
};

template <typename... Rs>
concept concat_indirectly_readable =
  std::common_reference_with<concat_reference_t<Rs...>&&, concat_value_t<Rs...>&>
  && std::
    common_reference_with<concat_reference_t<Rs...>&&, concat_rvalue_reference_t<Rs...>&&>
  && std::common_reference_with<
    concat_rvalue_reference_t<Rs...>&&,
    const concat_value_t<Rs...>&>
  && (concat_indirectly_readable_impl<concat_reference_t<Rs...>, concat_rvalue_reference_t<Rs...>, std::ranges::iterator_t<Rs>> && ...);

template <typename... Rs>
concept concatable = requires {
  typename concat_reference_t<Rs...>;
  typename concat_value_t<Rs...>;
  typename concat_rvalue_reference_t<Rs...>;
} && concat_indirectly_readable<Rs...>;

} // namespace kdl::ranges::detail
