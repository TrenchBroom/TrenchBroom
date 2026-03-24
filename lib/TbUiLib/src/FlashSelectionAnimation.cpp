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

#include "ui/FlashSelectionAnimation.h"

#include <QWidget>

#include "Color.h"
#include "render/MapRenderer.h"

#include <cmath>

namespace tb::ui
{
const Animation::Type FlashSelectionAnimation::AnimationType = Animation::freeType();

FlashSelectionAnimation::FlashSelectionAnimation(
  render::MapRenderer& renderer, QWidget* view, const double duration)
  : Animation{AnimationType, Curve::EaseInEaseOut, duration}
  , m_renderer{renderer}
  , m_view{view}
{
}

void FlashSelectionAnimation::doUpdate(const double progress)
{
  static constexpr auto flashCount = 2.0;
  static constexpr auto white = RgbaF{1.0f, 1.0f, 1.0f, 1.0f};

  if (progress < 1.0)
  {
    const auto flashProgress = progress * flashCount - std::floor(progress * flashCount);

    // factor ranges from 0 to 1 and then back to 0
    const auto factor =
      flashProgress < 0.5 ? 2.0 * flashProgress : 1.0 - 2.0 * (flashProgress - 0.5);
    m_renderer.overrideSelectionColors(white, float(factor * 0.8));
  }
  else
  {
    m_renderer.restoreSelectionColors();
  }

  m_view->update();
}

} // namespace tb::ui
