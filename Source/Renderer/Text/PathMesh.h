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
            typedef std::vector<Vec2f::List> PathMeshData;
            
            class PathMesh {
            protected:
                // raw data
                Vec2f::List m_triangleSet;
                PathMeshData m_triangleStrips;
                PathMeshData m_triangleFans;
                unsigned int m_triangleStripsVertexCount;
                unsigned int m_triangleFansVertexCount;
                GLenum m_currentType;
            public:
                PathMesh() :
                m_triangleStripsVertexCount(0),
                m_triangleFansVertexCount(0),
                m_currentType(0) {}
                
                inline const Vec2f::List& triangleSet() const {
                    return m_triangleSet;
                }
                
                inline const PathMeshData& triangleStrips() const {
                    return m_triangleStrips;
                }
                
                inline const PathMeshData& triangleFans() const {
                    return m_triangleFans;
                }
                
                inline unsigned int triangleStripsVertexCount() const {
                    return m_triangleStripsVertexCount;
                }

                inline unsigned int triangleFansVertexCount() const {
                    return m_triangleFansVertexCount;
                }
                
                inline void begin(GLenum type) {
                    assert(m_currentType == 0);
                    m_currentType = type;
                    switch (m_currentType) {
                        case GL_TRIANGLE_STRIP: {
                            m_triangleStrips.push_back(Vec2f::List());
                            break;
                        }
                        case GL_TRIANGLE_FAN: {
                            m_triangleFans.push_back(Vec2f::List());
                            break;
                        }
                        default:
                            break;
                    }
                }
                
                inline void append(const Vec2f& vertex) {
                    switch (m_currentType) {
                        case GL_TRIANGLES:
                            m_triangleSet.push_back(vertex);
                            break;
                        case GL_TRIANGLE_STRIP: {
                            assert(!m_triangleStrips.empty());
                            Vec2f::List& strip = m_triangleStrips.back();
                            strip.push_back(vertex);
                            m_triangleStripsVertexCount++;
                            break;
                        }
                        case GL_TRIANGLE_FAN: {
                            assert(!m_triangleFans.empty());
                            Vec2f::List& fan = m_triangleFans.back();
                            fan.push_back(vertex);
                            m_triangleFansVertexCount++;
                            break;
                        }
                        default:
                            break;
                    }
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