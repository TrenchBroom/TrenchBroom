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

#include "BrushFigure.h"

#include "GL/GLee.h"
#include "Controller/Options.h"
#include "Model/Assets/Texture.h"
#include "Model/Map/Brush.h"
#include "Model/Map/BrushGeometry.h"
#include "Model/Map/Face.h"
#include "Model/Preferences.h"
#include "Renderer/GridRenderer.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/Vbo.h"

namespace TrenchBroom {
    namespace Renderer {
        unsigned int VertexSize = 3 * sizeof(GLfloat);
        unsigned int CoordSize = 2 * sizeof(GLfloat);
        unsigned int EdgeVertexSize = VertexSize;
        unsigned int FaceVertexSize = VertexSize + CoordSize + CoordSize;
        
        BrushFigure::BrushFigure() : Figure(), m_faceBlock(NULL), m_edgeBlock(NULL), m_edgeVertexCount(0), m_valid(false), m_dummyTexture(new Model::Assets::Texture("dummy")) {}
        
        BrushFigure::~BrushFigure() {
            if (m_edgeBlock != NULL) {
                m_edgeBlock->freeBlock();
                m_edgeBlock = NULL;
            }
            
            if (m_faceBlock != NULL) {
                m_faceBlock->freeBlock();
                m_faceBlock = NULL;
            }
            
            if (m_dummyTexture != NULL) {
                delete m_dummyTexture;
                m_dummyTexture = NULL;
            }
        }
        
