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

#include "ManySpheresInstancedFigure.h"

#include "Renderer/ApplyMatrix.h"
#include "Renderer/AttributeArray.h"
#include "Renderer/InstancedVertexArray.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/Shader/ShaderProgram.h"

#include <cassert>

namespace TrenchBroom {
    namespace Renderer {
        ManySpheresInstancedFigure::ManySpheresInstancedFigure(float radius, unsigned int iterations) :
        SphereFigure(radius, iterations),
        m_valid(false) {}
        
        void ManySpheresInstancedFigure::add(const Vec3f& position) {
            m_positions.push_back(position);
            m_valid = false;
        }
        
        void ManySpheresInstancedFigure::clear() {
            m_valid &= m_positions.empty();
            m_positions.clear();
        }
        
        void ManySpheresInstancedFigure::render(Vbo& vbo, RenderContext& context) {
            SetVboState activateVbo(vbo, Vbo::VboActive);
            
            if (!m_valid) {
                if (!m_positions.empty()) {
                    Vec3f::List vertices = makeVertices();
                    
                    unsigned int vertexCount = static_cast<unsigned int>(vertices.size());
                    unsigned int instanceCount = static_cast<unsigned int>(m_positions.size());
                    m_vertexArray = InstancedVertexArrayPtr(new InstancedVertexArray(vbo, GL_TRIANGLES, vertexCount, instanceCount,
                                                                                     Attribute::position3f()));
                    
                    SetVboState mapVbo(vbo, Vbo::VboMapped);
                    Vec3f::List::iterator it, end;
                    for (it = vertices.begin(), end = vertices.end(); it != end; ++it)
                        m_vertexArray->addAttribute(*it);
                    
                    m_vertexArray->addAttributeArray("position", m_positions);
                } else if (m_vertexArray.get() != NULL) {
                    m_vertexArray = InstancedVertexArrayPtr(NULL);
                }
                m_valid = true;
            }
            
            if (m_vertexArray.get() != NULL) {
                Renderer::ActivateShader shader(context.shaderManager(), Renderer::Shaders::InstancedHandleShader);
                shader.currentShader().setUniformVariable("Color", m_color);
                m_vertexArray->render(shader.currentShader());
            }
        }
    }
}