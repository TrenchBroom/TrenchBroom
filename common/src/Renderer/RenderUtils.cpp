/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "RenderUtils.h"

#include "Assets/Texture.h"
#include "Renderer/GL.h"

namespace TrenchBroom {
    namespace Renderer {
        static const float EdgeOffset = 0.0001f;

        void glSetEdgeOffset(const float f) {
            glAssert(glDepthRange(0.0f, 1.0f - EdgeOffset * f));
        }
        
        void glResetEdgeOffset() {
            glAssert(glDepthRange(EdgeOffset, 1.0f));
        }
        
        BuildCoordinateSystem BuildCoordinateSystem::xy(const Color& x, const Color& y) {
            return BuildCoordinateSystem(x, y, Color(), true, true, false);
        }
        
        BuildCoordinateSystem BuildCoordinateSystem::xz(const Color& x, const Color& z) {
            return BuildCoordinateSystem(x, Color(), z, true, false, true);
        }
        
        BuildCoordinateSystem BuildCoordinateSystem::yz(const Color& y, const Color& z) {
            return BuildCoordinateSystem(Color(), y, z, false, true, true);
        }
        
        BuildCoordinateSystem BuildCoordinateSystem::xyz(const Color& x, const Color& y, const Color& z) {
            return BuildCoordinateSystem(x, y, z, true, true, true);
        }
        
        BuildCoordinateSystem::BuildCoordinateSystem(const Color& xColor, const Color& yColor, const Color& zColor, const bool xAxis, const bool yAxis, const bool zAxis) {
            m_colors[0] = xColor;
            m_colors[1] = yColor;
            m_colors[2] = zColor;
            m_axes[0] = xAxis;
            m_axes[1] = yAxis;
            m_axes[2] = zAxis;
        }

        VertexSpecs::P3C4::Vertex::List BuildCoordinateSystem::vertices(const BBox3f& bounds) const {
            const Vec3f center = bounds.center();
            
            typedef VertexSpecs::P3C4::Vertex Vertex;
            Vertex::List vertices(countVertices());

            size_t i = 0;
            if (m_axes[0]) {
                vertices[i++] = Vertex(Vec3f(bounds.min.x(),     center.y(),     center.z()), m_colors[0]);
                vertices[i++] = Vertex(Vec3f(bounds.max.x(),     center.y(),     center.z()), m_colors[0]);
            }
            if (m_axes[1]) {
                vertices[i++] = Vertex(Vec3f(    center.x(), bounds.min.y(),     center.z()), m_colors[1]);
                vertices[i++] = Vertex(Vec3f(    center.x(), bounds.max.y(),     center.z()), m_colors[1]);
            }
            if (m_axes[2]) {
                vertices[i++] = Vertex(Vec3f(    center.x(),     center.y(), bounds.min.z()), m_colors[2]);
                vertices[i++] = Vertex(Vec3f(    center.x(),     center.y(), bounds.max.z()), m_colors[2]);
            }

            return vertices;
        }

        size_t BuildCoordinateSystem::countVertices() const {
            size_t result = 0;
            for (size_t i = 0; i < 3; ++i)
                if (m_axes[i])
                    result += 2;
            return result;
        }

        TextureRenderFunc::~TextureRenderFunc() {}
        void TextureRenderFunc::before(const Assets::Texture* texture) {}
        void TextureRenderFunc::after(const Assets::Texture* texture) {}
        
        void DefaultTextureRenderFunc::before(const Assets::Texture* texture) {
            if (texture != NULL)
                texture->activate();
        }
        
        void DefaultTextureRenderFunc::after(const Assets::Texture* texture) {
            if (texture != NULL)
                texture->deactivate();
        }

        Vec2f::List circle2D(const float radius, const size_t segments) {
            Vec2f::List vertices = circle2D(radius, 0.0f, Math::Cf::twoPi(), segments);
            vertices.push_back(Vec2f::Null);
            return vertices;
        }

