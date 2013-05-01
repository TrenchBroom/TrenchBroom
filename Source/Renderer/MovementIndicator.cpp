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
        const float MovementIndicator::Width2 = 1.5f;
        const float MovementIndicator::Height = 5.0f;
        
        void MovementIndicator::validate(Vbo& vbo) {
            delete m_outline;
            delete m_triangles;
            
            const float offset = m_direction == Horizontal ? Width2 + 1.0f : 1.0f;
            
            Vec2f::List triangles;
            Vec2f::List outline;
            
            if (m_direction == Vertical || m_direction != HorizontalY)
                buildYArrows(offset, triangles, outline);
            
            if (m_direction != Vertical && m_direction != HorizontalX)
                buildXArrows(offset, triangles, outline);
            
            m_triangles = new VertexArray(vbo, GL_TRIANGLES, triangles.size(), Attribute::position2f(), 0);
            m_outline = new VertexArray(vbo, GL_LINES, outline.size(), Attribute::position2f(), 0);
            
            SetVboState mapVbo(vbo, Vbo::VboMapped);
            m_triangles->addAttributes(triangles);
            m_outline->addAttributes(outline);
        }
        
        void MovementIndicator::buildXArrows(const float offset, Vec2f::List& triangles, Vec2f::List& outline) const {
            triangles.push_back(Vec2f(offset, Width2));
            triangles.push_back(Vec2f(offset + Height, 0.0f));
            triangles.push_back(Vec2f(offset, -Width2));

            outline.push_back(Vec2f(offset, Width2));
            outline.push_back(Vec2f(offset + Height, 0.0f));
            outline.push_back(Vec2f(offset + Height, 0.0f));
            outline.push_back(Vec2f(offset, -Width2));
            outline.push_back(Vec2f(offset, -Width2));
            outline.push_back(Vec2f(offset, Width2));
            
            triangles.push_back(Vec2f(-offset, -Width2));
            triangles.push_back(Vec2f(-offset - Height, 0.0f));
            triangles.push_back(Vec2f(-offset, Width2));
            
            outline.push_back(Vec2f(-offset, -Width2));
            outline.push_back(Vec2f(-offset - Height, 0.0f));
            outline.push_back(Vec2f(-offset - Height, 0.0f));
            outline.push_back(Vec2f(-offset, Width2));
            outline.push_back(Vec2f(-offset, Width2));
            outline.push_back(Vec2f(-offset, -Width2));
        }
        
        void MovementIndicator::buildYArrows(const float offset, Vec2f::List& triangles, Vec2f::List& outline) const {
            triangles.push_back(Vec2f(-Width2, offset));
            triangles.push_back(Vec2f(0.0f, offset + Height));
            triangles.push_back(Vec2f(Width2, offset));
            
            outline.push_back(Vec2f(-Width2, offset));
            outline.push_back(Vec2f(0.0f, offset + Height));
            outline.push_back(Vec2f(0.0f, offset + Height));
            outline.push_back(Vec2f(Width2, offset));
            outline.push_back(Vec2f(Width2, offset));
            outline.push_back(Vec2f(-Width2, offset));
            
            triangles.push_back(Vec2f(Width2, -offset));
            triangles.push_back(Vec2f(0.0f, -offset - Height));
            triangles.push_back(Vec2f(-Width2, -offset));
            
            outline.push_back(Vec2f(Width2, -offset));
            outline.push_back(Vec2f(0.0f, -offset - Height));
            outline.push_back(Vec2f(0.0f, -offset - Height));
            outline.push_back(Vec2f(-Width2, -offset));
            outline.push_back(Vec2f(-Width2, -offset));
            outline.push_back(Vec2f(Width2, -offset));
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
            
            if (!m_valid)
                validate(vbo);

            assert(m_outline != NULL);
            assert(m_triangles != NULL);

            Mat4f matrix = translationMatrix(m_position);
            if (m_direction == Vertical)
                matrix *= context.camera().billboardMatrix(true);

            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            ActivateShader shader(context.shaderManager(), Shaders::HandleShader);
            renderArrow(matrix, shader, context);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
        }

        void MovementIndicator::renderArrow(const Mat4f& matrix, ActivateShader& shader, RenderContext& context) const {
            ApplyModelMatrix applyMatrix(context.transformation(), matrix);
            shader.setUniformVariable("Color", m_outlineColor);
            m_outline->render();
            shader.setUniformVariable("Color", m_fillColor);
            m_triangles->render();
        }
    }
}
