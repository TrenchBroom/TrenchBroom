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
                
                const Vec2f::List& vertices = mesh->vertices();
                unsigned int vertexCount = static_cast<unsigned int>(vertices.size());
                m_vertexArray = VertexArrayPtr(new VertexArray(vbo, GL_TRIANGLES, vertexCount, VertexAttribute(2, GL_FLOAT, VertexAttribute::Position)));
                
                for (unsigned int i = 0; i < vertices.size(); i++)
                    m_vertexArray->addAttribute(vertices[i]);
            }
            
            PathRenderer::PathRenderer(PathPtr path) :
            m_path(path),
            m_width(m_path->width()),
            m_height(m_path->height()) {}
            
            bool PathRenderer::prepare(PathTesselator& tesselator, Vbo& vbo) {
                PathMeshPtr mesh = tesselator.tesselate(m_path.get());
                assert(mesh.get() != NULL);
                
                uploadMeshData(mesh, vbo);
                m_path = PathPtr(NULL);
                return true;
            }

            void PathRenderer::render() {
                assert(m_vertexArray.get() != NULL);
                m_vertexArray->render();
            }
        }
    }
}