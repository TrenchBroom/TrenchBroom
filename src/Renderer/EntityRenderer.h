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

#include "Color.h"
#include "Model/ModelTypes.h"
#include "Renderer/EdgeRenderer.h"
#include "Renderer/EntityModelRenderer.h"
#include "Renderer/TextRenderer.h"

#include <map>

namespace TrenchBroom {
    namespace Model {
        class Filter;
    }
    
    namespace Renderer {
        class FontManager;
        class RenderContext;
        class Vbo;
        
        class EntityRenderer {
        private:
            typedef Model::Entity* Key;
            typedef TextRenderer<Key> ClassnameRenderer;
            
            class EntityClassnameAnchor : public TextAnchor {
            private:
                const Model::Entity* m_entity;
            protected:
                inline const Vec3f basePosition() const;
                inline const Alignment::Type alignment() const;
            public:
                EntityClassnameAnchor(const Model::Entity* entity);
            };
            
            class EntityClassnameFilter : public ClassnameRenderer::TextRendererFilter {
            private:
                const Model::Filter& m_filter;
            public:
                EntityClassnameFilter(const Model::Filter& filter);
                bool stringVisible(RenderContext& context, const Key& entity) const;
            };

            class EntityClassnameColorProvider : public ClassnameRenderer::TextColorProvider {
            private:
            public:
                Color textColor(RenderContext& context, const Key& entity) const;
                Color backgroundColor(RenderContext& context, const Key& entity) const;
            };
            
            const Model::Filter& m_filter;
            Model::EntitySet m_entities;
            EdgeRenderer m_boundsRenderer;
            EdgeRenderer m_selectedBoundsRenderer;
            ClassnameRenderer m_classnameRenderer;
            EntityModelRenderer m_modelRenderer;
            bool m_boundsValid;
        public:
            EntityRenderer(FontManager& m_fontManager, const Model::Filter& filter);
            ~EntityRenderer();

            void addEntity(Model::Entity* entity);
            void addEntities(const Model::EntityList& entities);
            void updateEntity(Model::Entity* entity);
            void updateEntities(const Model::EntityList& entities);
            void removeEntity(Model::Entity* entity);
            void removeEntities(const Model::EntityList& entities);
            void clear();
            void invalidateBounds();
            
            void render(RenderContext& context);
        private:
            void renderBounds(RenderContext& context);
            void renderClassnames(RenderContext& context);
            void renderModels(RenderContext& context);
            static TextureFont& font(FontManager& fontManager);
            void validateBounds();
            const Color& boundsColor(const Model::Entity& entity) const;
        };
    }
}

#endif /* defined(__TrenchBroom__EntityRenderer__) */
