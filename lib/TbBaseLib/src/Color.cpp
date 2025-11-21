
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

namespace tb
{

template struct ColorComponentType<ColorChannel::r, float, 0.0f, 1.0f, 0.0f>;
template struct ColorComponentType<ColorChannel::g, float, 0.0f, 1.0f, 0.0f>;
template struct ColorComponentType<ColorChannel::b, float, 0.0f, 1.0f, 0.0f>;
template struct ColorComponentType<ColorChannel::a, float, 0.0f, 1.0f, 1.0f>;

template struct ColorComponentType<ColorChannel::r, uint8_t, 0, 255, 0>;
template struct ColorComponentType<ColorChannel::g, uint8_t, 0, 255, 0>;
template struct ColorComponentType<ColorChannel::b, uint8_t, 0, 255, 0>;
template struct ColorComponentType<ColorChannel::a, uint8_t, 0, 255, 255>;

template class ColorT<ColorComponents::Rf, ColorComponents::Gf, ColorComponents::Bf>;
template class ColorT<ColorComponents::Rb, ColorComponents::Gb, ColorComponents::Bb>;

template class ColorT<
  ColorComponents::Rf,
  ColorComponents::Gf,
  ColorComponents::Bf,
  ColorComponents::Af>;
template class ColorT<
  ColorComponents::Rb,
  ColorComponents::Gb,
  ColorComponents::Bb,
  ColorComponents::Ab>;

template class ColorVariantT<RgbF, RgbB>;
template class ColorVariantT<RgbaF, RgbaB>;
template class ColorVariantT<RgbaF, RgbaB, RgbF, RgbB>;

RgbaF blendColor(const RgbaF& c, const float f)
{
  return RgbaF{c.to<RgbF>(), f * c.get<ColorChannel::a>()};
}

} // namespace tb
