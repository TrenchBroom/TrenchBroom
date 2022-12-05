/*
 Copyright 2022 Kristian Duske

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

#include "traits.h"

#include <ostream>

namespace kdl
{

namespace detail
{
template <typename T>
struct streamable_optional_wrapper;

template <typename T>
struct streamable_tuple_wrapper;

template <typename T>
struct streamable_pair_wrapper;

template <typename T>
struct streamable_variant_wrapper;

template <typename R>
struct streamable_range_wrapper;

template <typename T>
struct streamable_default_wrapper;
} // namespace detail

template <typename T>
auto make_streamable(const T& x)
{
  if constexpr (!is_streamable_v<T> && is_optional_v<T>)
  {
    return detail::streamable_optional_wrapper<T>{x};
  }
  else if constexpr (!is_streamable_v<T> && is_tuple_v<T>)
  {
    return detail::streamable_tuple_wrapper<T>{x};
  }
  else if constexpr (!is_streamable_v<T> && is_pair_v<T>)
  {
    return detail::streamable_pair_wrapper<T>{x};
  }
  else if constexpr (!is_streamable_v<T> && is_variant_v<T>)
  {
    return detail::streamable_variant_wrapper<T>{x};
  }
  else if constexpr (!is_streamable_v<T> && is_iterable_v<T>)
  {
    return detail::streamable_range_wrapper<T>{x};
  }
  else
  {
    return detail::streamable_default_wrapper<T>{x};
  }
}

namespace detail
{
template <typename T>
struct streamable_optional_wrapper
{
  const T& opt;
};

template <typename T>
std::ostream& operator<<(std::ostream& lhs, const streamable_optional_wrapper<T>& rhs)
{
  if (rhs.opt.has_value())
  {
    lhs << make_streamable(*rhs.opt);
  }
  else
  {
    lhs << "nullopt";
  }
  return lhs;
}

template <typename T, size_t Idx>
void print_tuple_element(std::ostream& str, const T& t)
{
  str << make_streamable(std::get<Idx>(t)) << ", ";
}

template <typename T, size_t... Idx>
void print_tuple(std::ostream& str, const T& t, std::index_sequence<Idx...>)
{
  (..., print_tuple_element<T, Idx>(str, t));
}

template <typename T>
struct streamable_tuple_wrapper
{
  const T& tuple;
};

template <typename T>
std::ostream& operator<<(std::ostream& lhs, const streamable_tuple_wrapper<T>& rhs)
{
  lhs << "{";
  constexpr auto size = std::tuple_size_v<T>;
  if constexpr (size > 0u)
  {
    kdl::detail::print_tuple(lhs, rhs.tuple, std::make_index_sequence<size - 1u>{});
    lhs << make_streamable(std::get<size - 1u>(rhs.tuple));
  }
  lhs << "}";
  return lhs;
}

template <typename T>
struct streamable_pair_wrapper
{
  const T& pair;
};

template <typename T>
std::ostream& operator<<(std::ostream& lhs, const streamable_pair_wrapper<T>& rhs)
{
  lhs << "{" << make_streamable(rhs.pair.first) << ", "
      << make_streamable(rhs.pair.second) << "}";
  return lhs;
}

template <typename T>
struct streamable_variant_wrapper
{
  const T& variant;
};

template <typename T>
std::ostream& operator<<(std::ostream& lhs, const streamable_variant_wrapper<T>& rhs)
{
  std::visit([&](const auto& x) { lhs << make_streamable(x); }, rhs.variant);
  return lhs;
}

template <typename R>
struct streamable_range_wrapper
{
  const R& range;
};

template <typename R>
std::ostream& operator<<(std::ostream& lhs, const streamable_range_wrapper<R>& adapter)
{
  using std::begin;
  using std::end;

  lhs << "[";
  auto cur = begin(adapter.range);
  if (cur != end(adapter.range))
  {
    lhs << make_streamable(*cur++);
    while (cur != end(adapter.range))
    {
      lhs << "," << make_streamable(*cur++);
    }
  }
  lhs << "]";
  return lhs;
}

template <typename T>
struct streamable_default_wrapper
{
  const T& x;
};

template <typename T>
std::ostream& operator<<(std::ostream& lhs, const streamable_default_wrapper<T>& rhs)
{
  lhs << rhs.x;
  return lhs;
}

} // namespace detail

} // namespace kdl
