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

#ifndef __TrenchBroom__CameraEvent__
#define __TrenchBroom__CameraEvent__

#include "Utility/VecMath.h"

#include <wx/event.h>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Controller {
        class CameraMoveEvent : public wxEvent {
        private:
            float m_forward;
            float m_right;
            float m_up;
        public:
            CameraMoveEvent(float forward, float right, float up) : m_forward(forward), m_right(right), m_up(up) {};

            inline float forward() const {
                return m_forward;
            }
            
            inline float right() const {
                return m_right;
            }
            
            inline float up() const {
                return m_up;
            }
            
            wxEvent* Clone() const {
                return new CameraMoveEvent(m_forward, m_right, m_up);
            }
        };
        
        class CameraLookEvent : public wxEvent {
        private:
            float m_hAngle;
            float m_vAngle;
        public:
            CameraLookEvent(float hAngle, float vAngle) : m_hAngle(hAngle), m_vAngle(vAngle) {}
            
            inline float hAngle() const {
                return m_hAngle;
            }
            
            inline float vAngle() const {
                return m_vAngle;
            }
            
            wxEvent* Clone() const {
                return new CameraLookEvent(m_hAngle, m_vAngle);
            }
        };
        
        class CameraOrbitEvent : public CameraLookEvent {
        private:
            Vec3f m_center;
        public:
            CameraOrbitEvent(float hAngle, float vAngle, const Vec3f& center) : CameraLookEvent(hAngle, vAngle), m_center(center) {}
            
            inline const Vec3f& center() const {
                return m_center;
            }

            wxEvent* Clone() const {
                return new CameraOrbitEvent(hAngle(), vAngle(), m_center);
            }
        };
    }
}

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(EVT_CAMERA_MOVE, -1)
DECLARE_EVENT_TYPE(EVT_CAMERA_LOOK, -1)
DECLARE_EVENT_TYPE(EVT_CAMERA_ORBIT, -1)
END_DECLARE_EVENT_TYPES()

typedef void (wxEvtHandler::*cameraMoveEventFunction)(TrenchBroom::Controller::CameraMoveEvent&);
typedef void (wxEvtHandler::*cameraLookEventFunction)(TrenchBroom::Controller::CameraLookEvent&);
typedef void (wxEvtHandler::*cameraOrbitEventFunction)(TrenchBroom::Controller::CameraOrbitEvent&);

#define EVT_CAMERA_MOVE(id,fn) \
    DECLARE_EVENT_TABLE_ENTRY( EVT_CAMERA_MOVE, id, -1, \
    (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) (wxNotifyEventFunction) \
    wxStaticCastEvent(cameraMoveEventFunction, & fn ), (wxObject *) NULL ),

#define EVT_CAMERA_LOOK(id,fn) \
    DECLARE_EVENT_TABLE_ENTRY( EVT_CAMERA_LOOK, id, -1, \
    (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) (wxNotifyEventFunction) \
    wxStaticCastEvent(cameraLookEventFunction, & fn ), (wxObject *) NULL ),

#define EVT_CAMERA_ORBIT(id,fn) \
    DECLARE_EVENT_TABLE_ENTRY( EVT_CAMERA_ORBIT, id, -1, \
    (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) (wxNotifyEventFunction) \
    wxStaticCastEvent(cameraOrbitEventFunction, & fn ), (wxObject *) NULL ),

#endif /* defined(__TrenchBroom__CameraEvent__) */
