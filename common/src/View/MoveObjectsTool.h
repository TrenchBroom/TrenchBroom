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

#ifndef __TrenchBroom__MoveObjectsTool__
#define __TrenchBroom__MoveObjectsTool__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "View/MapViewToolPage.h"
#include "View/MoveToolAdapter.h"
#include "View/Tool.h"

namespace TrenchBroom {
    class Hit;
    
    namespace View {
        class InputState;
        class MovementRestriction;
        
        class MoveObjectsTool : public MoveToolAdapter<NoPickingPolicy, NoMousePolicy, RenderPolicy>, public Tool, public MapViewToolPage {
        private:
            MapDocumentWPtr m_document;
            bool m_duplicateObjects;
        public:
            MoveObjectsTool(MapDocumentWPtr document, MovementRestriction& movementRestriction);
        private:
            Tool* doGetTool();
            
            bool doHandleMove(const InputState& inputState) const;
            Vec3 doGetMoveOrigin(const InputState& inputState) const;
            const Hit& findHit(const InputState& inputState) const;
            
            String doGetActionName(const InputState& inputState) const;
            bool doStartMove(const InputState& inputState);
            Vec3 doSnapDelta(const InputState& inputState, const Vec3& delta) const;
            MoveResult doMove(const InputState& inputState, const Vec3& delta);
            void doEndMove(const InputState& inputState);
            void doCancelMove();
            
            void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const;
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);

            bool duplicateObjects(const InputState& inputState) const;
            
            bool doCancel();
            
            wxWindow* doCreatePage(wxWindow* parent);
        };
    }
}

#endif /* defined(__TrenchBroom__MoveObjectsTool__) */
