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

#include "HandleFigure.h"

#include "GL/GLee.h"
#include "Model/Preferences.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/Vbo.h"

namespace TrenchBroom {
    const unsigned int VertexSize = 3 * sizeof(GLfloat);
    
    namespace Renderer {
        HandleFigure::~HandleFigure() {
            if (m_vboBlock != NULL) {
                m_vboBlock->freeBlock();
                m_vboBlock = NULL;
            }
        }
        
        void HandleFigure::setPositions(const Vec3fList& positions) {
            m_positions = positions;
            m_valid = false;
        }
        
        void HandleFigure::setColor(const Vec4f& color) {
            m_color = color;
        }
        
        void HandleFigure::setHiddenColor(const Vec4f& hiddenColor) {
            m_hiddenColor = hiddenColor;
        }
        
        void HandleFigure::render(RenderContext& context, Vbo& vbo) {
            if (m_positions.empty())
                return;
            
            unsigned int vertexCount = 6 * 6 * static_cast<unsigned int>(m_positions.size());
            if (!m_valid) {
                if (m_vboBlock != NULL)
                    m_vboBlock->freeBlock();
                
                Model::Preferences& prefs = *Model::Preferences::sharedPreferences;
                float handleSize = prefs.vertexHandleSize();
                m_vboBlock = vbo.allocBlock(vertexCount * VertexSize);

                vbo.map();
                
                unsigned int offset = 0;
                for (unsigned int i = 0; i < m_positions.size(); i++) {
                    const Vec3f& position = m_positions[i];
                    BBox handleBounds(position, handleSize);
                    std::vector<Vec3f> handleVertices = bboxTriangleVertices(handleBounds);
                    offset = m_vboBlock->writeVecs(handleVertices, offset);
                }
                
                vbo.unmap();
                m_valid = true;
            }

            glFrontFace(GL_CCW);
            glPolygonMode(GL_FRONT, GL_FILL);
            glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
            glEnableClientState(GL_VERTEX_ARRAY);
            glVertexPointer(3, GL_FLOAT, VertexSize, reinterpret_cast<const GLvoid*>(m_vboBlock->address));
            
            glDisable(GL_DEPTH_TEST);
            glColorV4f(m_hiddenColor);
            glDrawArrays(GL_TRIANGLES, 0, vertexCount);
            
            glEnable(GL_DEPTH_TEST);
            glColorV4f(m_color);
            glDrawArrays(GL_TRIANGLES, 0, vertexCount);
            
            glPopClientAttrib();
        }
    }
}
