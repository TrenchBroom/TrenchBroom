/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "CameraAnimation.h"

#include "Controller/CameraEvent.h"
#include "Renderer/Camera.h"
#include "View/EditorView.h"

namespace TrenchBroom {
    namespace View {
        void CameraAnimation::doUpdate(double progress) {
            float fltProgress = static_cast<float>(progress);
            const Vec3f position = m_startPosition + (m_targetPosition - m_startPosition) * fltProgress;
            const Vec3f direction = m_startDirection + (m_targetDirection - m_startDirection) * fltProgress;
            const Vec3f up = m_startUp + (m_targetUp - m_startUp) * fltProgress;
            
            Controller::CameraSetEvent cameraEvent;
            cameraEvent.set(position, direction, up);
            m_view.ProcessEvent(cameraEvent);
        }

        CameraAnimation::CameraAnimation(View::EditorView& view, const Vec3f& targetPosition, const Vec3f& targetDirection, const Vec3f& targetUp, wxLongLong duration) :
        Animation(EaseInEaseOutCurve, duration),
        m_view(view),
        m_startPosition(m_view.camera().position()),
        m_startDirection(m_view.camera().direction()),
        m_startUp(m_view.camera().up()),
        m_targetPosition(targetPosition),
        m_targetDirection(targetDirection),
        m_targetUp(targetUp) {}
    }
}
