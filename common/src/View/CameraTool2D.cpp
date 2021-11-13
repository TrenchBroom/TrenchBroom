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

#include "CameraTool2D.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/OrthographicCamera.h"
#include "View/DragTracker.h"
#include "View/InputState.h"

#include <vecmath/forward.h>
#include <vecmath/vec.h>

namespace TrenchBroom {
namespace View {
CameraTool2D::CameraTool2D(Renderer::OrthographicCamera& camera)
  : ToolController{}
  , Tool{true}
  , m_camera{camera} {}

Tool& CameraTool2D::tool() {
  return *this;
}

const Tool& CameraTool2D::tool() const {
  return *this;
}

static bool shouldZoom(const InputState& inputState) {
  return (
    inputState.mouseButtonsPressed(MouseButtons::MBNone) &&
    inputState.modifierKeysPressed(ModifierKeys::MKNone));
}

static void zoom(
  Renderer::OrthographicCamera& camera, const vm::vec2f& mousePos, const float factor) {
  const auto oldWorldPos = camera.unproject(mousePos.x(), mousePos.y(), 0.0f);

  camera.zoom(factor);

  const auto newWorldPos = camera.unproject(mousePos.x(), mousePos.y(), 0.0f);
  const auto delta = newWorldPos - oldWorldPos;
  camera.moveBy(-delta);
}

void CameraTool2D::mouseScroll(const InputState& inputState) {
  if (shouldZoom(inputState)) {
    if (inputState.scrollY() != 0.0f) {
      const float speed = pref(Preferences::CameraMouseWheelInvert) ? -1.0f : 1.0f;
      const float factor = 1.0f + inputState.scrollY() / 50.0f * speed;
      const auto mousePos = vm::vec2f{inputState.mouseX(), inputState.mouseY()};

      if (factor > 0.0f) {
        zoom(m_camera, mousePos, factor);
      }
    }
  }
}

namespace {
class PanDragTracker : public DragTracker {
private:
  Renderer::OrthographicCamera& m_camera;
  vm::vec2f m_lastMousePos;

public:
  PanDragTracker(Renderer::OrthographicCamera& camera, const vm::vec2f& lastMousePos)
    : m_camera{camera}
    , m_lastMousePos{lastMousePos} {}

  bool drag(const InputState& inputState) override {
    const auto currentMousePos = vm::vec2f{inputState.mouseX(), inputState.mouseY()};
    const auto lastWorldPos = m_camera.unproject(m_lastMousePos.x(), m_lastMousePos.y(), 0.0f);
    const auto currentWorldPos = m_camera.unproject(currentMousePos.x(), currentMousePos.y(), 0.0f);
    const auto delta = currentWorldPos - lastWorldPos;
    m_camera.moveBy(-delta);
    m_lastMousePos = currentMousePos;
    return true;
  }

  void end(const InputState&) override {}
  void cancel() override {}
};

class ZoomDragTracker : public DragTracker {
private:
  Renderer::OrthographicCamera& m_camera;
  vm::vec2f m_lastMousePos;

public:
  ZoomDragTracker(Renderer::OrthographicCamera& camera, const vm::vec2f& lastMousePos)
    : m_camera{camera}
    , m_lastMousePos{lastMousePos} {}

  bool drag(const InputState& inputState) override {
    const auto speed = pref(Preferences::CameraAltMoveInvert) ? 1.0f : -1.0f;
    const auto factor = 1.0f + static_cast<float>(inputState.mouseDY()) / 100.0f * speed;
    zoom(m_camera, m_lastMousePos, factor);
    return true;
  }

  void end(const InputState&) override {}
  void cancel() override {}
};
} // namespace

static bool shouldPan(const InputState& inputState) {
  return (
    inputState.mouseButtonsPressed(MouseButtons::MBRight) ||
    (inputState.mouseButtonsPressed(MouseButtons::MBMiddle) &&
     !pref(Preferences::CameraEnableAltMove)));
}

static bool shouldDragZoom(const InputState& inputState) {
  return (
    pref(Preferences::CameraEnableAltMove) &&
    inputState.mouseButtonsPressed(MouseButtons::MBMiddle) &&
    inputState.modifierKeysPressed(ModifierKeys::MKAlt));
}

std::unique_ptr<DragTracker> CameraTool2D::acceptMouseDrag(const InputState& inputState) {
  if (shouldPan(inputState)) {
    const auto lastMousePos = vm::vec2f{inputState.mouseX(), inputState.mouseY()};
    return std::make_unique<PanDragTracker>(m_camera, lastMousePos);
  }

  if (shouldDragZoom(inputState)) {
    const auto lastMousePos = vm::vec2f{inputState.mouseX(), inputState.mouseY()};
    return std::make_unique<ZoomDragTracker>(m_camera, lastMousePos);
  }

  return nullptr;
}

bool CameraTool2D::cancel() {
  return false;
}
} // namespace View
} // namespace TrenchBroom
