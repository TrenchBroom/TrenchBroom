/*
 Copyright (C) 2010 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "ColorChannel.h"

#include "vm/from_chars.h"

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <limits>
#include <optional>
#include <string_view>
#include <type_traits>

namespace tb
{
namespace detail
{

template <typename T>
  requires(std::is_floating_point_v<T>)
constexpr bool is_finite(const T x) noexcept
{
  return x == x && x != std::numeric_limits<T>::infinity()
         && x != -std::numeric_limits<T>::infinity();
}

template <typename T, typename U>
constexpr bool isWithinRange(const U u) noexcept
{
  using TL = std::numeric_limits<T>;

  if constexpr (std::is_integral_v<T> && std::is_integral_v<U>)
  {
    if constexpr (std::is_signed_v<T> == std::is_signed_v<U>)
    {
      return u >= static_cast<U>(TL::min()) && u <= static_cast<U>(TL::max());
    }
    else if constexpr (std::is_signed_v<U>)
    {
      // U is signed, T is unsigned
      return u >= 0 && static_cast<std::make_unsigned_t<U>>(u) <= TL::max();
    }
    else
    {
      // U is unsigned, T is signed
      return u <= static_cast<std::make_unsigned_t<T>>(TL::max());
    }
  }
  else if constexpr (std::is_floating_point_v<U> && std::is_integral_v<T>)
  {
    // Float → Int
    // Note: NaN and Inf fail the check
    return is_finite(u) && u >= static_cast<U>(TL::min())
           && u <= static_cast<U>(TL::max());
  }
  else if constexpr (std::is_integral_v<U> && std::is_floating_point_v<T>)
  {
    // Int → Float
    // Safe if representable in floating-point range (not necessarily precisely)
    return static_cast<long double>(u) >= static_cast<long double>(-TL::max())
           && static_cast<long double>(u) <= static_cast<long double>(TL::max());
  }
  else if constexpr (std::is_floating_point_v<T> && std::is_floating_point_v<U>)
  {
    // Float → Float
    return is_finite(u)
           && static_cast<long double>(u) >= static_cast<long double>(TL::lowest())
           && static_cast<long double>(u) <= static_cast<long double>(TL::max());
  }
  else
  {
    static_assert(
      std::is_arithmetic_v<T> && std::is_arithmetic_v<U>,
      "Both T and U must be arithmetic types");
    return false;
  }
}

} // namespace detail

template <ColorChannel Ch, typename T, T Min, T Max, T DefaultValue = Min>
struct ColorComponentType
{
  static constexpr auto Channel = Ch;
  using ValueType = T;
  using NormalizedValueType = double;

  static constexpr auto min = Min;
  static constexpr auto max = Max;

  template <typename U>
  static constexpr bool inValueRange(const U value)
  {
    return detail::isWithinRange<ValueType>(value) && ValueType(value) >= Min
           && ValueType(value) <= Max;
  }

  static constexpr ValueType defaultValue() { return DefaultValue; }

  static constexpr NormalizedValueType normalizeValue(const ValueType v)
  {
    return NormalizedValueType(v - Min) / NormalizedValueType(Max - Min);
  }

  static constexpr ValueType fromNormalizedValue(const NormalizedValueType v)
  {
    return ValueType(
      v * (NormalizedValueType(Max) - NormalizedValueType(Min))
      + NormalizedValueType(Min));
  }

  static std::optional<ValueType> parse(const std::string_view str)
  {
    auto value = ValueType{};
    if (vm::from_chars(str.data(), str.data() + str.length(), value).ec == std::errc{})
    {
      if (inValueRange(value))
      {
        return value;
      }
    }
    return std::nullopt;
  }
};

namespace detail
{

template <typename T>
struct IsColorComponentType : std::false_type
{
};

template <ColorChannel Ch, typename T, T Min, T Max, T DefaultValue>
struct IsColorComponentType<ColorComponentType<Ch, T, Min, Max, DefaultValue>>
  : std::true_type
{
};

} // namespace detail

template <typename C>
concept AnyColorComponentType = detail::IsColorComponentType<C>::value;

namespace detail
{

template <AnyColorComponentType... C>
struct ComponentsAreUnique;

template <>
struct ComponentsAreUnique<>
{
  static constexpr auto value = true;
};

template <AnyColorComponentType First>
struct ComponentsAreUnique<First>
{
  static constexpr auto value = true;
};

template <
  AnyColorComponentType First,
  AnyColorComponentType Second,
  AnyColorComponentType... Rest>
struct ComponentsAreUnique<First, Second, Rest...>
{
  static constexpr auto value = !std::is_same_v<First, Second>
                                && ComponentsAreUnique<First, Rest...>::value
                                && ComponentsAreUnique<Second, Rest...>::value;
};

} // namespace detail

template <typename... C>
concept ComponentsAreUnique = detail::ComponentsAreUnique<C...>::value;

} // namespace tb
