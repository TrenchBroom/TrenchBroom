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

#include "CameraLinkHelper.h"

#include "Macros.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/Camera.h"

#include "kdl/set_temp.h"
#include "kdl/vector_utils.h"

#include "vecmath/forward.h"
#include "vecmath/vec.h"

namespace TrenchBroom::View
{
void CameraLinkHelper::addCamera(Renderer::Camera* camera)
{
  ensure(camera != nullptr, "camera is null");
  assert(!kdl::vec_contains(m_cameras, camera));
  m_cameras.push_back(camera);
  m_notifierConnection +=
    camera->cameraDidChangeNotifier.connect(this, &CameraLinkHelper::cameraDidChange);
}

void CameraLinkHelper::updateCameras(const Renderer::Camera* masterCamera)
{
  for (auto* camera : m_cameras)
  {
    if (camera != masterCamera)
    {
      camera->setZoom(masterCamera->zoom());

      const auto oldPosition = camera->position();
      const auto factors =
        vm::vec3f::one() - abs(masterCamera->direction()) - abs(camera->direction());
      const auto newPosition =
        (vm::vec3f::one() - factors) * oldPosition + factors * masterCamera->position();
      camera->moveTo(newPosition);
    }
  }
}

void CameraLinkHelper::cameraDidChange(const Renderer::Camera* camera)
{
  if (!m_ignoreNotifications && pref(Preferences::Link2DCameras))
  {
    const auto ignoreNotifications = kdl::set_temp{m_ignoreNotifications};
    updateCameras(camera);
  }
}

CameraLinkableView::~CameraLinkableView() = default;
} // namespace TrenchBroom::View