        void BrushFigure::render(RenderContext& context, Vbo& vbo) {
            if (m_brushes.empty())
                return;
            
            // create VBOs
            if (!m_valid) {
                unsigned int faceVertexCount = 0;
                m_edgeVertexCount = 0;
                for (unsigned int i = 0; i < m_brushes.size(); i++) {
                    Model::Brush* brush = m_brushes[i];
                    m_edgeVertexCount += (2 * brush->geometry->edges.size());
                    for (unsigned int j = 0; j < brush->faces.size(); j++) {
                        Model::Face* face = brush->faces[j];
                        faceVertexCount += (3 * face->side->vertices.size() - 6);
                    }
                }
                
                if (m_faceBlock != NULL) {
                    m_faceBlock->freeBlock();
					m_faceBlock = NULL;
				}
                if (m_edgeBlock != NULL){
                    m_edgeBlock->freeBlock();
					m_edgeBlock = NULL;
				}

                m_faceBlock = vbo.allocBlock(faceVertexCount * FaceVertexSize);
                m_edgeBlock = vbo.allocBlock(m_edgeVertexCount * EdgeVertexSize);
                
                vbo.map();
                
                unsigned int edgeOffset = 0;
                unsigned int faceOffset = 0;
                
                for (unsigned int i = 0; i < m_brushes.size(); i++) {
                    Model::Brush* brush = m_brushes[i];
                    
                    for (unsigned int j = 0; j < brush->geometry->edges.size(); j++) {
                        Model::Edge* edge = brush->geometry->edges[j];
                        edgeOffset = m_edgeBlock->writeVec(edge->start->position, edgeOffset);
                        edgeOffset = m_edgeBlock->writeVec(edge->end->position, edgeOffset);
                    }
                    
                    for (unsigned int j = 0; j < brush->faces.size(); j++) {
                        Model::Face* face = brush->faces[j];
                        const std::vector<Model::Vertex*>& vertices = face->side->vertices;
                        const std::vector<Vec2f>& texCoords = face->texCoords();
                        const std::vector<Vec2f>& gridCoords = face->gridCoords();
                        for (unsigned int k = 1; k < vertices.size() - 1; k++) {
                            faceOffset = m_faceBlock->writeVec(gridCoords[0], faceOffset);
                            faceOffset = m_faceBlock->writeVec(texCoords[0], faceOffset);
                            faceOffset = m_faceBlock->writeVec(vertices[0]->position, faceOffset);
                            
                            faceOffset = m_faceBlock->writeVec(gridCoords[k], faceOffset);
                            faceOffset = m_faceBlock->writeVec(texCoords[k], faceOffset);
                            faceOffset = m_faceBlock->writeVec(vertices[k]->position, faceOffset);
                            
                            faceOffset = m_faceBlock->writeVec(gridCoords[k + 1], faceOffset);
                            faceOffset = m_faceBlock->writeVec(texCoords[k + 1], faceOffset);
                            faceOffset = m_faceBlock->writeVec(vertices[k + 1]->position, faceOffset);
                        }
                    }
                }
                
                vbo.unmap();
                m_valid = true;
            }
            
            // render faces
            glPolygonMode(GL_FRONT, GL_FILL);
            glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
            glEnableClientState(GL_VERTEX_ARRAY);
            
            glActiveTexture(GL_TEXTURE2);
            glEnable(GL_TEXTURE_2D);
            context.gridRenderer.activate(context.grid);
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
            
            glClientActiveTexture(GL_TEXTURE2);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer(2, GL_FLOAT, FaceVertexSize, reinterpret_cast<const GLvoid*>(m_faceBlock->address));
            
            const Vec4f& selectedFaceColor = context.preferences.selectedFaceColor();
            GLfloat color[4] = {selectedFaceColor.x, selectedFaceColor.y, selectedFaceColor.z, selectedFaceColor.w};
            
            glActiveTexture(GL_TEXTURE1);
            glEnable(GL_TEXTURE_2D);
            m_dummyTexture->activate();
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
            glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, color);
            glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PREVIOUS);
            glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PREVIOUS);
            glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_CONSTANT);
            glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 2);
            
            bool textureActive = true;
            glActiveTexture(GL_TEXTURE0);
            glEnable(GL_TEXTURE_2D);
            glSetBrightness(context.preferences.brightness(), false);
            
            glClientActiveTexture(GL_TEXTURE0);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer(2, GL_FLOAT, FaceVertexSize, reinterpret_cast<const GLvoid*>(m_faceBlock->address + CoordSize));
            
            glVertexPointer(3, GL_FLOAT, FaceVertexSize, reinterpret_cast<const GLvoid*>(m_faceBlock->address + CoordSize + CoordSize));
            
            unsigned int index = 0;
            for (unsigned int i = 0; i < m_brushes.size(); i++) {
                Model::Brush* brush = m_brushes[i];
                for (unsigned int j = 0; j < brush->faces.size(); j++) {
                    Model::Face* face = brush->faces[j];
                    if (face->texture == NULL) {
                        if (textureActive) {
                            glDisable(GL_TEXTURE_2D);
                            textureActive = false;
                        }
                        glColorV4f(context.preferences.faceColor());
                    } else {
                        if (!textureActive) {
                            glEnable(GL_TEXTURE_2D);
                            textureActive = true;
                        }
                        face->texture->activate();
                    }
                    
                    unsigned int vertexCount = static_cast<unsigned int>(3 * face->side->vertices.size() - 6);
                    glDrawArrays(GL_TRIANGLES, index, vertexCount);
                    index += vertexCount;
                    
                    if (face->texture != NULL)
                        face->texture->deactivate();
                }
            }
            
            if (textureActive)
                glDisable(GL_TEXTURE_2D);

            glActiveTexture(GL_TEXTURE1);
            m_dummyTexture->deactivate();
            glDisable(GL_TEXTURE_2D);
            glActiveTexture(GL_TEXTURE0);
            
            glActiveTexture(GL_TEXTURE2);
            context.gridRenderer.deactivate();
            glDisable(GL_TEXTURE_2D);
            glActiveTexture(GL_TEXTURE0);
            
            // render edges
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);

            glVertexPointer(3, GL_FLOAT, 0, reinterpret_cast<const GLvoid*>(m_edgeBlock->address));
            
            glDisable(GL_DEPTH_TEST);
            glColorV4f(context.preferences.hiddenSelectedEdgeColor());
            glDrawArrays(GL_LINES, 0, m_edgeVertexCount);
            
            glEnable(GL_DEPTH_TEST);
            glSetEdgeOffset(0.2f);
            glDepthFunc(GL_LEQUAL);
            glColorV4f(context.preferences.selectedEdgeColor());
            glDrawArrays(GL_LINES, 0, m_edgeVertexCount);
            glDepthFunc(GL_LESS);
            glResetEdgeOffset();
            
            glPopClientAttrib();       
        }

        void BrushFigure::setBrushes(const Model::BrushList& brushes) {
            m_brushes = brushes;
            m_valid = false;
        }
    }
}
