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

#ifndef __TrenchBroom__CameraTool__
#define __TrenchBroom__CameraTool__

#include "Controller/Tool.h"
#include "Model/Filter.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::VecMath;

namespace TrenchBroom {
    namespace Controller {
        class CameraTool : public Tool {
        private:
            Vec3f m_orbitCenter;
            bool m_orbit;
            Model::VisibleFilter m_filter;

            float lookSpeed(bool vertical);
            float panSpeed(bool vertical);
            float moveSpeed(bool altMode);
        protected:
            void handleScroll(InputState& inputState);

            bool handleStartDrag(InputState& inputState);
            bool handleDrag(InputState& inputState);
            void handleEndDrag(InputState& inputState);
        public:
            CameraTool(View::DocumentViewHolder& documentViewHolder, InputController& inputController) :
            Tool(documentViewHolder, inputController, false),
            m_orbit(false),
            m_filter(Model::VisibleFilter(documentViewHolder.view().filter())) {}
        };
    }
}

#endif /* defined(__TrenchBroom__CameraTool__) */
