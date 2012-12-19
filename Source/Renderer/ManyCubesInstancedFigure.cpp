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

#include "ManyCubesInstancedFigure.h"

#include "Renderer/AttributeArray.h"
#include "Renderer/InstancedVertexArray.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/Shader/ShaderProgram.h"

#include <cassert>

namespace TrenchBroom {
    namespace Renderer {
        ManyCubesInstancedFigure::ManyCubesInstancedFigure(float cubeSize) :
        m_offset(cubeSize / 2.0f),
        m_vertexArray(NULL),
        m_valid(false) {}
        
        ManyCubesInstancedFigure::~ManyCubesInstancedFigure() {
            delete m_vertexArray;
            m_vertexArray = NULL;
        }
        
        void ManyCubesInstancedFigure::add(const Vec3f& position) {
            m_positions.push_back(Vec4f(position.x, position.y, position.z, 0.0f));
            m_valid = false;
        }
        
        void ManyCubesInstancedFigure::clear() {
            m_valid &= m_positions.empty();
            m_positions.clear();
        }
        
        void ManyCubesInstancedFigure::render(Vbo& vbo, RenderContext& context) {
            SetVboState activateVbo(vbo, Vbo::VboActive);
            
            if (!m_valid) {
                delete m_vertexArray;
                m_vertexArray = NULL;
                if (!m_positions.empty()) {
                    unsigned int instanceCount = static_cast<unsigned int>(m_positions.size());
                    m_vertexArray = new InstancedVertexArray(vbo, GL_QUADS, 24, instanceCount,
                                                             Attribute::position3f());
                    
                    SetVboState mapVbo(vbo, Vbo::VboMapped);
                    
                    // south face
                    m_vertexArray->addAttribute(Vec3f(-m_offset, -m_offset, -m_offset));
                    m_vertexArray->addAttribute(Vec3f(-m_offset, -m_offset, +m_offset));
                    m_vertexArray->addAttribute(Vec3f(+m_offset, -m_offset, +m_offset));
                    m_vertexArray->addAttribute(Vec3f(+m_offset, -m_offset, -m_offset));
                    
                    // north face
                    m_vertexArray->addAttribute(Vec3f(+m_offset, +m_offset, +m_offset));
                    m_vertexArray->addAttribute(Vec3f(-m_offset, +m_offset, +m_offset));
                    m_vertexArray->addAttribute(Vec3f(-m_offset, +m_offset, -m_offset));
                    m_vertexArray->addAttribute(Vec3f(+m_offset, +m_offset, -m_offset));
                    
                    // west face
                    m_vertexArray->addAttribute(Vec3f(-m_offset, -m_offset, -m_offset));
                    m_vertexArray->addAttribute(Vec3f(-m_offset, +m_offset, -m_offset));
                    m_vertexArray->addAttribute(Vec3f(-m_offset, +m_offset, +m_offset));
                    m_vertexArray->addAttribute(Vec3f(-m_offset, -m_offset, +m_offset));
                    
                    // east face
                    m_vertexArray->addAttribute(Vec3f(+m_offset, +m_offset, +m_offset));
                    m_vertexArray->addAttribute(Vec3f(+m_offset, +m_offset, -m_offset));
                    m_vertexArray->addAttribute(Vec3f(+m_offset, -m_offset, -m_offset));
                    m_vertexArray->addAttribute(Vec3f(+m_offset, -m_offset, +m_offset));
                    
                    // top face
                    m_vertexArray->addAttribute(Vec3f(+m_offset, +m_offset, +m_offset));
                    m_vertexArray->addAttribute(Vec3f(+m_offset, -m_offset, +m_offset));
                    m_vertexArray->addAttribute(Vec3f(-m_offset, -m_offset, +m_offset));
                    m_vertexArray->addAttribute(Vec3f(-m_offset, +m_offset, +m_offset));
                    
                    // bottom face
                    m_vertexArray->addAttribute(Vec3f(-m_offset, -m_offset, -m_offset));
                    m_vertexArray->addAttribute(Vec3f(+m_offset, -m_offset, -m_offset));
                    m_vertexArray->addAttribute(Vec3f(+m_offset, +m_offset, -m_offset));
                    m_vertexArray->addAttribute(Vec3f(-m_offset, +m_offset, -m_offset));

                    m_vertexArray->addAttributeArray("position", m_positions);
                }
                m_valid = true;
            }
            
            if (m_vertexArray != NULL) {
                Renderer::ActivateShader shader(context.shaderManager(), Renderer::Shaders::InstancedHandleShader);
                shader.currentShader().setUniformVariable("Color", m_color);
                m_vertexArray->render(shader.currentShader());
            }
        }
    }
}