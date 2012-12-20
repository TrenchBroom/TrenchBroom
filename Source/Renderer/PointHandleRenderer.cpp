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

#include "PointHandleRenderer.h"

#include "Renderer/ApplyMatrix.h"
#include "Renderer/AttributeArray.h"
#include "Renderer/InstancedVertexArray.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/Shader/ShaderProgram.h"

#include <cassert>

namespace TrenchBroom {
    namespace Renderer {
        PointHandleRenderer::PointHandleRenderer(float radius, unsigned int iterations, float scalingFactor, float maximumDistance) :
        SphereFigure(radius, iterations),
        m_vertexArray(NULL),
        m_scalingFactor(scalingFactor),
        m_maximumDistance(maximumDistance),
        m_valid(false) {}
        
        PointHandleRenderer::~PointHandleRenderer() {
            delete m_vertexArray;
            m_vertexArray = NULL;
        }

        void PointHandleRenderer::add(const Vec3f& position) {
            m_positions.push_back(Vec4f(position.x, position.y, position.z, 1.0f));
            m_valid = false;
        }
        
        void PointHandleRenderer::clear() {
            m_valid &= m_positions.empty();
            m_positions.clear();
        }
        
        void PointHandleRenderer::render(Vbo& vbo, RenderContext& context) {
            SetVboState activateVbo(vbo, Vbo::VboActive);
            
            if (!m_valid) {
                delete m_vertexArray;
                m_vertexArray = NULL;
                
                if (!m_positions.empty()) {
                    Vec3f::List vertices = makeVertices();
                    
                    unsigned int vertexCount = static_cast<unsigned int>(vertices.size());
                    unsigned int instanceCount = static_cast<unsigned int>(m_positions.size());
                    m_vertexArray = new InstancedVertexArray(vbo, GL_TRIANGLES, vertexCount, instanceCount,
                                                             Attribute::position3f());
                    
                    SetVboState mapVbo(vbo, Vbo::VboMapped);
                    Vec3f::List::iterator it, end;
                    for (it = vertices.begin(), end = vertices.end(); it != end; ++it)
                        m_vertexArray->addAttribute(*it);
                    
                    m_vertexArray->addAttributeArray("position", m_positions);
                }
                m_valid = true;
            }
            
            if (m_vertexArray != NULL) {
                Renderer::ActivateShader shader(context.shaderManager(), Renderer::Shaders::InstancedHandleShader);
                shader.currentShader().setUniformVariable("Color", m_color);
                shader.currentShader().setUniformVariable("CameraPosition", context.camera().position());
                shader.currentShader().setUniformVariable("ScalingFactor", m_scalingFactor);
                shader.currentShader().setUniformVariable("MaximumDistance", m_maximumDistance);
                m_vertexArray->render(shader.currentShader());
            }
        }
    }
}
