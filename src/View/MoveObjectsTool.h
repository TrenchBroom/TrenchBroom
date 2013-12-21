/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#ifndef __TrenchBroom__MoveObjectsTool__
#define __TrenchBroom__MoveObjectsTool__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "View/MoveTool.h"
#include "View/Tool.h"

namespace TrenchBroom {
    namespace View {
        class InputState;
        class MovementRestriction;
        
        class MoveObjectsTool : public MoveTool<NoActivationPolicy, NoPickingPolicy, NoMousePolicy, RenderPolicy> {
        private:
            bool m_duplicateObjects;
        public:
            MoveObjectsTool(BaseTool* next, MapDocumentWPtr document, ControllerWPtr controller, MovementRestriction& movementRestriction);
        private:
            bool doHandleMove(const InputState& inputState) const;
            Vec3 doGetMoveOrigin(const InputState& inputState) const;
            String doGetActionName(const InputState& inputState) const;
            bool doStartMove(const InputState& inputState);
            Vec3 doSnapDelta(const InputState& inputState, const Vec3& delta) const;
            MoveResult doMove(const Vec3& delta);
            void doEndMove(const InputState& inputState);
            
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext);

            bool duplicateObjects(const InputState& inputState) const;
        };
    }
}

#endif /* defined(__TrenchBroom__MoveObjectsTool__) */