        Vec2f::List circle2D(const float radius, const float startAngle, const float angleLength, const size_t segments) {
            assert(radius > 0.0f);
            assert(segments > 0);
            if (angleLength == 0.0f)
                return Vec2f::List();
            
            Vec2f::List vertices(segments + 1);
            
            const float d = angleLength / segments;
            float a = startAngle;
            for (size_t i = 0; i <= segments; ++i) {
                vertices[i][0] = radius * std::sin(a);
                vertices[i][1] = radius * std::cos(a);
                a += d;
            }
            
            return vertices;
        }
        
        Vec3f::List circle2D(const float radius, const Math::Axis::Type axis, const float startAngle, const float angleLength, const size_t segments) {
            assert(radius > 0.0f);
            assert(segments > 0);
            if (angleLength == 0.0f)
                return Vec3f::List();
            
            Vec3f::List vertices(segments + 1);
            
            size_t x,y,z;
            switch (axis) {
                case Math::Axis::AX:
                    x = 1; y = 2; z = 0;
                    break;
                case Math::Axis::AY:
                    x = 2; y = 0; z = 1;
                    break;
                default:
                    x = 0; y = 1; z = 2;
                    break;
            }
            
            const float d = angleLength / segments;
            float a = startAngle;
            for (size_t i = 0; i <= segments; ++i) {
                vertices[i][x] = radius * std::cos(a);
                vertices[i][y] = radius * std::sin(a);
                vertices[i][z] = 0.0f;
                a += d;
            }
            
            return vertices;
        }

        std::pair<float, float> startAngleAndLength(const Math::Axis::Type axis, const Vec3f& startAxis, const Vec3f& endAxis) {
            float angle1, angle2, angleLength;
            switch (axis) {
                case Math::Axis::AX:
                    angle1 = angleBetween(startAxis, Vec3f::PosY, Vec3f::PosX);
                    angle2 = angleBetween(endAxis, Vec3f::PosY, Vec3f::PosX);
                    angleLength = std::min(angleBetween(startAxis, endAxis, Vec3f::PosX), angleBetween(endAxis, startAxis, Vec3f::PosX));
                    break;
                case Math::Axis::AY:
                    angle1 = angleBetween(startAxis, Vec3f::PosZ, Vec3f::PosY);
                    angle2 = angleBetween(endAxis, Vec3f::PosZ, Vec3f::PosY);
                    angleLength = std::min(angleBetween(startAxis, endAxis, Vec3f::PosY), angleBetween(endAxis, startAxis, Vec3f::PosY));
                    break;
                default:
                    angle1 = angleBetween(startAxis, Vec3f::PosX, Vec3f::PosZ);
                    angle2 = angleBetween(endAxis, Vec3f::PosX, Vec3f::PosZ);
                    angleLength = std::min(angleBetween(startAxis, endAxis, Vec3f::PosZ), angleBetween(endAxis, startAxis, Vec3f::PosZ));
                    break;
            }
            const float minAngle = std::min(angle1, angle2);
            const float maxAngle = std::max(angle1, angle2);
            const float startAngle = (maxAngle - minAngle <= Math::Cf::pi() ? minAngle : maxAngle);
            return std::make_pair(startAngle, angleLength);
        }

        size_t roundedRect2DVertexCount(const size_t cornerSegments) {
            return 4 * (3 * cornerSegments + 3);
        }

        Vec2f::List roundedRect2D(const Vec2f& size, const float cornerRadius, const size_t cornerSegments) {
            return roundedRect2D(size.x(), size.y(), cornerRadius, cornerSegments);
        }
        
