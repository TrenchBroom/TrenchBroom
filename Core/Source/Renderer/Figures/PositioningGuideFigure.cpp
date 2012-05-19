/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "PositioningGuideFigure.h"
#include "GL/GLee.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"

namespace TrenchBroom {
    namespace Renderer {
        void PositioningGuideFigure::renderLine(RenderContext& context, const Vec4f& color, const Vec3f& anchor, float size, const Vec3f& axis) {
            float length = 256;
            Vec3f outerOffset = axis * ((size / 2.0f) + length);
            Vec3f innerOffset = axis * (size / 2.0f);

            glColorV4f(color, 0.2f * color.w);
            glVertexV3f(anchor - outerOffset);
            glColorV4f(color, 0.6f * color.w);
            glVertexV3f(anchor - innerOffset);
            glVertexV3f(anchor - innerOffset);
            glVertexV3f(anchor + innerOffset);
            glVertexV3f(anchor + innerOffset);
            glColorV4f(color, 0.2f * color.w);
            glVertexV3f(anchor + outerOffset);
        }

        void PositioningGuideFigure::renderGuides(RenderContext& context, const Vec4f& color) {
            Vec3f v;
            Vec3f size = m_bounds.size();
            
            glBegin(GL_LINES);
            v = m_bounds.min;
            v.x += size.x / 2;
            renderLine(context, color, v, size.x, XAxisPos);
            v.y += size.y;
            renderLine(context, color, v, size.x, XAxisPos);
            v.z += size.z;
            renderLine(context, color, v, size.x, XAxisPos);
            v.y -= size.y;
            renderLine(context, color, v, size.x, XAxisPos);
            
            v = m_bounds.min;
            v.y += size.y / 2;
            renderLine(context, color, v, size.y, YAxisPos);
            v.x += size.x;
            renderLine(context, color, v, size.y, YAxisPos);
            v.z += size.z;
            renderLine(context, color, v, size.y, YAxisPos);
            v.x -= size.x;
            renderLine(context, color, v, size.y, YAxisPos);

            v = m_bounds.min;
            v.z += size.z / 2;
            renderLine(context, color, v, size.z, ZAxisPos);
            v.x += size.x;
            renderLine(context, color, v, size.z, ZAxisPos);
            v.y += size.y;
            renderLine(context, color, v, size.z, ZAxisPos);
            v.x -= size.x;
            renderLine(context, color, v, size.z, ZAxisPos);
            glEnd();
        }

        PositioningGuideFigure::PositioningGuideFigure(const BBox& bounds, const Vec4f& color, const Vec4f& hiddenColor) : m_bounds(bounds), m_color(color), m_hiddenColor(hiddenColor) {}
        
        void PositioningGuideFigure::updateBounds(const BBox& bounds) {
            m_bounds = bounds;
        }
        
        void PositioningGuideFigure::render(RenderContext& context) {
            glEnable(GL_DEPTH_TEST);
            glColorV4f(m_color);
            renderGuides(context, m_color);
        }
    }
}
