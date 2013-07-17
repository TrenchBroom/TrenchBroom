/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#ifndef __TrenchBroom__BrushRenderer__
#define __TrenchBroom__BrushRenderer__

#include "Renderer/EdgeRenderer.h"
#include "Renderer/FaceRenderer.h"

namespace TrenchBroom {
    namespace Renderer {
        class RenderContext;
        class Vbo;
        
        class BrushRenderer {
        private:
            bool m_grayScale;
            bool m_edgeDepthTesting;
            FaceRenderer m_faceRenderer;
            EdgeRenderer m_edgeRenderer;
        public:
            BrushRenderer();
            BrushRenderer(Vbo& vbo, const Model::BrushFace::Mesh& faces, const VertexSpecs::P3::Vertex::List& edges);
            BrushRenderer(Vbo& vbo, const Model::BrushFace::Mesh& faces, const VertexSpecs::P3C4::Vertex::List& edges);
            
            void setGrayScale(const bool grayScale);
            void setEdgeDepthTesting(const bool edgeDepthTesting);
            
            void render(RenderContext& context);
        private:
            static Color faceColor();
            static Color edgeColor();
        };
    }
}

#endif /* defined(__TrenchBroom__BrushRenderer__) */
