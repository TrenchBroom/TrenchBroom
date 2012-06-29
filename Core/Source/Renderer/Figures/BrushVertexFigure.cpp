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

#include "BrushVertexFigure.h"

#include "GL/GLee.h"
#include "Model/Map/Brush.h"
#include "Model/Map/BrushGeometry.h"
#include "Model/Preferences.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/Vbo.h"

namespace TrenchBroom {
    const unsigned int VertexSize = 3 * sizeof(GLfloat);
    
    namespace Renderer {
        BrushVertexFigure::~BrushVertexFigure() {
            if (m_vboBlock != NULL) {
                m_vboBlock->freeBlock();
                m_vboBlock = NULL;
            }
        }
        
        void BrushVertexFigure::setBrushes(const Model::BrushList& brushes) {
            m_brushes = brushes;
            m_valid = false;
        }
        
        void BrushVertexFigure::render(RenderContext& context, Vbo& vbo) {
            if (m_brushes.empty())
                return;
            
            Model::Preferences& prefs = *Model::Preferences::sharedPreferences;
            
            if (!m_valid) {
                unsigned int brushVertexCount = 0;
                for (unsigned int i = 0; i < m_brushes.size(); i++)
                    brushVertexCount += m_brushes[i]->geometry->vertices.size();
                
                m_vertexCount = 6 * 6 * brushVertexCount;
                
                if (m_vboBlock != NULL)
                    m_vboBlock->freeBlock();
                
                m_vboBlock = vbo.allocBlock(m_vertexCount * VertexSize);
                vbo.map();
                
                float handleSize = prefs.vertexHandleSize();
                
                unsigned int offset = 0;
                for (unsigned int i = 0; i < m_brushes.size(); i++) {
                    Model::Brush* brush = m_brushes[i];
                    for (unsigned int j = 0; j < brush->geometry->vertices.size(); j++) {
                        Model::Vertex* vertex = brush->geometry->vertices[j];
                        BBox handleBounds(vertex->position, handleSize);
                        std::vector<Vec3f> handleVertices = bboxTriangleVertices(handleBounds);
                        offset = m_vboBlock->writeVecs(handleVertices, offset);
                    }
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
            glColorV4f(prefs.hiddenSelectedEdgeColor());
            glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);
            
            glEnable(GL_DEPTH_TEST);
            glColorV4f(prefs.selectedEdgeColor());
            glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);
            
            glPopClientAttrib();
        }
    }
}
