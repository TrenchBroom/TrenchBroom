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

#ifndef __TrenchBroom__UVOriginTool__
#define __TrenchBroom__UVOriginTool__

#include "Hit.h"
#include "Renderer/VertexSpec.h"
#include "View/Tool.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Renderer {
        class OrthographicCamera;
        class RenderContext;
    }
    
    namespace View {
        class UVViewHelper;
        
        class UVOriginTool : public ToolImpl<NoActivationPolicy, PickingPolicy, NoMousePolicy, MouseDragPolicy, NoDropPolicy, RenderPolicy> {
        public:
            static const Hit::HitType XHandleHit;
            static const Hit::HitType YHandleHit;
        private:
            static const FloatType MaxPickDistance;
            static const float OriginHandleRadius;
            
            typedef Renderer::VertexSpecs::P3C4::Vertex EdgeVertex;

            UVViewHelper& m_helper;
            
            Vec2f m_lastPoint;
            Vec2f m_selector;
        public:
            UVOriginTool(MapDocumentWPtr document, ControllerWPtr controller, UVViewHelper& helper);
        private:
            void doPick(const InputState& inputState, Hits& hits);

            void computeOriginHandles(Line3& xHandle, Line3& yHandle) const;
            
            bool doStartMouseDrag(const InputState& inputState);
            bool doMouseDrag(const InputState& inputState);
            
            Vec2f computeHitPoint(const Ray3& ray) const;
            Vec2f snapDelta(const Vec2f& delta) const;
            
            void doEndMouseDrag(const InputState& inputState);
            void doCancelMouseDrag(const InputState& inputState);

            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext);
            
            void renderLineHandles(const InputState& inputState, Renderer::RenderContext& renderContext);
            EdgeVertex::List getHandleVertices(const Hits& hits) const;
            
            void renderOriginHandle(const InputState& inputState, Renderer::RenderContext& renderContext);
        };
    }
}

#endif /* defined(__TrenchBroom__UVOriginTool__) */
