
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

#include "vm/scalar.h"
#include "vm/vec.h"
#include "vm/vec_io.h"

#include <sstream>

namespace tb
{

std::optional<Color> Color::parse(const std::string& str)
{
  if (const auto c4 = vm::parse<float, 4>(str))
  {
    return Color{c4->x(), c4->y(), c4->z(), c4->w()};
  }
  if (const auto c3 = vm::parse<float, 3>(str))
  {
    return Color{c3->x(), c3->y(), c3->z()};
  }
  return std::nullopt;
}

std::string Color::toString() const
{
  auto ss = std::stringstream{};
  if (a() == 1.0f)
  {
    ss << xyz();
  }
  else
  {
    ss << xyzw();
  }
  return ss.str();
}

Color::Color()
  : vec<float, 4>{0.0f, 0.0f, 0.0f, 0.0f}
{
}

Color::Color(const vec<float, 4>& i_v)
  : vec<float, 4>{i_v}
{
}

Color::Color(const float r, const float g, const float b, const float a)
  : vec<float, 4>{r, g, b, a}
{
}

Color::Color(const Color& color, const float a)
  : vec<float, 4>{color.r(), color.g(), color.b(), a}
{
}

Color::Color(
  const unsigned char r,
  const unsigned char g,
  const unsigned char b,
  const unsigned char a)
  : vec<float, 4>{
      float(r) / 255.0f,
      float(g) / 255.0f,
      float(b) / 255.0f,
      float(a) / 255.0f,
    }
{
}

Color::Color(const int r, const int g, const int b, const int a)
  : vec<float, 4>{
      float(r) / 255.0f,
      float(g) / 255.0f,
      float(b) / 255.0f,
      float(a) / 255.0f,
    }
{
}

Color::Color(const int r, const int g, const int b, const float a)
  : vec<float, 4>{
      float(r) / 255.0f,
      float(g) / 255.0f,
      float(b) / 255.0f,
      a,
    }
{
}

float Color::r() const
{
  return x();
}

float Color::g() const
{
  return y();
}

float Color::b() const
{
  return z();
}

float Color::a() const
{
  return w();
}

void Color::rgbToHSB(
  const float r, const float g, const float b, float& h, float& s, float& br)
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
