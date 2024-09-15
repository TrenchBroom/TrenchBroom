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

#include "FloatType.h"
#include "Model/PickResult.h"
#include "View/PickRequest.h"

namespace TrenchBroom::Renderer
{
class Camera;
}

namespace TrenchBroom::View
{
using ModifierKeyState = unsigned int;
namespace ModifierKeys
{
static const ModifierKeyState None = 0;
static const ModifierKeyState Shift = 1 << 0;
static const ModifierKeyState MKCtrlCmd = 1 << 1; // Cmd on Mac, Ctrl on other systems
static const ModifierKeyState MKAlt = 1 << 2;
static const ModifierKeyState DontCare = 1 << 3;
} // namespace ModifierKeys

enum class ModifierKeyPressed
{
  Yes,
  No,
  DontCare
};

using MouseButtonState = unsigned int;
namespace MouseButtons
{
static const MouseButtonState None = 0;
static const MouseButtonState Left = 1 << 0;
static const MouseButtonState Right = 1 << 1;
static const MouseButtonState Middle = 1 << 2;
} // namespace MouseButtons

enum class ScrollSource
{
  Mouse,
  Trackpad,
};

enum class GestureType
{
  Pan,
  Zoom,
  Rotate,
};

class InputState
{
private:
  ModifierKeyState m_modifierKeys = ModifierKeys::None;
  MouseButtonState m_mouseButtons = MouseButtons::None;
  /** Mouse position in units of points, relative to top left of widget */
  float m_mouseX = 0.0f;
  float m_mouseY = 0.0f;
  float m_mouseDX = 0.0f;
  float m_mouseDY = 0.0f;

  ScrollSource m_scrollSource = ScrollSource::Mouse;
  float m_scrollX = 0.0f;
  float m_scrollY = 0.0f;

  bool m_gestureActive = false;
  float m_gesturePanX = 0.0;
  float m_gesturePanY = 0.0;
  float m_gesturePanDX = 0.0;
  float m_gesturePanDY = 0.0;
  float m_gestureZoomValue = 0.0f;
  float m_gestureZoomDValue = 0.0f;
  float m_gestureRotateValue = 0.0f;
  float m_gestureRotateDValue = 0.0f;

  bool m_anyToolDragging = false;
  PickRequest m_pickRequest;
  Model::PickResult m_pickResult;

public:
  InputState();
  InputState(float mouseX, float mouseY);
  virtual ~InputState();

  virtual ModifierKeyState modifierKeys() const;
  bool modifierKeysDown(ModifierKeyState keys) const;
  bool modifierKeysPressed(ModifierKeyState keys) const;
  bool checkModifierKeys(
    ModifierKeyState key1,
    ModifierKeyState key2 = ModifierKeys::DontCare,
    ModifierKeyState key3 = ModifierKeys::DontCare,
    ModifierKeyState key4 = ModifierKeys::DontCare) const;
  bool checkModifierKeys(
    ModifierKeyPressed ctrl, ModifierKeyPressed alt, ModifierKeyPressed shift) const;
  bool checkModifierKey(ModifierKeyPressed state, ModifierKeyState key) const;

  MouseButtonState mouseButtons() const;
  bool mouseButtonsDown(MouseButtonState buttons) const;
  /**
   * Checks whether only the given buttons are down (and no others).
   */
  bool mouseButtonsPressed(MouseButtonState buttons) const;
  float mouseX() const;
  float mouseY() const;
  float mouseDX() const;
  float mouseDY() const;

  ScrollSource scrollSource() const;

  /**
   * Number of "lines" to scroll horizontally.
   */
  float scrollX() const;
  /**
   * Number of "lines" to scroll vertically.
   */
  float scrollY() const;

  bool gestureActive() const;
  float gesturePanX() const;
  float gesturePanY() const;
  float gesturePanDX() const;
  float gesturePanDY() const;
  float gestureZoomValue() const;
  float gestureZoomDValue() const;
  float gestureRotateValue() const;
  float gestureRotateDValue() const;

  void setModifierKeys(ModifierKeyState keys);
  void clearModifierKeys();
  void mouseDown(MouseButtonState button);
  void mouseUp(MouseButtonState button);
  void clearMouseButtons();
  void mouseMove(float mouseX, float mouseY, float mouseDX, float mouseDY);
  void scroll(ScrollSource scrollSource, float scrollX, float scrollY);

  void startGesture();
  void gesturePan(float x, float y, float dx, float dy);
  void gestureZoom(float value, float delta);
  void gestureRotate(float value, float delta);
  void endGesture();

  bool anyToolDragging() const;
  void setAnyToolDragging(bool anyToolDragging);

  const vm::ray3& pickRay() const;
  const vm::vec3 defaultPoint() const;
  const vm::vec3 defaultPointUnderMouse() const;
  const Renderer::Camera& camera() const;
  void setPickRequest(const PickRequest& pickRequest);

  const Model::PickResult& pickResult() const;
  void setPickResult(Model::PickResult pickResult);
};

} // namespace TrenchBroom::View
