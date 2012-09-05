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
            typedef std::vector<GLfloat> FloatBuffer;
            typedef std::vector<FloatBuffer*> PathMeshData;

            class PathMesh {
            protected:
                // raw data
                FloatBuffer m_triangleSet;
                PathMeshData m_triangleStrips;
                PathMeshData m_triangleFans;
                unsigned int m_vertexCount;
                GLenum m_currentType;
            public:
                PathMesh() :
                m_vertexCount(0),
                m_currentType(0) {}
                
                ~PathMesh() {
                    m_triangleSet.clear();
                    while (!m_triangleStrips.empty()) delete m_triangleStrips.back(), m_triangleStrips.pop_back();
                    while (!m_triangleFans.empty()) delete m_triangleFans.back(), m_triangleFans.pop_back();
                    m_vertexCount = 0;
                }
                
                inline const FloatBuffer& triangleSet() const {
                    return m_triangleSet;
                }
                
                inline const PathMeshData& triangleStrips() const {
                    return m_triangleStrips;
                }
                
                inline const PathMeshData& triangleFans() const {
                    return m_triangleFans;
                }
                
                inline unsigned int vertexCount() const {
                    return m_vertexCount;
                }
                
                inline void begin(GLenum type) {
                    assert(m_currentType == 0);
                    m_currentType = type;
                    switch (m_currentType) {
                        case GL_TRIANGLE_STRIP: {
                            FloatBuffer* strip = new FloatBuffer();
                            m_triangleStrips.push_back(strip);
                            break;
                        }
                        case GL_TRIANGLE_FAN: {
                            FloatBuffer* fan = new FloatBuffer();
                            m_triangleFans.push_back(fan);
                            break;
                        }
                        default:
                            break;
                    }
                }
                
                inline void append(const Vec2f& vertex) {
                    switch (m_currentType) {
                        case GL_TRIANGLES:
                            m_triangleSet.push_back(vertex.x);
                            m_triangleSet.push_back(vertex.y);
                            break;
                        case GL_TRIANGLE_STRIP: {
                            assert(!m_triangleStrips.empty());
                            FloatBuffer* strip = m_triangleStrips.back();
                            strip->push_back(vertex.x);
                            strip->push_back(vertex.y);
                            break;
                        }
                        case GL_TRIANGLE_FAN: {
                            assert(!m_triangleFans.empty());
                            FloatBuffer* fan = m_triangleFans.back();
                            fan->push_back(vertex.x);
                            fan->push_back(vertex.y);
                            break;
                        }
                        default:
                            break;
                    }
                    m_vertexCount++;
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
