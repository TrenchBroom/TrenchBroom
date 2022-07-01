/*
 Copyright 2020 Kristian Duske

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

#include <vecmath/forward.h>
#include <vecmath/scalar.h>

#include <cassert>
#include <ostream>
#include <vector>

namespace vm {
template <typename T> class approx {
private:
  const T m_value;
  const T m_epsilon;

public:
  constexpr explicit approx(const T value, const T epsilon)
    : m_value(value)
    , m_epsilon(epsilon) {
    assert(epsilon >= T(0));
  }
  constexpr explicit approx(const T value)
    : approx(value, vm::constants<T>::almost_zero()) {}

  friend constexpr bool operator==(const T lhs, const approx<T>& rhs) {
    return lhs >= (rhs.m_value - rhs.m_epsilon) && lhs <= (rhs.m_value + rhs.m_epsilon);
  }

  friend constexpr bool operator==(const approx<T>& lhs, const T rhs) { return rhs == lhs; }

  friend constexpr bool operator!=(const T lhs, const approx<T>& rhs) { return !(lhs == rhs); }

  friend constexpr bool operator!=(const approx<T>& lhs, const T rhs) { return !(lhs == rhs); }

  friend constexpr bool operator<(const T lhs, const approx<T>& rhs) {
    return lhs < (rhs.m_value - rhs.m_epsilon);
  }

  friend constexpr bool operator<(const approx<T> lhs, const T rhs) {
    return lhs.m_value < (rhs - lhs.m_epsilon);
  }

  friend constexpr bool operator<=(const T lhs, const approx<T>& rhs) {
    return lhs <= (rhs.m_value + rhs.m_epsilon);
  }

  friend constexpr bool operator<=(const approx<T> lhs, const T rhs) {
    return lhs.m_value <= (rhs + lhs.m_epsilon);
  }

  friend constexpr bool operator>(const T lhs, const approx<T>& rhs) {
    return lhs > (rhs.m_value + rhs.m_epsilon);
  }

  friend constexpr bool operator>(const approx<T> lhs, const T rhs) {
    return lhs.m_value > (rhs + lhs.m_epsilon);
  }

  friend constexpr bool operator>=(const T lhs, const approx<T>& rhs) {
    return lhs >= (rhs.m_value - rhs.m_epsilon);
  }

  friend constexpr bool operator>=(const approx<T> lhs, const T rhs) {
    return lhs.m_value >= (rhs - lhs.m_epsilon);
  }

  friend std::ostream& operator<<(std::ostream& str, const approx<T>& a) {
    str << a.m_value;
    return str;
  }
};

template <typename T>
bool operator==(const std::vector<T>& lhs, const std::vector<approx<T>>& rhs) {
  if (lhs.size() != rhs.size()) {
    return false;
  }

  for (size_t i = 0u; i < lhs.size(); ++i) {
    if (lhs[i] != rhs[i]) {
      return false;
    }
  }

  return true;
}

template <typename T>
bool operator==(const std::vector<approx<T>>& lhs, const std::vector<T>& rhs) {
  return rhs == lhs;
}

template <typename T>
bool operator!=(const std::vector<T>& lhs, const std::vector<approx<T>>& rhs) {
  return !(lhs == rhs);
}

template <typename T>
bool operator!=(const std::vector<approx<T>>& lhs, const std::vector<T>& rhs) {
  return !(lhs == rhs);
}

template <typename T, std::size_t S> class approx<vec<T, S>> {
private:
  const vec<T, S> m_value;
  const T m_epsilon;

public:
  constexpr explicit approx(const vec<T, S> value, const T epsilon)
    : m_value(value)
    , m_epsilon(epsilon) {
    assert(epsilon >= T(0));
  }
  constexpr explicit approx(const vec<T, S> value)
    : approx(value, vm::constants<T>::almost_zero()) {}

  friend constexpr bool operator==(const vec<T, S>& lhs, const approx<vec<T, S>>& rhs) {
    return is_equal(lhs, rhs.m_value, rhs.m_epsilon);
  }

  friend constexpr bool operator==(const approx<vec<T, S>>& lhs, const vec<T, S>& rhs) {
    return rhs == lhs;
  }

  friend constexpr bool operator!=(const vec<T, S>& lhs, const approx<vec<T, S>>& rhs) {
    return !(lhs == rhs);
  }

  friend constexpr bool operator!=(const approx<vec<T, S>>& lhs, const vec<T, S>& rhs) {
    return !(lhs == rhs);
  }

  friend constexpr bool operator<(const vec<T, S>& lhs, const approx<vec<T, S>>& rhs) {
    return lhs < (rhs.m_value - vec<T, S>::fill(rhs.m_epsilon));
  }

  friend constexpr bool operator<(const approx<vec<T, S>> lhs, const vec<T, S>& rhs) {
    return lhs.m_value < (rhs - vec<T, S>::fill(lhs.m_epsilon));
  }

  friend constexpr bool operator<=(const vec<T, S>& lhs, const approx<vec<T, S>>& rhs) {
    return lhs <= (rhs.m_value + vec<T, S>::fill(rhs.m_epsilon));
  }

  friend constexpr bool operator<=(const approx<vec<T, S>> lhs, const vec<T, S>& rhs) {
    return lhs.m_value <= (rhs + vec<T, S>::fill(lhs.m_epsilon));
  }

  friend constexpr bool operator>(const vec<T, S>& lhs, const approx<vec<T, S>>& rhs) {
    return lhs > (rhs.m_value + vec<T, S>::fill(rhs.m_epsilon));
  }

  friend constexpr bool operator>(const approx<vec<T, S>> lhs, const vec<T, S>& rhs) {
    return lhs.m_value > (rhs + vec<T, S>::fill(lhs.m_epsilon));
  }

  friend constexpr bool operator>=(const vec<T, S>& lhs, const approx<vec<T, S>>& rhs) {
    return lhs >= (rhs.m_value - rhs.m_epsilon);
  }

  friend constexpr bool operator>=(const approx<vec<T, S>> lhs, const vec<T, S>& rhs) {
    return lhs.m_value >= (rhs - lhs.m_epsilon);
  }

  friend std::ostream& operator<<(std::ostream& str, const approx<vec<T, S>>& a) {
    str << a.m_value;
    return str;
  }
};

template <typename T, std::size_t R, std::size_t C> class approx<mat<T, R, C>> {
private:
  const mat<T, R, C> m_value;
  const T m_epsilon;

public:
  constexpr explicit approx(const mat<T, R, C> value, const T epsilon)
    : m_value(value)
    , m_epsilon(epsilon) {
    assert(epsilon >= T(0));
  }
  constexpr explicit approx(const mat<T, R, C> value)
    : approx(value, vm::constants<T>::almost_zero()) {}

  friend constexpr bool operator==(const mat<T, R, C>& lhs, const approx<mat<T, R, C>>& rhs) {
    return is_equal(lhs, rhs.m_value, rhs.m_epsilon);
  }

  friend constexpr bool operator==(const approx<mat<T, R, C>>& lhs, const mat<T, R, C>& rhs) {
    return rhs == lhs;
  }

  friend constexpr bool operator!=(const mat<T, R, C>& lhs, const approx<mat<T, R, C>>& rhs) {
    return !(lhs == rhs);
  }

  friend constexpr bool operator!=(const approx<mat<T, R, C>>& lhs, const mat<T, R, C>& rhs) {
    return !(lhs == rhs);
  }

  friend std::ostream& operator<<(std::ostream& str, const approx<mat<T, R, C>>& a) {
    str << a.m_value;
    return str;
  }
};

template <typename T, std::size_t S> class approx<line<T, S>> {
private:
  const line<T, S> m_value;
  const T m_epsilon;

public:
  constexpr explicit approx(const line<T, S> value, const T epsilon)
    : m_value(value)
    , m_epsilon(epsilon) {
    assert(epsilon >= T(0));
  }
  constexpr explicit approx(const line<T, S> value)
    : approx(value, vm::constants<T>::almost_zero()) {}

  friend constexpr bool operator==(const line<T, S>& lhs, const approx<line<T, S>>& rhs) {
    return is_equal(lhs, rhs.m_value, rhs.m_epsilon);
  }

  friend constexpr bool operator==(const approx<line<T, S>>& lhs, const line<T, S>& rhs) {
    return rhs == lhs;
  }

  friend std::ostream& operator<<(std::ostream& str, const approx<line<T, S>>& a) {
    str << a.m_value;
    return str;
  }
};
} // namespace vm
