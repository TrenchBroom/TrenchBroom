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

#ifndef TrenchBroom_GridRenderer
#define TrenchBroom_GridRenderer

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Renderer/Renderable.h"
#include "Renderer/VertexArray.h"
#include "Renderer/VertexSpec.h"

namespace TrenchBroom {
    namespace Renderer {
        class OrthographicCamera;
        class RenderContext;
        class Vbo;
        
        class GridRenderer : public DirectRenderable {
        private:
            typedef VertexSpecs::P3::Vertex Vertex;
            VertexArray m_vertexArray;
        public:
            GridRenderer(const OrthographicCamera& camera, const BBox3& worldBounds);
        private:
            static Vertex::List vertices(const OrthographicCamera& camera, const BBox3& worldBounds);
            
            void doPrepareVertices(Vbo& vertexVbo);
            void doRender(RenderContext& renderContext);
        };
    }
}

#endif /* defined(TrenchBroom_GridRenderer) */
