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
        void PositioningGuideFigure::renderGuides(RenderContext& context) {
            glBegin(GL_LINES);
            // X guides
            glVertex3f(m_worldBounds.min.x, m_bounds.min.y, m_bounds.min.z);
            glVertex3f(m_worldBounds.max.x, m_bounds.min.y, m_bounds.min.z);
            glVertex3f(m_worldBounds.min.x, m_bounds.min.y, m_bounds.max.z);
            glVertex3f(m_worldBounds.max.x, m_bounds.min.y, m_bounds.max.z);
            glVertex3f(m_worldBounds.min.x, m_bounds.max.y, m_bounds.min.z);
            glVertex3f(m_worldBounds.max.x, m_bounds.max.y, m_bounds.min.z);
            glVertex3f(m_worldBounds.min.x, m_bounds.max.y, m_bounds.max.z);
            glVertex3f(m_worldBounds.max.x, m_bounds.max.y, m_bounds.max.z);
            
            // Y guides
            glVertex3f(m_bounds.min.x, m_worldBounds.min.y, m_bounds.min.z);
            glVertex3f(m_bounds.min.x, m_worldBounds.max.y, m_bounds.min.z);
            glVertex3f(m_bounds.min.x, m_worldBounds.min.y, m_bounds.max.z);
            glVertex3f(m_bounds.min.x, m_worldBounds.max.y, m_bounds.max.z);
            glVertex3f(m_bounds.max.x, m_worldBounds.min.y, m_bounds.min.z);
            glVertex3f(m_bounds.max.x, m_worldBounds.max.y, m_bounds.min.z);
            glVertex3f(m_bounds.max.x, m_worldBounds.min.y, m_bounds.max.z);
            glVertex3f(m_bounds.max.x, m_worldBounds.max.y, m_bounds.max.z);
            
            // Z guides
            glVertex3f(m_bounds.min.x, m_bounds.min.y, m_worldBounds.min.z);
            glVertex3f(m_bounds.min.x, m_bounds.min.y, m_worldBounds.max.z);
            glVertex3f(m_bounds.min.x, m_bounds.max.y, m_worldBounds.min.z);
            glVertex3f(m_bounds.min.x, m_bounds.max.y, m_worldBounds.max.z);
            glVertex3f(m_bounds.max.x, m_bounds.min.y, m_worldBounds.min.z);
            glVertex3f(m_bounds.max.x, m_bounds.min.y, m_worldBounds.max.z);
            glVertex3f(m_bounds.max.x, m_bounds.max.y, m_worldBounds.min.z);
            glVertex3f(m_bounds.max.x, m_bounds.max.y, m_worldBounds.max.z);
            glEnd();
        }

        PositioningGuideFigure::PositioningGuideFigure(const BBox& worldBounds, const BBox& bounds, const Vec4f& color, const Vec4f& hiddenColor) : m_worldBounds(worldBounds), m_bounds(bounds), m_color(color), m_hiddenColor(hiddenColor) {}
        
        void PositioningGuideFigure::updateBounds(const BBox& bounds) {
            m_bounds = bounds;
        }
        
        void PositioningGuideFigure::render(RenderContext& context) {
            glDisable(GL_DEPTH_TEST);
            glColorV4f(m_hiddenColor);
            renderGuides(context);
            glEnable(GL_DEPTH_TEST);
            glColorV4f(m_color);
            renderGuides(context);
        }
    }
}
