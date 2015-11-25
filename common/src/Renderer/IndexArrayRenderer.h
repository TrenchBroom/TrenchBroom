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

#ifndef IndexArrayRenderer_h
#define IndexArrayRenderer_h

#include "Renderer/IndexArray.h"
#include "Renderer/Renderable.h"
#include "Renderer/VertexArray.h"

namespace TrenchBroom {
    namespace Renderer {
        class RenderContext;
        class Vbo;
        
        class IndexArrayRenderer : public Renderable {
        private:
            VertexArray m_vertexArray;
            IndexArray m_indexArray;
        public:
            IndexArrayRenderer(const VertexArray& vertexArray, const IndexArray& indexArray);
        private:
            void doPrepare(Vbo& vbo);
            void doRender(RenderContext& renderContext);
        };
    }
}

#endif /* IndexArrayRenderer_h */
