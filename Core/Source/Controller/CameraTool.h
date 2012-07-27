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

#ifndef TrenchBroom_CameraTool_h
#define TrenchBroom_CameraTool_h

#include "Controller/Tool.h"
#include "Utilities/VecMath.h"

namespace TrenchBroom {
    namespace Controller {
        class Camera;
    }
    
    namespace Controller {
        class CameraTool : public Tool {
        private:
            Vec3f m_orbitCenter;
            bool m_orbit;

            float lookSpeed(bool vertical);
            float panSpeed(bool vertical);
            float moveSpeed();
        public:
            CameraTool(Editor& editor) : Tool(editor), m_orbit(false) {}

            bool handleScrolled(InputEvent& event);
            bool handleBeginDrag(InputEvent& event);
            bool handleDrag(InputEvent& event);
            void handleEndDrag(InputEvent& event);
            
            static bool cameraModiferPressed(InputEvent& event);
            static bool orbitModifierPressed(InputEvent& event);
        };
    }
}

#endif
