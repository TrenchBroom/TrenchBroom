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

#ifndef __TrenchBroom__UVRotateTool__
#define __TrenchBroom__UVRotateTool__

#include "Hit.h"
#include "View/Tool.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Renderer {
        class RenderBatch;
        class RenderContext;
    }

    namespace View {
        class UVViewHelper;

        class UVRotateTool : public ToolImpl<NoActivationPolicy, PickingPolicy, NoMousePolicy, MouseDragPolicy, NoDropPolicy, RenderPolicy> {
        public:
            static const Hit::HitType AngleHandleHit;
        private:
            static const float CenterHandleRadius;
            static const float RotateHandleRadius;
            static const float RotateHandleWidth;
            
            UVViewHelper& m_helper;

            float m_initalAngle;
        public:
            UVRotateTool(MapDocumentWPtr document, UVViewHelper& helper);
        private:
            void doPick(const InputState& inputState, Hits& hits);
            
            bool doStartMouseDrag(const InputState& inputState);
            bool doMouseDrag(const InputState& inputState);
            
            float measureAngle(const Vec2f& point) const;
            float snapAngle(float angle) const;
            
            void doEndMouseDrag(const InputState& inputState);
            void doCancelMouseDrag();

            class Render;
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
        };
    }
}

#endif /* defined(__TrenchBroom__UVRotateTool__) */
