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

#include "ColorComponentType.h"
#include "ColorT.h"
#include "ColorVariantT.h"

#include "vm/scalar.h"
#include "vm/vec.h"

#include <cstdint>

namespace tb
{
extern template struct ColorComponentType<ColorChannel::r, float, 0.0f, 1.0f, 0.0f>;
extern template struct ColorComponentType<ColorChannel::g, float, 0.0f, 1.0f, 0.0f>;
extern template struct ColorComponentType<ColorChannel::b, float, 0.0f, 1.0f, 0.0f>;
extern template struct ColorComponentType<ColorChannel::a, float, 0.0f, 1.0f, 1.0f>;

extern template struct ColorComponentType<ColorChannel::r, uint8_t, 0, 255, 0>;
extern template struct ColorComponentType<ColorChannel::g, uint8_t, 0, 255, 0>;
extern template struct ColorComponentType<ColorChannel::b, uint8_t, 0, 255, 0>;
extern template struct ColorComponentType<ColorChannel::a, uint8_t, 0, 255, 255>;

namespace ColorComponents
{

using Rf = ColorComponentType<ColorChannel::r, float, 0.0f, 1.0f, 0.0f>;
using Gf = ColorComponentType<ColorChannel::g, float, 0.0f, 1.0f, 0.0f>;
using Bf = ColorComponentType<ColorChannel::b, float, 0.0f, 1.0f, 0.0f>;
using Af = ColorComponentType<ColorChannel::a, float, 0.0f, 1.0f, 1.0f>;

using Rb = ColorComponentType<ColorChannel::r, uint8_t, 0, 255, 0>;
using Gb = ColorComponentType<ColorChannel::g, uint8_t, 0, 255, 0>;
using Bb = ColorComponentType<ColorChannel::b, uint8_t, 0, 255, 0>;
using Ab = ColorComponentType<ColorChannel::a, uint8_t, 0, 255, 255>;

} // namespace ColorComponents

extern template class ColorT<
  ColorComponents::Rf,
  ColorComponents::Gf,
  ColorComponents::Bf>;
extern template class ColorT<
  ColorComponents::Rb,
  ColorComponents::Gb,
  ColorComponents::Bb>;

extern template class ColorT<
  ColorComponents::Rf,
  ColorComponents::Gf,
  ColorComponents::Bf,
  ColorComponents::Af>;
extern template class ColorT<
  ColorComponents::Rb,
  ColorComponents::Gb,
  ColorComponents::Bb,
  ColorComponents::Ab>;

using RgbF = ColorT<ColorComponents::Rf, ColorComponents::Gf, ColorComponents::Bf>;
using RgbB = ColorT<ColorComponents::Rb, ColorComponents::Gb, ColorComponents::Bb>;

using RgbaF = ColorT<
  ColorComponents::Rf,
  ColorComponents::Gf,
  ColorComponents::Bf,
  ColorComponents::Af>;
using RgbaB = ColorT<
  ColorComponents::Rb,
  ColorComponents::Gb,
  ColorComponents::Bb,
  ColorComponents::Ab>;

extern template class ColorVariantT<RgbF, RgbB>;
extern template class ColorVariantT<RgbaF, RgbaB>;
extern template class ColorVariantT<RgbaF, RgbaB, RgbF, RgbB>;

using Rgb = ColorVariantT<RgbF, RgbB>;
using Rgba = ColorVariantT<RgbaF, RgbaB>;
using Color = ColorVariantT<RgbaF, RgbaB, RgbF, RgbB>;

template <typename T>
concept AnyColor =
  (std::is_same_v<T, RgbF> || std::is_same_v<T, RgbB> || std::is_same_v<T, RgbaF>
   || std::is_same_v<T, RgbaB> || std::is_same_v<T, Rgb> || std::is_same_v<T, Rgba>
   || std::is_same_v<T, Color>);

template <typename T>
concept AnyColorF = (std::is_same_v<T, RgbF> || std::is_same_v<T, RgbaF>);

template <AnyColorF C>
auto mixColors(const C& lhs, const C& rhs, const float f)
{
  // Assuming that lhs and rhs are valid colors, clamping f ensures that the result is
  // also valid
  static constexpr auto N = C::NumComponents;
  return C::fromVec(
           vm::mix(lhs.toVec(), rhs.toVec(), vm::vec<float, N>::fill(vm::clamp(f))))
         | kdl::value();
}

RgbaF blendColor(const RgbaF& c, float f);

} // namespace tb
