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

#include "vm/vec.h"

#include <cstdint>

class QKeyEvent;

namespace tb::render
{
class Camera;
}

namespace tb::ui
{

class FlyModeHelper
{
private:
  render::Camera& m_camera;

  bool m_forward = false;
  bool m_backward = false;
  bool m_left = false;
  bool m_right = false;
  bool m_up = false;
  bool m_down = false;
  bool m_fast = false;
  bool m_slow = false;

  int64_t m_lastPollTime;

public:
  explicit FlyModeHelper(render::Camera& camera);

  void pollAndUpdate();

public:
  void keyDown(QKeyEvent* event);
  void keyUp(QKeyEvent* event);
  /**
   * Returns whether the camera is currently moving due to a fly key being held down.
   */
  bool anyKeyDown() const;
  void resetKeys();

private:
  vm::vec3f moveDelta(float time);
  float moveSpeed() const;
};

} // namespace tb::ui
