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

#ifndef __TrenchBroom__PathMesh__
#define __TrenchBroom__PathMesh__

#include "Utility/GLee.h"
#include "Utility/VecMath.h"

#include <cassert>
#include <vector>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Renderer {
        class RenderContext;
        class Vbo;
        class VboBlock;
        
        namespace Text {
            class PathMesh {
            protected:
                Vec2f::List m_vertices;
                GLenum m_currentType;
                size_t m_currentVertexCount;
            public:
                PathMesh() :
                m_currentType(0) {}
                
                inline const Vec2f::List& vertices() const {
                    return m_vertices;
                }
                
                inline void begin(GLenum type) {
                    assert(m_currentType == 0);
                    m_currentType = type;
                    m_currentVertexCount = 0;
                }
                
                inline void append(const Vec2f& vertex) {
                    switch (m_currentType) {
                        case GL_TRIANGLES:
                            m_vertices.push_back(vertex);
                            break;
                        case GL_TRIANGLE_STRIP:
                            if (m_currentVertexCount < 3) {
                                m_vertices.push_back(vertex);
                            } else if (m_currentVertexCount % 2 == 1) {
                                m_vertices.push_back(vertex);
                                m_vertices.push_back(m_vertices[m_currentVertexCount - 2]);
                                m_vertices.push_back(m_vertices[m_currentVertexCount - 1]);
                            } else {
                                m_vertices.push_back(vertex);
                                m_vertices.push_back(m_vertices[m_currentVertexCount - 1]);
                                m_vertices.push_back(m_vertices[m_currentVertexCount - 2]);
                            }
                            break;
                        case GL_TRIANGLE_FAN:
                            if (m_currentVertexCount < 3) {
                                m_vertices.push_back(vertex);
                            } else {
                                m_vertices.push_back(vertex);
                                m_vertices.push_back(m_vertices[0]);
                                m_vertices.push_back(m_vertices[m_currentVertexCount - 1]);
                            }
                            break;
                        default:
                            break;
                    }
                    m_currentVertexCount++;
                }
                
                inline void end() {
                    assert(m_currentType != 0);
                    m_currentType = 0;
                }
            };
            
            typedef std::auto_ptr<const PathMesh> PathMeshPtr;
        }
    }
}

#endif /* defined(__TrenchBroom__PathMesh__) */
