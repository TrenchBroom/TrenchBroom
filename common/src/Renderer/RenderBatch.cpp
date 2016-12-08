/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "RenderBatch.h"

#include "CollectionUtils.h"
#include "Renderer/Renderable.h"
#include "Renderer/Vbo.h"

namespace TrenchBroom {
    namespace Renderer {
        class RenderBatch::IndexedRenderableWrapper : public IndexedRenderable {
        private:
            Vbo& m_indexBuffer;
            IndexedRenderable* m_wrappee;
        public:
            IndexedRenderableWrapper(Vbo& indexBuffer, IndexedRenderable* wrappee) :
            m_indexBuffer(indexBuffer),
            m_wrappee(wrappee) {
                ensure(m_wrappee != NULL, "wrappee is null");
            }
        private:
            void doPrepareVertices(Vbo& vertexVbo) {
                m_wrappee->prepareVertices(vertexVbo);
            }
            
            void doPrepareIndices(Vbo& indexVbo) {
                m_wrappee->prepareIndices(indexVbo);
            }
            
            void doRender(RenderContext& renderContext) {
                ActivateVbo activate(m_indexBuffer);
                m_wrappee->render(renderContext);
            }
        };
        
        RenderBatch::RenderBatch(Vbo& vertexVbo, Vbo& indexVbo) :
        m_vertexVbo(vertexVbo),
        m_indexVbo(indexVbo) {}
        
        RenderBatch::~RenderBatch() {
            ListUtils::clearAndDelete(m_oneshots);
            ListUtils::clearAndDelete(m_indexedRenderables);
        }
        
        void RenderBatch::add(Renderable* renderable) {
            doAdd(renderable);
        }
        
        void RenderBatch::add(DirectRenderable* renderable) {
            doAdd(renderable);
            m_directRenderables.push_back(renderable);
        }
        
        void RenderBatch::add(IndexedRenderable* renderable) {
            IndexedRenderableWrapper* wrapper = new IndexedRenderableWrapper(m_indexVbo, renderable);
            doAdd(wrapper);
            m_indexedRenderables.push_back(wrapper);
        }
        
        void RenderBatch::addOneShot(Renderable* renderable) {
            doAdd(renderable);
            m_oneshots.push_back(renderable);
        }

        void RenderBatch::addOneShot(DirectRenderable* renderable) {
            doAdd(renderable);
            m_directRenderables.push_back(renderable);
            m_oneshots.push_back(renderable);
        }
        
        void RenderBatch::addOneShot(IndexedRenderable* renderable) {
            IndexedRenderableWrapper* wrapper = new IndexedRenderableWrapper(m_indexVbo, renderable);

            doAdd(wrapper);
            m_indexedRenderables.push_back(wrapper);
            m_oneshots.push_back(renderable);
        }
        
        void RenderBatch::render(RenderContext& renderContext) {
            ActivateVbo activate(m_vertexVbo);

            prepareRenderables();
            renderRenderables(renderContext);
        }

        void RenderBatch::doAdd(Renderable* renderable) {
            ensure(renderable != NULL, "renderable is null");
            m_batch.push_back(renderable);
        }

        void RenderBatch::prepareRenderables() {
            prepareVertices();
            prepareIndices();
        }
        
        void RenderBatch::prepareVertices() {
            ActivateVbo activate(m_vertexVbo);
            
            DirectRenderableList::const_iterator dIt, dEnd;
            for (dIt = std::begin(m_directRenderables), dEnd = std::end(m_directRenderables); dIt != dEnd; ++dIt) {
                DirectRenderable* renderable = *dIt;
                renderable->prepareVertices(m_vertexVbo);
            }
            
            IndexedRenderableList::const_iterator iIt, iEnd;
            for (iIt = std::begin(m_indexedRenderables), iEnd = std::end(m_indexedRenderables); iIt != iEnd; ++iIt) {
                IndexedRenderable* renderable = *iIt;
                renderable->prepareVertices(m_vertexVbo);
            }
        }
        
        void RenderBatch::prepareIndices() {
            ActivateVbo activate(m_indexVbo);
            
            IndexedRenderableList::const_iterator it, end;
            for (it = std::begin(m_indexedRenderables), end = std::end(m_indexedRenderables); it != end; ++it) {
                IndexedRenderable* renderable = *it;
                renderable->prepareIndices(m_indexVbo);
            }
        }

        void RenderBatch::renderRenderables(RenderContext& renderContext) {
            RenderableList::const_iterator it, end;
            for (it = std::begin(m_batch), end = std::end(m_batch); it != end; ++it) {
                Renderable* renderable = *it;
                renderable->render(renderContext);
            }
        }
    }
}
