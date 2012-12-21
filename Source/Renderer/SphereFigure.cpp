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
#include "Renderer/RenderUtils.h"
#include "Renderer/Vbo.h"
#include "Renderer/VertexArray.h"

#include <map>
#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        SphereFigure::SphereFigure(float radius, unsigned int iterations) :
        m_radius(radius),
        m_iterations(iterations),
        m_vertexArray(NULL) {}

        SphereFigure::~SphereFigure() {
            delete m_vertexArray;
            m_vertexArray = NULL;
        }

        void SphereFigure::render(Vbo& vbo, RenderContext& context) {
            SetVboState activateVbo(vbo, Vbo::VboActive);

            if (m_vertexArray == NULL) {
                Vec3f::List vertices = sphere(m_radius, m_iterations);

                unsigned int vertexCount = static_cast<unsigned int>(vertices.size());
                m_vertexArray = new VertexArray(vbo, GL_TRIANGLES, vertexCount,
                                                Attribute::position3f());
                
                SetVboState mapVbo(vbo, Vbo::VboMapped);
                Vec3f::List::iterator it, end;
                for (it = vertices.begin(), end = vertices.end(); it != end; ++it)
                    m_vertexArray->addAttribute(*it);
            }
            
            assert(m_vertexArray != NULL);
            m_vertexArray->render();
        }
    }
}
