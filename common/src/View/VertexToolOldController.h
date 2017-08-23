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

#ifndef TrenchBroom_VertexToolOldController
#define TrenchBroom_VertexToolOldController

#include "Model/Hit.h"
#include "View/MoveToolController.h"
#include "View/ToolController.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace View {
        class Lasso;
        class InputState;
        class MovementRestriction;
        class VertexToolOld;
        
        class VertexToolOldController : public ToolControllerGroup {
        private:
            static const FloatType MaxVertexDistance;
            class VertexPartBase;
            class SelectVertexPart;
            class MoveVertexPart;
            class SnapVertexPart;
        protected:
            VertexToolOld* m_tool;
        public:
            VertexToolOldController(VertexToolOld* tool);
            ~VertexToolOldController();
        private:
            Tool* doGetTool();
        };
    }
}

#endif /* defined(TrenchBroom_VertexToolOldController) */
