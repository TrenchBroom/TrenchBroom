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

#ifndef __TrenchBroom__UVViewRotateTool__
#define __TrenchBroom__UVViewRotateTool__

#include "Hit.h"
#include "View/Tool.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Renderer {
        class RenderContext;
    }

    namespace View {
        class UVViewHelper;

        class UVViewRotateTool : public ToolImpl<NoActivationPolicy, PickingPolicy, NoMousePolicy, MouseDragPolicy, NoDropPolicy, RenderPolicy> {
        public:
            static const Hit::HitType AngleHandleHit;
        private:
            static const float CenterHandleRadius;
            static const float RotateHandleRadius;
            static const float RotateHandleWidth;
            
            UVViewHelper& m_helper;

            Vec2f m_offset;
        public:
            UVViewRotateTool(MapDocumentWPtr document, ControllerWPtr controller, UVViewHelper& helper);
        private:
            void doPick(const InputState& inputState, Hits& hits);
            
            bool doStartMouseDrag(const InputState& inputState);
            bool doMouseDrag(const InputState& inputState);
            
            float measureAngle(const Vec2f& point) const;
            float snapAngle(float angle) const;
            
            void doEndMouseDrag(const InputState& inputState);
            void doCancelMouseDrag(const InputState& inputState);

            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext);
        };
    }
}

#endif /* defined(__TrenchBroom__UVViewRotateTool__) */
