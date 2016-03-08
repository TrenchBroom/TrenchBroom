/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#ifndef TrenchBroom_FlyModeHelper
#define TrenchBroom_FlyModeHelper

#include "VecMath.h"

#include <iostream>

#include <wx/thread.h>
#include <wx/wx.h>

namespace TrenchBroom {
    namespace Renderer {
        class Camera;
    }
    
    namespace View {
        class FlyModeHelper : public wxThread {
        private:
            wxWindow* m_window;
            Renderer::Camera& m_camera;
            
            wxCriticalSection m_critical;
            bool m_forward;
            bool m_backward;
            bool m_left;
            bool m_right;
            
            bool m_enabled;
            
            wxPoint m_originalMousePos;
            wxPoint m_lastMousePos;
            wxPoint m_currentMouseDelta;
            bool m_ignoreMotionEvents;
            
            wxLongLong m_lastPollTime;
            
            class CameraEvent;
        public:
            FlyModeHelper(wxWindow* window, Renderer::Camera& camera);
            ~FlyModeHelper();
            
            void enable();
            void disable();
            bool enabled() const;
            bool cancel();
        private:
            void lockMouse();
            void unlockMouse();
        public:
            bool keyDown(wxKeyEvent& event);
            bool keyUp(wxKeyEvent& event);
        public:
            void motion(wxMouseEvent& event);
        private:
            void resetMouse();
            wxPoint windowCenter() const;
        private:
            ExitCode Entry();
            Vec3f moveDelta();
            Vec2f lookDelta();
            Vec2f lookSpeed() const;
            float moveSpeed() const;
        };
    }
}

#endif /* defined(TrenchBroom_FlyModeHelper) */
