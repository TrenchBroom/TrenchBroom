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

#include "LineMeshRenderer.h"

namespace TrenchBroom {
    namespace Renderer {
        LineMeshRenderer::LineMeshRenderer() :
        m_prepared(false) {}
        
        bool LineMeshRenderer::prepared() const {
            return m_prepared;
        }
        
        void LineMeshRenderer::prepare(Vbo& vbo) {
            if (m_prepared)
                return;
            
            m_renderData.lines.prepare(vbo);
            m_renderData.lineStrips.prepare(vbo);
            m_renderData.lineLoops.prepare(vbo);
            m_prepared = true;
        }
        
        void LineMeshRenderer::render() {
            m_renderData.lines.render();
            m_renderData.lineStrips.render();
            m_renderData.lineLoops.render();
        }
    }
}
