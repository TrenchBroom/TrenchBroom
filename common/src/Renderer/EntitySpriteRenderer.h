/*
 Copyright (C) 2020 MaxED

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

#pragma once

#include "Color.h"
#include "Assets/Texture.h"
#include "Renderer/Renderable.h"
#include "Renderer/VertexArray.h"

namespace TrenchBroom {
    class Logger;

    namespace Assets {
        class EntitySpriteManager;
        class Texture;
    }

    namespace Model {
        class EditorContext;
        class EntityNode;
    }

    namespace Renderer {
        class RenderBatch;
        class SpriteRenderer;

        class EntitySpriteRenderer : public DirectRenderable {
        private:
            struct EntityInfo {
                const Model::EntityNode* entity;
                const Assets::Texture* sprite;
                float size;
                Color tintColor;
                bool applyTint;
                
                EntityInfo(const Model::EntityNode* i_entity, const Assets::Texture* i_sprite, float i_size, Color& i_tintColor, bool i_applyTint = false);
            };

            using EntityMap = std::map<Model::EntityNode*, EntityInfo>;
            using EntityByTextureMap = std::map<const Assets::Texture*, std::vector<const EntityInfo*>>;

            EntityMap m_entities;
            EntityByTextureMap m_entitiesByTexture;

            bool m_entitiesListChanged;

            static const float DefaultMaxViewDistance;
            static const float DefaultMinZoomFactor;

            float m_maxViewDistance;
            float m_minZoomFactor;

            Logger& m_logger;

            Assets::EntitySpriteManager& m_entitySpriteManager;
            const Model::EditorContext& m_editorContext;

            VertexArray m_vertexArray;

            using SpriteVertex = GLVertexTypes::P3T2::Vertex;

            bool m_applyTinting;
            Color m_tintColor;

            bool m_showHiddenEntities;
        public:
            explicit EntitySpriteRenderer(Logger& logger, Assets::EntitySpriteManager& entitySpriteManager, const Model::EditorContext& editorContext);
            ~EntitySpriteRenderer() override;

            template <typename I>
            void setEntities(I cur, I end) {
                clear();
                addEntities(cur, end);
            }

            template <typename I>
            void addEntities(I cur, I end) {
                while (cur != end) {
                    addEntity(*cur);
                    ++cur;
                }
            }

            template <typename I>
            void updateEntities(I cur, I end) {
                while (cur != end) {
                    updateEntity(*cur);
                    ++cur;
                }
            }

            void addEntity(Model::EntityNode* entity);
            void updateEntity(Model::EntityNode* entity);
            void clear();

            bool applyTinting() const;
            void setApplyTinting(const bool applyTinting);
            const Color& tintColor() const;
            void setTintColor(const Color& tintColor);

            bool showHiddenEntities() const;
            void setShowHiddenEntities(bool showHiddenEntities);

            void render(RenderBatch& renderBatch);
        private:
            EntityInfo createEntityInfo(const Model::EntityNode* entity) const;
            bool isVisible(const Model::EntityNode* entity, RenderContext& renderContext) const;
            void updateEntityByTextureList();
            void doPrepareVertices(VboManager& vboManager) override;
            void doRender(RenderContext& renderContext) override;
        };
    }
}