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

#ifndef VertexArrayRenderer_h
#define VertexArrayRenderer_h

#include "Renderer/VertexArray.h"
#include "Renderer/VertexRenderSpec.h"

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }

    namespace Renderer {
        template <typename Key, typename Func>
        class VertexArrayRenderer {
        private:
            VertexArray m_vertexArray;
            KeyedVertexRenderSpec<Key, Func> m_renderSpec;
        public:
            VertexArrayRenderer(const VertexArray& vertexArray, const KeyedVertexRenderSpec<Key, Func>& renderSpec) :
            m_vertexArray(vertexArray),
            m_renderSpec(renderSpec) {}
            
            void render() {
                m_renderSpec.render(m_vertexArray);
            }
        };
        
        typedef VertexArrayRenderer<const Assets::Texture*, TextureFunc> TexturedVertexArrayRenderer;
    }
}

#endif /* VertexArrayRenderer_h */
