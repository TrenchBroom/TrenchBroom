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

#include "Result.h"

#include "kdl/reflection_decl.h"
#include "kdl/reflection_impl.h"

#include "vm/scalar.h"
#include "vm/vec.h"

#include <fmt/format.h>

#include <cstdint>
#include <string>
#include <string_view>
#include <variant>

namespace tb
{

template <typename T, size_t S>
bool isFloatColorRange(const vm::vec<T, S>& vec)
{
  for (size_t i = 0; i < S; ++i)
  {
    if (vec[i] < T(0) || vec[i] > T(1))
    {
      return false;
    }
  }

  return true;
}

template <typename T, size_t S>
bool isByteColorRange(const vm::vec<T, S>& vec)
{
  for (size_t i = 0; i < S; ++i)
  {
    if (vec[i] < T(0) || vec[i] > T(255))
    {
      return false;
    }

    if constexpr (std::is_floating_point_v<T>)
    {
      if (std::truncf(vec[i]) != vec[i])
      {
        return false;
      }
    }
  }

  return true;
}

class RgbF;
class RgbB;
class RgbaF;
class RgbaB;

class RgbF
{

public:
  static constexpr auto S = 3;

private:
  vm::vec<float, S> m_v;

public:
  RgbF();
  RgbF(float r, float g, float b);
  explicit RgbF(const vm::vec<float, S>& v);

  static Result<RgbF> parse(std::string_view str);

  float r() const;
  float g() const;
  float b() const;

  // NOLINTNEXTLINE(google-explicit-constructor)
  operator vm::vec<float, S>() const;

  vm::vec<float, S> vec() const;

  bool isFloat() const;
  bool isByte() const;

  RgbF toFloat() const;
  RgbB toByte() const;
  RgbF toRgbF() const;
  RgbB toRgbB() const;
  RgbaF toRgbaF() const;
  RgbaB toRgbaB() const;
  std::string toString() const;

  kdl_reflect_decl(RgbF, m_v);
};

class RgbB
{
public:
  static constexpr auto S = 3;

private:
  vm::vec<uint8_t, S> m_v;

public:
  RgbB();
  RgbB(uint8_t r, uint8_t g, uint8_t b);
  explicit RgbB(const vm::vec<uint8_t, S>& v);

  static Result<RgbB> parse(std::string_view str);

  uint8_t r() const;
  uint8_t g() const;
  uint8_t b() const;

  // NOLINTNEXTLINE(google-explicit-constructor)
  operator vm::vec<uint8_t, S>() const;

  vm::vec<uint8_t, S> vec() const;

  bool isFloat() const;
  bool isByte() const;

  RgbF toFloat() const;
  RgbB toByte() const;
  RgbF toRgbF() const;
  RgbB toRgbB() const;
  RgbaF toRgbaF() const;
  RgbaB toRgbaB() const;
  std::string toString() const;

  kdl_reflect_decl(RgbB, m_v);
};

class RgbaF
{
public:
  static constexpr auto S = 4;

private:
  vm::vec<float, S> m_v;

public:
  RgbaF();
  RgbaF(float r, float g, float b, float a);
  RgbaF(const RgbF& rgb, float a);
  explicit RgbaF(const vm::vec<float, S>& v);

  static Result<RgbaF> parse(std::string_view str);

  float r() const;
  float g() const;
  float b() const;
  float a() const;

  // NOLINTNEXTLINE(google-explicit-constructor)
  operator vm::vec<float, S>() const;

  vm::vec<float, S> vec() const;

  bool isFloat() const;
  bool isByte() const;

  RgbaF toFloat() const;
  RgbaB toByte() const;
  RgbF toRgbF() const;
  RgbB toRgbB() const;
  RgbaF toRgbaF() const;
  RgbaB toRgbaB() const;
  std::string toString() const;

  kdl_reflect_decl(RgbaF, m_v);
};

class RgbaB
{
public:
  static constexpr auto S = 4;

private:
  vm::vec<uint8_t, S> m_v;

public:
  RgbaB();
  RgbaB(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
  RgbaB(const RgbB& rgb, uint8_t a);
  explicit RgbaB(const vm::vec<uint8_t, S>& v);

  static Result<RgbaB> parse(std::string_view str);

  uint8_t r() const;
  uint8_t g() const;
  uint8_t b() const;
  uint8_t a() const;

  // NOLINTNEXTLINE(google-explicit-constructor)
  operator vm::vec<uint8_t, S>() const;

  vm::vec<uint8_t, S> vec() const;

  bool isFloat() const;
  bool isByte() const;

  RgbaF toFloat() const;
  RgbaB toByte() const;
  RgbF toRgbF() const;
  RgbB toRgbB() const;
  RgbaF toRgbaF() const;
  RgbaB toRgbaB() const;
  std::string toString() const;

  kdl_reflect_decl(RgbaB, m_v);
};

template <typename T, typename... ColorTypes>
concept AnyTypeOf = (std::is_same_v<std::remove_cvref_t<T>, ColorTypes> || ...);

template <typename... ColorTypes>
class ColorT
{
private:
  std::variant<ColorTypes...> m_value;

public:
  template <typename C>
  static constexpr auto AnyColor = AnyTypeOf<C, ColorTypes...>;

