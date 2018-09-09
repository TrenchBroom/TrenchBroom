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

#ifndef TrenchBroom_RenderService
#define TrenchBroom_RenderService

#include "AttrString.h"
#include "TrenchBroom.h"
#include <vecmath/VecMath.h>
#include "Color.h"
#include "Renderer/PrimitiveRenderer.h"

namespace TrenchBroom {
    
    namespace Renderer {
        class FontDescriptor;
        class PointHandleRenderer;
        class PrimitiveRenderer;
        class RenderBatch;
        class RenderContext;
        class TextAnchor;
        class TextRenderer;
        class Vbo;
        
        class RenderService {
        private:
            typedef PrimitiveRenderer::OcclusionPolicy OcclusionPolicy;
            typedef PrimitiveRenderer::CullingPolicy CullingPolicy;
            class HeadsUpTextAnchor;
            
            RenderContext& m_renderContext;
            RenderBatch& m_renderBatch;
            TextRenderer* m_textRenderer;
            PointHandleRenderer* m_pointHandleRenderer;
            PrimitiveRenderer* m_primitiveRenderer;
            
            Color m_foregroundColor;
            Color m_backgroundColor;
            float m_lineWidth;
            OcclusionPolicy m_occlusionPolicy;
            CullingPolicy m_cullingPolicy;
        public:
            RenderService(RenderContext& renderContext, RenderBatch& renderBatch);
            ~RenderService();

            void setForegroundColor(const Color& foregroundColor);
            void setBackgroundColor(const Color& backgroundColor);
            void setLineWidth(float lineWidth);
            
            void setShowOccludedObjects();
            void setShowOccludedObjectsTransparent();
            void setHideOccludedObjects();
            
            void setShowBackfaces();
            void setCullBackfaces();
            
            void renderString(const AttrString& string, const vm::vec3f& position);
            void renderString(const AttrString& string, const TextAnchor& position);
            void renderHeadsUp(const AttrString& string);
            
            void renderHandles(const vm::vec3f::List& positions);
            void renderHandle(const vm::vec3f& position);
            void renderHandleHighlight(const vm::vec3f& position);
            
            void renderHandles(const vm::segment3f::List& positions);
            void renderHandle(const vm::segment3f& position);
            void renderHandleHighlight(const vm::segment3f& position);
            
            void renderHandles(const vm::polygon3f::List& positions);
            void renderHandle(const vm::polygon3f& position);
            void renderHandleHighlight(const vm::polygon3f& position);

            void renderLine(const vm::vec3f& start, const vm::vec3f& end);
            void renderLines(const vm::vec3f::List& positions);
            void renderLineStrip(const vm::vec3f::List& positions);
            void renderCoordinateSystem(const vm::bbox3f& bounds);
            
            void renderPolygonOutline(const vm::vec3f::List& positions);
            void renderFilledPolygon(const vm::vec3f::List& positions);
            
            void renderBounds(const vm::bbox3f& bounds);
            
            void renderCircle(const vm::vec3f& position, vm::axis::type normal, size_t segments, float radius, const vm::vec3f& startAxis, const vm::vec3f& endAxis);
            void renderCircle(const vm::vec3f& position, vm::axis::type normal, size_t segments, float radius, float startAngle = 0.0f, float angleLength = vm::Cf::twoPi());
            
            void renderFilledCircle(const vm::vec3f& position, vm::axis::type normal, size_t segments, float radius, const vm::vec3f& startAxis, const vm::vec3f& endAxis);
            void renderFilledCircle(const vm::vec3f& position, vm::axis::type normal, size_t segments, float radius, float startAngle = 0.0f, float angleLength = vm::Cf::twoPi());
        private:
            void flush();
        };
    }
}

#endif /* defined(TrenchBroom_RenderService) */
