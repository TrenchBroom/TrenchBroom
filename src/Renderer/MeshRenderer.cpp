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

#include "MeshRenderer.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/Vbo.h"

namespace TrenchBroom {
    namespace Renderer {
        MeshRenderer::MeshRenderer() :
        m_prepared(true) {}

        bool MeshRenderer::empty() const {
            return m_renderData.empty();
        }

        void MeshRenderer::prepare(Vbo& vbo) {
            if (m_prepared)
                return;
            
            SetVboState mapVbo(vbo);
            mapVbo.mapped();
            
            RenderData::List::iterator it, end;
            for (it = m_renderData.begin(),  end = m_renderData.end(); it != end; ++it) {
                RenderData& renderData = *it;
                renderData.triangles.prepare(vbo);
                renderData.triangleFans.prepare(vbo);
                renderData.triangleStrips.prepare(vbo);
            }
            
            m_prepared = true;
        }

        struct NopFunc {
            void operator()(const Assets::Texture* texture) const {}
        };
        
        void MeshRenderer::render() {
            render(NopFunc());
        }
    }
}
