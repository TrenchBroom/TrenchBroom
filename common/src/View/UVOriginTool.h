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

#ifndef TrenchBroom_UVOriginTool
#define TrenchBroom_UVOriginTool

#include "Model//Hit.h"
#include "Renderer/VertexSpec.h"
#include "View/Tool.h"
#include "View/ToolController.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Model {
        class PickResult;
    }
    
    namespace Renderer {
        class RenderBatch;
        class RenderContext;
    }
    
    namespace View {
        class UVViewHelper;
        
        class UVOriginTool : public ToolControllerBase<PickingPolicy, NoKeyPolicy, NoMousePolicy, MouseDragPolicy, RenderPolicy, NoDropPolicy>, public Tool {
        public:
            static const Model::Hit::HitType XHandleHit;
            static const Model::Hit::HitType YHandleHit;
        private:
            static const FloatType MaxPickDistance;
            static const float OriginHandleRadius;
            
            typedef Renderer::VertexSpecs::P3C4::Vertex EdgeVertex;

            UVViewHelper& m_helper;
            
            vec2f m_lastPoint;
            vec2f m_selector;
        public:
            UVOriginTool(UVViewHelper& helper);
        private:
            Tool* doGetTool() override;
            
            void doPick(const InputState& inputState, Model::PickResult& pickResult) override;

            void computeOriginHandles(line3& xHandle, line3& yHandle) const;
            
            bool doStartMouseDrag(const InputState& inputState) override;
            bool doMouseDrag(const InputState& inputState) override;
            
            vec2f computeHitPoint(const Ray3& ray) const;
            vec2f snapDelta(const vec2f& delta) const;
            
            void doEndMouseDrag(const InputState& inputState) override;
            void doCancelMouseDrag() override;

            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override;
            
            void renderLineHandles(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            EdgeVertex::List getHandleVertices(const InputState& inputState) const;
            
            class RenderOrigin;
            void renderOriginHandle(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            
            bool doCancel() override;
        };
    }
}

#endif /* defined(TrenchBroom_UVOriginTool) */
