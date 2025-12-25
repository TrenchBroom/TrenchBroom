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

#include "InputState.h"

#include <QCursor>

#include "Macros.h"
#include "gl/Camera.h"

#include "kd/contracts.h"

#include "vm/vec.h"

namespace tb::ui
{

InputState::InputState()
{
  const auto mouseState = QCursor::pos();
  m_mouseX = float(mouseState.x());
  m_mouseY = float(mouseState.y());
}

InputState::InputState(const float mouseX, const float mouseY)
  : m_mouseX{mouseX}
  , m_mouseY{mouseY}
{
}

InputState::~InputState() {}

ModifierKeyState InputState::modifierKeys() const
{
  return m_modifierKeys;
}

bool InputState::modifierKeysDown(const ModifierKeyState keys) const
{
  return (modifierKeys() & keys) != 0;
}

bool InputState::modifierKeysPressed(const ModifierKeyState keys) const
{
  return modifierKeys() == keys;
}

bool InputState::checkModifierKeys(
  const ModifierKeyState key1,
  const ModifierKeyState key2,
  const ModifierKeyState key3,
  const ModifierKeyState key4) const
{
  contract_pre(key1 != ModifierKeys::DontCare);

  if (modifierKeysPressed(key1))
  {
    return true;
  }
  if (key2 != ModifierKeys::DontCare && modifierKeysPressed(key2))
  {
    return true;
  }
  if (key3 != ModifierKeys::DontCare && modifierKeysPressed(key3))
  {
    return true;
  }
  if (key4 != ModifierKeys::DontCare && modifierKeysPressed(key4))
  {
    return true;
  }
  return false;
}

bool InputState::checkModifierKeys(
  const ModifierKeyPressed ctrl,
  const ModifierKeyPressed alt,
  const ModifierKeyPressed shift) const
{
  return (
    checkModifierKey(ctrl, ModifierKeys::CtrlCmd)
    && checkModifierKey(alt, ModifierKeys::Alt)
    && checkModifierKey(shift, ModifierKeys::Shift));
}

bool InputState::checkModifierKey(ModifierKeyPressed state, ModifierKeyState key) const
{
  switch (state)
  {
  case ModifierKeyPressed::Yes:
    return modifierKeysDown(key);
  case ModifierKeyPressed::No:
    return !modifierKeysDown(key);
  case ModifierKeyPressed::DontCare:
    return true;
    switchDefault();
  }
}

MouseButtonState InputState::mouseButtons() const
{
  return m_mouseButtons;
}

bool InputState::mouseButtonsDown(const MouseButtonState buttons) const
{
  return (mouseButtons() & buttons) != 0;
}

bool InputState::mouseButtonsPressed(const MouseButtonState buttons) const
{
  return mouseButtons() == buttons;
}

float InputState::mouseX() const
{
  return m_mouseX;
}

float InputState::mouseY() const
{
  return m_mouseY;
}

float InputState::mouseDX() const
{
  return m_mouseDX;
}

float InputState::mouseDY() const
{
  return m_mouseDY;
}

ScrollSource InputState::scrollSource() const
{
  return m_scrollSource;
}

float InputState::scrollX() const
{
  return m_scrollX;
}

float InputState::scrollY() const
{
  return m_scrollY;
}

bool InputState::gestureActive() const
{
  return m_gestureActive;
}

float InputState::gesturePanX() const
{
  return m_gesturePanX;
}

float InputState::gesturePanY() const
{
  return m_gesturePanY;
}

float InputState::gesturePanDX() const
{
  return m_gesturePanDX;
}
float InputState::gesturePanDY() const
{
  return m_gesturePanDY;
}

float InputState::gestureZoomValue() const
{
  return m_gestureZoomValue;
}

float InputState::gestureRotateValue() const
{
  return m_gestureRotateValue;
}

void InputState::setModifierKeys(const ModifierKeyState keys)
{
  m_modifierKeys = keys;
}

void InputState::clearModifierKeys()
{
  m_modifierKeys = ModifierKeys::None;
}

void InputState::mouseDown(const MouseButtonState button)
{
  m_mouseButtons |= button;
}

void InputState::mouseUp(const MouseButtonState button)
{
  m_mouseButtons &= ~button;
}

void InputState::clearMouseButtons()
{
  m_mouseButtons = MouseButtons::None;
}

void InputState::mouseMove(
  const float mouseX, const float mouseY, const float mouseDX, const float mouseDY)
{
  m_mouseX = mouseX;
  m_mouseY = mouseY;
  m_mouseDX = mouseDX;
  m_mouseDY = mouseDY;
}

void InputState::scroll(
  const ScrollSource scrollSource, const float scrollX, const float scrollY)
{
  m_scrollSource = scrollSource;
  m_scrollX = scrollX;
  m_scrollY = scrollY;
}

void InputState::startGesture()
{
  m_gestureActive = true;
}

void InputState::gesturePan(const float x, const float y, const float dx, const float dy)
{
  m_gesturePanX = x;
  m_gesturePanY = y;
  m_gesturePanDX = dx;
  m_gesturePanDY = dy;
}

void InputState::gestureZoom(const float value)
{
  m_gestureZoomValue = value;
}

void InputState::gestureRotate(const float value)
{
  m_gestureRotateValue = value;
}

void InputState::endGesture()
{
  m_gestureActive = false;
  m_gesturePanX = 0.0f;
  m_gesturePanY = 0.0f;
  m_gestureZoomValue = 0.0f;
  m_gestureRotateValue = 0.0f;
}

bool InputState::anyToolDragging() const
{
  return m_anyToolDragging;
}

void InputState::setAnyToolDragging(const bool anyToolDragging)
{
  m_anyToolDragging = anyToolDragging;
}

const vm::ray3d& InputState::pickRay() const
{
  return m_pickRequest.pickRay();
}

const vm::vec3d InputState::defaultPoint() const
{
  return vm::vec3d{camera().defaultPoint()};
}

const vm::vec3d InputState::defaultPointUnderMouse() const
{
  return vm::vec3d{camera().defaultPoint(pickRay())};
}

const gl::Camera& InputState::camera() const
{
  return m_pickRequest.camera();
}

void InputState::setPickRequest(const PickRequest& pickRequest)
{
  m_pickRequest = pickRequest;
}

const mdl::PickResult& InputState::pickResult() const
{
  return m_pickResult;
}

void InputState::setPickResult(mdl::PickResult pickResult)
{
  m_pickResult = std::move(pickResult);
}
} // namespace tb::ui
