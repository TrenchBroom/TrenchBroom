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

#ifndef TrenchBroom_TriangleRenderer
#define TrenchBroom_TriangleRenderer

#include "Color.h"
#include "Renderer/IndexRangeMap.h"
#include "Renderer/Renderable.h"
#include "Renderer/VertexSpec.h"
#include "Renderer/VertexArray.h"

#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        class RenderContext;
        
        class TriangleRenderer : public DirectRenderable {
        private:
            VertexArray m_vertexArray;
            IndexRangeMap m_indexArray;
            
            Color m_color;
            bool m_useColor;
            Color m_tintColor;
            bool m_applyTinting;
        public:
            TriangleRenderer();
            TriangleRenderer(const VertexArray& vertexArray, const IndexRangeMap& indexArray);
            TriangleRenderer(const VertexArray& vertexArray, PrimType primType);
            TriangleRenderer(const TriangleRenderer& other);
            TriangleRenderer& operator=(TriangleRenderer other);
            
            friend void swap(TriangleRenderer& left, TriangleRenderer& right);
            
            void setUseColor(bool useColor);
            void setColor(const Color& color);
            void setApplyTinting(bool applyTinting);
            void setTintColor(const Color& tintColor);
        private:
            void doPrepareVertices(Vbo& vertexVbo);
            void doRender(RenderContext& context);
        };
    }
}

#endif /* defined(TrenchBroom_TriangleRenderer) */
