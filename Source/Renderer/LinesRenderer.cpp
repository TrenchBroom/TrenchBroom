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

#include "LinesRenderer.h"

#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/Shader/ShaderProgram.h"
#include "Renderer/Vbo.h"
#include "Renderer/VertexArray.h"

namespace TrenchBroom {
    namespace Renderer {
        LinesRenderer::LinesRenderer() :
        m_vertexArray(NULL),
        m_valid(false) {}
        
        LinesRenderer::~LinesRenderer() {
            delete m_vertexArray;
            m_vertexArray = NULL;
        }
        
        void LinesRenderer::render(Vbo& vbo, RenderContext& context) {
            Renderer::SetVboState activateVbo(vbo, Renderer::Vbo::VboActive);
            
            if (!m_valid) {
                delete m_vertexArray;
                m_vertexArray = NULL;
                
                if (!m_vertices.empty()) {
                    Renderer::SetVboState mapVbo(vbo, Renderer::Vbo::VboMapped);

                    m_vertexArray = new VertexArray(vbo, GL_LINES, static_cast<unsigned int>(m_vertices.size()), Attribute::position3f(), 0);
                    m_vertexArray->addAttributes(m_vertices);
                    m_vertices.clear();
                    m_valid = true;
                }
            }
            
            if (m_vertexArray != NULL) {
                Renderer::glSetEdgeOffset(0.3f);
                
                Renderer::ActivateShader handleShader(context.shaderManager(), Renderer::Shaders::HandleShader);
                
                glDisable(GL_DEPTH_TEST);
                handleShader.currentShader().setUniformVariable("Color", m_occludedColor);
                m_vertexArray->render();
                glEnable(GL_DEPTH_TEST);
                
                handleShader.currentShader().setUniformVariable("Color", m_color);
                m_vertexArray->render();
                
                Renderer::glResetEdgeOffset();
            }
        }
    }
}
