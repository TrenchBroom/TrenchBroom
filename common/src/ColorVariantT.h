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

#include "ColorT.h"
#include "Result.h"

#include "kdl/reflection_impl.h"

#include <string>
#include <string_view>
#include <variant>

namespace tb
{
namespace detail
{
#include <type_traits>


// contains_v<T, Ts...> : true if T is equal to any of Ts...
template <typename T, typename... Ts>
inline constexpr bool contains_v = (std::is_same_v<T, Ts> || ...);

// Put Super... into a helper so the inner fold expands only Super...,
// and the outer fold expands only Sub...
template <typename... Super>
struct subset
{
  template <typename... Sub>
  static inline constexpr bool check_v = (contains_v<Sub, Super...> && ...);
};

} // namespace detail

template <AnyColorT... Colors>
class ColorVariantT
{
private:
  // cannot use a requires clause for this because clang then complains about the friend
  // declaration further below having differing requires clauses
  static_assert(sizeof...(Colors) > 0);

  std::variant<Colors...> m_color;

  template <AnyColorT... OtherColors>
  friend class ColorVariantT;

public:
  constexpr ColorVariantT()
    : m_color{std::tuple_element_t<0, std::tuple<Colors...>>{}}
  {
  }

  template <AnyColorT Color>
  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr ColorVariantT(const Color& color)
    : m_color{color}
  {
  }

  template <AnyColorT... OtherColors>
    requires(detail::subset<Colors...>::template check_v<OtherColors...>)
  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr ColorVariantT(const ColorVariantT<OtherColors...>& colorVariant)
    : m_color{std::visit(
        [](const auto& color) { return std::variant<Colors...>{color}; },
        colorVariant.m_color)}
  {
  }

  template <AnyColorT Color>
  constexpr ColorVariantT& operator=(const Color& color)
  {
    m_color = color;
    return *this;
  }

  template <AnyColorT... OtherColors>
    requires(detail::subset<Colors...>::template check_v<OtherColors...>)
  constexpr ColorVariantT& operator=(const ColorVariantT<OtherColors...>& colorVariant)
  {
    m_color = std::visit(
      [](const auto& color) { return std::variant<Colors...>{color}; },
      colorVariant.m_color);
    return *this;
  }

  template <typename U, size_t N>
  static Result<ColorVariantT> fromVec(const vm::vec<U, N>& vec)
  {
    return [&]<std::size_t... I>(std::index_sequence<I...>) {
      return fromValues(std::tuple{vec[I]...});
    }(std::make_index_sequence<N>{});
  }

  template <typename... U>
  static Result<ColorVariantT> fromValues(const U... values)
  {
    return fromValuesImpl<Colors...>(std::tuple{values...});
  }

  template <std::ranges::random_access_range R>
  static Result<ColorVariantT> parseComponents(const R& components)
  {
    // for parsing to succeed, Colors must be ordered by number of components
    return parseComponentsImpl<Colors...>(components);
  }

  static Result<ColorVariantT> parse(const std::string_view str)
  {
    return parseComponents(kdl::str_split(str, " "));
  }

  size_t numComponents() const
  {
    return std::visit([](const auto& color) { return color.numComponents(); }, m_color);
  }

  template <AnyColorT... Color>
  constexpr bool is() const
  {
    return (std::holds_alternative<Color>(m_color) || ...);
  }

  template <AnyColorT ToColor>
  constexpr ToColor to() const
  {
    return std::visit([](const auto& x) { return x.template to<ToColor>(); }, m_color);
  }

  std::string toString() const
  {
    return std::visit([](const auto& x) { return x.toString(); }, m_color);
  }

  kdl_reflect_inline(ColorVariantT, m_color);

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4702)
#endif
  template <AnyColorT ColorTypeToTry, AnyColorT... MoreColorTypes, typename... U>
  static Result<ColorVariantT> fromValuesImpl(const std::tuple<U...>& values)
  {
    if (auto result = ColorTypeToTry::fromValues(values))
    {
      return result | kdl::transform([](const auto& x) { return ColorVariantT{x}; });
    }

    if constexpr (sizeof...(MoreColorTypes) > 0)
    {
      return fromValuesImpl<MoreColorTypes...>(values);
    }

    return Error{
      fmt::format("Failed to create color from values {}", fmt::join(values, ", "))};
  }

  template <
    AnyColorT ColorTypeToTry,
    AnyColorT... MoreColorTypes,
    std::ranges::random_access_range R>
  static Result<ColorVariantT> parseComponentsImpl(const R& components)
  {
    if (auto result = ColorTypeToTry::parseComponents(components))
    {
      return result | kdl::transform([](const auto& x) { return ColorVariantT{x}; });
    }

    if constexpr (sizeof...(MoreColorTypes) > 0)
    {
      return parseComponentsImpl<MoreColorTypes...>(components);
    }

    return Error{
      fmt::format("Failed to parse '{}' as color", fmt::join(components, " "))};
  }
#ifdef _MSC_VER
#pragma warning(pop)
#endif
};

} // namespace tb
