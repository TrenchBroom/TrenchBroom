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
  requires std::move_constructible<T> && std::is_object_v<T>
class movable_box : private std::optional<T>
{
public:
  using std::optional<T>::optional;
  using std::optional<T>::operator=;
  using std::optional<T>::operator->;
  using std::optional<T>::operator*;
  using std::optional<T>::operator bool;
  using std::optional<T>::has_value;
  using std::optional<T>::reset;
  using std::optional<T>::emplace;

  constexpr movable_box() noexcept(std::is_nothrow_default_constructible_v<T>)
    requires std::default_initializable<T>
    : movable_box{std::in_place}
  {
  }

  constexpr movable_box(const movable_box& other) noexcept(
    std::is_nothrow_copy_constructible_v<T>)
    : std::optional<T>{static_cast<const std::optional<T>&>(other)}
  {
  }

  constexpr movable_box(movable_box&& other) noexcept(
    std::is_nothrow_move_constructible_v<T>)
    : std::optional<T>{static_cast<std::optional<T>&&>(std::move(other))}
  {
  }

  constexpr movable_box& operator=(const movable_box& other) noexcept(
    std::is_nothrow_copy_constructible_v<T>)
    requires std::copy_constructible<T>
  {
    if constexpr (!std::copyable<T>)
    {
      if (this != std::addressof(other))
      {
        if (other)
        {
          emplace(*other);
        }
        else
        {
          reset();
        }
      }
      return *this;
    }
    else
    {
      return std::optional<T>::operator=(other);
    }
  }

  constexpr movable_box& operator=(movable_box&& other) noexcept(
    std::is_nothrow_move_constructible_v<T>)
    requires(!std::movable<T>)
  {
    if constexpr (!std::movable<T>)
    {
      if (this != std::addressof(other))
      {
        if (other)
        {
          emplace(std::move(*other));
        }
        else
        {
          reset();
        }
      }
      return *this;
    }
    else
    {
      return std::optional<T>::operator=(std::move(other));
    }
  }
};

} // namespace kdl::ranges
