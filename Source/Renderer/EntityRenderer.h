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
#include "Renderer/RenderContext.h"
#include "Renderer/Shader/Shader.h"
#include "Renderer/Text/TextRenderer.h"
#include "Utility/String.h"

#include "Utility/Color.h"

#include <map>
#include <set>

namespace TrenchBroom {
    namespace Model {
        class Entity;
        class MapDocument;
    }
    
    namespace Renderer {
        class EntityModelRenderer;
        class Vbo;
        class VertexArray;
        
        class EntityRenderer {
        private:
            class CachedEntityModelRenderer {
            public:
                EntityModelRenderer* renderer;
                String classname;
                
                CachedEntityModelRenderer() : renderer(NULL), classname("") {}
                
                CachedEntityModelRenderer(EntityModelRenderer* i_renderer, const String& i_classname) :
                renderer(i_renderer),
                classname(i_classname) {}
            };
            
            class EntityClassnameAnchor : public Text::TextAnchor {
            private:
                Model::Entity* m_entity;
                Renderer::EntityModelRenderer* m_renderer;
            protected:
                inline const Vec3f basePosition() const;
                inline const Text::Alignment::Type alignment() const;
            public:
                EntityClassnameAnchor(Model::Entity& entity, Renderer::EntityModelRenderer* renderer);
            };
            
            typedef Model::Entity* EntityKey;
            typedef std::map<EntityKey, CachedEntityModelRenderer> EntityModelRenderers;
            typedef Text::TextRenderer<EntityKey> EntityClassnameRenderer;
            
            class EntityClassnameFilter : public EntityClassnameRenderer::TextRendererFilter {
            public:
                inline bool stringVisible(RenderContext& context, const EntityKey& entity) const;
            };
            
            Vbo& m_boundsVbo;
            Model::MapDocument& m_document;
            
            Model::EntitySet m_entities;
            VertexArray* m_boundsVertexArray;
            bool m_boundsValid;
            EntityModelRenderers m_modelRenderers;
            bool m_modelRendererCacheValid;
            EntityClassnameRenderer* m_classnameRenderer;
            
            Color m_classnameColor;
            Color m_classnameBackgroundColor;
            bool m_renderOccludedClassnames;
            Color m_occludedClassnameColor;
            Color m_occludedClassnameBackgroundColor;
            bool m_overrideBoundsColor;
            Color m_boundsColor;
            bool m_renderOccludedBounds;
            Color m_occludedBoundsColor;
            bool m_applyTinting;
            Color m_tintColor;
            bool m_grayscale;
            
            void writeColoredBounds(RenderContext& context, const Model::EntityList& entities);
            void writeBounds(RenderContext& context, const Model::EntityList& entities);
            void validateBounds(RenderContext& context);
            void validateModels(RenderContext& context);
            
            void renderBounds(RenderContext& context);
            void renderClassnames(RenderContext& context);
            void renderModels(RenderContext& context);
            void renderFigures(RenderContext& context);

            // prevent copying
            EntityRenderer(const EntityRenderer& other);
            void operator= (const EntityRenderer& other);
        public:
            EntityRenderer(Vbo& boundsVbo, Model::MapDocument& document);
            ~EntityRenderer();
            
            void setClassnameFadeDistance(float classnameFadeDistance);
            
            inline void setClassnameColor(const Color& classnameColor, const Color& classnameBackgroundColor) {
                m_classnameColor = classnameColor;
                m_classnameBackgroundColor = classnameBackgroundColor;
            }
            
            inline void setOccludedClassnameColor(const Color& occludedClassnameColor, const Color& occludedClassnameBackgroundColor) {
                m_occludedClassnameColor = occludedClassnameColor;
                m_occludedClassnameBackgroundColor = occludedClassnameBackgroundColor;
                m_renderOccludedClassnames = true;
            }
            
            inline void disableRenderOccludedClassnames() {
                m_renderOccludedClassnames = false;
            }
            
            inline void setBoundsColor(const Color& boundsColor) {
                m_boundsColor = boundsColor;
                m_overrideBoundsColor = true;
            }
            
            inline void disableOverrideBoundsColor() {
                m_overrideBoundsColor = false;
            }
            
            inline void setOccludedBoundsColor(const Color& occludedBoundsColor) {
                m_occludedBoundsColor = occludedBoundsColor;
                m_renderOccludedBounds = true;
            }
            
            inline void disableRenderOccludedBounds() {
                m_renderOccludedBounds = false;
            }
            
            inline void setTintColor(const Color& tintColor) {
                m_applyTinting = true;
                m_tintColor = tintColor;
            }
            
            inline void disableTinting() {
                m_applyTinting = false;
            }
            
            inline void setGrayscale(bool grayscale) {
                m_grayscale = grayscale;
            }

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
