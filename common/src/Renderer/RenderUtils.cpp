/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include <vecmath/forward.h>
#include <vecmath/vec.h>
#include <vecmath/bbox.h>
#include <vecmath/util.h>

#include <map>

namespace TrenchBroom {
    namespace Renderer {
        static const double EdgeOffset = 0.0001;

        vm::vec3f gridColorForTexture(const Assets::Texture* texture) {
            if (texture == nullptr) {
                return vm::vec3f::fill(1.0f);
            }
            if ((texture->averageColor().r() +
                 texture->averageColor().g() +
                 texture->averageColor().b()) / 3.0f > 0.50f) {
                // bright texture grid color
                return vm::vec3f::fill(0.0f);
            } else {
                // dark texture grid color
                return vm::vec3f::fill(1.0f);
            }
        }

        void glSetEdgeOffset(const double f) {
            glAssert(glDepthRange(0.0, 1.0 - EdgeOffset * f))
        }

        void glResetEdgeOffset() {
            glAssert(glDepthRange(EdgeOffset, 1.0))
        }

        void coordinateSystemVerticesX(const vm::bbox3f& bounds, vm::vec3f& start, vm::vec3f& end) {
            const auto center = bounds.center();
            start = vm::vec3f(bounds.min.x(), center.y(),     center.z());
            end   = vm::vec3f(bounds.max.x(), center.y(),     center.z());
        }

        void coordinateSystemVerticesY(const vm::bbox3f& bounds, vm::vec3f& start, vm::vec3f& end) {
            const auto center = bounds.center();
            start = vm::vec3f(center.x(),     bounds.min.y(), center.z());
            end   = vm::vec3f(center.x(),     bounds.max.y(), center.z());
        }

        void coordinateSystemVerticesZ(const vm::bbox3f& bounds, vm::vec3f& start, vm::vec3f& end) {
            const auto center = bounds.center();
            start = vm::vec3f(center.x(),     center.y(),     bounds.min.z());
            end   = vm::vec3f(center.x(),     center.y(),     bounds.max.z());
        }

        TextureRenderFunc::~TextureRenderFunc() {}
        void TextureRenderFunc::before(const Assets::Texture* /* texture */) {}
        void TextureRenderFunc::after(const Assets::Texture* /* texture */) {}

        void DefaultTextureRenderFunc::before(const Assets::Texture* texture) {
            if (texture != nullptr) {
                texture->activate();
            }
        }

        void DefaultTextureRenderFunc::after(const Assets::Texture* texture) {
            if (texture != nullptr) {
                texture->deactivate();
            }
        }

        std::vector<vm::vec2f> circle2D(const float radius, const size_t segments) {
            std::vector<vm::vec2f> vertices = circle2D(radius, 0.0f, vm::Cf::two_pi(), segments);
            vertices.push_back(vm::vec2f::zero());
            return vertices;
        }

        std::vector<vm::vec2f> circle2D(const float radius, const float startAngle, const float angleLength, const size_t segments) {
            assert(radius > 0.0f);
            assert(segments > 0);
            if (angleLength == 0.0f)
                return std::vector<vm::vec2f>();

            std::vector<vm::vec2f> vertices(segments + 1);

            const float d = angleLength / static_cast<float>(segments);
            float a = startAngle;
            for (size_t i = 0; i <= segments; ++i) {
                vertices[i][0] = radius * std::sin(a);
                vertices[i][1] = radius * std::cos(a);
                a += d;
            }

            return vertices;
        }

        std::vector<vm::vec3f> circle2D(const float radius, const vm::axis::type axis, const float startAngle, const float angleLength, const size_t segments) {
            assert(radius > 0.0f);
            assert(segments > 0);
            if (angleLength == 0.0f)
                return std::vector<vm::vec3f>();

            std::vector<vm::vec3f> vertices(segments + 1);

            size_t x,y,z;
            switch (axis) {
                case vm::axis::x:
                    x = 1; y = 2; z = 0;
                    break;
                case vm::axis::y:
                    x = 2; y = 0; z = 1;
                    break;
                default:
                    x = 0; y = 1; z = 2;
                    break;
            }

            const float d = angleLength / static_cast<float>(segments);
            float a = startAngle;
            for (size_t i = 0; i <= segments; ++i) {
                vertices[i][x] = radius * std::cos(a);
                vertices[i][y] = radius * std::sin(a);
                vertices[i][z] = 0.0f;
                a += d;
            }

            return vertices;
        }

