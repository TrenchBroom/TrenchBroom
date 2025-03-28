/*
 Copyright 2020 Kristian Duske

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

#include "vm/line.h"
#include "vm/mat.h"
#include "vm/scalar.h"
#include "vm/vec.h"

#include <cassert>
#include <optional>
#include <ostream>
#include <vector>

namespace vm
{
template <typename T>
class approx
{
private:
  const T m_value;
  const T m_epsilon;

public:
  constexpr explicit approx(const T value, const T epsilon)
    : m_value{value}
    , m_epsilon{epsilon}
  {
    assert(epsilon >= T(0));
  }
  constexpr explicit approx(const T value)
    : approx{value, vm::constants<T>::almost_zero()}
  {
  }

  constexpr std::strong_ordering operator<=>(const T& rhs) const
  {
    return m_value == rhs              ? std::strong_ordering::equal
           : m_value < rhs - m_epsilon ? std::strong_ordering::less
           : m_value > rhs + m_epsilon ? std::strong_ordering::greater
                                       : std::strong_ordering::equivalent;
  }

  constexpr bool operator==(const T& rhs) const { return *this <=> rhs == 0; }

  constexpr std::strong_ordering operator<=>(const std::optional<T>& rhs) const
  {
    return !rhs ? std::strong_ordering::greater : *this <=> *rhs;
  }

  constexpr bool operator==(const std::optional<T>& rhs) const
  {
    return *this <=> rhs == 0;
  }

  friend std::ostream& operator<<(std::ostream& str, const approx<T>& a)
  {
    str << a.m_value;
    return str;
  }
};

template <typename T, std::size_t S>
class approx<vec<T, S>>
{
private:
  const vec<T, S> m_value;
  const T m_epsilon;

public:
  constexpr explicit approx(const vec<T, S> value, const T epsilon)
    : m_value{value}
    , m_epsilon{epsilon}
  {
    assert(epsilon >= T(0));
  }
  constexpr explicit approx(const vec<T, S> value)
    : approx{value, vm::constants<T>::almost_zero()}
  {
  }

  constexpr std::strong_ordering operator<=>(const vec<T, S>& rhs) const
  {
    return compare(m_value, rhs, m_epsilon);
  }

  constexpr bool operator==(const T& rhs) const { return *this <=> rhs == 0; }

  constexpr std::strong_ordering operator<=>(const std::optional<vec<T, S>>& rhs) const
  {
    return !rhs ? std::strong_ordering::greater : *this <=> *rhs;
  }

  constexpr bool operator==(const std::optional<vec<T, S>>& rhs) const
  {
    return *this <=> rhs == 0;
  }

  friend std::ostream& operator<<(std::ostream& str, const approx<vec<T, S>>& a)
  {
    str << a.m_value;
    return str;
  }
};

template <typename T, std::size_t R, std::size_t C>
class approx<mat<T, R, C>>
{
private:
  const mat<T, R, C> m_value;
  const T m_epsilon;

public:
  constexpr explicit approx(const mat<T, R, C> value, const T epsilon)
    : m_value{value}
    , m_epsilon{epsilon}
  {
    assert(epsilon >= T(0));
  }
  constexpr explicit approx(const mat<T, R, C> value)
    : approx{value, vm::constants<T>::almost_zero()}
  {
  }

  constexpr std::strong_ordering operator<=>(const mat<T, R, C>& rhs) const
  {
    return compare(m_value, rhs, m_epsilon);
  }

  constexpr bool operator==(const mat<T, R, C>& rhs) const { return *this <=> rhs == 0; }

  constexpr std::strong_ordering operator<=>(const std::optional<mat<T, R, C>>& rhs) const
  {
    return !rhs ? std::strong_ordering::greater : *this <=> *rhs;
  }

  constexpr bool operator==(const std::optional<mat<T, R, C>>& rhs) const
  {
    return *this <=> rhs == 0;
  }

  friend std::ostream& operator<<(std::ostream& str, const approx<mat<T, R, C>>& a)
  {
    str << a.m_value;
    return str;
  }
};

template <typename T, std::size_t S>
class approx<line<T, S>>
{
private:
  const line<T, S> m_value;
  const T m_epsilon;

public:
  constexpr explicit approx(const line<T, S> value, const T epsilon)
    : m_value{value}
    , m_epsilon{epsilon}
  {
    assert(epsilon >= T(0));
  }
  constexpr explicit approx(const line<T, S> value)
    : approx{value, vm::constants<T>::almost_zero()}
  {
  }

  constexpr bool operator==(const line<T, S>& rhs) const
  {
    return is_equal(m_value, rhs, m_epsilon);
  }

  friend std::ostream& operator<<(std::ostream& str, const approx<line<T, S>>& a)
  {
    str << a.m_value;
    return str;
  }
};

template <typename T>
class optional_approx
{
private:
  const std::optional<T> m_value;
  const T m_epsilon;

public:
  constexpr explicit optional_approx(std::optional<T> value, const T epsilon)
    : m_value{std::move(value)}
    , m_epsilon{epsilon}
  {
    assert(epsilon >= T(0));
  }
  constexpr explicit optional_approx(std::optional<T> value)
    : optional_approx<T>{std::move(value), vm::constants<T>::almost_zero()}
  {
  }

  constexpr std::strong_ordering operator<=>(const std::optional<T>& rhs) const
  {
    return m_value == rhs                ? std::strong_ordering::equal
           : !m_value                    ? std::strong_ordering::less
           : !rhs                        ? std::strong_ordering::greater
           : *m_value < *rhs - m_epsilon ? std::strong_ordering::less
           : *m_value > *rhs + m_epsilon ? std::strong_ordering::greater
                                         : std::strong_ordering::equivalent;
  }

  constexpr bool operator==(const std::optional<T>& rhs) const
  {
    return *this <=> rhs == 0;
  }

  friend std::ostream& operator<<(std::ostream& str, const optional_approx<T>& a)
  {
    if (a.m_value)
    {
      str << *a.m_value;
    }
    else
    {
      str << "nullopt";
    }
    return str;
  }
};

template <typename T>
constexpr std::strong_ordering operator<=>(
  const std::vector<T>& lhs, const std::vector<approx<T>>& rhs)
{
  if (lhs.size() < rhs.size())
  {
    return std::strong_ordering::less;
  }

  if (lhs.size() > rhs.size())
  {
    return std::strong_ordering::greater;
  }

  for (size_t i = 0u; i < lhs.size(); ++i)
  {
    if (const auto cmp = lhs[i] <=> rhs[i]; cmp != 0)
    {
      return cmp;
    }
  }

  return std::strong_ordering::equal;
}

template <typename T>
constexpr bool operator==(const std::vector<T>& lhs, const std::vector<approx<T>>& rhs)
{
  return lhs <=> rhs == 0;
}

} // namespace vm
