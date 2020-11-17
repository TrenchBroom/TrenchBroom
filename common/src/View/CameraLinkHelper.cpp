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

#include <kdl/set_temp.h>
#include <kdl/vector_utils.h>

#include <vecmath/forward.h>
#include <vecmath/vec.h>

namespace TrenchBroom {
    namespace View {
        CameraLinkHelper::CameraLinkHelper() :
        m_ignoreNotifications(false) {}

        CameraLinkHelper::~CameraLinkHelper() {
            for (Renderer::Camera* camera : m_cameras) {
                camera->cameraDidChangeNotifier.removeObserver(this, &CameraLinkHelper::cameraDidChange);
            }
        }

        void CameraLinkHelper::addCamera(Renderer::Camera* camera) {
            ensure(camera != nullptr, "camera is null");
            assert(!kdl::vec_contains(m_cameras, camera));
            m_cameras.push_back(camera);
            camera->cameraDidChangeNotifier.addObserver(this, &CameraLinkHelper::cameraDidChange);
        }

        void CameraLinkHelper::removeCamera(Renderer::Camera* camera) {
            ensure(camera != nullptr, "camera is null");
            m_cameras = kdl::vec_erase(std::move(m_cameras), camera);
            camera->cameraDidChangeNotifier.removeObserver(this, &CameraLinkHelper::cameraDidChange);
        }

        void CameraLinkHelper::cameraDidChange(const Renderer::Camera* camera) {
            if (!m_ignoreNotifications && pref(Preferences::Link2DCameras)) {
                const kdl::set_temp ignoreNotifications(m_ignoreNotifications);

                for (Renderer::Camera* other : m_cameras) {
                    if (camera != other) {
                        other->setZoom(camera->zoom());

                        const vm::vec3f oldPosition = other->position();
                        const vm::vec3f factors = vm::vec3f::one() - abs(camera->direction()) - abs(other->direction());
                        const vm::vec3f newPosition = (vm::vec3f::one() - factors) * oldPosition + factors * camera->position();
                        other->moveTo(newPosition);
                    }
                }
            }
        }

        CameraLinkableView::~CameraLinkableView() = default;

        void CameraLinkableView::linkCamera(CameraLinkHelper& linkHelper) {
            doLinkCamera(linkHelper);
        }
    }
}
