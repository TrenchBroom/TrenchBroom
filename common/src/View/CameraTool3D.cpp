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

#include "CameraTool3D.h"

#include "Model/Hit.h"
#include "Model/HitFilter.h"
#include "Model/ModelUtils.h"
#include "Model/PickResult.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/PerspectiveCamera.h"
#include "View/GestureTracker.h"
#include "View/InputState.h"

#include "vm/forward.h"
#include "vm/intersection.h"
#include "vm/plane.h"
#include "vm/scalar.h"
#include "vm/vec.h"

namespace TrenchBroom::View
{
static bool shouldMove(const InputState& inputState)
{
  return (
    inputState.mouseButtonsPressed(MouseButtons::None)
    && inputState.checkModifierKeys(
      ModifierKeyPressed::No, ModifierKeyPressed::No, ModifierKeyPressed::DontCare));
}

static bool shouldLook(const InputState& inputState)
{
  return (
    inputState.mouseButtonsPressed(MouseButtons::Right)
    && inputState.modifierKeysPressed(ModifierKeys::None));
}

static bool shouldPan(const InputState& inputState)
{
  return (
    inputState.mouseButtonsPressed(MouseButtons::Middle)
    && (inputState.modifierKeysPressed(ModifierKeys::None) || inputState.modifierKeysPressed(ModifierKeys::MKAlt)));
}

static bool shouldOrbit(const InputState& inputState)
{
  return (
    inputState.mouseButtonsPressed(MouseButtons::Right)
    && inputState.modifierKeysPressed(ModifierKeys::MKAlt));
}

static bool shouldAdjustFlySpeed(const InputState& inputState)
{
  return (
    inputState.mouseButtonsPressed(MouseButtons::Right)
    && inputState.checkModifierKeys(
      ModifierKeyPressed::No, ModifierKeyPressed::No, ModifierKeyPressed::No));
}

static float adjustSpeedToZoom(
  const Renderer::PerspectiveCamera& camera, const float speed)
{
  return speed * vm::min(1.0f, camera.zoomedFov() / camera.fov());
}

static float lookSpeedH(const Renderer::PerspectiveCamera& camera)
{
  float speed = pref(Preferences::CameraLookSpeed) / -50.0f;
  if (pref(Preferences::CameraLookInvertH))
  {
    speed *= -1.0f;
  }
  return adjustSpeedToZoom(camera, speed);
}

static float lookSpeedV(const Renderer::PerspectiveCamera& camera)
{
  float speed = pref(Preferences::CameraLookSpeed) / -50.0f;
  if (pref(Preferences::CameraLookInvertV))
  {
    speed *= -1.0f;
  }
  return adjustSpeedToZoom(camera, speed);
}

static float panSpeedH(const Renderer::PerspectiveCamera& camera)
{
  float speed = pref(Preferences::CameraPanSpeed);
  if (pref(Preferences::CameraPanInvertH))
  {
    speed *= -1.0f;
  }
  return adjustSpeedToZoom(camera, speed);
}

static float panSpeedV(const Renderer::PerspectiveCamera& camera)
{
  float speed = pref(Preferences::CameraPanSpeed);
  if (pref(Preferences::CameraPanInvertV))
  {
    speed *= -1.0f;
  }
  return adjustSpeedToZoom(camera, speed);
}

static float moveSpeed(const Renderer::PerspectiveCamera& camera, const bool altMode)
{
  float speed = pref(Preferences::CameraMoveSpeed) * 20.0f;
  if (altMode && pref(Preferences::CameraAltMoveInvert))
  {
    speed *= -1.0f;
  }
  return adjustSpeedToZoom(camera, speed);
}

CameraTool3D::CameraTool3D(Renderer::PerspectiveCamera& camera)
  : ToolController{}
  , Tool{true}
  , m_camera{camera}
{
}

Tool& CameraTool3D::tool()
{
  return *this;
}

const Tool& CameraTool3D::tool() const
{
  return *this;
}

void CameraTool3D::mouseScroll(const InputState& inputState)
{
  const float factor = pref(Preferences::CameraMouseWheelInvert) ? -1.0f : 1.0f;
  const bool zoom = inputState.modifierKeysPressed(ModifierKeys::Shift);
  const float scrollDist =
#ifdef __APPLE__
    inputState.modifierKeysPressed(ModifierKeys::Shift) ? inputState.scrollX()
                                                        : inputState.scrollY();
#else
    inputState.scrollY();
#endif

  if (shouldMove(inputState))
  {
    if (zoom)
    {
      const float zoomFactor = 1.0f + scrollDist / 50.0f * factor;
      m_camera.zoom(zoomFactor);
    }
    else
    {
      const auto moveDirection = pref(Preferences::CameraMoveInCursorDir)
                                   ? vm::vec3f{inputState.pickRay().direction}
                                   : m_camera.direction();
      const float distance = scrollDist * moveSpeed(m_camera, false);
      m_camera.moveBy(factor * distance * moveDirection);
    }
  }
}

void CameraTool3D::mouseUp(const InputState& inputState)
{
  if (inputState.mouseButtonsPressed(MouseButtons::Right))
  {
    auto& prefs = PreferenceManager::instance();
    if (!prefs.saveInstantly())
    {
      prefs.saveChanges();
    }
  }
}

namespace
{
class OrbitDragTracker : public GestureTracker
{
private:
  Renderer::PerspectiveCamera& m_camera;
  vm::vec3f m_orbitCenter;

public:
  OrbitDragTracker(Renderer::PerspectiveCamera& camera, const vm::vec3f& orbitCenter)
    : m_camera{camera}
    , m_orbitCenter{orbitCenter}
  {
  }

