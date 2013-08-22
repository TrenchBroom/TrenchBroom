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

using namespace TrenchBroom::VecMath;

namespace TrenchBroom {
    namespace Controller {
        class CameraMoveEvent : public wxEvent {
        protected:
            Vec3f m_delta;
        public:
            CameraMoveEvent();
            
            inline float forward() const {
                return m_delta[0];
            }
            
            inline void setForward(float forward) {
                m_delta[0] = forward;
            }
            
            inline float right() const {
                return m_delta[1];
            }
            
            inline void setRight(float right) {
                m_delta[1] = right;
            }
            
            inline float up() const {
                return m_delta[2];
            }
            
            inline void setUp(float up) {
                m_delta[2] = up;
            }
            
            inline const Vec3f& delta() const {
                return m_delta;
            }
            
            inline void setDelta(const Vec3f& delta) {
                m_delta = delta;
            }
            
            virtual wxEvent* Clone() const;

            DECLARE_DYNAMIC_CLASS(CameraMoveEvent)
        };
        
        class CameraLookEvent : public wxEvent {
        protected:
            float m_hAngle;
            float m_vAngle;
        public:
            CameraLookEvent();
            
            inline float hAngle() const {
                return m_hAngle;
            }
            
            inline void setHAngle(float hAngle) {
                m_hAngle = hAngle;
            }
            
            inline float vAngle() const {
                return m_vAngle;
            }
            
            inline void setVAngle(float vAngle) {
                m_vAngle = vAngle;
            }
            
            virtual wxEvent* Clone() const;

            DECLARE_DYNAMIC_CLASS(CameraLookEvent)
        };
        
        class CameraOrbitEvent : public CameraLookEvent {
        protected:
            Vec3f m_center;
        public:
            CameraOrbitEvent();
            
            inline const Vec3f& center() const {
                return m_center;
            }

            inline void setCenter(const Vec3f& center) {
                m_center = center;
            }
            
            virtual wxEvent* Clone() const;
            
            DECLARE_DYNAMIC_CLASS(CameraOrbitEvent)
        };
        
        class CameraSetEvent : public wxEvent {
        protected:
            Vec3f m_position;
            Vec3f m_direction;
            Vec3f m_up;
        public:
            CameraSetEvent();
            
            inline const Vec3f& position() const {
                return m_position;
            }
            
            inline const Vec3f& direction() const {
                return m_direction;
            }
            
            inline const Vec3f& up() const {
                return m_up;
            }

            inline void set(const Vec3f& position, const Vec3f& direction, const Vec3f& up) {
                m_position = position;
                m_direction = direction;
                m_up = up;
            }
            
            virtual wxEvent* Clone() const;
            
            DECLARE_DYNAMIC_CLASS(CameraSetEvent)
        };
    }
}

#define WXDLLIMPEXP_CUSTOM_EVENT

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_CUSTOM_EVENT, EVT_CAMERA_MOVE_EVENT, 1)
DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_CUSTOM_EVENT, EVT_CAMERA_LOOK_EVENT, 1)
DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_CUSTOM_EVENT, EVT_CAMERA_ORBIT_EVENT, 1)
DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_CUSTOM_EVENT, EVT_CAMERA_SET_EVENT, 1)
//DECLARE_EVENT_TYPE(EVT_CAMERA_MOVE_EVENT,     1)
//DECLARE_EVENT_TYPE(EVT_CAMERA_LOOK_EVENT,     1)
//DECLARE_EVENT_TYPE(EVT_CAMERA_ORBIT_EVENT,    1)
END_DECLARE_EVENT_TYPES()


typedef void (wxEvtHandler::*cameraMoveEventFunction)(TrenchBroom::Controller::CameraMoveEvent&);
typedef void (wxEvtHandler::*cameraLookEventFunction)(TrenchBroom::Controller::CameraLookEvent&);
typedef void (wxEvtHandler::*cameraOrbitEventFunction)(TrenchBroom::Controller::CameraOrbitEvent&);
typedef void (wxEvtHandler::*cameraSetEventFunction)(TrenchBroom::Controller::CameraSetEvent&);

#define EVT_CAMERA_MOVE(func) \
    DECLARE_EVENT_TABLE_ENTRY( EVT_CAMERA_MOVE_EVENT, \
        wxID_ANY, \
        wxID_ANY, \
        (wxObjectEventFunction) \
        (cameraMoveEventFunction) & func, \
        (wxObject *) NULL),

#define EVT_CAMERA_LOOK(func) \
    DECLARE_EVENT_TABLE_ENTRY( EVT_CAMERA_LOOK_EVENT, \
        wxID_ANY, \
        wxID_ANY, \
        (wxObjectEventFunction) \
        (cameraLookEventFunction) & func, \
        (wxObject *) NULL ),

#define EVT_CAMERA_ORBIT(func) \
    DECLARE_EVENT_TABLE_ENTRY( EVT_CAMERA_ORBIT_EVENT, \
        wxID_ANY, \
        wxID_ANY, \
        (wxObjectEventFunction) \
        (cameraOrbitEventFunction) & func, \
        (wxObject *) NULL ),


#define EVT_CAMERA_SET(func) \
    DECLARE_EVENT_TABLE_ENTRY( EVT_CAMERA_SET_EVENT, \
        wxID_ANY, \
        wxID_ANY, \
        (wxObjectEventFunction) \
        (cameraSetEventFunction) & func, \
        (wxObject *) NULL ),

#endif /* defined(__TrenchBroom__CameraEvent__) */
