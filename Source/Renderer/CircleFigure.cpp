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

#include "CircleFigure.h"

#include "Renderer/VertexArray.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Renderer {
        CircleFigure::CircleFigure(float radius, unsigned int segments, const Color& color, bool filled) :
        m_radius(radius),
        m_segments(segments),
        m_color(color),
        m_filled(filled) {}
        
        void CircleFigure::render(Vbo& vbo, RenderContext& context) {
            SetVboState setVboState(vbo, Vbo::VboActive);
            if (m_vertexArray.get() == NULL) {
                if (m_filled)
                    m_vertexArray = VertexArrayPtr(new VertexArray(vbo, GL_TRIANGLE_FAN, m_segments,
                                                                   VertexAttribute(3, GL_FLOAT, VertexAttribute::Position),
                                                                   VertexAttribute(4, GL_FLOAT, VertexAttribute::Color)));
                else
                    m_vertexArray = VertexArrayPtr(new VertexArray(vbo, GL_LINE_LOOP, m_segments,
                                                                   VertexAttribute(3, GL_FLOAT, VertexAttribute::Position),
                                                                   VertexAttribute(4, GL_FLOAT, VertexAttribute::Color)));

                SetVboState setVboState(vbo, Vbo::VboMapped);
                float d = 2.0f * Math::Pi / m_segments;
                float a = 0.0f;
                for (unsigned int i = 0; i < m_segments; i++) {
                    float s = sin(a);
                    float c = cos(a);
                    m_vertexArray->addAttribute(Vec3f(m_radius * s, m_radius * c, 0.0f));
                    m_vertexArray->addAttribute(m_color);
                    a += d;
                }
            }
            
            m_vertexArray->render();
        }
    }
}