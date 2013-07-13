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

#ifndef __TrenchBroom__FaceRenderer__
#define __TrenchBroom__FaceRenderer__

#include "Color.h"
#include "Model/BrushFace.h"
#include "Model/Texture.h"
#include "Renderer/VertexArray.h"

#include <map>

namespace TrenchBroom {
    namespace Renderer {
        class ActiveShader;
        class RenderContext;
        class Vbo;
        
        class FaceRenderer {
        private:
            typedef std::map<Model::Texture::Ptr, VertexArray> VertexArrayMap;
            
            VertexArrayMap m_arrays;
            Color m_faceColor;
        public:
            FaceRenderer();
            FaceRenderer(Vbo& vbo, const Model::BrushFace::Mesh& mesh, const Color& faceColor);

            void render(RenderContext& context, const bool grayScale);
            void render(RenderContext& context, const bool grayScale, const Color& tintColor);
        private:
            void render(RenderContext& context, bool grayScale, const Color* tintColor);
            void renderOpaqueFaces(ActiveShader& shader, const bool applyTexture);
            void renderTransparentFaces(ActiveShader& shader, const bool applyTexture);
            void renderFaces(VertexArrayMap& arrays, ActiveShader& shader, const bool applyTexture);
        };
    }
}

#endif /* defined(__TrenchBroom__FaceRenderer__) */
