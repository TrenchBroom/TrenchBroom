/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "EdgeRenderer.h"

#include "Renderer/RenderContext.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/ShaderProgram.h"

namespace TrenchBroom {
    namespace Renderer {
        EdgeRenderer::EdgeRenderer() {}
        
        EdgeRenderer::EdgeRenderer(Vbo& vbo, const VertexSpecs::P3::Vertex::List& vertices, const Color& color) :
        m_vertexArray(vbo, GL_LINES, vertices),
        m_color(color),
        m_useColor(true) {}
        
        EdgeRenderer::EdgeRenderer(Vbo& vbo, const VertexSpecs::P3C4::Vertex::List& vertices) :
        m_vertexArray(vbo, GL_LINES, vertices),
        m_useColor(false) {}
        
        void EdgeRenderer::render(RenderContext& context) {
            if (m_useColor) {
                ActiveShader shader(context.shaderManager(), Shaders::EdgeShader);
                shader.set("Color", m_color);
                m_vertexArray.render();
            } else {
                ActiveShader shader(context.shaderManager(), Shaders::ColoredEdgeShader);
                m_vertexArray.render();
            }
        }
    }
}
