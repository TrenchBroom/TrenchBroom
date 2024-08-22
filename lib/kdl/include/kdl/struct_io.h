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

#include "kdl/std_io.h"
#include "kdl/traits.h"

#include <ostream>

namespace kdl
{

class struct_stream
{
private:
  enum class expected
  {
    type_name,
    attr_name,
    attr_value
  };

  std::ostream& m_str;
  expected m_expected = expected::type_name;
  bool m_first_attr = true;

public:
  explicit struct_stream(std::ostream& str)
    : m_str{str}
  {
  }

  ~struct_stream() { m_str << "}"; }

  template <typename T>
  friend struct_stream& operator<<(struct_stream& lhs, const T& rhs)
  {
    append_to_stream(lhs, rhs);
    return lhs;
  }

  template <typename T>
  friend struct_stream& operator<<(struct_stream&& lhs, const T& rhs)
  {
    append_to_stream(lhs, rhs);
    return lhs;
  }

private:
  template <typename T>
  static void append_to_stream(struct_stream& lhs, const T& rhs)
  {
    switch (lhs.m_expected)
    {
    case expected::type_name:
      lhs.m_str << make_streamable(rhs);
      lhs.m_str << "{";
      lhs.m_expected = expected::attr_name;
      break;
    case expected::attr_name:
      if (!lhs.m_first_attr)
      {
        lhs.m_str << ", ";
      }
      lhs.m_first_attr = false;
      lhs.m_str << make_streamable(rhs);
      lhs.m_str << ": ";
      lhs.m_expected = expected::attr_value;
      break;
    case expected::attr_value:
      lhs.m_str << make_streamable(rhs);
      lhs.m_expected = expected::attr_name;
      break;
    }
  }
};

} // namespace kdl
