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

#ifndef __TrenchBroom__EntityRenderer__
#define __TrenchBroom__EntityRenderer__

#include "Model/ModelTypes.h"
#include "Renderer/EdgeRenderer.h"

#include <map>

namespace TrenchBroom {
    namespace Renderer {
        class RenderContext;
        class SingleEntityRenderer;
        class Vbo;
        class VboBlock;
        
        class EntityRenderer {
        private:
            typedef std::map<const Model::Entity*, SingleEntityRenderer*> Cache;
            Cache m_renderers;
            EdgeRenderer m_boundsRenderer;
            bool m_boundsValid;
        public:
            EntityRenderer();
            ~EntityRenderer();

            void addEntity(const Model::Entity* entity);
            void addEntities(const Model::EntityList& entities);
            void updateEntity(const Model::Entity* entity);
            void updateEntities(const Model::EntityList& entities);
            void removeEntity(const Model::Entity* entity);
            void removeEntities(const Model::EntityList& entities);
            void clear();
            
            bool boundsValid() const;
            void validateBounds(Vbo& boundsVbo);
            void render(RenderContext& context);
        private:
            SingleEntityRenderer* createRenderer(const Model::Entity* entity) const;
            void invalidateBounds();
        };
    }
}

#endif /* defined(__TrenchBroom__EntityRenderer__) */
