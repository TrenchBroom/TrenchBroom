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

#include "ui/Animation.h"

#include "vm/vec.h"

namespace tb::render
{
class Camera;
}

namespace tb::ui
{
class CameraAnimation : public Animation
{
private:
  static const Type AnimationType;

  render::Camera& m_camera;

  const vm::vec3f m_startPosition;
  const vm::vec3f m_startDirection;
  const vm::vec3f m_startUp;
  const float m_startZoom;
  const vm::vec3f m_targetPosition;
  const vm::vec3f m_targetDirection;
  const vm::vec3f m_targetUp;
  const float m_targetZoom;

public:
  CameraAnimation(
    render::Camera& camera,
    const vm::vec3f& targetPosition,
    const vm::vec3f& targetDirection,
    const vm::vec3f& targetUp,
    float targetZoom,
    double duration);

private:
  void doUpdate(double progress) override;
};

} // namespace tb::ui
