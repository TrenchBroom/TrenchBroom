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

#include "Utility/Color.h"

#include <map>
#include <memory>
#include <set>

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

            Vbo& m_boundsVbo;
            Model::MapDocument& m_document;
            
            Model::EntitySet m_entities;
            VertexArrayPtr m_boundsVertexArray;
            bool m_boundsValid;
            EntityModelRenderers m_modelRenderers;
            bool m_modelRendererCacheValid;
            EntityClassnameRendererPtr m_classnameRenderer;
            
            Color m_classnameColor;
            Color m_classnameBackgroundColor;
            bool m_applyColor;
            Color m_color;
            bool m_renderOcclusion;
            Color m_occlusionColor;
            
            void writeColoredBounds(RenderContext& context, const Model::EntityList& entities);
            void writeBounds(RenderContext& context, const Model::EntityList& entities);
            void validateBounds(RenderContext& context);
            void validateModels(RenderContext& context);
            
            void renderBounds(RenderContext& context);
            void renderClassnames(RenderContext& context);
            void renderModels(RenderContext& context);
            void renderFigures(RenderContext& context);
        public:
            EntityRenderer(Vbo& boundsVbo, Model::MapDocument& document, float classnameFadeDistance);
            EntityRenderer(Vbo& boundsVbo, Model::MapDocument& document, float classnameFadeDistance, const Color& color);
            EntityRenderer(Vbo& boundsVbo, Model::MapDocument& document, float classnameFadeDistance, const Color& color, const Color& occlusionColor);
            
            void addEntity(Model::Entity& entity);
            void addEntities(const Model::EntityList& entities);
            void removeEntity(Model::Entity& entity);
            void removeEntities(const Model::EntityList& entities);
            void invalidateBounds();
            void invalidateModels();
            void clear();
            
            void render(RenderContext& context);
        };
    }
}

#endif /* defined(__TrenchBroom__EntityRenderer__) */
