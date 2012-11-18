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

#ifndef __TrenchBroom__EdgeRenderer__
#define __TrenchBroom__EdgeRenderer__

#include "Model/BrushTypes.h"
#include "Model/FaceTypes.h"
#include "Renderer/RenderTypes.h"
#include "Utility/Color.h"

namespace TrenchBroom {
    namespace Renderer {
        class RenderContext;
        class Vbo;
        
        class EdgeRenderer {
        protected:
            VertexArrayPtr m_vertexArray;
            
            unsigned int vertexCount(const Model::BrushList& brushes, const Model::FaceList& faces);
            void writeEdgeData(Vbo& vbo, const Model::BrushList& brushes, const Model::FaceList& faces);
            void writeEdgeData(Vbo& vbo, const Model::BrushList& brushes, const Model::FaceList& faces, const Color& defaultColor);
        public:
            EdgeRenderer(Vbo& vbo, const Model::BrushList& brushes, const Model::FaceList& faces);
            EdgeRenderer(Vbo& vbo, const Model::BrushList& brushes, const Model::FaceList& faces, const Color& defaultColor);

            void render(RenderContext& context);
            void render(RenderContext& context, const Color& color);
        };
    }
}

#endif /* defined(__TrenchBroom__EdgeRenderer__) */
