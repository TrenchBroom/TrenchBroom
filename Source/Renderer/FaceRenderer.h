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

#ifndef __TrenchBroom__FaceRenderer__
#define __TrenchBroom__FaceRenderer__

#include "Renderer/TexturedPolygonSorter.h"
#include "Renderer/TextureVertexArray.h"
#include "Utility/Color.h"

namespace TrenchBroom {
    namespace Model {
        class Face;
        class Texture;
    }
    
    namespace Renderer {
        class RenderContext;
        class TextureRendererManager;
        class Vbo;
        
        class FaceRenderer {
        public:
            typedef TexturedPolygonSorter<Model::Texture, Model::Face*> Sorter;
        protected:
            typedef Sorter::PolygonCollection FaceCollection;
            typedef Sorter::PolygonCollectionMap FaceCollectionMap;

            Color m_faceColor;
            TextureVertexArrayList m_vertexArrays;
            TextureVertexArrayList m_transparentVertexArrays;
            
            static String AlphaBlendedTextures[];
            
            inline static bool alphaBlend(const String& textureName) {
                if (textureName.empty())
                    return false;
                if (textureName[0] == '*')
                    return true;
                for (size_t i = 0; i < 4; i++)
                    if (textureName == AlphaBlendedTextures[i])
                        return true;
                return false;
            }
            
            void writeFaceData(Vbo& vbo, TextureRendererManager& textureRendererManager, const Sorter& faceSorter);
            void render(RenderContext& context, bool grayScale, const Color* tintColor);
            void renderOpaqueFaces(ShaderProgram& shader, const bool applyTexture);
            void renderTransparentFaces(ShaderProgram& shader, const bool applyTexture);
            void renderFaces(const TextureVertexArrayList& vertexArrays, ShaderProgram& shader, const bool applyTexture);
        public:
            FaceRenderer(Vbo& vbo, TextureRendererManager& textureRendererManager, const Sorter& faceSorter, const Color& faceColor);
            
            void render(RenderContext& context, bool grayScale);
            void render(RenderContext& context, bool grayScale, const Color& tintColor);
        };
    }
}

#endif /* defined(__TrenchBroom__FaceRenderer__) */
