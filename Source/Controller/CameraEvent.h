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

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(EVT_CAMERA_MOVE,     7777)
DECLARE_EVENT_TYPE(EVT_CAMERA_LOOK,     7777)
DECLARE_EVENT_TYPE(EVT_CAMERA_ORBIT,    7777)
END_DECLARE_EVENT_TYPES()

namespace TrenchBroom {
    namespace Controller {
        class CameraMoveEvent : public wxEvent {
        private:
            float m_forward;
            float m_right;
            float m_up;
        public:
            CameraMoveEvent();
            
            inline float forward() const {
                return m_forward;
            }
            
            inline void setForward(float forward) {
                m_forward = forward;
            }
            
            inline float right() const {
                return m_right;
            }
            
            inline void setRight(float right) {
                m_right = right;
            }
            
            inline float up() const {
                return m_up;
            }
            
            inline void setUp(float up) {
                m_up = up;
            }
            
            virtual wxEvent* Clone() const;

            DECLARE_DYNAMIC_CLASS(CameraMoveEvent)
        };
        
        class CameraLookEvent : public wxEvent {
        private:
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
        private:
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
    }
}

typedef void (wxEvtHandler::*cameraMoveEventFunction)(TrenchBroom::Controller::CameraMoveEvent&);
typedef void (wxEvtHandler::*cameraLookEventFunction)(TrenchBroom::Controller::CameraLookEvent&);
typedef void (wxEvtHandler::*cameraOrbitEventFunction)(TrenchBroom::Controller::CameraOrbitEvent&);

#define EVT_CAMERA_MOVE(fn) \
DECLARE_EVENT_TABLE_ENTRY( EVT_CAMERA_MOVE, wxID_ANY, wxID_ANY, \
(wxObjectEventFunction) (wxEventFunction) \
wxStaticCastEvent(cameraMoveEventFunction, & fn ), (wxObject *) NULL ),

#define EVT_CAMERA_LOOK(fn) \
DECLARE_EVENT_TABLE_ENTRY( EVT_CAMERA_LOOK, wxID_ANY, wxID_ANY, \
(wxObjectEventFunction) (wxEventFunction) \
wxStaticCastEvent(cameraLookEventFunction, & fn ), (wxObject *) NULL ),

#define EVT_CAMERA_ORBIT(fn) \
DECLARE_EVENT_TABLE_ENTRY( EVT_CAMERA_ORBIT, wxID_ANY, wxID_ANY, \
(wxObjectEventFunction) (wxEventFunction) \
wxStaticCastEvent(cameraOrbitEventFunction, & fn ), (wxObject *) NULL ),

#endif /* defined(__TrenchBroom__CameraEvent__) */
