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

#include "FlyModeHelper.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/Camera.h"

#include <vecmath/vec.h>

#include <QElapsedTimer>
#include <QMouseEvent>

namespace TrenchBroom {
namespace View {
static qint64 msecsSinceReference() {
  QElapsedTimer timer;
  timer.start();
  return timer.msecsSinceReference();
}

FlyModeHelper::FlyModeHelper(Renderer::Camera& camera)
  : m_camera(camera)
  , m_forward(false)
  , m_backward(false)
  , m_left(false)
  , m_right(false)
  , m_up(false)
  , m_down(false)
  , m_lastPollTime(msecsSinceReference()) {}

void FlyModeHelper::pollAndUpdate() {
  const auto currentTime = msecsSinceReference();
  const auto time = float(currentTime - m_lastPollTime);
  m_lastPollTime = currentTime;

  if (anyKeyDown()) {
    const auto delta = moveDelta(time);
    if (!vm::is_zero(delta, vm::Cf::almost_zero())) {
      m_camera.moveBy(delta);
    }
  }
}

static bool eventMatchesShortcut(const QKeySequence& shortcut, QKeyEvent* event) {
  if (shortcut.isEmpty()) {
    return false;
  }

  // NOTE: For triggering fly mode we only support single keys.
  // e.g. you can't bind Shift+W to fly forward, only Shift or W.
  const int ourKey = shortcut[0];
  const int theirKey = event->key();
  return ourKey == theirKey;
}

void FlyModeHelper::keyDown(QKeyEvent* event) {
  const QKeySequence& forward = pref(Preferences::CameraFlyForward());
  const QKeySequence& backward = pref(Preferences::CameraFlyBackward());
  const QKeySequence& left = pref(Preferences::CameraFlyLeft());
  const QKeySequence& right = pref(Preferences::CameraFlyRight());
  const QKeySequence& up = pref(Preferences::CameraFlyUp());
  const QKeySequence& down = pref(Preferences::CameraFlyDown());

  const auto wasAnyKeyDown = anyKeyDown();

  if (eventMatchesShortcut(forward, event)) {
    m_forward = true;
  }
  if (eventMatchesShortcut(backward, event)) {
    m_backward = true;
  }
  if (eventMatchesShortcut(left, event)) {
    m_left = true;
  }
  if (eventMatchesShortcut(right, event)) {
    m_right = true;
  }
  if (eventMatchesShortcut(up, event)) {
    m_up = true;
  }
  if (eventMatchesShortcut(down, event)) {
    m_down = true;
  }

  if (anyKeyDown() && !wasAnyKeyDown) {
    // Reset the last polling time, otherwise the view will jump!
    m_lastPollTime = msecsSinceReference();
  }
}

void FlyModeHelper::keyUp(QKeyEvent* event) {
  const QKeySequence& forward = pref(Preferences::CameraFlyForward());
  const QKeySequence& backward = pref(Preferences::CameraFlyBackward());
  const QKeySequence& left = pref(Preferences::CameraFlyLeft());
  const QKeySequence& right = pref(Preferences::CameraFlyRight());
  const QKeySequence& up = pref(Preferences::CameraFlyUp());
  const QKeySequence& down = pref(Preferences::CameraFlyDown());

  if (event->isAutoRepeat()) {
    // If it's an auto-repeat event, exit early without clearing the key down state.
    // Otherwise, the fake keyUp()/keyDown() calls would introduce movement stutters.
    return;
  }

  if (eventMatchesShortcut(forward, event)) {
    m_forward = false;
  }
  if (eventMatchesShortcut(backward, event)) {
    m_backward = false;
  }
  if (eventMatchesShortcut(left, event)) {
    m_left = false;
  }
  if (eventMatchesShortcut(right, event)) {
    m_right = false;
  }
  if (eventMatchesShortcut(up, event)) {
    m_up = false;
  }
  if (eventMatchesShortcut(down, event)) {
    m_down = false;
  }
}

bool FlyModeHelper::anyKeyDown() const {
  return m_forward || m_backward || m_left || m_right || m_up || m_down;
}

void FlyModeHelper::resetKeys() {
  m_forward = m_backward = m_left = m_right = m_up = m_down = false;
}

vm::vec3f FlyModeHelper::moveDelta(const float time) {
  const float dist = moveSpeed() * time;

  vm::vec3f delta;
  if (m_forward) {
    delta = delta + m_camera.direction() * dist;
  }
  if (m_backward) {
    delta = delta - m_camera.direction() * dist;
  }
  if (m_left) {
    delta = delta - m_camera.right() * dist;
  }
  if (m_right) {
    delta = delta + m_camera.right() * dist;
  }
  if (m_up) {
    delta = delta + vm::vec3f::pos_z() * dist;
  }
  if (m_down) {
    delta = delta - vm::vec3f::pos_z() * dist;
  }
  return delta;
}

float FlyModeHelper::moveSpeed() const {
  return pref(Preferences::CameraFlyMoveSpeed);
}
} // namespace View
} // namespace TrenchBroom