        std::pair<float, float> startAngleAndLength(const vm::axis::type axis, const vm::vec3f& startAxis, const vm::vec3f& endAxis) {
            float angle1, angle2, angleLength;
            switch (axis) {
                case vm::axis::x:
                    angle1 = vm::measure_angle(startAxis, vm::vec3f::pos_y(), vm::vec3f::pos_x());
                    angle2 = vm::measure_angle(endAxis, vm::vec3f::pos_y(), vm::vec3f::pos_x());
                    angleLength = vm::min(vm::measure_angle(startAxis, endAxis, vm::vec3f::pos_x()),
                                          vm::measure_angle(endAxis, startAxis, vm::vec3f::pos_x()));
                    break;
                case vm::axis::y:
                    angle1 = vm::measure_angle(startAxis, vm::vec3f::pos_z(), vm::vec3f::pos_y());
                    angle2 = vm::measure_angle(endAxis, vm::vec3f::pos_z(), vm::vec3f::pos_y());
                    angleLength = vm::min(vm::measure_angle(startAxis, endAxis, vm::vec3f::pos_y()),
                                          vm::measure_angle(endAxis, startAxis, vm::vec3f::pos_y()));
                    break;
                default:
                    angle1 = vm::measure_angle(startAxis, vm::vec3f::pos_x(), vm::vec3f::pos_z());
                    angle2 = vm::measure_angle(endAxis, vm::vec3f::pos_x(), vm::vec3f::pos_z());
                    angleLength = vm::min(vm::measure_angle(startAxis, endAxis, vm::vec3f::pos_z()),
                                          vm::measure_angle(endAxis, startAxis, vm::vec3f::pos_z()));
                    break;
            }
            const float minAngle = vm::min(angle1, angle2);
            const float maxAngle = std::max(angle1, angle2);
            const float startAngle = (maxAngle - minAngle <= vm::Cf::pi() ? minAngle : maxAngle);
            return std::make_pair(startAngle, angleLength);
        }

        size_t roundedRect2DVertexCount(const size_t cornerSegments) {
            return 4 * (3 * cornerSegments + 3);
        }

        std::vector<vm::vec2f> roundedRect2D(const vm::vec2f& size, const float cornerRadius, const size_t cornerSegments) {
            return roundedRect2D(size.x(), size.y(), cornerRadius, cornerSegments);
        }

