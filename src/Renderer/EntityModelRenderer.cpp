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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "EntityModelRenderer.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Assets/Texture.h"
#include "Renderer/RenderContext.h"
#include "Renderer/ShaderManager.h"

namespace TrenchBroom {
    namespace Renderer {
        void EntityModelRenderer::render(RenderContext& context, const Model::Entity& entity) {
            VertexArrayMap::iterator it, end;
            for (it = m_vertexArrays.begin(),  end = m_vertexArrays.end(); it != end; ++it) {
                const Assets::Texture* texture = it->first;
                VertexArray& vertexArray = it->second;
                
                texture->activate();
                vertexArray.render();
                texture->deactivate();
            }
        }
    }
}
