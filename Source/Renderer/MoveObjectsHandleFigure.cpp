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

#include "MoveObjectsHandleFigure.h"

#include "Renderer/VertexArray.h"

namespace TrenchBroom {
    namespace Renderer {
        void MoveObjectsHandleFigure::setPosition(const Vec3f& position) {
            m_position = position;
        }
        
        void MoveObjectsHandleFigure::render(Vbo& vbo, RenderContext& context) {
            VertexArray axisArray(vbo, GL_LINES, 6,
                                  VertexAttribute(3, GL_FLOAT, VertexAttribute::Position),
                                  VertexAttribute(4, GL_FLOAT, VertexAttribute::Color));
            axisArray.addAttribute(Vec3f(m_position.x - 128.0f, m_position.y, m_position.z));
            axisArray.addAttribute(Color(1.0f, 0.0f, 0.0f, 1.0f));
            axisArray.addAttribute(Vec3f(m_position.x + 128.0f, m_position.y, m_position.z));
            axisArray.addAttribute(Color(1.0f, 0.0f, 0.0f, 1.0f));
            axisArray.addAttribute(Vec3f(m_position.x, m_position.y - 128.0f, m_position.z));
            axisArray.addAttribute(Color(0.0f, 1.0f, 0.0f, 1.0f));
            axisArray.addAttribute(Vec3f(m_position.x, m_position.y + 128.0f, m_position.z));
            axisArray.addAttribute(Color(0.0f, 1.0f, 0.0f, 1.0f));
            axisArray.addAttribute(Vec3f(m_position.x, m_position.y, m_position.z - 128.0f));
            axisArray.addAttribute(Color(0.0f, 0.0f, 1.0f, 1.0f));
            axisArray.addAttribute(Vec3f(m_position.x, m_position.y, m_position.z + 128.0f));
            axisArray.addAttribute(Color(0.0f, 0.0f, 1.0f, 1.0f));
            
            axisArray.render();
        }
    }
}