        std::vector<vm::vec2f> roundedRect2D(const float width, const float height, const float cornerRadius, const size_t cornerSegments) {
            assert(cornerSegments > 0);
            assert(cornerRadius <= width / 2.0f &&
                   cornerRadius <= height / 2.0f);

            std::vector<vm::vec2f> vertices;
            vertices.resize(roundedRect2DVertexCount(cornerSegments));
            size_t vertexIndex = 0;

            const float angle = vm::Cf::half_pi() / static_cast<float>(cornerSegments);
            vm::vec2f center(0.0f, 0.0f);
            vm::vec2f translation;

            float curAngle = 0.0f;
            float x = std::cos(curAngle) * cornerRadius;
            float y = std::sin(curAngle) * cornerRadius;

            // lower right corner
            translation = vm::vec2f( (width  / 2.0f - cornerRadius),
                                -(height / 2.0f - cornerRadius));
            for (size_t i = 0; i < cornerSegments; ++i) {
                vertices[vertexIndex++] = center;
                vertices[vertexIndex++] = translation + vm::vec2f(x, y);

                curAngle -= angle;
                x = std::cos(curAngle) * cornerRadius;
                y = std::sin(curAngle) * cornerRadius;
                vertices[vertexIndex++] = translation + vm::vec2f(x, y);
            }

            // lower left corner
            translation = vm::vec2f(-(width  / 2.0f - cornerRadius),
                                -(height / 2.0f - cornerRadius));
            for (size_t i = 0; i < cornerSegments; ++i) {
                vertices[vertexIndex++] = center;
                vertices[vertexIndex++] = translation + vm::vec2f(x, y);

                curAngle -= angle;
                x = std::cos(curAngle) * cornerRadius;
                y = std::sin(curAngle) * cornerRadius;
                vertices[vertexIndex++] = translation + vm::vec2f(x, y);
            }

            // upper left corner
            translation = vm::vec2f(-(width  / 2.0f - cornerRadius),
                                (height / 2.0f - cornerRadius));
            for (size_t i = 0; i < cornerSegments; ++i) {
                vertices[vertexIndex++] = center;
                vertices[vertexIndex++] = translation + vm::vec2f(x, y);

                curAngle -= angle;
                x = std::cos(curAngle) * cornerRadius;
                y = std::sin(curAngle) * cornerRadius;
                vertices[vertexIndex++] = translation + vm::vec2f(x, y);
            }

            // upper right corner
            translation = vm::vec2f( (width  / 2.0f - cornerRadius),
                                (height / 2.0f - cornerRadius));
            for (size_t i = 0; i < cornerSegments; ++i) {
                vertices[vertexIndex++] = center;
                vertices[vertexIndex++] = translation + vm::vec2f(x, y);

                curAngle -= angle;
                x = std::cos(curAngle) * cornerRadius;
                y = std::sin(curAngle) * cornerRadius;
                vertices[vertexIndex++] = translation + vm::vec2f(x, y);
            }

            // upper body triangle
            vertices[vertexIndex++] = center;
            vertices[vertexIndex++] = vm::vec2f(-(width / 2.0f - cornerRadius), height / 2.0f);
            vertices[vertexIndex++] = vm::vec2f( (width / 2.0f - cornerRadius), height / 2.0f);

            // right body triangle
            vertices[vertexIndex++] = center;
            vertices[vertexIndex++] = vm::vec2f(width / 2.0f,  (height / 2.0f - cornerRadius));
            vertices[vertexIndex++] = vm::vec2f(width / 2.0f, -(height / 2.0f - cornerRadius));

            // lower body triangle
            vertices[vertexIndex++] = center;
            vertices[vertexIndex++] = vm::vec2f( (width / 2.0f - cornerRadius), -height / 2.0f);
            vertices[vertexIndex++] = vm::vec2f(-(width / 2.0f - cornerRadius), -height / 2.0f);

            // left body triangle
            vertices[vertexIndex++] = center;
            vertices[vertexIndex++] = vm::vec2f(-width / 2.0f, -(height / 2.0f - cornerRadius));
            vertices[vertexIndex++] = vm::vec2f(-width / 2.0f,  (height / 2.0f - cornerRadius));

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

            using MidPointCache = std::map<SphereBuilder::MidPointIndex, size_t>;
            size_t midPoint(std::vector<vm::vec3f>& vertices, MidPointCache& cache, const size_t index1, const size_t index2);

            size_t midPoint(std::vector<vm::vec3f>& vertices, MidPointCache& cache, const size_t index1, const size_t index2) {
                MidPointCache::iterator it = cache.find(MidPointIndex(index1, index2));
                if (it == std::end(cache)) {
                    const vm::vec3f& vertex1 = vertices[index1];
                    const vm::vec3f& vertex2 = vertices[index2];
                    vm::vec3f midPoint = (vertex1 + vertex2) / 2.0f;
                    vertices.push_back(normalize(midPoint));
                    size_t midPointIndex = vertices.size() - 1;
                    cache[MidPointIndex(index1, index2)] = midPointIndex;
                    cache[MidPointIndex(index2, index1)] = midPointIndex;
                    return midPointIndex;
                }
                return it->second;
            }
        }

