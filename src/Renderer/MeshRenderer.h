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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__MeshRenderer__
#define __TrenchBroom__MeshRenderer__

#include "Assets/Texture.h"
#include "Renderer/VertexSpec.h"
#include "Renderer/Mesh.h"

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }
    
    namespace Model {
        class Entity;
    }
    
    namespace Renderer {
        class RenderContext;
        class VertexArray;
        
        class MeshRenderer {
        private:
            typedef MeshRenderData<Assets::Texture*> RenderData;
            RenderData::List m_renderData;
            bool m_prepared;
        public:
            MeshRenderer();
            
            template <typename VertexSpec>
            MeshRenderer(Vbo& vbo, const Mesh<Assets::Texture*, VertexSpec>& mesh) :
            m_renderData(mesh.renderData(vbo)),
            m_prepared(false) {}
            
            bool empty() const;
            
            void prepare();
            void render();

            template <class MeshFunc>
            void render(const MeshFunc& func) {
                RenderData::List::iterator it, end;
                for (it = m_renderData.begin(),  end = m_renderData.end(); it != end; ++it) {
                    RenderData& renderData = *it;
                    Assets::Texture* texture = renderData.key;
                    
                    if (texture != NULL)
                        texture->activate();
                    func(texture);
                    renderData.triangles.render();
                    renderData.triangleFans.render();
                    renderData.triangleStrips.render();
                    if (texture != NULL)
                        texture->deactivate();
                }
            }
        };
    }
}

#endif /* defined(__TrenchBroom__MeshRenderer__) */
