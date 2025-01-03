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

#include "FlyModeHelper.h"

#include <QElapsedTimer>
#include <QMouseEvent>

#include "PreferenceManager.h"
#include "Preferences.h"
#include "render/Camera.h"

#include "vm/vec.h"

namespace tb::ui
{
namespace
{

qint64 msecsSinceReference()
{
  auto timer = QElapsedTimer{};
  timer.start();
  return timer.msecsSinceReference();
}

bool eventMatchesShortcut(const QKeySequence& shortcut, QKeyEvent* event)
{
  if (shortcut.isEmpty())
  {
    return false;
  }

  // NOTE: For triggering fly mode we only support single keys.
  // e.g. you can't bind Shift+W to fly forward, only Shift or W.
  const auto ourKey = shortcut[0].key();
  const auto theirKey = event->key();
  return ourKey == theirKey;
}

} // namespace

FlyModeHelper::FlyModeHelper(render::Camera& camera)
  : m_camera{camera}
  , m_lastPollTime{msecsSinceReference()}
{
}

void FlyModeHelper::pollAndUpdate()
{
  const auto currentTime = msecsSinceReference();
  const auto time = float(currentTime - m_lastPollTime);
  m_lastPollTime = currentTime;

  if (anyKeyDown())
  {
    const auto delta = moveDelta(time);
    if (!vm::is_zero(delta, vm::Cf::almost_zero()))
    {
      m_camera.moveBy(delta);
    }
  }
}

void FlyModeHelper::keyDown(QKeyEvent* event)
{
  const auto& forward = pref(Preferences::CameraFlyForward());
  const auto& backward = pref(Preferences::CameraFlyBackward());
  const auto& left = pref(Preferences::CameraFlyLeft());
  const auto& right = pref(Preferences::CameraFlyRight());
  const auto& up = pref(Preferences::CameraFlyUp());
  const auto& down = pref(Preferences::CameraFlyDown());

  const auto wasAnyKeyDown = anyKeyDown();

  if (eventMatchesShortcut(forward, event))
  {
    m_forward = true;
  }
  if (eventMatchesShortcut(backward, event))
  {
    m_backward = true;
  }
  if (eventMatchesShortcut(left, event))
  {
    m_left = true;
  }
  if (eventMatchesShortcut(right, event))
  {
    m_right = true;
  }
  if (eventMatchesShortcut(up, event))
  {
    m_up = true;
  }
  if (eventMatchesShortcut(down, event))
  {
    m_down = true;
  }
  if (event->key() == Qt::Key_Shift)
  {
    m_fast = true;
  }
  if (event->key() == Qt::Key_Alt)
  {
    m_slow = true;
  }

  if (anyKeyDown() && !wasAnyKeyDown)
  {
    // Reset the last polling time, otherwise the view will jump!
    m_lastPollTime = msecsSinceReference();
  }
}

void FlyModeHelper::keyUp(QKeyEvent* event)
{
  const auto& forward = pref(Preferences::CameraFlyForward());
  const auto& backward = pref(Preferences::CameraFlyBackward());
  const auto& left = pref(Preferences::CameraFlyLeft());
  const auto& right = pref(Preferences::CameraFlyRight());
  const auto& up = pref(Preferences::CameraFlyUp());
  const auto& down = pref(Preferences::CameraFlyDown());

  if (event->isAutoRepeat())
  {
    // If it's an auto-repeat event, exit early without clearing the key down state.
    // Otherwise, the fake keyUp()/keyDown() calls would introduce movement stutters.
    return;
  }

  if (eventMatchesShortcut(forward, event))
  {
    m_forward = false;
  }
  if (eventMatchesShortcut(backward, event))
  {
    m_backward = false;
  }
  if (eventMatchesShortcut(left, event))
  {
    m_left = false;
  }
  if (eventMatchesShortcut(right, event))
  {
    m_right = false;
  }
  if (eventMatchesShortcut(up, event))
  {
    m_up = false;
  }
  if (eventMatchesShortcut(down, event))
  {
    m_down = false;
  }
  if (event->key() == Qt::Key_Shift)
  {
    m_fast = false;
  }
  if (event->key() == Qt::Key_Alt)
  {
    m_slow = false;
  }
}

bool FlyModeHelper::anyKeyDown() const
{
  return m_forward || m_backward || m_left || m_right || m_up || m_down;
}

void FlyModeHelper::resetKeys()
{
  m_forward = m_backward = m_left = m_right = m_up = m_down = m_fast = m_slow = false;
}

vm::vec3f FlyModeHelper::moveDelta(const float time)
{
  const float dist = moveSpeed() * time;

  auto delta = vm::vec3f{};
  if (m_forward)
  {
    delta = delta + m_camera.direction() * dist;
  }
  if (m_backward)
  {
    delta = delta - m_camera.direction() * dist;
  }
  if (m_left)
  {
    delta = delta - m_camera.right() * dist;
  }
  if (m_right)
  {
    delta = delta + m_camera.right() * dist;
  }
  if (m_up)
  {
    delta = delta + vm::vec3f{0, 0, 1} * dist;
  }
  if (m_down)
  {
    delta = delta - vm::vec3f{0, 0, 1} * dist;
  }
  return delta;
}

const auto SpeedModifier = 2.0f;

float FlyModeHelper::moveSpeed() const
{
  return m_fast   ? pref(Preferences::CameraFlyMoveSpeed) * SpeedModifier
         : m_slow ? pref(Preferences::CameraFlyMoveSpeed) / SpeedModifier
                  : pref(Preferences::CameraFlyMoveSpeed);
}

} // namespace tb::ui
