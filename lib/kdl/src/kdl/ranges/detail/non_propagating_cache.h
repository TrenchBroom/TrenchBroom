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

#include <memory>
#include <optional>
#include <type_traits>

namespace kdl::ranges
{

template <typename T>
  requires std::is_object_v<T>
class non_propagating_cache : private std::optional<T>
{
public:
  using std::optional<T>::optional;
  using std::optional<T>::operator=;
  using std::optional<T>::operator->;
  using std::optional<T>::operator*;
  using std::optional<T>::operator bool;
  using std::optional<T>::has_value;
  using std::optional<T>::value;
  using std::optional<T>::reset;
  using std::optional<T>::emplace;

  constexpr non_propagating_cache(const non_propagating_cache&) noexcept
  {
    // do nothing
  }

  constexpr non_propagating_cache(non_propagating_cache&& other) noexcept
  {
    other.reset();
  }

  constexpr non_propagating_cache& operator=(const non_propagating_cache& other) noexcept
  {
    if (std::addressof(other) != this)
    {
      reset();
    }
    return *this;
  }

  constexpr non_propagating_cache& operator=(non_propagating_cache&& other) noexcept
  {
    reset();
    other.reset();
    return *this;
  }

  template <typename I>
  constexpr T& emplace_deref(const I& i)
  {
    if (has_value())
    {
      reset();
    }

    emplace(*i);
    return **this;
  }
};

} // namespace kdl::ranges
