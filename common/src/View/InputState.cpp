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

#include "InputState.h"

#include <QCursor>

#include "FloatType.h"
#include "Macros.h"
#include "Model/CompareHits.h"
#include "Renderer/Camera.h"

#include <vecmath/vec.h>

namespace TrenchBroom
{
namespace View
{
InputState::InputState()
  : m_modifierKeys(ModifierKeys::MKNone)
  , m_mouseButtons(MouseButtons::MBNone)
  , m_mouseX(0.0f)
  , m_mouseY(0.0f)
  , m_mouseDX(0.0f)
  , m_mouseDY(0.0f)
  , m_scrollX(0.0f)
  , m_scrollY(0.0f)
  , m_anyToolDragging(false)
{
  const QPoint mouseState = QCursor::pos();
  m_mouseX = static_cast<float>(mouseState.x());
  m_mouseY = static_cast<float>(mouseState.y());
}

InputState::InputState(const float mouseX, const float mouseY)
  : m_modifierKeys(ModifierKeys::MKNone)
  , m_mouseButtons(MouseButtons::MBNone)
  , m_mouseX(mouseX)
  , m_mouseY(mouseY)
  , m_mouseDX(0.0f)
  , m_mouseDY(0.0f)
  , m_scrollX(0.0f)
  , m_scrollY(0.0f)
  , m_anyToolDragging(false)
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
  assert(key1 != ModifierKeys::MKDontCare);
  if (modifierKeysPressed(key1))
    return true;
  if (key2 != ModifierKeys::MKDontCare && modifierKeysPressed(key2))
    return true;
  if (key3 != ModifierKeys::MKDontCare && modifierKeysPressed(key3))
    return true;
  if (key4 != ModifierKeys::MKDontCare && modifierKeysPressed(key4))
    return true;
  return false;
}

bool InputState::checkModifierKeys(
  const ModifierKeyPressed ctrl,
  const ModifierKeyPressed alt,
  const ModifierKeyPressed shift) const
{
  return (
    checkModifierKey(ctrl, ModifierKeys::MKCtrlCmd)
    && checkModifierKey(alt, ModifierKeys::MKAlt)
    && checkModifierKey(shift, ModifierKeys::MKShift));
}

bool InputState::checkModifierKey(ModifierKeyPressed state, ModifierKeyState key) const
{
  switch (state)
  {
  case MK_Yes:
    return modifierKeysDown(key);
  case MK_No:
    return !modifierKeysDown(key);
  case MK_DontCare:
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

float InputState::scrollX() const
{
  return m_scrollX;
}

float InputState::scrollY() const
{
  return m_scrollY;
}

void InputState::setModifierKeys(const ModifierKeyState keys)
{
  m_modifierKeys = keys;
}

void InputState::clearModifierKeys()
{
  m_modifierKeys = ModifierKeys::MKNone;
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
  m_mouseButtons = MouseButtons::MBNone;
}

void InputState::mouseMove(
  const float mouseX, const float mouseY, const float mouseDX, const float mouseDY)
{
  m_mouseX = mouseX;
  m_mouseY = mouseY;
  m_mouseDX = mouseDX;
  m_mouseDY = mouseDY;
}

void InputState::scroll(const float scrollX, const float scrollY)
{
  m_scrollX = scrollX;
  m_scrollY = scrollY;
}

bool InputState::anyToolDragging() const
{
  return m_anyToolDragging;
}

void InputState::setAnyToolDragging(const bool anyToolDragging)
{
  m_anyToolDragging = anyToolDragging;
}

const vm::ray3& InputState::pickRay() const
{
  return m_pickRequest.pickRay();
}

const vm::vec3 InputState::defaultPoint() const
{
  return vm::vec3(camera().defaultPoint());
}

const vm::vec3 InputState::defaultPointUnderMouse() const
{
  return vm::vec3(camera().defaultPoint(pickRay()));
}

const Renderer::Camera& InputState::camera() const
{
  return m_pickRequest.camera();
}

void InputState::setPickRequest(const PickRequest& pickRequest)
{
  m_pickRequest = pickRequest;
}

const Model::PickResult& InputState::pickResult() const
{
  return m_pickResult;
}

void InputState::setPickResult(Model::PickResult&& pickResult)
{
  m_pickResult = std::move(pickResult);
}
} // namespace View
} // namespace TrenchBroom
