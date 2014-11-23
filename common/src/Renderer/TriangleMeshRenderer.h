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

#ifndef __TrenchBroom__TriangleMeshRenderer__
#define __TrenchBroom__TriangleMeshRenderer__

#include "Assets/Texture.h"
#include "Renderer/VertexSpec.h"
#include "Renderer/TriangleMesh.h"

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }
    
    namespace Renderer {
        class RenderContext;
        class VertexArray;
        
        template <typename Key>
        class TriangleMeshRendererBase {
        private:
            typedef TriangleMeshRenderData<Key> RenderData;
            typename RenderData::List m_renderData;
            bool m_prepared;
        public:
            struct MeshFuncBase {
                virtual ~MeshFuncBase() {}
                virtual void before(const Key& key) const = 0;
                virtual void after(const Key& key) const = 0;
            };
        private:
            struct NopMeshFunc : public MeshFuncBase {
                void before(const Key& key) const {}
                void after(const Key& key) const {}
            };
        protected:
            TriangleMeshRendererBase() :
            m_prepared(true) {}
            
            template <typename VertexSpec>
            TriangleMeshRendererBase(TriangleMesh<VertexSpec, Key>& mesh) :
            m_renderData(mesh.renderData()),
            m_prepared(false) {}
        public:
            bool empty() const {
                return m_renderData.empty();
            }
            
            bool prepared() const {
                return m_prepared;
            }
            
            void prepare(Vbo& vbo) {
                if (m_prepared)
                    return;
                
                typename RenderData::List::iterator it, end;
                for (it = m_renderData.begin(),  end = m_renderData.end(); it != end; ++it) {
                    RenderData& renderData = *it;
                    renderData.triangles.prepare(vbo);
                    renderData.triangleFans.prepare(vbo);
                    renderData.triangleStrips.prepare(vbo);
                }
                
                m_prepared = true;
            }
        public:
            void render() {
                render(NopMeshFunc());
            }
            
            void render(const MeshFuncBase& func) {
                typename RenderData::List::iterator it, end;
                for (it = m_renderData.begin(),  end = m_renderData.end(); it != end; ++it) {
                    RenderData& renderData = *it;
                    const Key& key = renderData.key;
                    
                    func.before(key);
                    renderData.triangles.render();
                    renderData.triangleFans.render();
                    renderData.triangleStrips.render();
                    func.after(key);
                }
            }
        };
        
        class TexturedTriangleMeshRenderer : public TriangleMeshRendererBase<const Assets::Texture*> {
        public:
            TexturedTriangleMeshRenderer() :
            TriangleMeshRendererBase() {}
            
            template <typename VertexSpec>
            TexturedTriangleMeshRenderer(TriangleMesh<VertexSpec, const Assets::Texture*>& mesh) :
            TriangleMeshRendererBase(mesh) {}
        };
        
        class SimpleTriangleMeshRenderer : public TriangleMeshRendererBase<int> {
        public:
            SimpleTriangleMeshRenderer() :
            TriangleMeshRendererBase() {}
            
            template <typename VertexSpec>
            SimpleTriangleMeshRenderer(TriangleMesh<VertexSpec, int>& mesh) :
            TriangleMeshRendererBase(mesh) {}
        };
    }
}

#endif /* defined(__TrenchBroom__TriangleMeshRenderer__) */
