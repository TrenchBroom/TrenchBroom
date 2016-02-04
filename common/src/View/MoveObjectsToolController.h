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

            void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const;

            bool doShouldStartMove(const InputState& inputState, Vec3& initialPoint) const;
            void doStartMove(const InputState& inputState, const Vec3& initialPoint);
            bool doMove(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint);
            void doEndMove(const InputState& inputState);
            void doCancelMove(const InputState& inputState);
            bool doSnapMove(const InputState& inputState, const Vec3& lastPoint, Vec3& point) const;
            
            DragRestricter* doCreateDefaultDragRestricter(const InputState& inputState, const Vec3& curPoint) const;
            DragRestricter* doCreateVerticalDragRestricter(const InputState& inputState, const Vec3& curPoint) const;
            DragRestricter* doCreateRestrictedDragRestricter(const InputState& inputState, const Vec3& initialPoint, const Vec3& curPoint) const;

            bool doCancel();
        };
        
        class MoveObjectsToolController2D : public MoveObjectsToolController {
        public:
            MoveObjectsToolController2D(MoveObjectsTool* tool);
        };
        
        class MoveObjectsToolController3D : public MoveObjectsToolController {
        public:
            MoveObjectsToolController3D(MoveObjectsTool* tool, MovementRestriction& movementRestriction);
        };
    }
}

#endif /* defined(TrenchBroom_MoveObjectsToolController) */
