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

#ifndef TexturedIndexArrayRenderer_h
#define TexturedIndexArrayRenderer_h

#include "SharedPointer.h"
#include "Renderer/IndexArray.h"
#include "Renderer/TexturedIndexArrayMap.h"

#include <map>

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }
    
    namespace Renderer {
        class Vbo;
        class TextureRenderFunc;
        
        class TexturedIndexArrayRenderer {
        private:
        private:
            IndexArray m_indexArray;
            TexturedIndexArrayMap m_indexRanges;
        public:
            TexturedIndexArrayRenderer();
            TexturedIndexArrayRenderer(const IndexArray& indexArray, const TexturedIndexArrayMap& indexArrayMap);

            bool empty() const;
            
            void prepare(Vbo& indexVbo);
            void render();
            void render(TextureRenderFunc& func);
        };
    }
}

#endif /* TexturedIndexArrayRenderer_h */
