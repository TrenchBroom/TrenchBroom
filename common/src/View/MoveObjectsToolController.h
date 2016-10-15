/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#ifndef TrenchBroom_MoveObjectsToolController
#define TrenchBroom_MoveObjectsToolController

#include "View/MoveToolController.h"

namespace TrenchBroom {
    namespace View {
        class MoveObjectsTool;
        
        class MoveObjectsToolController : public MoveToolController<NoPickingPolicy, NoMousePolicy> {
        private:
            MoveObjectsTool* m_tool;
        public:
            MoveObjectsToolController(MoveObjectsTool* tool);
            virtual ~MoveObjectsToolController();
        private:
            Tool* doGetTool();

            MoveInfo doStartMove(const InputState& inputState);
            DragResult doMove(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint);
            void doEndMove(const InputState& inputState);
            void doCancelMove();
            
            void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const;
            
            bool doCancel();
        };
    }
}

#endif /* defined(TrenchBroom_MoveObjectsToolController) */