        Vec2f::List roundedRect2D(const float width, const float height, const float cornerRadius, const size_t cornerSegments) {
            assert(cornerSegments > 0);
            assert(cornerRadius <= width / 2.0f &&
                   cornerRadius <= height / 2.0f);
            
            Vec2f::List vertices;
            vertices.resize(roundedRect2DVertexCount(cornerSegments));
            size_t vertexIndex = 0;
            
            const float angle = Math::Cf::piOverTwo() / cornerSegments;
            Vec2f center(0.0f, 0.0f);
            Vec2f translation;
            
            float curAngle = 0.0f;
            float x = std::cos(curAngle) * cornerRadius;
            float y = std::sin(curAngle) * cornerRadius;
            
            // lower right corner
            translation = Vec2f( (width  / 2.0f - cornerRadius),
                                -(height / 2.0f - cornerRadius));
            for (size_t i = 0; i < cornerSegments; ++i) {
                vertices[vertexIndex++] = center;
                vertices[vertexIndex++] = translation + Vec2f(x, y);
                
                curAngle -= angle;
                x = std::cos(curAngle) * cornerRadius;
                y = std::sin(curAngle) * cornerRadius;
                vertices[vertexIndex++] = translation + Vec2f(x, y);
            }
            
            // lower left corner
            translation = Vec2f(-(width  / 2.0f - cornerRadius),
                                -(height / 2.0f - cornerRadius));
            for (size_t i = 0; i < cornerSegments; ++i) {
                vertices[vertexIndex++] = center;
                vertices[vertexIndex++] = translation + Vec2f(x, y);
                
                curAngle -= angle;
                x = std::cos(curAngle) * cornerRadius;
                y = std::sin(curAngle) * cornerRadius;
                vertices[vertexIndex++] = translation + Vec2f(x, y);
            }
            
            // upper left corner
            translation = Vec2f(-(width  / 2.0f - cornerRadius),
                                (height / 2.0f - cornerRadius));
            for (size_t i = 0; i < cornerSegments; ++i) {
                vertices[vertexIndex++] = center;
                vertices[vertexIndex++] = translation + Vec2f(x, y);
                
                curAngle -= angle;
                x = std::cos(curAngle) * cornerRadius;
                y = std::sin(curAngle) * cornerRadius;
                vertices[vertexIndex++] = translation + Vec2f(x, y);
            }
            
            // upper right corner
            translation = Vec2f( (width  / 2.0f - cornerRadius),
                                (height / 2.0f - cornerRadius));
            for (size_t i = 0; i < cornerSegments; ++i) {
                vertices[vertexIndex++] = center;
                vertices[vertexIndex++] = translation + Vec2f(x, y);
                
                curAngle -= angle;
                x = std::cos(curAngle) * cornerRadius;
                y = std::sin(curAngle) * cornerRadius;
                vertices[vertexIndex++] = translation + Vec2f(x, y);
            }
            
            // upper body triangle
            vertices[vertexIndex++] = center;
            vertices[vertexIndex++] = Vec2f(-(width / 2.0f - cornerRadius), height / 2.0f);
            vertices[vertexIndex++] = Vec2f( (width / 2.0f - cornerRadius), height / 2.0f);
            
            // right body triangle
            vertices[vertexIndex++] = center;
            vertices[vertexIndex++] = Vec2f(width / 2.0f,  (height / 2.0f - cornerRadius));
            vertices[vertexIndex++] = Vec2f(width / 2.0f, -(height / 2.0f - cornerRadius));
            
            // lower body triangle
            vertices[vertexIndex++] = center;
            vertices[vertexIndex++] = Vec2f( (width / 2.0f - cornerRadius), -height / 2.0f);
            vertices[vertexIndex++] = Vec2f(-(width / 2.0f - cornerRadius), -height / 2.0f);
            
            // left body triangle
            vertices[vertexIndex++] = center;
            vertices[vertexIndex++] = Vec2f(-width / 2.0f, -(height / 2.0f - cornerRadius));
            vertices[vertexIndex++] = Vec2f(-width / 2.0f,  (height / 2.0f - cornerRadius));
            
            return vertices;
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
                
                const size_t& operator[] (size_t i) const {
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
                
                bool operator<(const MidPointIndex& other) const {
                    if (m_index1 < other.m_index1)
                        return true;
                    if (m_index1 > other.m_index1)
                        return false;
                    return m_index2 < other.m_index2;
                }
            };
            
            typedef std::map<SphereBuilder::MidPointIndex, size_t> MidPointCache;
            size_t midPoint(Vec3f::List& vertices, MidPointCache& cache, const size_t index1, const size_t index2);
            
            size_t midPoint(Vec3f::List& vertices, MidPointCache& cache, const size_t index1, const size_t index2) {
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
        
        Vec3f::List sphere3D(const float radius, const size_t iterations) {
            assert(radius > 0.0f);
            assert(iterations > 0);
            
            typedef std::vector<SphereBuilder::Triangle> TriangleList;
            
            Vec3f::List vertices;
            TriangleList triangles;
            
            // build initial icosahedron
            const float t = static_cast<float>((1.0 + std::sqrt(5.0)) / 2.0);
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
            for (size_t i = 0; i < iterations; ++i) {
                TriangleList newTriangles;
                TriangleList::iterator it, end;
                for (it = triangles.begin(), end = triangles.end(); it != end; ++it) {
                    SphereBuilder::Triangle& triangle = *it;
                    const size_t index1 = SphereBuilder::midPoint(vertices, cache, triangle[0], triangle[1]);
                    const size_t index2 = SphereBuilder::midPoint(vertices, cache, triangle[1], triangle[2]);
                    const size_t index3 = SphereBuilder::midPoint(vertices, cache, triangle[2], triangle[0]);
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
                for (size_t i = 0; i < 3; ++i)
                    allVertices.push_back(radius * vertices[triangle[i]]);
            }
            
            return allVertices;
        }

        VertsAndNormals::VertsAndNormals(const size_t vertexCount) :
        vertices(vertexCount),
        normals(vertexCount) {}

        VertsAndNormals circle3D(const float radius, const size_t segments) {
            assert(radius > 0.0f);
            assert(segments > 2);
            
            VertsAndNormals result(segments);
            
            float a = 0.0f;
            const float d = 2.0f * Math::Cf::pi() / static_cast<float>(segments);
            for (size_t i = 0; i < segments; i++) {
                result.vertices[i] = Vec3f(radius * std::sin(a), radius * std::cos(a), 0.0f);
                result.normals[i] = Vec3f::PosZ;
                a += d;
            }
            return result;
        }

        VertsAndNormals cylinder3D(const float radius, const float length, const size_t segments) {
            assert(radius > 0.0f);
            assert(length > 0.0f);
            assert(segments > 2);
            
            VertsAndNormals result(2 * (segments + 1));
            
            float a = 0.0f;
            const float d = 2.0f * Math::Cf::pi() / static_cast<float>(segments);
            for (size_t i = 0; i <= segments; ++i) {
                const float s = std::sin(a);
                const float c = std::cos(a);
                const float x = radius * s;
                const float y = radius * c;
                result.vertices[2*i+0] = Vec3f(x, y, length);
                result.vertices[2*i+1] = Vec3f(x, y, 0.0f);
                result.normals[2*i+0] = result.normals[2*i+1] = Vec3f(s, c, 0.0f);
                a += d;
            }
            return result;
        }

        VertsAndNormals cone3D(const float radius, const float length, const size_t segments) {
            assert(radius > 0.0f);
            assert(length > 0.0f);
            assert(segments > 2);
            
            VertsAndNormals result(3 * (segments + 1));
            
            const float t = std::atan(length / radius);
            const float n = std::cos(Math::Cf::piOverTwo() - t);
            
            float a = 0.0f;
            const float d = 2.0f * Math::Cf::pi() / static_cast<float>(segments);
            float lastS = std::sin(a);
            float lastC = std::cos(a);
            a += d;
            
            for (size_t i = 0; i <= segments; ++i) {
                const float s = std::sin(a);
                const float c = std::cos(a);
                
                result.vertices[3*i+0] = Vec3f(0.0f, 0.0f, length);
                result.vertices[3*i+1] = Vec3f(radius * lastS, radius * lastC, 0.0f);
                result.vertices[3*i+2] = Vec3f(radius * s, radius * c, 0.0f);
                
                result.normals[3*i+0] = Vec3f(std::sin(a - d / 2.0f), std::cos(a - d / 2.0f), n).normalize();
                result.normals[3*i+1] = Vec3f(lastS, lastC, n).normalize();
                result.normals[3*i+2] = Vec3f(s, c, n).normalize();
                
                lastS = s;
                lastC = c;
                a += d;
            }
            return result;
        }
    }
}
