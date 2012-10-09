/*
 Copyright (C) 2010-2012 Kristian Duske
 
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

#include "Model/EntityTypes.h"
#include "Renderer/Shader/Shader.h"
#include "Renderer/Text/TextRenderer.h"
#include "Utility/String.h"

#include <memory>

namespace TrenchBroom {
    namespace Model {
        class Entity;
        class MapDocument;
    }
    
    namespace Renderer {
        class EntityModelRenderer;
        class Vbo;
        
        class EntityRenderer {
        private:
            class CachedEntityModelRenderer {
            public:
                EntityModelRenderer* renderer;
                String classname;
                CachedEntityModelRenderer() : renderer(NULL), classname("") {}
                CachedEntityModelRenderer(EntityModelRenderer* renderer, const String& classname) : renderer(renderer), classname(classname) {}
            };

            typedef std::map<Model::Entity*, CachedEntityModelRenderer> EntityModelRenderers;
            typedef Text::TextRenderer<Model::Entity*> EntityClassnameRenderer;
            typedef std::auto_ptr<EntityClassnameRenderer> EntityClassnameRendererPtr;

            Vbo& m_vbo;
            Model::MapDocument& m_document;

            VertexArrayPtr m_entityBoundsVertexArray;
            bool m_boundsValid;
            
            EntityModelRenderers m_modelRenderers;
            bool m_modelRendererCacheValid;
            
            EntityClassnameRendererPtr m_classnameRenderer;
        public:
            EntityRenderer(Vbo& vbo, Model::MapDocument& document);
            
            void addEntity(Model::Entity& entity);
            void addEntities(const Model::EntityList& entities);
            void updateEntity(Model::Entity& entity);
            void updateEntities(const Model::EntityList& entities);
            void removeEntity(Model::Entity& entity);
            void removeEntities(const Model::EntityList& entities);
            
            void render(RenderContext& context);
        };
    }
}

#endif /* defined(__TrenchBroom__EntityRenderer__) */
