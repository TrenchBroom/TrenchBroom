/*
 Copyright (C) 2026 Kristian Duske

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

#include <ranges>
#include <utility>

namespace kdl
{
namespace ranges
{
namespace views
{
namespace detail
{

struct raw_pointers_fn
{
  template <std::ranges::viewable_range R>
  constexpr auto operator()(R&& r) const
  {
    return std::views::transform(
      std::forward<R>(r), [](const auto& ptr) { return ptr.get(); });
  }

  template <std::ranges::viewable_range R>
  constexpr friend auto operator|(R&& r, const raw_pointers_fn& fn)
  {
    return fn(std::forward<R>(r));
  }
};

} // namespace detail

/**
 * A range adaptor that yields the raw pointers held by the smart pointers of the given
 * range.
 */
constexpr detail::raw_pointers_fn raw_pointers;

} // namespace views
} // namespace ranges

namespace views
{

constexpr ranges::views::detail::raw_pointers_fn raw_pointers;

} // namespace views
} // namespace kdl
