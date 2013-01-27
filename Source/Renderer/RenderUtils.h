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


#ifndef TrenchBroom_RenderUtils_h
#define TrenchBroom_RenderUtils_h

#include <GL/glew.h>
#include "Model/Texture.h"
#include "Utility/Color.h"
#include "Utility/VecMath.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <string>
#include <vector>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Renderer {
        static const float EdgeOffset = 0.0001f;
        
        inline void glVertexV3f(const Vec3f& vertex) {
            glVertex3f(vertex.x, vertex.y, vertex.z);
        }
        
        inline void glColorV4f(const Color& color) {
            glColor4f(color.x, color.y, color.z, color.w);
        }
        
        inline void glColorV4f(const Color& color, float blendFactor) {
            glColor4f(color.x, color.y, color.z, color.w * blendFactor);
        }
        
        inline void glSetEdgeOffset(float f) {
            glDepthRange(0.0f, 1.0f - EdgeOffset * f);
        }
        
        inline void glResetEdgeOffset() {
            glDepthRange(EdgeOffset, 1.0f);
        }
        
        inline void arrow(float shaftLength, float shaftWidth, float headLength, float headWidth, Vec2f::List& outline, Vec2f::List& triangles) {
            assert(shaftLength > 0.0f);
            assert(shaftWidth > 0.0f);
            assert(headLength > 0.0f);
            assert(headWidth > 0.0f);

            outline.reserve(8);
            triangles.reserve(3 * 3);

            outline.push_back(Vec2f(0.0f, shaftWidth / 2.0f));
            outline.push_back(Vec2f(shaftLength, shaftWidth / 2.0f));
            outline.push_back(Vec2f(shaftLength, headWidth / 2.0f));
            outline.push_back(Vec2f(shaftLength + headLength, 0.0f));
            outline.push_back(Vec2f(shaftLength, -headWidth / 2.0f));
            outline.push_back(Vec2f(shaftLength, -shaftWidth / 2.0f));
            outline.push_back(Vec2f(0.0f, shaftWidth / 2.0f));
            
            triangles.push_back(Vec2f(0.0f, shaftWidth / 2.0f));
            triangles.push_back(Vec2f(shaftLength, shaftWidth / 2.0f));
            triangles.push_back(Vec2f(shaftLength, -shaftWidth / 2.0f));
            
            triangles.push_back(Vec2f(shaftLength, -shaftWidth / 2.0f));
            triangles.push_back(Vec2f(0.0f, -shaftWidth / 2.0f));
            triangles.push_back(Vec2f(0.0f, shaftWidth / 2.0f));
            
            triangles.push_back(Vec2f(shaftLength, headWidth / 2.0f));
            triangles.push_back(Vec2f(shaftLength + headLength, 0.0f));
            triangles.push_back(Vec2f(shaftLength, -headWidth / 2.0f));
        }
        
        inline void circle(float radius, unsigned int segments, Vec2f::List& vertices) {
            assert(radius > 0.0f);
            assert(segments > 2);
            
            vertices.clear();
            vertices.reserve(segments);
            
            float a = 0.0f;
            float d = 2.0f * Math::Pi / static_cast<float>(segments);
            for (unsigned int i = 0; i < segments; i++) {
                float s = std::sin(a);
                float c = std::cos(a);
                
                vertices.push_back(Vec2f(radius * s, radius * c));
                a += d;
            }
        }
        
        inline void roundedRect(float width, float height, float cornerRadius, unsigned int cornerSegments, Vec2f::List& vertices) {
            assert(cornerSegments > 0);
            assert(cornerRadius <= width / 2.0f &&
                   cornerRadius <= height / 2.0f);
            
            const float angle = Math::Pi / 2.0f / cornerSegments;
            Vec2f center(0.0f, 0.0f);
            Vec2f translation;

            float curAngle = 0.0f;
            float x = std::cos(curAngle) * cornerRadius;
            float y = std::sin(curAngle) * cornerRadius;

            // lower right corner
            translation.x =  (width  / 2.0f - cornerRadius);
            translation.y = -(height / 2.0f - cornerRadius);
            for (unsigned int i = 0; i < cornerSegments; i++) {
                vertices.push_back(center);
                vertices.push_back(translation + Vec2f(x, y));

                curAngle -= angle;
                x = std::cos(curAngle) * cornerRadius;
                y = std::sin(curAngle) * cornerRadius;
                vertices.push_back(translation + Vec2f(x, y));
            }

            // lower left corner
            translation.x = -(width  / 2.0f - cornerRadius);
            translation.y = -(height / 2.0f - cornerRadius);
            for (unsigned int i = 0; i < cornerSegments; i++) {
                vertices.push_back(center);
                vertices.push_back(translation + Vec2f(x, y));
                
                curAngle -= angle;
                x = std::cos(curAngle) * cornerRadius;
                y = std::sin(curAngle) * cornerRadius;
                vertices.push_back(translation + Vec2f(x, y));
            }
            
            // upper left corner
            translation.x = -(width  / 2.0f - cornerRadius);
            translation.y =  (height / 2.0f - cornerRadius);
            for (unsigned int i = 0; i < cornerSegments; i++) {
                vertices.push_back(center);
                vertices.push_back(translation + Vec2f(x, y));
                
                curAngle -= angle;
                x = std::cos(curAngle) * cornerRadius;
                y = std::sin(curAngle) * cornerRadius;
                vertices.push_back(translation + Vec2f(x, y));
            }

            // upper right corner
            translation.x =  (width  / 2.0f - cornerRadius);
            translation.y =  (height / 2.0f - cornerRadius);
            for (unsigned int i = 0; i < cornerSegments; i++) {
                vertices.push_back(center);
                vertices.push_back(translation + Vec2f(x, y));
                
                curAngle -= angle;
                x = std::cos(curAngle) * cornerRadius;
                y = std::sin(curAngle) * cornerRadius;
                vertices.push_back(translation + Vec2f(x, y));
            }
            
            // upper body triangle
            vertices.push_back(center);
            vertices.push_back(Vec2f(-(width / 2.0f - cornerRadius), height / 2.0f));
            vertices.push_back(Vec2f( (width / 2.0f - cornerRadius), height / 2.0f));
            
            // right body triangle
            vertices.push_back(center);
            vertices.push_back(Vec2f(width / 2.0f,  (height / 2.0f - cornerRadius)));
            vertices.push_back(Vec2f(width / 2.0f, -(height / 2.0f - cornerRadius)));

            // lower body triangle
            vertices.push_back(center);
            vertices.push_back(Vec2f( (width / 2.0f - cornerRadius), -height / 2.0f));
            vertices.push_back(Vec2f(-(width / 2.0f - cornerRadius), -height / 2.0f));

            // left body triangle
            vertices.push_back(center);
            vertices.push_back(Vec2f(-width / 2.0f, -(height / 2.0f - cornerRadius)));
            vertices.push_back(Vec2f(-width / 2.0f,  (height / 2.0f - cornerRadius)));
        }
        
        namespace SphereBuilder {
            class Triangle {
            private:
                size_t m_indices[3];
            public:
                Triangle(size_t index1, size_t index2, size_t index3) {
                    m_indices[0] = index1;
                    m_indices[1] = index2;
                    m_indices[2] = index3;
                }
                
                inline const size_t& operator[] (size_t i) const {
                    assert(i < 3);
                    return m_indices[i];
                }
            };
            
            class MidPointIndex {
            private:
                size_t m_index1;
                size_t m_index2;
            public:
                MidPointIndex(size_t index1, size_t index2) :
                m_index1(index1),
                m_index2(index2) {}
                
                inline bool operator< (const MidPointIndex& other) const {
                    if (m_index1 < other.m_index1)
                        return true;
                    if (m_index1 > other.m_index1)
                        return false;
                    return m_index2 < other.m_index2;
                }
            };

            typedef std::map<SphereBuilder::MidPointIndex, size_t> MidPointCache;

            inline size_t midPoint(Vec3f::List& vertices, MidPointCache& cache, size_t index1, size_t index2) {
                MidPointCache::iterator it = cache.find(MidPointIndex(index1, index2));
                if (it == cache.end()) {
                    const Vec3f& vertex1 = vertices[index1];
                    const Vec3f& vertex2 = vertices[index2];
                    Vec3f midPoint = (vertex1 + vertex2) / 2.0f;
                    vertices.push_back(midPoint.normalize());
                    size_t midPointIndex = vertices.size() - 1;
                    cache[MidPointIndex(index1, index2)] = midPointIndex;
                    cache[MidPointIndex(index2, index1)] = midPointIndex;
                    return midPointIndex;
                }
                return it->second;
            }
        }
        
        inline Vec3f::List sphere(float radius, unsigned int iterations) {
            typedef std::vector<SphereBuilder::Triangle> TriangleList;

            Vec3f::List vertices;
            TriangleList triangles;

            // build initial icosahedron
            float t = static_cast<float>((1.0 + std::sqrt(5.0)) / 2.0);
            vertices.push_back(Vec3f(-1.0f,     t,  0.0f).normalize());
            vertices.push_back(Vec3f( 1.0f,     t,  0.0f).normalize());
            vertices.push_back(Vec3f(-1.0f,    -t,  0.0f).normalize());
            vertices.push_back(Vec3f( 1.0f,    -t,  0.0f).normalize());
            
            vertices.push_back(Vec3f( 0.0f, -1.0f,     t).normalize());
            vertices.push_back(Vec3f( 0.0f,  1.0f,     t).normalize());
            vertices.push_back(Vec3f( 0.0f, -1.0f,    -t).normalize());
            vertices.push_back(Vec3f( 0.0f,  1.0f,    -t).normalize());
            
            vertices.push_back(Vec3f(    t,  0.0f, -1.0f).normalize());
            vertices.push_back(Vec3f(    t,  0.0f,  1.0f).normalize());
            vertices.push_back(Vec3f(   -t,  0.0f, -1.0f).normalize());
            vertices.push_back(Vec3f(   -t,  0.0f,  1.0f).normalize());
            
            // 5 triangles around point 0
            triangles.push_back(SphereBuilder::Triangle( 0,  5, 11));
            triangles.push_back(SphereBuilder::Triangle( 0,  1,  5));
            triangles.push_back(SphereBuilder::Triangle( 0,  7,  1));
            triangles.push_back(SphereBuilder::Triangle( 0, 10,  7));
            triangles.push_back(SphereBuilder::Triangle( 0, 11, 10));
            
            // 5 adjacent faces
            triangles.push_back(SphereBuilder::Triangle( 4, 11,  5));
            triangles.push_back(SphereBuilder::Triangle( 9,  5,  1));
            triangles.push_back(SphereBuilder::Triangle( 8,  1,  7));
            triangles.push_back(SphereBuilder::Triangle( 6,  7, 10));
            triangles.push_back(SphereBuilder::Triangle( 2, 10, 11));
            
            // 5 faces around point 3
            triangles.push_back(SphereBuilder::Triangle( 3,  2,  4));
            triangles.push_back(SphereBuilder::Triangle( 3,  6,  2));
            triangles.push_back(SphereBuilder::Triangle( 3,  8,  6));
            triangles.push_back(SphereBuilder::Triangle( 3,  9,  8));
            triangles.push_back(SphereBuilder::Triangle( 3,  4,  9));
            
            // 5 adjacent faces
            triangles.push_back(SphereBuilder::Triangle(11,  4,  2));
            triangles.push_back(SphereBuilder::Triangle(10,  2,  6));
            triangles.push_back(SphereBuilder::Triangle( 7,  6,  8));
            triangles.push_back(SphereBuilder::Triangle( 1,  8,  9));
            triangles.push_back(SphereBuilder::Triangle( 5,  9,  4));

            // subdivide the icosahedron
            SphereBuilder::MidPointCache cache;
            for (unsigned int i = 0; i < iterations; i++) {
                TriangleList newTriangles;
                TriangleList::iterator it, end;
                for (it = triangles.begin(), end = triangles.end(); it != end; ++it) {
                    SphereBuilder::Triangle& triangle = *it;
                    size_t index1 = SphereBuilder::midPoint(vertices, cache, triangle[0], triangle[1]);
                    size_t index2 = SphereBuilder::midPoint(vertices, cache, triangle[1], triangle[2]);
                    size_t index3 = SphereBuilder::midPoint(vertices, cache, triangle[2], triangle[0]);
                    newTriangles.push_back(SphereBuilder::Triangle(triangle[0], index1, index3));
                    newTriangles.push_back(SphereBuilder::Triangle(triangle[1], index2, index1));
                    newTriangles.push_back(SphereBuilder::Triangle(triangle[2], index3, index2));
                    newTriangles.push_back(SphereBuilder::Triangle(index1, index2, index3));
                }
                triangles = newTriangles;
            }

            Vec3f::List allVertices;
            allVertices.reserve(3 * triangles.size());
            TriangleList::iterator it, end;
            for (it = triangles.begin(), end = triangles.end(); it != end; ++it) {
                SphereBuilder::Triangle& triangle = *it;
                for (unsigned int i = 0; i < 3; i++)
                    allVertices.push_back(radius * vertices[triangle[i]]);
            }
            
            return allVertices;
        }
    }
}

#endif
