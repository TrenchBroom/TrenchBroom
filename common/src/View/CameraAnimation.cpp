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

#include "CameraAnimation.h"

#include "Renderer/Camera.h"

#include <vecmath/forward.h>
#include <vecmath/vec.h>

namespace TrenchBroom::View
{
const Animation::Type CameraAnimation::AnimationType = Animation::freeType();

CameraAnimation::CameraAnimation(
  Renderer::Camera& camera,
  const vm::vec3f& targetPosition,
  const vm::vec3f& targetDirection,
  const vm::vec3f& targetUp,
  const double duration)
  : Animation{AnimationType, Curve::EaseInEaseOut, duration}
  , m_camera{camera}
  , m_startPosition{m_camera.position()}
  , m_startDirection{m_camera.direction()}
  , m_startUp{m_camera.up()}
  , m_targetPosition{targetPosition}
  , m_targetDirection{targetDirection}
  , m_targetUp{targetUp}
{
}

void CameraAnimation::doUpdate(const double progress)
{
  const auto fltProgress = float(progress);
  const auto position =
    m_startPosition + (m_targetPosition - m_startPosition) * fltProgress;
  const auto direction =
    m_startDirection + (m_targetDirection - m_startDirection) * fltProgress;
  const auto up = m_startUp + (m_targetUp - m_startUp) * fltProgress;

  m_camera.moveTo(position);
  m_camera.setDirection(direction, up);
}
} // namespace TrenchBroom::View
