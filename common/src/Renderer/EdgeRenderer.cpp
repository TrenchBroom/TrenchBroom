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
#include "Renderer/Shaders.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/ShaderProgram.h"

namespace TrenchBroom {
    namespace Renderer {
        EdgeRenderer::EdgeRenderer() :
        m_useColor(false),
        m_prepared(false) {}
        
        EdgeRenderer::EdgeRenderer(const VertexArray& vertexArray, const IndexArray& indexArray) :
        m_vertexArray(vertexArray),
        m_indexArray(indexArray),
        m_useColor(false),
        m_prepared(false) {}
        
        EdgeRenderer::EdgeRenderer(const VertexArray& vertexArray, const PrimType primType) :
        m_vertexArray(vertexArray),
        m_indexArray(primType, 0, m_vertexArray.vertexCount()),
        m_useColor(false),
        m_prepared(false) {}

        EdgeRenderer::EdgeRenderer(const EdgeRenderer& other) :
        m_vertexArray(other.m_vertexArray),
        m_color(other.m_color),
        m_useColor(other.m_useColor),
        m_prepared(other.m_prepared) {}
        
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
                m_indexArray.render(m_vertexArray);
            } else {
                ActiveShader shader(context.shaderManager(), Shaders::VaryingPCShader);
                m_indexArray.render(m_vertexArray);
            }
        }

        RenderEdges::RenderEdges(const TypedReference<EdgeRenderer>& edgeRenderer) :
        m_edgeRenderer(edgeRenderer),
        m_onTop(false),
        m_useColor(false),
        m_offset(0.0f),
        m_width(1.0f) {}
        
        void RenderEdges::setOnTop(const bool onTop) {
            m_onTop = onTop;
        }
        
        void RenderEdges::setColor(const Color& color) {
            m_useColor = true;
            m_edgeColor = color;
        }

        void RenderEdges::setWidth(const float width) {
            m_width = width;
        }
        
        void RenderEdges::setOffset(const float offset) {
            m_offset = offset;
        }

        void RenderEdges::setRenderOccluded(const float offset) {
            setOnTop(true);
            setOffset(offset);
        }

        void RenderEdges::doPrepare(Vbo& vbo) {
            EdgeRenderer& edgeRenderer = m_edgeRenderer.get();
            edgeRenderer.prepare(vbo);
        }
        
        void RenderEdges::doRender(RenderContext& renderContext) {
            if (m_offset != 0.0f)
                glSetEdgeOffset(m_offset);
            
            if (m_width != 1.0f)
                glLineWidth(m_width);
            
            if (m_onTop)
                glDisable(GL_DEPTH_TEST);
            
            EdgeRenderer& edgeRenderer = m_edgeRenderer.get();
            edgeRenderer.setUseColor(m_useColor);
            edgeRenderer.setColor(m_edgeColor);
            edgeRenderer.render(renderContext);
            
            if (m_onTop)
                glEnable(GL_DEPTH_TEST);
            
            if (m_width != 1.0f)
                glLineWidth(1.0f);
            
            if (m_offset != 0.0f)
                glResetEdgeOffset();
        }
    }
}