  void mouseScroll(const InputState& inputState) override
  {
    const float factor = pref(Preferences::CameraMouseWheelInvert) ? -1.0f : 1.0f;
    const float scrollDist = inputState.scrollY();

    const auto orbitPlane = vm::plane3f{m_orbitCenter, m_camera.direction()};
    if (const auto hit = vm::intersect_ray_plane(m_camera.viewRay(), orbitPlane))
    {
      const float maxDistance = vm::max(*hit - 32.0f, 0.0f);
      const float distance =
        vm::min(factor * scrollDist * moveSpeed(m_camera, false), maxDistance);

      m_camera.moveBy(distance * m_camera.direction());
    }
  }

  bool update(const InputState& inputState) override
  {
    const float hAngle = static_cast<float>(inputState.mouseDX()) * lookSpeedH(m_camera);
    const float vAngle = static_cast<float>(inputState.mouseDY()) * lookSpeedV(m_camera);
    m_camera.orbit(m_orbitCenter, hAngle, vAngle);
    return true;
  }

  void end(const InputState&) override {}
  void cancel() override {}
};

class LookDragTracker : public GestureTracker
{
private:
  Renderer::PerspectiveCamera& m_camera;

public:
  explicit LookDragTracker(Renderer::PerspectiveCamera& camera)
    : m_camera{camera}
  {
  }

  void mouseScroll(const InputState& inputState) override
  {
    if (shouldAdjustFlySpeed(inputState))
    {
      const float factor = pref(Preferences::CameraMouseWheelInvert) ? -1.0f : 1.0f;
      const float scrollDist = inputState.scrollY();

      const float speed = pref(Preferences::CameraFlyMoveSpeed);
      // adjust speed by 5% of the current speed per scroll line
      const float deltaSpeed = factor * speed * 0.05f * scrollDist;
      const float newSpeed = vm::clamp(
        speed + deltaSpeed,
        Preferences::MinCameraFlyMoveSpeed,
        Preferences::MaxCameraFlyMoveSpeed);

      // prefs are only changed when releasing RMB
      auto& prefs = PreferenceManager::instance();
      prefs.set(Preferences::CameraFlyMoveSpeed, newSpeed);
    }
  }

  bool update(const InputState& inputState) override
  {
    const float hAngle = static_cast<float>(inputState.mouseDX()) * lookSpeedH(m_camera);
    const float vAngle = static_cast<float>(inputState.mouseDY()) * lookSpeedV(m_camera);
    m_camera.rotate(hAngle, vAngle);
    return true;
  }

