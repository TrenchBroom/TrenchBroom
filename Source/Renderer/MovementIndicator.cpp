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

#include "MovementIndicator.h"

#include "Renderer/ApplyMatrix.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/Shader/Shader.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/Vbo.h"
#include "Renderer/VertexArray.h"

#include <cassert>

namespace TrenchBroom {
    namespace Renderer {
        void MovementIndicator::renderArrow(const Mat4f& matrix, ShaderProgram& shader, RenderContext& context) const {
            ApplyMatrix applyMatrix(context.transformation(), matrix);
            shader.setUniformVariable("Color", m_outlineColor);
            m_outline->render();
            shader.setUniformVariable("Color", m_fillColor);
            m_triangles->render();
        }
        
        MovementIndicator::MovementIndicator() :
        m_direction(Horizontal),
        m_outlineColor(Color(1.0f, 1.0f, 1.0f, 1.0f)),
        m_fillColor(Color(0.0f, 0.0f, 0.0f, 0.5f)),
        m_outline(NULL),
        m_triangles(NULL),
        m_valid(false) {}
        
        MovementIndicator::~MovementIndicator() {
            delete m_outline;
            m_outline = NULL;
            delete m_triangles;
            m_triangles = NULL;
        }

        void MovementIndicator::render(Vbo& vbo, RenderContext& context) {
            SetVboState activateVbo(vbo, Vbo::VboActive);
            if (!m_valid) {
                delete m_outline;
                delete m_triangles;

                const float width2 = 1.5f;
                const float height = 5.0f;
                const float offset = m_direction == Horizontal ? width2 + 1.0f : 1.0f;
                
                Vec2f::List triangles;
                Vec2f::List outline;
                
                triangles.push_back(Vec2f(-width2, offset));
                triangles.push_back(Vec2f(0.0f, offset + height));
                triangles.push_back(Vec2f(width2, offset));
                
                outline.push_back(Vec2f(-width2, offset));
                outline.push_back(Vec2f(0.0f, offset + height));
                outline.push_back(Vec2f(0.0f, offset + height));
                outline.push_back(Vec2f(width2, offset));
                outline.push_back(Vec2f(width2, offset));
                outline.push_back(Vec2f(-width2, offset));
                
                triangles.push_back(Vec2f(width2, -offset));
                triangles.push_back(Vec2f(0.0f, -offset - height));
                triangles.push_back(Vec2f(-width2, -offset));
                
                outline.push_back(Vec2f(width2, -offset));
                outline.push_back(Vec2f(0.0f, -offset - height));
                outline.push_back(Vec2f(0.0f, -offset - height));
                outline.push_back(Vec2f(-width2, -offset));
                outline.push_back(Vec2f(-width2, -offset));
                outline.push_back(Vec2f(width2, -offset));

                if (m_direction != Vertical) {
                    triangles.push_back(Vec2f(offset, width2));
                    triangles.push_back(Vec2f(offset + height, 0.0f));
                    triangles.push_back(Vec2f(offset, -width2));
                    
                    outline.push_back(Vec2f(offset, width2));
                    outline.push_back(Vec2f(offset + height, 0.0f));
                    outline.push_back(Vec2f(offset + height, 0.0f));
                    outline.push_back(Vec2f(offset, -width2));
                    outline.push_back(Vec2f(offset, -width2));
                    outline.push_back(Vec2f(offset, width2));

                    triangles.push_back(Vec2f(-offset, -width2));
                    triangles.push_back(Vec2f(-offset - height, 0.0f));
                    triangles.push_back(Vec2f(-offset, width2));

                    outline.push_back(Vec2f(-offset, -width2));
                    outline.push_back(Vec2f(-offset - height, 0.0f));
                    outline.push_back(Vec2f(-offset - height, 0.0f));
                    outline.push_back(Vec2f(-offset, width2));
                    outline.push_back(Vec2f(-offset, width2));
                    outline.push_back(Vec2f(-offset, -width2));
                }

                unsigned int triangleVertexCount = static_cast<unsigned int>(triangles.size());
                unsigned int outlineVertexCount = static_cast<unsigned int>(outline.size());
                
                m_triangles = new VertexArray(vbo, GL_TRIANGLES, triangleVertexCount, Attribute::position2f(), 0);
                m_outline = new VertexArray(vbo, GL_LINES, outlineVertexCount, Attribute::position2f(), 0);
                
                SetVboState mapVbo(vbo, Vbo::VboMapped);
                m_triangles->addAttributes(triangles);
                m_outline->addAttributes(outline);
            }
            
            assert(m_outline != NULL);
            assert(m_triangles != NULL);
            
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            ActivateShader shader(context.shaderManager(), Shaders::HandleShader);
            
            Mat4f matrix;
            matrix.translate(m_position);
            if (m_direction != Horizontal)
                matrix *= context.camera().billboardMatrix(true);
            
            ApplyMatrix applyMatrix(context.transformation(), matrix);
            shader.currentShader().setUniformVariable("Color", m_fillColor);
            m_triangles->render();
            shader.currentShader().setUniformVariable("Color", m_outlineColor);
            m_outline->render();

            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
        }
    }
}
