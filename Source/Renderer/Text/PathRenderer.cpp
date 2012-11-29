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
                
                const Vec2f::List& triangleSet = mesh->triangleSet();
                const PathMeshData& triangleStrips = mesh->triangleStrips();
                const PathMeshData& triangleFans = mesh->triangleFans();
                
                if (!triangleSet.empty()) {
                    unsigned int vertexCount = static_cast<unsigned int>(triangleSet.size());
                    m_triangleSetVertexArray = VertexArrayPtr(new VertexArray(vbo, GL_TRIANGLES, vertexCount,
                                                                              Attribute::position2f()));

                    for (unsigned int i = 0; i < triangleSet.size(); i++)
                        m_triangleSetVertexArray->addAttribute(triangleSet[i]);
                }
                
                if (!triangleStrips.empty()) {
                    unsigned int vertexCount = mesh->triangleStripsVertexCount();
                    m_triangleStripVertexArray = IndexedVertexArrayPtr(new IndexedVertexArray(vbo, GL_TRIANGLE_STRIP, vertexCount,
                                                                                              Attribute::position2f()));
                    
                    for (unsigned int i = 0; i < triangleStrips.size(); i++) {
                        const Vec2f::List& vertices = triangleStrips[i];
                        for (unsigned int j = 0; j < vertices.size(); j++)
                            m_triangleStripVertexArray->addAttribute(vertices[j]);
                        m_triangleStripVertexArray->endPrimitive();
                    }
                }
                
                if (!triangleFans.empty()) {
                    unsigned int vertexCount = mesh->triangleFansVertexCount();
                    m_triangleFanVertexArray = IndexedVertexArrayPtr(new IndexedVertexArray(vbo, GL_TRIANGLE_FAN, vertexCount,
                                                                                            Attribute::position2f()));
                    
                    for (unsigned int i = 0; i < triangleFans.size(); i++) {
                        const Vec2f::List& vertices = triangleFans[i];
                        for (unsigned int j = 0; j < vertices.size(); j++)
                            m_triangleFanVertexArray->addAttribute(vertices[j]);
                        m_triangleFanVertexArray->endPrimitive();
                    }
                }
            }
            
            PathRenderer::PathRenderer(PathPtr path) :
            m_path(path),
            m_width(m_path->width()),
            m_height(m_path->height()),
            m_listId(0) {}
            
            PathRenderer::~PathRenderer() {
                if (m_listId != 0) {
                    glDeleteLists(m_listId, 1);
                    m_listId = 0;
                }
            }
            
            bool PathRenderer::prepare(PathTesselator& tesselator, Vbo& vbo) {
                PathMeshPtr mesh = tesselator.tesselate(m_path.get());
                assert(mesh.get() != NULL);
                
                uploadMeshData(mesh, vbo);
                m_path = PathPtr(NULL);
                return true;
            }

            void PathRenderer::render() {
                if (m_listId == 0) {
                    m_listId = glGenLists(1);
                    assert(m_listId > 0);
                    
                    glNewList(m_listId, GL_COMPILE);
                    if (m_triangleSetVertexArray.get() != NULL)
                        m_triangleSetVertexArray->render();
                    if (m_triangleStripVertexArray.get() != NULL)
                        m_triangleStripVertexArray->render();
                    if (m_triangleFanVertexArray.get() != NULL)
                        m_triangleFanVertexArray->render();
                    glEndList();
                }
                
                glCallList(m_listId);
            }
        }
    }
}