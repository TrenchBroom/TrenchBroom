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

#pragma once

#include "NotifierConnection.h"

#include <vector>

namespace tb
{
namespace gl
{
class Camera;
}

namespace ui
{

class CameraLinkHelper
{
private:
  std::vector<gl::Camera*> m_cameras;
  bool m_ignoreNotifications = false;
  NotifierConnection m_notifierConnection;

public:
  void addCamera(gl::Camera* camera);
  void updateCameras(const gl::Camera& masterCamera);

private:
  void cameraDidChange(const gl::Camera& camera);
};

class CameraLinkableView
{
public:
  virtual ~CameraLinkableView();
  virtual void linkCamera(CameraLinkHelper& linkHelper) = 0;
};

} // namespace ui
} // namespace tb
