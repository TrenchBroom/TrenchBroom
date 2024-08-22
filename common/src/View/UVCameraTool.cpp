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

#include "UVCameraTool.h"

#include "Renderer/OrthographicCamera.h"
#include "View/DragTracker.h"
#include "View/InputState.h"

#include "vm/vec.h"

namespace TrenchBroom
{
namespace View
{
UVCameraTool::UVCameraTool(Renderer::OrthographicCamera& camera)
  : ToolController{}
  , Tool{true}
  , m_camera{camera}
{
}

Tool& UVCameraTool::tool()
{
  return *this;
}

const Tool& UVCameraTool::tool() const
{
  return *this;
}

void UVCameraTool::mouseScroll(const InputState& inputState)
{
  const auto oldWorldPos =
    m_camera.unproject(float(inputState.mouseX()), float(inputState.mouseY()), 0.0f);

  // NOTE: some events will have scrollY() == 0, and have horizontal scorlling. We only
  // care about scrollY().

  if (inputState.scrollY() > 0)
  {
    if (m_camera.zoom() < 10.0f)
    {
      m_camera.zoom(1.1f);
    }
  }

  if (inputState.scrollY() < 0)
  {
    if (m_camera.zoom() > 0.1f)
    {
      m_camera.zoom(1.0f / 1.1f);
    }
  }

  const auto newWorldPos =
    m_camera.unproject(float(inputState.mouseX()), float(inputState.mouseY()), 0.0f);
  const auto delta = oldWorldPos - newWorldPos;
  m_camera.moveBy(delta);
}

namespace
{
class UVCameraToolDragTracker : public DragTracker
{
private:
  Renderer::Camera& m_camera;

public:
  UVCameraToolDragTracker(Renderer::Camera& camera)
    : m_camera{camera}
  {
  }

  bool drag(const InputState& inputState)
  {
    const auto oldX = inputState.mouseX() - inputState.mouseDX();
    const auto oldY = inputState.mouseY() - inputState.mouseDY();

    const auto oldWorldPos = m_camera.unproject(float(oldX), float(oldY), 0.0f);
    const auto newWorldPos =
      m_camera.unproject(float(inputState.mouseX()), float(inputState.mouseY()), 0.0f);
    const auto delta = oldWorldPos - newWorldPos;
    m_camera.moveBy(delta);
    return true;
  }

  void end(const InputState&) {}
  void cancel() {}
};
} // namespace

std::unique_ptr<DragTracker> UVCameraTool::acceptMouseDrag(const InputState& inputState)
{
  if (
    !inputState.mouseButtonsPressed(MouseButtons::MBRight)
    && !inputState.mouseButtonsPressed(MouseButtons::MBMiddle))
  {
    return nullptr;
  }

  return std::make_unique<UVCameraToolDragTracker>(m_camera);
}

bool UVCameraTool::cancel()
{
  return false;
}
} // namespace View
} // namespace TrenchBroom
