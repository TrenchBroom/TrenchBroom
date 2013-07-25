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

#include "SingleEntityRenderer.h"

#include "TrenchBroom.h"
#include "VecMath.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Model/Entity.h"
#include "Assets/EntityDefinition.h"
#include "Renderer/Vbo.h"
#include "Renderer/VboBlock.h"
#include "Renderer/VertexSpec.h"

#include <cassert>

namespace TrenchBroom {
    namespace Renderer {
        struct BuildBoundsVertices {
            VertexSpecs::P3C4::Vertex::List& vertices;
            Color color;
            
            BuildBoundsVertices(VertexSpecs::P3C4::Vertex::List& i_vertices, const Color& i_color) :
            vertices(i_vertices),
            color(i_color) {}
            
            inline void operator()(const Vec3& v1, const Vec3& v2) {
                vertices.push_back(VertexSpecs::P3C4::Vertex(v1, color));
                vertices.push_back(VertexSpecs::P3C4::Vertex(v2, color));
            }
        };
        
        SingleEntityRenderer::SingleEntityRenderer(const Model::Entity* entity) :
        m_entity(entity) {
            assert(m_entity != NULL);
        }
        
        void SingleEntityRenderer::getBoundsVertices(VertexSpecs::P3C4::Vertex::List& vertices) const {
            BuildBoundsVertices builder(vertices, boundsColor());
            const BBox3 bounds = m_entity->bounds();
            eachBBoxEdge(bounds, builder);
        }
        
        void SingleEntityRenderer::render(RenderContext& context) {
        }

        const Color& SingleEntityRenderer::boundsColor() const {
            const Assets::EntityDefinition* definition = m_entity->definition();
            if (definition == NULL) {
                PreferenceManager& prefs = PreferenceManager::instance();
                return prefs.getColor(Preferences::UndefinedEntityColor);
            }
            return definition->color();
        }
    }
}
