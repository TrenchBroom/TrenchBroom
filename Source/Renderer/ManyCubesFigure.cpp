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

#include "ManyCubesFigure.h"

#include "Renderer/VertexArray.h"

#include <cassert>

namespace TrenchBroom {
    namespace Renderer {
        ManyCubesFigure::ManyCubesFigure(float cubeSize) :
        m_offset(cubeSize / 2.0f),
        m_valid(false) {}
        
        void ManyCubesFigure::addCube(const Vec3f& position) {
            m_positions.push_back(position);
            m_valid = false;
        }
        
        void ManyCubesFigure::clear() {
            m_valid &= m_positions.empty();
            m_positions.clear();
        }
        
        void ManyCubesFigure::render(Vbo& vbo, RenderContext& context) {
            SetVboState activateVbo(vbo, Vbo::VboActive);
            
            if (!m_valid) {
                if (!m_positions.empty()) {
                    unsigned int vertexCount = static_cast<unsigned int>(m_positions.size() * 24);
                    m_vertexArray = VertexArrayPtr(new VertexArray(vbo, GL_QUADS, vertexCount,
                                                                   VertexAttribute::position3f()));
                    
                    SetVboState mapVbo(vbo, Vbo::VboMapped);
                    
                    Vec3f::List::const_iterator positionIt, positionEnd;
                    for (positionIt = m_positions.begin(), positionEnd = m_positions.end(); positionIt != positionEnd; ++positionIt) {
                        const Vec3f& position = *positionIt;
                        
                        // south face
                        m_vertexArray->addAttribute(Vec3f(position.x - m_offset, position.y - m_offset, position.z - m_offset));
                        m_vertexArray->addAttribute(Vec3f(position.x - m_offset, position.y - m_offset, position.z + m_offset));
                        m_vertexArray->addAttribute(Vec3f(position.x + m_offset, position.y - m_offset, position.z + m_offset));
                        m_vertexArray->addAttribute(Vec3f(position.x + m_offset, position.y - m_offset, position.z - m_offset));
                        
                        // north face
                        m_vertexArray->addAttribute(Vec3f(position.x + m_offset, position.y + m_offset, position.z + m_offset));
                        m_vertexArray->addAttribute(Vec3f(position.x - m_offset, position.y + m_offset, position.z + m_offset));
                        m_vertexArray->addAttribute(Vec3f(position.x - m_offset, position.y + m_offset, position.z - m_offset));
                        m_vertexArray->addAttribute(Vec3f(position.x + m_offset, position.y + m_offset, position.z - m_offset));
                        
                        // west face
                        m_vertexArray->addAttribute(Vec3f(position.x - m_offset, position.y - m_offset, position.z - m_offset));
                        m_vertexArray->addAttribute(Vec3f(position.x - m_offset, position.y + m_offset, position.z - m_offset));
                        m_vertexArray->addAttribute(Vec3f(position.x - m_offset, position.y + m_offset, position.z + m_offset));
                        m_vertexArray->addAttribute(Vec3f(position.x - m_offset, position.y - m_offset, position.z + m_offset));
                        
                        // east face
                        m_vertexArray->addAttribute(Vec3f(position.x + m_offset, position.y + m_offset, position.z + m_offset));
                        m_vertexArray->addAttribute(Vec3f(position.x + m_offset, position.y + m_offset, position.z - m_offset));
                        m_vertexArray->addAttribute(Vec3f(position.x + m_offset, position.y - m_offset, position.z - m_offset));
                        m_vertexArray->addAttribute(Vec3f(position.x + m_offset, position.y - m_offset, position.z + m_offset));
                        
                        // top face
                        m_vertexArray->addAttribute(Vec3f(position.x + m_offset, position.y + m_offset, position.z + m_offset));
                        m_vertexArray->addAttribute(Vec3f(position.x + m_offset, position.y - m_offset, position.z + m_offset));
                        m_vertexArray->addAttribute(Vec3f(position.x - m_offset, position.y - m_offset, position.z + m_offset));
                        m_vertexArray->addAttribute(Vec3f(position.x - m_offset, position.y + m_offset, position.z + m_offset));
                        
                        // bottom face
                        m_vertexArray->addAttribute(Vec3f(position.x - m_offset, position.y - m_offset, position.z - m_offset));
                        m_vertexArray->addAttribute(Vec3f(position.x + m_offset, position.y - m_offset, position.z - m_offset));
                        m_vertexArray->addAttribute(Vec3f(position.x + m_offset, position.y + m_offset, position.z - m_offset));
                        m_vertexArray->addAttribute(Vec3f(position.x - m_offset, position.y + m_offset, position.z - m_offset));
                    }
                } else if (m_vertexArray.get() != NULL) {
                    m_vertexArray = VertexArrayPtr(NULL);
                }
                m_valid = true;
            }
            
            if (m_vertexArray.get() != NULL)
                m_vertexArray->render();
        }
    }
}