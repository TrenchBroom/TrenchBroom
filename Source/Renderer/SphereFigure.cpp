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

#include "SphereFigure.h"

#include "Renderer/ApplyMatrix.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Vbo.h"
#include "Renderer/VertexArray.h"

#include <map>
#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        size_t SphereFigure::midPoint(Vec3f::List& vertices, MidPointCache& cache, size_t index1, size_t index2) {
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

        /**
         see http://blog.andreaskahler.com/2009/06/creating-icosphere-mesh-in-code.html
         */
        Vec3f::List SphereFigure::makeVertices() {
            typedef std::vector<Triangle> TriangleList;
            Vec3f::List vertices;
            TriangleList triangles;

            // build initial icosahedron
            float t = static_cast<float>((1.0 + sqrt(5.0)) / 2.0);
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
            triangles.push_back(Triangle( 0,  5, 11));
            triangles.push_back(Triangle( 0,  1,  5));
            triangles.push_back(Triangle( 0,  7,  1));
            triangles.push_back(Triangle( 0, 10,  7));
            triangles.push_back(Triangle( 0, 11, 10));
            
            // 5 adjacent faces
            triangles.push_back(Triangle( 4, 11,  5));
            triangles.push_back(Triangle( 9,  5,  1));
            triangles.push_back(Triangle( 8,  1,  7));
            triangles.push_back(Triangle( 6,  7, 10));
            triangles.push_back(Triangle( 2, 10, 11));
            
            // 5 faces around point 3
            triangles.push_back(Triangle( 3,  2,  4));
            triangles.push_back(Triangle( 3,  6,  2));
            triangles.push_back(Triangle( 3,  8,  6));
            triangles.push_back(Triangle( 3,  9,  8));
            triangles.push_back(Triangle( 3,  4,  9));
            
            // 5 adjacent faces
            triangles.push_back(Triangle(11,  4,  2));
            triangles.push_back(Triangle(10,  2,  6));
            triangles.push_back(Triangle( 7,  6,  8));
            triangles.push_back(Triangle( 1,  8,  9));
            triangles.push_back(Triangle( 5,  9,  4));
            
            // subdivide the icosahedron
            MidPointCache cache;
            for (unsigned int i = 0; i < m_iterations; i++) {
                TriangleList newTriangles;
                TriangleList::iterator it, end;
                for (it = triangles.begin(), end = triangles.end(); it != end; ++it) {
                    Triangle& triangle = *it;
                    size_t index1 = midPoint(vertices, cache, triangle[0], triangle[1]);
                    size_t index2 = midPoint(vertices, cache, triangle[1], triangle[2]);
                    size_t index3 = midPoint(vertices, cache, triangle[2], triangle[0]);
                    newTriangles.push_back(Triangle(triangle[0], index1, index3));
                    newTriangles.push_back(Triangle(triangle[1], index2, index1));
                    newTriangles.push_back(Triangle(triangle[2], index3, index2));
                    newTriangles.push_back(Triangle(index1, index2, index3));
                }
                triangles = newTriangles;
            }
            
            Vec3f::List allVertices;
            allVertices.reserve(3 * triangles.size());
            TriangleList::iterator it, end;
            for (it = triangles.begin(), end = triangles.end(); it != end; ++it) {
                Triangle& triangle = *it;
                for (unsigned int i = 0; i < 3; i++)
                    allVertices.push_back(m_radius * vertices[triangle[i]]);
            }
            
            return allVertices;
        }

        SphereFigure::SphereFigure(float radius, unsigned int iterations) :
        m_radius(radius),
        m_iterations(iterations) {}

        void SphereFigure::render(Vbo& vbo, RenderContext& context) {
            SetVboState activateVbo(vbo, Vbo::VboActive);

            if (m_vertexArray.get() == NULL) {
                Vec3f::List vertices = makeVertices();

                unsigned int vertexCount = static_cast<unsigned int>(vertices.size());
                m_vertexArray = VertexArrayPtr(new VertexArray(vbo, GL_TRIANGLES, vertexCount,
                                                               Attribute::position3f()));
                
                SetVboState mapVbo(vbo, Vbo::VboMapped);
                Vec3f::List::iterator it, end;
                for (it = vertices.begin(), end = vertices.end(); it != end; ++it)
                    m_vertexArray->addAttribute(*it);
            }
            
            m_vertexArray->render();
        }
    }
}