  void end(const InputState&) override {}
  void cancel() override {}
};

class PanDragTracker : public GestureTracker
{
private:
  Renderer::PerspectiveCamera& m_camera;

public:
  explicit PanDragTracker(Renderer::PerspectiveCamera& camera)
    : m_camera{camera}
  {
  }

  bool update(const InputState& inputState) override
  {
    const bool altMove = pref(Preferences::CameraEnableAltMove);
    auto delta = vm::vec3f{};
    if (altMove && inputState.modifierKeysPressed(ModifierKeys::MKAlt))
    {
      delta = delta
              + static_cast<float>(inputState.mouseDX()) * panSpeedH(m_camera)
                  * m_camera.right();
      delta = delta
              + static_cast<float>(inputState.mouseDY()) * -moveSpeed(m_camera, altMove)
                  * m_camera.direction();
    }
    else
    {
      delta = delta
              + static_cast<float>(inputState.mouseDX()) * panSpeedH(m_camera)
                  * m_camera.right();
      delta =
        delta
        + static_cast<float>(inputState.mouseDY()) * panSpeedV(m_camera) * m_camera.up();
    }
    m_camera.moveBy(delta);
    return true;
  }

  void end(const InputState&) override {}
  void cancel() override {}
};

class OrbitGestureTracker : public GestureTracker
{
private:
  Renderer::PerspectiveCamera& m_camera;
  vm::vec3f m_orbitCenter;

public:
  OrbitGestureTracker(Renderer::PerspectiveCamera& camera, const vm::vec3f& orbitCenter)
    : m_camera{camera}
    , m_orbitCenter{orbitCenter}
  {
  }

  bool update(const InputState& inputState) override
  {
    const auto zoomValue = inputState.gestureZoomValue();
    if (zoomValue != 0.0f)
    {
      if (inputState.modifierKeysPressed(ModifierKeys::Shift))
      {
        m_camera.zoom(1.0f - zoomValue);
      }
      else
      {
        const auto moveDirection = pref(Preferences::CameraMoveInCursorDir)
                                     ? vm::vec3f{inputState.pickRay().direction}
                                     : m_camera.direction();
        const auto distance = 50.0f * zoomValue * moveSpeed(m_camera, false);
        m_camera.moveBy(distance * moveDirection);
      }
    }

    const auto rotateValue = inputState.gestureRotateValue();
    if (rotateValue != 0.0f)
    {
      const float hAngle = vm::to_radians(rotateValue);
      m_camera.orbit(m_orbitCenter, hAngle, 0.0);
    }
    return true;
  }

  void end(const InputState&) override {}
  void cancel() override {}
};

} // namespace

std::unique_ptr<GestureTracker> CameraTool3D::acceptMouseDrag(
  const InputState& inputState)
{
  using namespace Model::HitFilters;

  if (shouldOrbit(inputState))
  {
    const auto& hit =
      inputState.pickResult().first(type(Model::nodeHitType()) && minDistance(3.0));
    const auto orbitCenter = vm::vec3f{
      hit.isMatch() ? hit.hitPoint() : m_camera.defaultPoint(inputState.pickRay())};
    return std::make_unique<OrbitDragTracker>(m_camera, orbitCenter);
  }

  if (shouldLook(inputState))
  {
    return std::make_unique<LookDragTracker>(m_camera);
  }

  if (shouldPan(inputState))
  {
    return std::make_unique<PanDragTracker>(m_camera);
  }

  return nullptr;
}

std::unique_ptr<GestureTracker> CameraTool3D::acceptGesture(const InputState& inputState)
{
  using namespace Model::HitFilters;

  const auto& hit =
    inputState.pickResult().first(type(Model::nodeHitType()) && minDistance(3.0));
  const auto orbitCenter = vm::vec3f{
    hit.isMatch() ? hit.hitPoint() : m_camera.defaultPoint(inputState.pickRay())};
  return std::make_unique<OrbitGestureTracker>(m_camera, orbitCenter);
}

bool CameraTool3D::cancel()
{
  return false;
}
} // namespace TrenchBroom::View
