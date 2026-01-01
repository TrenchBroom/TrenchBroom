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

#include "ui/QColorUtils.h"

#include <QColor>

namespace tb::ui
{

Color fromQColor(const QColor& color)
{
  return RgbaF{
    static_cast<float>(color.redF()),
    static_cast<float>(color.greenF()),
    static_cast<float>(color.blueF()),
    static_cast<float>(color.alphaF())};
}

QColor toQColor(const Color& color)
{
  const auto rgbaF = color.to<RgbaF>();
  return QColor::fromRgbF(
    rgbaF.get<ColorChannel::r>(),
    rgbaF.get<ColorChannel::g>(),
    rgbaF.get<ColorChannel::b>(),
    rgbaF.get<ColorChannel::a>());
}

} // namespace tb::ui