        std::vector<vm::vec3f> sphere3D(const float radius, const size_t iterations) {
            assert(radius > 0.0f);
            assert(iterations > 0);

            using TriangleList = std::vector<SphereBuilder::Triangle>;

            std::vector<vm::vec3f> vertices;
            TriangleList triangles;

            // build initial icosahedron
            const float t = static_cast<float>((1.0 + std::sqrt(5.0)) / 2.0);
            vertices.push_back(normalize(vm::vec3f(-1.0f,     t,  0.0f)));
            vertices.push_back(normalize(vm::vec3f( 1.0f,     t,  0.0f)));
            vertices.push_back(normalize(vm::vec3f(-1.0f,    -t,  0.0f)));
            vertices.push_back(normalize(vm::vec3f( 1.0f,    -t,  0.0f)));

            vertices.push_back(normalize(vm::vec3f( 0.0f, -1.0f,     t)));
            vertices.push_back(normalize(vm::vec3f( 0.0f,  1.0f,     t)));
            vertices.push_back(normalize(vm::vec3f( 0.0f, -1.0f,    -t)));
            vertices.push_back(normalize(vm::vec3f( 0.0f,  1.0f,    -t)));

            vertices.push_back(normalize(vm::vec3f(    t,  0.0f, -1.0f)));
            vertices.push_back(normalize(vm::vec3f(    t,  0.0f,  1.0f)));
            vertices.push_back(normalize(vm::vec3f(   -t,  0.0f, -1.0f)));
            vertices.push_back(normalize(vm::vec3f(   -t,  0.0f,  1.0f)));

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
                for (SphereBuilder::Triangle& triangle : triangles) {
                    const size_t index1 = SphereBuilder::midPoint(vertices, cache, triangle[0], triangle[1]);
                    const size_t index2 = SphereBuilder::midPoint(vertices, cache, triangle[1], triangle[2]);
                    const size_t index3 = SphereBuilder::midPoint(vertices, cache, triangle[2], triangle[0]);
                    newTriangles.push_back(SphereBuilder::Triangle(triangle[0], index1, index3));
                    newTriangles.push_back(SphereBuilder::Triangle(triangle[1], index2, index1));
                    newTriangles.push_back(SphereBuilder::Triangle(triangle[2], index3, index2));
                    newTriangles.push_back(SphereBuilder::Triangle(index1, index2, index3));
                }
                triangles = std::move(newTriangles);
            }

            std::vector<vm::vec3f> allVertices;
            allVertices.reserve(3 * triangles.size());

            for (SphereBuilder::Triangle& triangle : triangles) {
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
            const float d = 2.0f * vm::Cf::pi() / static_cast<float>(segments);
            for (size_t i = 0; i < segments; i++) {
                result.vertices[i] = vm::vec3f(radius * std::sin(a), radius * std::cos(a), 0.0f);
                result.normals[i] = vm::vec3f::pos_z();
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
            const float d = 2.0f * vm::Cf::pi() / static_cast<float>(segments);
            for (size_t i = 0; i <= segments; ++i) {
                const float s = std::sin(a);
                const float c = std::cos(a);
                const float x = radius * s;
                const float y = radius * c;
                result.vertices[2*i+0] = vm::vec3f(x, y, length);
                result.vertices[2*i+1] = vm::vec3f(x, y, 0.0f);
                result.normals[2*i+0] = result.normals[2*i+1] = vm::vec3f(s, c, 0.0f);
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
            const float n = std::cos(vm::Cf::half_pi() - t);

            float a = 0.0f;
            const float d = 2.0f * vm::Cf::pi() / static_cast<float>(segments);
            float lastS = std::sin(a);
            float lastC = std::cos(a);
            a += d;

            for (size_t i = 0; i <= segments; ++i) {
                const float s = std::sin(a);
                const float c = std::cos(a);

                result.vertices[3*i+0] = vm::vec3f(0.0f, 0.0f, length);
                result.vertices[3*i+1] = vm::vec3f(radius * lastS, radius * lastC, 0.0f);
                result.vertices[3*i+2] = vm::vec3f(radius * s, radius * c, 0.0f);

                result.normals[3*i+0] = normalize(vm::vec3f(std::sin(a - d / 2.0f), std::cos(a - d / 2.0f), n));
                result.normals[3*i+1] = normalize(vm::vec3f(lastS, lastC, n));
                result.normals[3*i+2] = normalize(vm::vec3f(s, c, n));

                lastS = s;
                lastC = c;
                a += d;
            }
            return result;
        }
    }
}
