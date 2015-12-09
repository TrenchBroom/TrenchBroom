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

#ifndef TrenchBroom_Renderable
#define TrenchBroom_Renderable

#include <stdio.h>

namespace TrenchBroom {
    namespace Renderer {
        class RenderContext;
        class Vbo;
        
        class Renderable {
        public:
            virtual ~Renderable();
            void render(RenderContext& renderContext);
        private:
            virtual void doRender(RenderContext& renderContext) = 0;
        };
        
        class DirectRenderable : public Renderable {
        public:
            virtual ~DirectRenderable();
            void prepareVertices(Vbo& vertexVbo);
        private:
            virtual void doPrepareVertices(Vbo& vertexVbo) = 0;
        };

        class IndexedRenderable : public Renderable {
        public:
            virtual ~IndexedRenderable();
            void prepareVertices(Vbo& vertexVbo);
            void prepareIndices(Vbo& indexVbo);
        private:
            virtual void doPrepareVertices(Vbo& vertexVbo) = 0;
            virtual void doPrepareIndices(Vbo& indexVbo) = 0;
        };
    }
}

#endif /* defined(TrenchBroom_Renderable) */
