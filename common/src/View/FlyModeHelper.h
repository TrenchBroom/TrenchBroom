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

#ifndef TrenchBroom_FlyModeHelper
#define TrenchBroom_FlyModeHelper

#include <vecmath/forward.h>
#include <vecmath/vec.h>

#include <iostream>

#include <wx/gdicmn.h>
#include <wx/longlong.h>

class wxKeyEvent;

namespace TrenchBroom {
    namespace Renderer {
        class Camera;
    }
    namespace IO {
        class Path;
    }
    
    namespace View {
        class FlyModeHelper {
        private:
            Renderer::Camera& m_camera;
            
            bool m_forward;
            bool m_backward;
            bool m_left;
            bool m_right;
            bool m_up;
            bool m_down;
            
            wxLongLong m_lastPollTime;
        public:
            FlyModeHelper(Renderer::Camera& camera);
            ~FlyModeHelper();
            
            void pollAndUpdate();
        public:
            bool keyDown(wxKeyEvent& event);
            bool keyUp(wxKeyEvent& event);
            bool anyKeyDown() const;
            void resetKeys();
        private:
            vm::vec3f moveDelta(float time);
            float moveSpeed() const;
        };
    }
}

#endif /* defined(TrenchBroom_FlyModeHelper) */
