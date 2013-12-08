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

#include "TriangleRenderer.h"

#include "Renderer/Camera.h"
#include "Renderer/RenderContext.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/ShaderProgram.h"

namespace TrenchBroom {
    namespace Renderer {
        TriangleRenderer::TriangleRenderer() :
        m_useColor(false),
        m_applyTinting(false),
        m_prepared(false) {}
        
        TriangleRenderer::TriangleRenderer(const VertexArray& vertexArray) :
        m_vbo(new Vbo(vertexArray.size())),
        m_vertexArray(vertexArray),
        m_useColor(false),
        m_applyTinting(false),
        m_prepared(false) {}
        
        TriangleRenderer::TriangleRenderer(const TriangleRenderer& other) {
            m_vertexArray = other.m_vertexArray;
            m_vbo = other.m_vbo;
            m_color = other.m_color;
            m_useColor = other.m_useColor;
            m_prepared = other.m_prepared;
        }
        
        TriangleRenderer& TriangleRenderer::operator= (TriangleRenderer other) {
            using std::swap;
            swap(*this, other);
            return *this;
        }
        
        void swap(TriangleRenderer& left, TriangleRenderer& right) {
            using std::swap;
            swap(left.m_vbo, right.m_vbo);
            swap(left.m_vertexArray, right.m_vertexArray);
            swap(left.m_color, right.m_color);
            swap(left.m_useColor, right.m_useColor);
            swap(left.m_prepared, right.m_prepared);
        }
        
        void TriangleRenderer::setUseColor(const bool useColor) {
            m_useColor = useColor;
        }

        void TriangleRenderer::setColor(const Color& color) {
            m_color = color;
        }
        
        void TriangleRenderer::setApplyTinting(const bool applyTinting) {
            m_applyTinting = applyTinting;
        }

        void TriangleRenderer::setTintColor(const Color& tintColor) {
            m_tintColor = tintColor;
        }

        void TriangleRenderer::render(RenderContext& context) {
            if (m_vertexArray.vertexCount() == 0)
                return;
            
            SetVboState setVboState(*m_vbo);
            setVboState.active();
            if (!m_prepared)
                prepare();
            
            ActiveShader shader(context.shaderManager(), Shaders::TriangleShader);
            shader.set("ApplyTinting", m_applyTinting);
            shader.set("TintColor", m_tintColor);
            shader.set("UseColor", m_useColor);
            shader.set("Color", m_color);
            shader.set("CameraPosition", context.camera().position());
            m_vertexArray.render();
        }
        
        void TriangleRenderer::prepare() {
            assert(!m_prepared);
            SetVboState setVboState(*m_vbo);
            setVboState.mapped();
            m_vertexArray.prepare(*m_vbo);
            m_prepared = true;
        }
    }
}
