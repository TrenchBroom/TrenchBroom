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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "MoveIndicatorRenderer.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderContext.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Transformation.h"
#include "Renderer/VertexArray.h"

namespace TrenchBroom {
    namespace Renderer {
        const float MoveIndicatorRenderer::HalfWidth = 1.5f;
        const float MoveIndicatorRenderer::Height = 5.0f;

        MoveIndicatorRenderer::MoveIndicatorRenderer() :
        m_vbo(0xFF) {}

        void MoveIndicatorRenderer::render(RenderContext& renderContext, const Vec3f& position, const Direction direction) {
            const float offset = direction == HorizontalXY ? HalfWidth + 1.0f : 1.0f;
            
            Vertex::List triangleVerts;
            Vertex::List outlineVerts;
            
            if (direction == Vertical || direction != HorizontalY) {
                makeSolidYArrows(offset, triangleVerts);
                makeOutlineYArrows(offset, outlineVerts);
            }
            
            if (direction != Vertical && direction != HorizontalX) {
                makeSolidXArrows(offset, triangleVerts);
                makeOutlineXArrows(offset, outlineVerts);
            }
            
            VertexArray triangleArray = VertexArray::swap(m_vbo, GL_TRIANGLES, triangleVerts);
            VertexArray outlineArray = VertexArray::swap(m_vbo, GL_LINES, outlineVerts);
            
            SetVboState vboState(m_vbo);
            vboState.mapped();
            triangleArray.prepare();
            outlineArray.prepare();
            
            vboState.active();
            
            Mat4x4f matrix = translationMatrix(position);
            if (direction == Vertical)
                matrix *= renderContext.camera().verticalBillboardMatrix();
            MultiplyModelMatrix applyTransformation(renderContext.transformation(), matrix);

            PreferenceManager& prefs = PreferenceManager::instance();
            ActiveShader shader(renderContext.shaderManager(), Shaders::HandleShader);
            
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            
            shader.set("Color", prefs.get(Preferences::MoveIndicatorOutlineColor));
            outlineArray.render();
            shader.set("Color", prefs.get(Preferences::MoveIndicatorFillColor));
            triangleArray.render();

            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
        }

        void MoveIndicatorRenderer::makeSolidXArrows(const float offset, Vertex::List& vertices) const {
            vertices.push_back(Vertex(Vec2f(offset, +HalfWidth)));
            vertices.push_back(Vertex(Vec2f(offset + Height, 0.0f)));
            vertices.push_back(Vertex(Vec2f(offset, -HalfWidth)));

            vertices.push_back(Vertex(Vec2f(-offset, -HalfWidth)));
            vertices.push_back(Vertex(Vec2f(-offset - Height, 0.0f)));
            vertices.push_back(Vertex(Vec2f(-offset, +HalfWidth)));
        }
        
        void MoveIndicatorRenderer::makeSolidYArrows(const float offset, Vertex::List& vertices) const {
            vertices.push_back(Vertex(Vec2f(-HalfWidth, offset)));
            vertices.push_back(Vertex(Vec2f(0.0f, offset + Height)));
            vertices.push_back(Vertex(Vec2f(+HalfWidth, offset)));

            vertices.push_back(Vertex(Vec2f(+HalfWidth, -offset)));
            vertices.push_back(Vertex(Vec2f(0.0f, -offset - Height)));
            vertices.push_back(Vertex(Vec2f(-HalfWidth, -offset)));
        }
        
        void MoveIndicatorRenderer::makeOutlineXArrows(const float offset, Vertex::List& vertices) const {
            vertices.push_back(Vertex(Vec2f(offset, HalfWidth)));
            vertices.push_back(Vertex(Vec2f(offset + Height, 0.0f)));
            vertices.push_back(Vertex(Vec2f(offset + Height, 0.0f)));
            vertices.push_back(Vertex(Vec2f(offset, -HalfWidth)));
            vertices.push_back(Vertex(Vec2f(offset, -HalfWidth)));
            vertices.push_back(Vertex(Vec2f(offset, HalfWidth)));

            vertices.push_back(Vertex(Vec2f(-offset, -HalfWidth)));
            vertices.push_back(Vertex(Vec2f(-offset - Height, 0.0f)));
            vertices.push_back(Vertex(Vec2f(-offset - Height, 0.0f)));
            vertices.push_back(Vertex(Vec2f(-offset, HalfWidth)));
            vertices.push_back(Vertex(Vec2f(-offset, HalfWidth)));
            vertices.push_back(Vertex(Vec2f(-offset, -HalfWidth)));
        }
        
        void MoveIndicatorRenderer::makeOutlineYArrows(const float offset, Vertex::List& vertices) const {
            vertices.push_back(Vertex(Vec2f(-HalfWidth, offset)));
            vertices.push_back(Vertex(Vec2f(0.0f, offset + Height)));
            vertices.push_back(Vertex(Vec2f(0.0f, offset + Height)));
            vertices.push_back(Vertex(Vec2f(HalfWidth, offset)));
            vertices.push_back(Vertex(Vec2f(HalfWidth, offset)));
            vertices.push_back(Vertex(Vec2f(-HalfWidth, offset)));
            
            vertices.push_back(Vertex(Vec2f(HalfWidth, -offset)));
            vertices.push_back(Vertex(Vec2f(0.0f, -offset - Height)));
            vertices.push_back(Vertex(Vec2f(0.0f, -offset - Height)));
            vertices.push_back(Vertex(Vec2f(-HalfWidth, -offset)));
            vertices.push_back(Vertex(Vec2f(-HalfWidth, -offset)));
            vertices.push_back(Vertex(Vec2f(HalfWidth, -offset)));
        }
    }
}
