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
        public:
            typedef enum {
                BRUnselected,
                BRSelected
            } Config;
        private:
            Config m_config;
            FaceRenderer m_faceRenderer;
            EdgeRenderer m_edgeRenderer;
        public:
            BrushRenderer(const Config config);
            
            void update(Vbo& vbo, const Model::BrushFace::Mesh& faces, const VertexSpecs::P3::Vertex::List& edges);
            void update(Vbo& vbo, const Model::BrushFace::Mesh& faces, const VertexSpecs::P3C4::Vertex::List& edges);
            
            void render(RenderContext& context);
        private:
            bool grayScale() const;
            const Color& faceColor() const;
            const Color& tintColor() const;
            const Color& edgeColor() const;
            const Color& occludedEdgeColor() const;
        };
    }
}

#endif /* defined(__TrenchBroom__BrushRenderer__) */
