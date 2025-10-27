
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

#include "Color.h"

#include "kdl/reflection_impl.h"

#include "vm/vec_io.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <optional>

namespace tb
{

kdl_reflect_impl(RgbF);
kdl_reflect_impl(RgbB);
kdl_reflect_impl(RgbaF);
kdl_reflect_impl(RgbaB);
kdl_reflect_impl(Color);

RgbF::RgbF()
  : m_v{0.0f, 0.0f, 0.0f}
{
}

RgbF::RgbF(const float r, const float g, const float b)
  : m_v{r, g, b}
{
}

RgbF::RgbF(const vm::vec<float, RgbF::S>& v)
  : m_v{v}
{
}

Result<RgbF> RgbF::parse(const std::string_view str)
{
  if (const auto v3f = vm::parse<float, 3>(str); v3f && isFloatColorRange(*v3f))
  {
    return RgbF{*v3f};
  }

  return Error{fmt::format("Failed to parse '{}' as RgbF", str)};
}


float RgbF::r() const
{
  return m_v[0];
}

float RgbF::g() const
{
  return m_v[1];
}

float RgbF::b() const
{
  return m_v[2];
}

RgbF::operator vm::vec<float, RgbF::S>() const
{
  return vec();
}

vm::vec<float, RgbF::S> RgbF::vec() const
{
  return m_v;
}

RgbF RgbF::toFloat() const
{
  return toRgbF();
}

RgbB RgbF::toByte() const
{
  return toRgbB();
}

RgbF RgbF::toRgbF() const
{
  return *this;
}

RgbB RgbF::toRgbB() const
{
  return RgbB{vm::vec<uint8_t, 3>{vec() * 255.0f}};
}

RgbaF RgbF::toRgbaF() const
{
  return RgbaF{r(), g(), b(), 1.0f};
}

RgbaB RgbF::toRgbaB() const
{
  return RgbaB{toRgbB(), 255};
}

std::string RgbF::toString() const
{
  return fmt::format("{} {} {}", r(), g(), b());
}

RgbB::RgbB()
  : m_v{0, 0, 0}
{
}

RgbB::RgbB(const uint8_t r, const uint8_t g, const uint8_t b)
  : m_v{r, g, b}
{
}

RgbB::RgbB(const vm::vec<uint8_t, RgbB::S>& v)
  : m_v{v}
{
}

Result<RgbB> RgbB::parse(const std::string_view str)
{
  if (const auto v3c = vm::parse<uint8_t, 3>(str); v3c && isByteColorRange(*v3c))
  {
    return RgbB{*v3c};
  }

  return Error{fmt::format("Failed to parse '{}' as RgbB", str)};
}

uint8_t RgbB::r() const
{
  return m_v[0];
}

uint8_t RgbB::g() const
{
  return m_v[1];
}

uint8_t RgbB::b() const
{
  return m_v[2];
}

RgbB::operator vm::vec<uint8_t, RgbB::S>() const
{
  return vec();
}

vm::vec<uint8_t, RgbB::S> RgbB::vec() const
{
  return m_v;
}

RgbF RgbB::toFloat() const
{
  return toRgbF();
}

RgbB RgbB::toByte() const
{
  return toRgbB();
}

RgbF RgbB::toRgbF() const
{
  return RgbF{vm::vec3f{vec()} / 255.0f};
}

RgbB RgbB::toRgbB() const
{
  return *this;
}

RgbaF RgbB::toRgbaF() const
{
  return RgbaF{toRgbF(), 1.0f};
}

RgbaB RgbB::toRgbaB() const
{
  return RgbaB{*this, 255};
}

std::string RgbB::toString() const
{
  return fmt::format("{} {} {}", r(), g(), b());
}

RgbaF::RgbaF()
  : m_v{0.0f, 0.0f, 0.0f, 0.0f}
{
}

RgbaF::RgbaF(const float r, const float g, const float b, const float a)
  : m_v{r, g, b, a}
{
}

RgbaF::RgbaF(const RgbF& rgb, float a)
  : m_v{rgb.vec(), a}
{
}

RgbaF::RgbaF(const vm::vec<float, RgbaF::S>& v)
  : m_v{v}
{
}

Result<RgbaF> RgbaF::parse(const std::string_view str)
{
  if (const auto v4f = vm::parse<float, 4>(str); v4f && isFloatColorRange(*v4f))
  {
    return RgbaF{*v4f};
  }

  return Error{fmt::format("Failed to parse '{}' as RgbaF", str)};
}

float RgbaF::r() const
{
  return m_v[0];
}

float RgbaF::g() const
{
  return m_v[1];
}

float RgbaF::b() const
{
  return m_v[2];
}

float RgbaF::a() const
{
  return m_v[3];
}

RgbaF::operator vm::vec<float, RgbaF::S>() const
{
  return vec();
}

vm::vec<float, RgbaF::S> RgbaF::vec() const
{
  return m_v;
}

RgbaF RgbaF::toFloat() const
{
  return toRgbaF();
}

RgbaB RgbaF::toByte() const
{
  return toRgbaB();
}

RgbF RgbaF::toRgbF() const
{
  return RgbF{r(), g(), b()};
}

RgbB RgbaF::toRgbB() const
{
  return toRgbF().toRgbB();
}

RgbaF RgbaF::toRgbaF() const
{
  return *this;
}

RgbaB RgbaF::toRgbaB() const
{
  return RgbaB{vm::vec<uint8_t, 4>{vec() * 255.0f}};
}

std::string RgbaF::toString() const
{
  return fmt::format("{} {} {} {}", r(), g(), b(), a());
}

RgbaB::RgbaB()
  : m_v{0, 0, 0, 0}
{
}

RgbaB::RgbaB(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a)
  : m_v{r, g, b, a}
{
}

RgbaB::RgbaB(const RgbB& rgbB, const uint8_t a)
  : m_v{rgbB.vec(), a}
{
}

RgbaB::RgbaB(const vm::vec<uint8_t, RgbaB::S>& v)
  : m_v{v}
{
}

Result<RgbaB> RgbaB::parse(const std::string_view str)
{
  if (const auto v4c = vm::parse<uint8_t, 4>(str); v4c && isByteColorRange(*v4c))
  {
    return RgbaB{*v4c};
  }

  return Error{fmt::format("Failed to parse '{}' as RgbaB", str)};
}

uint8_t RgbaB::r() const
{
  return m_v[0];
}

uint8_t RgbaB::g() const
{
  return m_v[1];
}

uint8_t RgbaB::b() const
{
  return m_v[2];
}

uint8_t RgbaB::a() const
{
  return m_v[3];
}

RgbaB::operator vm::vec<uint8_t, RgbaB::S>() const
{
  return vec();
}

vm::vec<uint8_t, RgbaB::S> RgbaB::vec() const
{
  return m_v;
}

RgbaF RgbaB::toFloat() const
{
  return toRgbaF();
}

RgbaB RgbaB::toByte() const
{
  return toRgbaB();
}

RgbF RgbaB::toRgbF() const
{
  return toRgbaF().toRgbF();
}

RgbB RgbaB::toRgbB() const
{
  return RgbB{r(), g(), b()};
}

RgbaF RgbaB::toRgbaF() const
{
  return RgbaF{vm::vec4f{vec()} / 255.0f};
}

RgbaB RgbaB::toRgbaB() const
{
  return *this;
}

std::string RgbaB::toString() const
{
  return fmt::format("{} {} {} {}", r(), g(), b(), a());
}

Color::Color()
  : m_value{RgbaF{}}
{
}

Color::Color(const RgbF& rgbF)
  : m_value{rgbF}
{
}

Color::Color(const RgbB& rgbB)
  : m_value{rgbB}
{
}

Color::Color(const RgbaF& rgbaF)
  : m_value{rgbaF}
{
}

Color::Color(const RgbaB& rgbaB)
  : m_value{rgbaB}
{
}

Result<Color> Color::parse(const std::string_view str)
{
  const auto toColor = [](const auto& x) { return Color{x}; };

  const auto toColorError = [&](const auto&) {
    return Result<Color>{Error{fmt::format("Failed to parse '{}' as color", str)}};
  };

  return RgbaF::parse(str) | kdl::transform(toColor) | kdl::or_else([&](const auto&) {
           return RgbaB::parse(str) | kdl::transform(toColor);
         })
         | kdl::or_else(
           [&](const auto&) { return RgbF::parse(str) | kdl::transform(toColor); })
         | kdl::or_else(
           [&](const auto&) { return RgbB::parse(str) | kdl::transform(toColor); })
         | kdl::or_else(toColorError);
}

bool Color::isFloat() const
{
  return std::holds_alternative<RgbF>(m_value) || std::holds_alternative<RgbaF>(m_value);
}

bool Color::isByte() const
{
  return std::holds_alternative<RgbB>(m_value) || std::holds_alternative<RgbaB>(m_value);
}

Color Color::toFloat() const
{
  return std::visit([](const auto& x) { return Color{x.toFloat()}; }, m_value);
}

Color Color::toByte() const
{
  return std::visit([](const auto& x) { return Color{x.toByte()}; }, m_value);
}

RgbF Color::toRgbF() const
{
  return std::visit([](const auto& x) { return x.toRgbF(); }, m_value);
}

RgbB Color::toRgbB() const
{
  return std::visit([](const auto& x) { return x.toRgbB(); }, m_value);
}

RgbaF Color::toRgbaF() const
{
  return std::visit([](const auto& x) { return x.toRgbaF(); }, m_value);
}

RgbaB Color::toRgbaB() const
{
  return std::visit([](const auto& x) { return x.toRgbaB(); }, m_value);
}

std::string Color::toString() const
{
  return std::visit([](const auto& x) { return x.toString(); }, m_value);
}

Color mixColors(const Color& lhs, const Color& rhs, const float f)
{
  return mixColors(lhs.toRgbF(), rhs.toRgbF(), f);
}

RgbaF blendColor(const RgbaF& c, const float f)
{
  return RgbaF{vm::vec4f{c.toRgbF().vec(), f * c.a()}};
}

void rgbToHSB(const float r, const float g, const float b, float& h, float& s, float& br)
{
  assert(r >= 0.0f && r <= 1.0f);
  assert(g >= 0.0f && g <= 1.0f);
  assert(b >= 0.0f && b <= 1.0f);

  const auto max = vm::max(r, g, b);
  const auto min = vm::min(r, g, b);
  const auto dist = max - min;

  br = max;
  s = br != 0.0f ? dist / max : s;

  if (s == 0.0f)
  {
    h = 0.0f;
  }
  else
  {
    const auto rc = (max - r) / dist;
    const auto gc = (max - g) / dist;
    const auto bc = (max - b) / dist;
    h = r == max ? bc - gc : g == max ? 2.0f + rc - bc : 4.0f + gc - rc;
    h = h / 6.0f;
    if (h < 0)
    {
      h = h + 1.0f;
    }
  }
}

} // namespace tb
