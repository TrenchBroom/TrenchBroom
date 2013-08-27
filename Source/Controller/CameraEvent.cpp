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


#include "CameraEvent.h"

DEFINE_EVENT_TYPE(EVT_CAMERA_MOVE_EVENT)
DEFINE_EVENT_TYPE(EVT_CAMERA_LOOK_EVENT)
DEFINE_EVENT_TYPE(EVT_CAMERA_ORBIT_EVENT)
DEFINE_EVENT_TYPE(EVT_CAMERA_SET_EVENT)

namespace TrenchBroom {
    namespace Controller {
        IMPLEMENT_DYNAMIC_CLASS(CameraMoveEvent, wxEvent)
        CameraMoveEvent::CameraMoveEvent() :
        wxEvent(wxID_ANY, EVT_CAMERA_MOVE_EVENT) {}
        
        wxEvent* CameraMoveEvent::Clone() const {
            return new CameraMoveEvent(*this);
        }
        
        IMPLEMENT_DYNAMIC_CLASS(CameraLookEvent, wxEvent)
        CameraLookEvent::CameraLookEvent() :
        wxEvent(wxID_ANY, EVT_CAMERA_LOOK_EVENT),
        m_hAngle(0.0f),
        m_vAngle(0.0f) {}

        wxEvent* CameraLookEvent::Clone() const {
            return new CameraLookEvent(*this);
        }

        IMPLEMENT_DYNAMIC_CLASS(CameraOrbitEvent, CameraLookEvent)
        CameraOrbitEvent::CameraOrbitEvent() :
        CameraLookEvent(),
        m_center(Vec3f::Null) {
            SetEventType(EVT_CAMERA_ORBIT_EVENT);
        }

        wxEvent* CameraOrbitEvent::Clone() const {
            return new CameraOrbitEvent(*this);
        }

        IMPLEMENT_DYNAMIC_CLASS(CameraSetEvent, wxEvent)
        CameraSetEvent::CameraSetEvent() :
        wxEvent(wxID_ANY, EVT_CAMERA_SET_EVENT) {
            SetEventType(EVT_CAMERA_SET_EVENT);
        }
        
        wxEvent* CameraSetEvent::Clone() const {
            return new CameraSetEvent(*this);
        }
    }
}
