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

#include "AxisFigure.h"

#include "Renderer/Vbo.h"
#include "Renderer/VertexArray.h"

namespace TrenchBroom {
    namespace Renderer {
        AxisFigure::AxisFigure(float axisLength) :
        m_axisLength(axisLength),
        m_axes(Axis::AX | Axis::AY | Axis::AZ),
        m_xColor(Color(1.0f, 0.0f, 0.0f, 1.0f)),
        m_yColor(Color(0.0f, 1.0f, 0.0f, 1.0f)),
        m_zColor(Color(0.0f, 0.0f, 1.0f, 1.0f)),
        m_valid(false),
        m_vertexArray(NULL) {}
        
        AxisFigure::~AxisFigure() {
            delete m_vertexArray;
            m_vertexArray = NULL;
        }
        
        void AxisFigure::render(Vbo& vbo, RenderContext& context) {
            SetVboState activateVbo(vbo, Vbo::VboActive);
            if (!m_valid) {
                unsigned int vertexCount = 0;
                if ((m_axes & Axis::AX) != 0)
                    vertexCount += 2;
                if ((m_axes & Axis::AY) != 0)
                    vertexCount += 2;
                if ((m_axes & Axis::AZ) != 0)
                    vertexCount += 2;
                m_vertexArray = new VertexArray(vbo, GL_LINES, vertexCount,
                                                Attribute::position3f(),
                                                Attribute::color4f());
                SetVboState mapVbo(vbo, Vbo::VboMapped);
                if ((m_axes & Axis::AX) != 0) {
                    m_vertexArray->addAttribute(Vec3f(-m_axisLength, 0.0f, 0.0f));
                    m_vertexArray->addAttribute(m_xColor);
                    m_vertexArray->addAttribute(Vec3f(+m_axisLength, 0.0f, 0.0f));
                    m_vertexArray->addAttribute(m_xColor);
                }
                if ((m_axes & Axis::AY) != 0) {
                    m_vertexArray->addAttribute(Vec3f(0.0f, -m_axisLength, 0.0f));
                    m_vertexArray->addAttribute(m_yColor);
                    m_vertexArray->addAttribute(Vec3f(0.0f, +m_axisLength, 0.0f));
                    m_vertexArray->addAttribute(m_yColor);
                }
                if ((m_axes & Axis::AZ) != 0) {
                    m_vertexArray->addAttribute(Vec3f(0.0f, 0.0f, -m_axisLength));
                    m_vertexArray->addAttribute(m_zColor);
                    m_vertexArray->addAttribute(Vec3f(0.0f, 0.0f, +m_axisLength));
                    m_vertexArray->addAttribute(m_zColor);
                }
                m_valid = true;
            }

            assert(m_vertexArray != NULL);
            m_vertexArray->render();
        }
    }
}