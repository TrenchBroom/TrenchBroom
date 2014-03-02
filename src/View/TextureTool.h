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

#ifndef __TrenchBroom__TextureTool__
#define __TrenchBroom__TextureTool__

#include "StringUtils.h"
#include "TrenchBroom.h"
#include "VecMath.h"
#include "View/Tool.h"

namespace TrenchBroom {
    namespace Model {
        class BrushFace;
    }
    
    namespace Renderer {
        class EdgeRenderer;
    }
    
    namespace View {
        class TextureToolHelper {
        public:
            virtual ~TextureToolHelper();
            
            bool startDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint);
            bool drag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint);
            void endDrag(const InputState& inputState);
            void cancelDrag(const InputState& inputState);
            
            void setRenderOptions(const InputState& inputState, bool dragging, Renderer::RenderContext& renderContext) const;
            void render(const InputState& inputState, bool dragging, Renderer::RenderContext& renderContext);
        private:
            virtual bool doStartDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint) = 0;
            virtual bool doDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint) = 0;
            virtual void doEndDrag(const InputState& inputState) = 0;
            virtual void doCancelDrag(const InputState& inputState) = 0;
            
            virtual void doSetRenderOptions(const InputState& inputState, bool dragging, Renderer::RenderContext& renderContext) const = 0;
            virtual void doRender(const InputState& inputState, bool dragging, Renderer::RenderContext& renderContext) = 0;
        };
        
        class MoveTextureHelper;
        
        class TextureTool : public Tool<ActivationPolicy, NoPickingPolicy, NoMousePolicy, PlaneDragPolicy, NoDropPolicy, RenderPolicy> {
        private:
            MoveTextureHelper* m_moveTextureHelper;
            TextureToolHelper* m_currentHelper;
        public:
            TextureTool(BaseTool* next, MapDocumentWPtr document, ControllerWPtr controller);
            ~TextureTool();
        private:
            bool initiallyActive() const;
            bool doActivate(const InputState& inputState);
            bool doDeactivate(const InputState& inputState);
            
            bool doStartPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint);
            bool doPlaneDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint);
            void doEndPlaneDrag(const InputState& inputState);
            void doCancelPlaneDrag(const InputState& inputState);
            
            void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const;
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext);
        };
    }
}

#endif /* defined(__TrenchBroom__TextureTool__) */
