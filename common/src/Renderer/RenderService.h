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

#ifndef __TrenchBroom__RenderService__
#define __TrenchBroom__RenderService__

#include "AttrString.h"
#include "TrenchBroom.h"
#include "VecMath.h"

class Color;

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
            TextRenderer* m_textRenderer;
            PointHandleRenderer* m_pointHandleRenderer;
            PrimitiveRenderer* m_primitiveRenderer;
        public:
            RenderService(const FontDescriptor& fontDescriptor);
            ~RenderService();

            
            void renderString(RenderContext& renderContext, const Color& textColor, const Color& backgroundColor, const AttrString& string, const TextAnchor& position);
            void renderStringOnTop(RenderContext& renderContext, const Color& textColor, const Color& backgroundColor, const AttrString& string, const TextAnchor& position);
            
            void renderPointHandles(const Vec3f::List& positions);
            void renderPointHandle(const Vec3f& position);
            void renderSelectedPointHandles(const Vec3f::List& positions);
            void renderSelectedPointHandle(const Vec3f& position);
            void renderPointHandleHighlight(const Vec3f& position);
            
            void renderLine(const Color& color, const Vec3f& start, const Vec3f& end);
            void renderLines(const Color& color, const Vec3f::List& positions);
            void renderCoordinateSystem(const BBox3f& bounds);
            
            void renderCircle(const Color& color, const Vec3f& position, Math::Axis::Type normal, size_t segments, float radius, const Vec3f& startAxis, const Vec3f& endAxis);
            void renderCircle(const Color& color, const Vec3f& position, Math::Axis::Type normal, size_t segments, float radius, float startAngle = 0.0f, float angleLength = Math::Cf::twoPi());
            
            void renderFilledCircle(const Color& color, const Vec3f& position, Math::Axis::Type normal, size_t segments, float radius, const Vec3f& startAxis, const Vec3f& endAxis);
            void renderFilledCircle(const Color& color, const Vec3f& position, Math::Axis::Type normal, size_t segments, float radius, float startAngle = 0.0f, float angleLength = Math::Cf::twoPi());
            
            void render(RenderBatch& renderBatch);
        };
    }
}

#endif /* defined(__TrenchBroom__RenderService__) */
