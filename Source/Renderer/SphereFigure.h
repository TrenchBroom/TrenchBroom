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

#ifndef __TrenchBroom__SphereFigure__
#define __TrenchBroom__SphereFigure__

#include "Renderer/Figure.h"
#include "Utility/Color.h"
#include "Utility/VecMath.h"

#include <cassert>
#include <map>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Renderer {
        class Vbo;
        class VertexArray;
        
        class SphereFigure : public Figure {
        protected:
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
            
            typedef std::map<MidPointIndex, size_t> MidPointCache;
                    
            float m_radius;
            unsigned int m_iterations;
            VertexArray* m_vertexArray;

            size_t midPoint(Vec3f::List& vertices, MidPointCache& cache, size_t index1, size_t index2);
            Vec3f::List makeVertices();
        public:
            SphereFigure(float radius, unsigned int iterations);
            ~SphereFigure();
                    
            void render(Vbo& vbo, RenderContext& context);
        };
    }
}

#endif /* defined(__TrenchBroom__SphereFigure__) */
