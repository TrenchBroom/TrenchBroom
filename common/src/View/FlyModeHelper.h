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

#pragma once

#include <vecmath/forward.h>

#include <cstdint>

class QKeyEvent;

namespace TrenchBroom {
namespace Renderer {
class Camera;
}

namespace View {
class FlyModeHelper {
private:
  Renderer::Camera& m_camera;

  bool m_forward;
  bool m_backward;
  bool m_left;
  bool m_right;
  bool m_up;
  bool m_down;
  bool m_fast;
  bool m_slow;

  int64_t m_lastPollTime;

public:
  explicit FlyModeHelper(Renderer::Camera& camera);

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
} // namespace View
} // namespace TrenchBroom