  ColorT()
    : m_value{RgbaF{}}
  {
  }

  // NOLINTBEGIN(google-explicit-constructor)
  template <typename Color>
  ColorT(const Color& color)
    requires(AnyTypeOf<Color, ColorTypes...>)
    : m_value{color}
  {
  }
  // NOLINTEND(google-explicit-constructor)

  template <typename T, size_t S>
  static Result<ColorT> fromVec(const vm::vec<T, S>& vec)
  {
    if constexpr (S == 3)
    {
      if constexpr (AnyTypeOf<RgbF, ColorTypes...>)
      {
        if (isFloatColorRange(vec))
        {
          return ColorT{RgbF{vm::vec<float, S>{vec}}};
        }
      }

      if constexpr (AnyTypeOf<RgbB, ColorTypes...>)
      {
        if (isByteColorRange(vec))
        {
          return ColorT{RgbB{vm::vec<uint8_t, S>{vec}}};
        }
      }
    }

    if constexpr (S == 4)
    {
      if constexpr (AnyTypeOf<RgbaF, ColorTypes...>)
      {
        if (isFloatColorRange(vec))
        {
          return ColorT{RgbaF{vm::vec<float, S>{vec}}};
        }
      }

      if constexpr (AnyTypeOf<RgbaB, ColorTypes...>)
      {
        if (isByteColorRange(vec))
        {
          return ColorT{RgbaB{vm::vec<uint8_t, S>{vec}}};
        }
      }
    }

    return Error{"Invalid color values"};
  }

  static Result<ColorT> parse(std::string_view str)
  {
    return parseImpl<ColorTypes...>(str);
  }

  bool isFloat() const
  {
    return std::visit([](const auto& x) { return x.isFloat(); }, m_value);
  }

  bool isByte() const
  {
    return std::visit([](const auto& x) { return x.isByte(); }, m_value);
  }

  ColorT toFloat() const
  {
    return std::visit([](const auto& x) { return ColorT{x.toFloat()}; }, m_value);
  }

  ColorT toByte() const
  {
    return std::visit([](const auto& x) { return ColorT{x.toByte()}; }, m_value);
  }

  RgbF toRgbF() const
  {
    return std::visit([](const auto& x) { return x.toRgbF(); }, m_value);
  }

  RgbB toRgbB() const
  {
    return std::visit([](const auto& x) { return x.toRgbB(); }, m_value);
  }

  RgbaF toRgbaF() const
  {
    return std::visit([](const auto& x) { return x.toRgbaF(); }, m_value);
  }

  RgbaB toRgbaB() const
  {
    return std::visit([](const auto& x) { return x.toRgbaB(); }, m_value);
  }

  std::string toString() const
  {
    return std::visit([](const auto& x) { return x.toString(); }, m_value);
  }

  kdl_reflect_inline(ColorT, m_value);

private:
  template <typename ColorTypeToTry, typename... MoreColorTypes>
  static Result<ColorT> parseImpl(std::string_view str)
  {
    if (auto result = ColorTypeToTry::parse(str))
    {
      return result | kdl::transform([](const auto& x) { return ColorT{x}; });
    }

    if constexpr (sizeof...(MoreColorTypes) > 0)
    {
      return parseImpl<MoreColorTypes...>(str);
    }

    return Error{fmt::format("Failed to parse '{}' as color", str)};
  }
};

using Color = ColorT<RgbaF, RgbaB, RgbF, RgbB>;
using Rgb = ColorT<RgbF, RgbB>;
using Rgba = ColorT<RgbaF, RgbaB>;

template <typename C>
auto mixColors(const C& lhs, const C& rhs, const float f)
  requires(Color::AnyColor<C>)
{
  return C{vm::mix(
    lhs.toFloat().vec(), rhs.toFloat().vec(), vm::vec<float, C::S>::fill(vm::clamp(f)))};
}

Color mixColors(const Color& lhs, const Color& rhs, float f);

RgbaF blendColor(const RgbaF& c, float f);

void rgbToHSB(float r, float g, float b, float& h, float& s, float& br);

} // namespace tb
