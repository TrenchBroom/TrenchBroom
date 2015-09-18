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

#ifndef TrenchBroom_LineMeshRenderer
#define TrenchBroom_LineMeshRenderer

#include "Renderer/LineMesh.h"

namespace TrenchBroom {
    namespace Renderer {
        class Vbo;
        class LineMeshRenderer {
        private:
            LineMeshRenderData m_renderData;
            bool m_prepared;
        public:
            LineMeshRenderer();
            
            template <typename VertexSpec>
            LineMeshRenderer(LineMesh<VertexSpec>& mesh) :
            m_renderData(mesh.renderData()),
            m_prepared(false) {}
            
            bool prepared() const;
            void prepare(Vbo& vbo);
            
            void render();
        };
    }
}

#endif /* defined(TrenchBroom_LineMeshRenderer) */
