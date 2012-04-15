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
            bool m_invert;
            float m_lookSensitivity;
            float m_panSensitivity;
            float m_moveSensitivity;
        public:
            CameraTool(Editor& editor) : Tool(editor), m_orbit(false), m_invert(false), m_lookSensitivity(1 / 90.0f), m_panSensitivity(1.0f), m_moveSensitivity(6.0f) {}

            bool scrolled(ToolEvent& event);
            bool beginLeftDrag(ToolEvent& event);
            void leftDrag(ToolEvent& event);
            void endLeftDrag(ToolEvent& event);
            bool beginRightDrag(ToolEvent& event);
            void rightDrag(ToolEvent& event);

            static bool cameraModiferPressed(ToolEvent& event);
            static bool orbitModifierPressed(ToolEvent& event);
        };
    }
}

#endif
