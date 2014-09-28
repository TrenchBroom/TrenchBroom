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

#include "MapRenderer.h"

#include "CollectionUtils.h"
#include "Renderer/ObjectRenderer.h"

namespace TrenchBroom {
    namespace Renderer {
        void MapRenderer::render(RenderContext& renderContext) {
            renderLayers(renderContext);
            renderSelection(renderContext);
            renderEntityLinks(renderContext);
        }
        
        void MapRenderer::renderLayers(RenderContext& renderContext) {
            RendererMap::iterator it, end;
            for (it = m_layerRenderers.begin(), end = m_layerRenderers.end(); it != end; ++it) {
                // const Model::Layer* layer = it->first;
                ObjectRenderer& renderer = it->second;
                renderer.render(renderContext);
            }
        }
        
        void MapRenderer::renderSelection(RenderContext& renderContext) {
            m_selectionRenderer.render(renderContext);
        }
        
        void MapRenderer::renderEntityLinks(RenderContext& renderContext) {
        }
    }
}
