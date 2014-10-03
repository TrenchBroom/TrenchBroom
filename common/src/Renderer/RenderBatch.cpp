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

#include "RenderBatch.h"

#include "CollectionUtils.h"
#include "Renderer/Renderable.h"

namespace TrenchBroom {
    namespace Renderer {
        RenderBatch::RenderBatch(Vbo& vbo) :
        m_vbo(vbo) {}
        
        RenderBatch::~RenderBatch() {
            VectorUtils::clearAndDelete(m_oneshots);
        }
        
        void RenderBatch::add(Renderable* renderable) {
            assert(renderable != NULL);
            m_batch.push_back(renderable);
        }
        
        void RenderBatch::addOneShot(Renderable* renderable) {
            assert(renderable != NULL);
            add(renderable);
            m_oneshots.push_back(renderable);
        }
        
        void RenderBatch::render(RenderContext& renderContext) {
            SetVboState setVboState(m_vbo);
            setVboState.mapped();
            prepareRenderables();
            setVboState.active();
            renderRenderables(renderContext);
            
        }

        void RenderBatch::prepareRenderables() {
            RenderableList::const_iterator it, end;
            for (it = m_batch.begin(), end = m_batch.end(); it != end; ++it) {
                Renderable* renderable = *it;
                renderable->prepare(m_vbo);
            }
        }
        
        void RenderBatch::renderRenderables(RenderContext& renderContext) {
            RenderableList::const_iterator it, end;
            for (it = m_batch.begin(), end = m_batch.end(); it != end; ++it) {
                Renderable* renderable = *it;
                renderable->render(renderContext);
            }
        }
    }
}
