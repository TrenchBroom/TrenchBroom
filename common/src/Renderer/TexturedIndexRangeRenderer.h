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

#ifndef TexturedIndexRangeRenderer_h
#define TexturedIndexRangeRenderer_h

#include "Renderer/TexturedIndexRangeMap.h"
#include "Renderer/VertexArray.h"

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }
    
    namespace Renderer {
        class Vbo;
        class TextureRenderFunc;
        
        class TexturedIndexRangeRenderer {
        private:
            VertexArray m_vertexArray;
            TexturedIndexRangeMap m_indexRange;
        public:
            TexturedIndexRangeRenderer();
            TexturedIndexRangeRenderer(const VertexArray& vertexArray, const TexturedIndexRangeMap& indexRange);
            TexturedIndexRangeRenderer(const VertexArray& vertexArray, const Assets::Texture* texture, const IndexRangeMap& indexRange);

            bool empty() const;
            
            void prepare(Vbo& vbo);
            void render();
            void render(TextureRenderFunc& func);
        };
    }
}

#endif /* TexturedIndexRangeRenderer_h */
