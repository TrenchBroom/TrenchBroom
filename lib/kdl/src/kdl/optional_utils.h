/*
 Copyright 2024 Kristian Duske

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

#include <optional>

namespace kdl
{
namespace detail
{

// Type acts as a tag to find the correct operator| overload
template <typename F>
struct and_then_helper
{
  const F& f;
};

// This actually does the work
template <typename T, typename F>
auto operator|(std::optional<T>&& o, and_then_helper<F> h)
{
  return o ? h.f(std::move(*o)) : std::nullopt;
}

template <typename T, typename F>
auto operator|(const std::optional<T>& o, const and_then_helper<F>& h)
{
  return o ? h.f(*o) : std::nullopt;
}

// Type acts as a tag to find the correct operator| overload
template <typename F>
struct or_else_helper
{
  const F& f;
};

template <typename T, typename F>
auto operator|(std::optional<T>&& o, const or_else_helper<F>& h)
{
  return o ? std::move(o) : h.f();
}

template <typename T, typename F>
auto operator|(const std::optional<T>& o, const or_else_helper<F>& h)
{
  return o ? o : h.f();
}

// Type acts as a tag to find the correct operator| overload
template <typename F>
struct transform_helper
{
  const F& f;
};

// This actually does the work
template <typename T, typename F>
auto operator|(std::optional<T>&& o, const transform_helper<F>& h)
{
  return o ? std::optional{h.f(std::move(*o))} : std::nullopt;
}

template <typename T, typename F>
auto operator|(const std::optional<T>& o, const transform_helper<F>& h)
{
  return o ? std::optional{h.f(*o)} : std::nullopt;
}

} // namespace detail

template <typename F>
auto optional_and_then(const F& f)
{
  return detail::and_then_helper<F>{f};
}

template <typename F>
auto optional_or_else(const F& f)
{
  return detail::or_else_helper<F>{f};
}

template <typename F>
auto optional_transform(const F& f)
{
  return detail::transform_helper<F>{f};
}

} // namespace kdl
