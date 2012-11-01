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

#include "RingFigure.h"

#include "Renderer/Vbo.h"
#include "Renderer/VertexArray.h"

namespace TrenchBroom {
    namespace Renderer {
        RingFigure::RingFigure(Axis::Type normal, float startAngle, float angleLength, float radius, float thickness, unsigned int segments, const Color& color) :
        m_normal(normal),
        m_startAngle(startAngle),
        m_angleLength(angleLength),
        m_innerRadius(radius),
        m_outerRadius(radius + thickness),
        m_segments(segments),
        m_color(color) {}
        
        RingFigure::RingFigure(Axis::Type normal, const Vec3f& startAxis, const Vec3f& endAxis, float radius, float thickness, unsigned int segments, const Color& color) :
        m_normal(normal),
        m_innerRadius(radius),
        m_outerRadius(radius + thickness),
        m_segments(segments),
        m_color(color) {
            float angle1, angle2;
            if (m_normal == Axis::AX) {
                angle1 = startAxis.angleFrom(Vec3f::PosZ, Vec3f::PosX);
                angle2 = endAxis.angleFrom(Vec3f::PosZ, Vec3f::PosX);
                m_angleLength = fminf(startAxis.angleFrom(endAxis, Vec3f::PosX), endAxis.angleFrom(startAxis, Vec3f::PosX));
            } else if (m_normal == Axis::AY) {
                angle1 = startAxis.angleFrom(Vec3f::PosX, Vec3f::PosY);
                angle2 = endAxis.angleFrom(Vec3f::PosX, Vec3f::PosY);
                m_angleLength = fminf(startAxis.angleFrom(endAxis, Vec3f::PosY), endAxis.angleFrom(startAxis, Vec3f::PosY));
            } else {
                angle1 = startAxis.angleFrom(Vec3f::PosY, Vec3f::PosZ);
                angle2 = endAxis.angleFrom(Vec3f::PosY, Vec3f::PosZ);
                m_angleLength = fminf(startAxis.angleFrom(endAxis, Vec3f::PosZ), endAxis.angleFrom(startAxis, Vec3f::PosZ));
            }
            float minAngle = fminf(angle1, angle2);
            float maxAngle = fmaxf(angle1, angle2);
            m_startAngle = (maxAngle - minAngle <= Math::Pi ? minAngle : maxAngle);
        }
        
        void RingFigure::render(Vbo& vbo, RenderContext& context) {
            SetVboState activateVbo(vbo, Vbo::VboActive);
            if (m_vertexArray.get() == NULL) {
                SetVboState mapVbo(vbo, Vbo::VboMapped);
                m_vertexArray = VertexArrayPtr(new VertexArray(vbo, GL_TRIANGLE_STRIP, 2 * m_segments + 2,
                                                               VertexAttribute::position3f(),
                                                               VertexAttribute::color4f()));
                
                float d = m_angleLength / m_segments;
                float a = m_startAngle;
                for (unsigned int i = 0; i <= m_segments; i++) {
                    float s = sin(a);
                    float c = cos(a);
                    if (m_normal == Axis::AX) {
                        m_vertexArray->addAttribute(Vec3f(0.0f, m_outerRadius * s, m_outerRadius * c));
                        m_vertexArray->addAttribute(m_color);
                        m_vertexArray->addAttribute(Vec3f(0.0f, m_innerRadius * s, m_innerRadius * c));
                        m_vertexArray->addAttribute(m_color);
                    } else if (m_normal == Axis::AY) {
                        m_vertexArray->addAttribute(Vec3f(m_outerRadius * c, 0.0f, m_outerRadius * s));
                        m_vertexArray->addAttribute(m_color);
                        m_vertexArray->addAttribute(Vec3f(m_innerRadius * c, 0.0f, m_innerRadius * s));
                        m_vertexArray->addAttribute(m_color);
                    } else {
                        m_vertexArray->addAttribute(Vec3f(m_outerRadius * s, m_outerRadius * c, 0.0f));
                        m_vertexArray->addAttribute(m_color);
                        m_vertexArray->addAttribute(Vec3f(m_innerRadius * s, m_innerRadius * c, 0.0f));
                        m_vertexArray->addAttribute(m_color);
                    }
                    a += d;
                }
            }
            
            m_vertexArray->render();
        }
    }
}