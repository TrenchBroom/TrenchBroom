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

#include "PointTraceFigure.h"

#include "Renderer/RenderContext.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/Shader/ShaderProgram.h"
#include "Renderer/Vbo.h"
#include "Renderer/VertexArray.h"

namespace TrenchBroom {
    namespace Renderer {
        PointTraceFigure::PointTraceFigure(const Vec3f::List& points) :
        m_points(points),
        m_vertexArray(NULL) {}
        
        PointTraceFigure::~PointTraceFigure() {
            delete m_vertexArray;
            m_vertexArray = NULL;
        }
        
        void PointTraceFigure::render(Vbo& vbo, RenderContext& context) {
            SetVboState activateVbo(vbo, Vbo::VboActive);
            if (m_vertexArray == NULL) {
                m_vertexArray = new VertexArray(vbo, GL_LINE_STRIP, static_cast<unsigned int>(m_points.size()), Attribute::position3f(), 0);

                SetVboState mapVbo(vbo, Vbo::VboMapped);
                m_vertexArray->addAttributes(m_points);
            }
            
            ActivateShader shader(context.shaderManager(), Shaders::HandleShader);
            
            glDisable(GL_DEPTH_TEST);
            shader.currentShader().setUniformVariable("Color", Color(m_color, 0.5f));
            m_vertexArray->render();
            glEnable(GL_DEPTH_TEST);
            shader.currentShader().setUniformVariable("Color", m_color);
            m_vertexArray->render();
        }
    }
}
