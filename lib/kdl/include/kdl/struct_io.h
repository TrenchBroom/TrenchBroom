/*
 Copyright 2022 Kristian Duske

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
 NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
 OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include <kdl/optional_io.h>
#include <kdl/range_io.h>
#include <kdl/traits.h>
#include <kdl/tuple_io.h>
#include <kdl/variant_io.h>

#include <ostream>
#include <string_view>

namespace kdl {
namespace detail {
template <typename T, typename = void> struct can_make_streamable : std::false_type {};

template <typename T>
struct can_make_streamable<T, std::void_t<decltype(kdl::make_streamable(std::declval<const T&>()))>>
  : std::true_type {};

template <typename T = std::ostream>
inline constexpr bool can_make_streamable_v = can_make_streamable<T>::value;

} // namespace detail

class struct_stream {
private:
  enum class expected {
    type_name,
    attr_name,
    attr_value
  };

  std::ostream& m_str;
  expected m_expected = expected::type_name;
  bool m_first_attr = true;

public:
  explicit struct_stream(std::ostream& str)
    : m_str{str} {}

  ~struct_stream() { m_str << "}"; }

  template <typename T> friend struct_stream& operator<<(struct_stream& lhs, const T& rhs) {
    append_to_stream(lhs, rhs);
    return lhs;
  }

  template <typename T> friend struct_stream& operator<<(struct_stream&& lhs, const T& rhs) {
    append_to_stream(lhs, rhs);
    return lhs;
  }

private:
  template <typename T, typename std::enable_if<can_print_v<T>, bool>::type = true>
  static void append_to_stream(std::ostream& lhs, const T& rhs) {
    lhs << rhs;
  }

  template <
    typename T,
    typename std::enable_if<!can_print_v<T> && detail::can_make_streamable_v<T>, bool>::type = true>
  static void append_to_stream(std::ostream& lhs, const T& rhs) {
    lhs << make_streamable(rhs);
  }

  template <typename T> static void append_to_stream(struct_stream& lhs, const T& rhs) {
    switch (lhs.m_expected) {
      case expected::type_name:
        append_to_stream(lhs.m_str, rhs);
        lhs.m_str << "{";
        lhs.m_expected = expected::attr_name;
        break;
      case expected::attr_name:
        if (!lhs.m_first_attr) {
          lhs.m_str << ", ";
        }
        lhs.m_first_attr = false;
        append_to_stream(lhs.m_str, rhs);
        lhs.m_str << ": ";
        lhs.m_expected = expected::attr_value;
        break;
      case expected::attr_value:
        append_to_stream(lhs.m_str, rhs);
        lhs.m_expected = expected::attr_name;
        break;
    }
  }
};

} // namespace kdl
