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
            if (m_vbo != NULL) {
                delete m_vbo;
                m_vbo = NULL;
            }
        }
        
        void BrushVertexFigure::setBrushes(const Model::BrushList& brushes) {
            m_brushes = brushes;
            m_valid = false;
        }
        
        void BrushVertexFigure::render(RenderContext& context) {
            if (m_brushes.empty())
                return;
            
            if (!m_valid) {
                unsigned int brushVertexCount = 0;
                for (unsigned int i = 0; i < m_brushes.size(); i++)
                    brushVertexCount += m_brushes[i]->geometry->vertices.size();
                
                m_vertexCount = 6 * 6 * brushVertexCount;
                if (m_vbo == NULL)
                    m_vbo = new Vbo(GL_ARRAY_BUFFER, m_vertexCount * VertexSize);
                else
                    m_vbo->freeAllBlocks();
                
                VboBlock* block = m_vbo->allocBlock(m_vertexCount * VertexSize);
                m_vbo->activate();
                m_vbo->map();
                
                unsigned int offset = 0;
                for (unsigned int i = 0; i < m_brushes.size(); i++) {
                    Model::Brush* brush = m_brushes[i];
                    for (unsigned int j = 0; j < brush->geometry->vertices.size(); j++) {
                        Model::Vertex* vertex = brush->geometry->vertices[j];
                        BBox handleBounds(vertex->position, 2);
                        std::vector<Vec3f> handleVertices = bboxTriangleVertices(handleBounds);
                        offset = block->writeVecs(handleVertices, offset);
                    }
                }
                
                m_vbo->unmap();
                m_valid = true;
            } else {
                m_vbo->activate();
            }

            Model::Preferences& prefs = *Model::Preferences::sharedPreferences;
            
            glFrontFace(GL_CCW);
            glPolygonMode(GL_FRONT, GL_FILL);
            glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
            glEnableClientState(GL_VERTEX_ARRAY);
            glVertexPointer(3, GL_FLOAT, VertexSize, reinterpret_cast<const GLvoid*>(0));
            
            glDisable(GL_DEPTH_TEST);
            glColorV4f(prefs.hiddenSelectedEdgeColor());
            glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);
            
            glEnable(GL_DEPTH_TEST);
            glColorV4f(prefs.selectedEdgeColor());
            glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);
            
            glPopClientAttrib();
            m_vbo->deactivate();
        }
    }
}
