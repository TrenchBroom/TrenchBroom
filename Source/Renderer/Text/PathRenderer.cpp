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

#include "PathRenderer.h"

#include "Renderer/Vbo.h"
#include "Renderer/Text/PathTesselator.h"

namespace TrenchBroom {
    namespace Renderer {
        namespace Text {
            void PathRenderer::uploadMeshData(PathMeshPtr mesh, Vbo& vbo) {
                // upload mesh data into VBO and clear mesh data
                
                unsigned int vertexCount = mesh->vertexCount();
                const FloatBuffer& triangleSet = mesh->triangleSet();
                const PathMeshData& triangleStrips = mesh->triangleStrips();
                const PathMeshData& triangleFans = mesh->triangleFans();

                m_hasTriangleSet = !triangleSet.empty();
                m_hasTriangleStrips = !triangleStrips.empty();
                m_hasTriangleFans = !triangleFans.empty();
                
                m_block = vbo.allocBlock(2 * vertexCount * sizeof(GLfloat));
                assert(m_block != NULL);
                
                unsigned int offset = 0;
                if (m_hasTriangleSet) {
                    m_triangleSetIndex = static_cast<GLint>((m_block->address() + offset) / (2 * sizeof(GLfloat)));
                    m_triangleSetCount = static_cast<GLsizei>(triangleSet.size() / 2);
                    
                    const unsigned char* buffer = reinterpret_cast<const unsigned char*>(&triangleSet[0]);
                    unsigned int bufferSize = static_cast<unsigned int>(triangleSet.size()) * sizeof(GLfloat);
                    offset = m_block->writeBuffer(buffer, offset, bufferSize);
                }
                
                if (m_hasTriangleStrips) {
                    for (unsigned int i = 0; i < triangleStrips.size(); i++) {
                        FloatBuffer* strip = triangleStrips[i];
                        m_triangleStripIndices.push_back(static_cast<GLint>((m_block->address() + offset) / (2 * sizeof(float))));
                        m_triangleStripCounts.push_back(static_cast<GLsizei>(strip->size()) / 2);
                        
                        const unsigned char* buffer = reinterpret_cast<const unsigned char*>(&(*strip)[0]);
                        unsigned int bufferSize = static_cast<unsigned int>(strip->size()) * sizeof(float);
                        offset = m_block->writeBuffer(buffer, offset, bufferSize);
                    }
                }
                
                if (m_hasTriangleFans) {
                    for (unsigned int i = 0; i < triangleFans.size(); i++) {
                        FloatBuffer* fan = triangleFans[i];
                        m_triangleFanIndices.push_back(static_cast<GLint>((m_block->address() + offset) / (2 * sizeof(float))));
                        m_triangleFanCounts.push_back(static_cast<GLsizei>(fan->size()) / 2);
                        
                        const unsigned char* buffer = reinterpret_cast<const unsigned char*>(&(*fan)[0]);
                        unsigned int bufferSize = static_cast<unsigned int>(fan->size()) * sizeof(float);
                        offset = m_block->writeBuffer(buffer, offset, bufferSize);
                    }
                }
            }
            
            PathRenderer::PathRenderer(PathPtr path) :
            m_path(path),
            m_width(m_path->width()),
            m_height(m_path->height()),
            m_listId(0),
            m_block(NULL),
            m_hasTriangleSet(false),
            m_hasTriangleStrips(false),
            m_hasTriangleFans(false),
            m_triangleSetIndex(0),
            m_triangleSetCount(0) {}
            
            PathRenderer::~PathRenderer() {
                if (m_listId > 0) {
                    glDeleteLists(m_listId, 1);
                    m_listId = 0;
                }
                if (m_block != NULL) {
                    m_block->freeBlock();
                    m_block = NULL;
                }
            }

            bool PathRenderer::prepare(PathTesselator& tesselator, Vbo& vbo) {
                PathMeshPtr mesh = tesselator.tesselate(m_path.get());
                assert(mesh.get() != NULL);
                
                uploadMeshData(mesh, vbo);
                m_path = PathPtr(NULL);
                return true;
            }

            void PathRenderer::renderBackground(RenderContext& context, float hInset, float vInset) {
                glBegin(GL_QUADS);
                glVertex3f(-hInset, -vInset, 0);
                glVertex3f(-hInset, m_height + vInset, 0);
                glVertex3f(m_width + hInset, m_height + vInset, 0);
                glVertex3f(m_width + hInset, -vInset, 0);
                glEnd();
            }
            
            void PathRenderer::render(RenderContext& context) {
                assert(m_block != NULL);
                if (m_listId == 0) {
                    m_listId = glGenLists(1);
                    assert(m_listId > 0);
                    
                    glNewList(m_listId, GL_COMPILE);
                    if (m_hasTriangleSet) {
                        glDrawArrays(GL_TRIANGLES, m_triangleSetIndex, m_triangleSetCount);
                        m_hasTriangleSet = false;
                    }
                    if (m_hasTriangleStrips) {
                        GLint* indexPtr = &m_triangleStripIndices[0];
                        GLsizei* countPtr = &m_triangleStripCounts[0];
                        GLsizei primCount = static_cast<GLsizei>(m_triangleStripIndices.size());
                        glMultiDrawArrays(GL_TRIANGLE_STRIP, indexPtr, countPtr, primCount);
                        m_triangleStripIndices.clear();
                        m_triangleStripCounts.clear();
                        m_hasTriangleStrips = false;
                    }
                    if (m_hasTriangleFans) {
                        GLint* indexPtr = &m_triangleFanIndices[0];
                        GLsizei* countPtr = &m_triangleFanCounts[0];
                        GLsizei primCount = static_cast<GLsizei>(m_triangleFanIndices.size());
                        glMultiDrawArrays(GL_TRIANGLE_FAN, indexPtr, countPtr, primCount);
                        m_triangleStripIndices.clear();
                        m_triangleFanIndices.clear();
                        m_hasTriangleFans = false;
                    }
                    glEndList();
                }
                
                glCallList(m_listId);
            }
        }
    }
}