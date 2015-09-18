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

#ifndef TrenchBroom_RenderService
#define TrenchBroom_RenderService

#include "AttrString.h"
#include "TrenchBroom.h"
#include "VecMath.h"
#include "Color.h"

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
            RenderContext& m_renderContext;
            RenderBatch& m_renderBatch;
            TextRenderer* m_textRenderer;
            PointHandleRenderer* m_pointHandleRenderer;
            PrimitiveRenderer* m_primitiveRenderer;
            
            Color m_foregroundColor;
            Color m_backgroundColor;
            float m_lineWidth;
        public:
            RenderService(RenderContext& renderContext, RenderBatch& renderBatch);
            ~RenderService();

            void setForegroundColor(const Color& foregroundColor);
            void setBackgroundColor(const Color& backgroundColor);
            void setLineWidth(float lineWidth);
            
            void renderString(const AttrString& string, const Vec3f& position);
            void renderString(const AttrString& string, const TextAnchor& position);
            void renderStringOnTop(const AttrString& string, const Vec3f& position);
            void renderStringOnTop(const AttrString& string, const TextAnchor& position);
            
            void renderPointHandles(const Vec3f::List& positions);
            void renderPointHandle(const Vec3f& position);
            void renderPointHandleHighlight(const Vec3f& position);
            
            void renderLine(const Vec3f& start, const Vec3f& end);
            void renderLines(const Vec3f::List& positions);
            void renderCoordinateSystem(const BBox3f& bounds);
            
            void renderPolygonOutline(const Vec3f::List& positions);
            void renderFilledPolygon(const Vec3f::List& positions);
            
            void renderBounds(const BBox3f& bounds);
            
            void renderCircle(const Vec3f& position, Math::Axis::Type normal, size_t segments, float radius, const Vec3f& startAxis, const Vec3f& endAxis);
            void renderCircle(const Vec3f& position, Math::Axis::Type normal, size_t segments, float radius, float startAngle = 0.0f, float angleLength = Math::Cf::twoPi());
            
            void renderFilledCircle(const Vec3f& position, Math::Axis::Type normal, size_t segments, float radius, const Vec3f& startAxis, const Vec3f& endAxis);
            void renderFilledCircle(const Vec3f& position, Math::Axis::Type normal, size_t segments, float radius, float startAngle = 0.0f, float angleLength = Math::Cf::twoPi());
        private:
            void flush();
        };
    }
}

#endif /* defined(TrenchBroom_RenderService) */
