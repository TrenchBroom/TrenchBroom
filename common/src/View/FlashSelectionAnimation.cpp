/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "FlashSelectionAnimation.h"

#include "Color.h"
#include "Renderer/MapRenderer.h"

#include <QWidget>

namespace TrenchBroom {
namespace View {
const Animation::Type FlashSelectionAnimation::AnimationType = Animation::freeType();

FlashSelectionAnimation::FlashSelectionAnimation(
  Renderer::MapRenderer& renderer, QWidget* view, const double duration)
  : Animation(AnimationType, Curve::EaseInEaseOut, duration)
  , m_renderer(renderer)
  , m_view(view) {}

void FlashSelectionAnimation::doUpdate(const double progress) {
  static const Color white(1.0f, 1.0f, 1.0f, 1.0f);

  const float fltProgress = static_cast<float>(progress);
  if (fltProgress < 1.0f) {
    // factor ranges from 0 to 1 and then back to 0
    const float factor =
      fltProgress < 0.5f ? 2.0f * fltProgress : 1.0f - 2.0f * (fltProgress - 0.5f);
    m_renderer.overrideSelectionColors(white, factor * 0.8f);
  } else {
    m_renderer.restoreSelectionColors();
  }

  m_view->update();
}
} // namespace View
} // namespace TrenchBroom
