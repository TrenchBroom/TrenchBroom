/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "EdgeRenderer.h"

#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/ShaderProgram.h"

namespace TrenchBroom {
    namespace Renderer {
        EdgeRenderer::EdgeRenderer() :
        m_useColor(false),
        m_prepared(false) {}
        
        EdgeRenderer::EdgeRenderer(const VertexArray& vertexArray) :
        m_vertexArray(vertexArray),
        m_useColor(false),
        m_prepared(false) {}
        
        EdgeRenderer::EdgeRenderer(const EdgeRenderer& other) {
            m_vertexArray = other.m_vertexArray;
            m_color = other.m_color;
            m_useColor = other.m_useColor;
            m_prepared = other.m_prepared;
        }
        
        EdgeRenderer& EdgeRenderer::operator= (EdgeRenderer other) {
            using std::swap;
            swap(*this, other);
            return *this;
        }
        
        void swap(EdgeRenderer& left, EdgeRenderer& right) {
            using std::swap;
            swap(left.m_vertexArray, right.m_vertexArray);
            swap(left.m_color, right.m_color);
            swap(left.m_useColor, right.m_useColor);
            swap(left.m_prepared, right.m_prepared);
        }

        void EdgeRenderer::setUseColor(bool useColor) {
            m_useColor = useColor;
        }

        void EdgeRenderer::setColor(const Color& color) {
            m_color = color;
        }

        void EdgeRenderer::doPrepare(Vbo& vbo) {
            if (!m_prepared) {
                m_vertexArray.prepare(vbo);
                m_prepared = true;
            }
        }
        
        void EdgeRenderer::doRender(RenderContext& context) {
            assert(m_prepared);
            if (m_vertexArray.vertexCount() == 0)
                return;
            
            if (m_useColor) {
                ActiveShader shader(context.shaderManager(), Shaders::VaryingPUniformCShader);
                shader.set("Color", m_color);
                m_vertexArray.render();
            } else {
                ActiveShader shader(context.shaderManager(), Shaders::VaryingPCShader);
                m_vertexArray.render();
            }
        }

        RenderEdges::RenderEdges(EdgeRenderer& edgeRenderer, const bool useColor, const Color& edgeColor) :
        m_edgeRenderer(edgeRenderer),
        m_useColor(useColor),
        m_edgeColor(edgeColor) {}
        
        RenderEdges::~RenderEdges() {}

        void RenderEdges::doPrepare(Vbo& vbo) {
            m_edgeRenderer.prepare(vbo);
        }
        
        RenderUnoccludedEdges::RenderUnoccludedEdges(EdgeRenderer& edgeRenderer, const bool useColor, const Color& edgeColor) :
        RenderEdges(edgeRenderer, useColor, edgeColor) {}

        void RenderUnoccludedEdges::doRender(RenderContext& renderContext) {
            glSetEdgeOffset(0.02f);
            m_edgeRenderer.setUseColor(true);
            m_edgeRenderer.setColor(m_edgeColor);
            m_edgeRenderer.render(renderContext);
            glResetEdgeOffset();
        }
        
        RenderOccludedEdges::RenderOccludedEdges(EdgeRenderer& edgeRenderer, const bool useColor, const Color& edgeColor) :
        RenderEdges(edgeRenderer, useColor, edgeColor) {}

        void RenderOccludedEdges::doRender(RenderContext& renderContext) {
            glDisable(GL_DEPTH_TEST);
            m_edgeRenderer.setUseColor(true);
            m_edgeRenderer.setColor(m_edgeColor);
            m_edgeRenderer.render(renderContext);
            glResetEdgeOffset();
            glEnable(GL_DEPTH_TEST);
        }
    }
}
