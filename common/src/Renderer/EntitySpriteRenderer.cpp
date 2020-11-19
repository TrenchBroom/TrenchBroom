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

#include "EntitySpriteRenderer.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Assets/EntitySpriteManager.h"
#include "Model/EditorContext.h"
#include "Model/EntityNode.h"
#include "Renderer/ActiveShader.h"
#include "Renderer/Camera.h"
#include "Renderer/PrimType.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Shaders.h"

namespace TrenchBroom::Renderer {
    const float EntitySpriteRenderer::DefaultMaxViewDistance = 1536.0f; //TODO: make these configurable in Preferences?
    const float EntitySpriteRenderer::DefaultMinZoomFactor = 0.5f;

    EntitySpriteRenderer::EntityInfo::EntityInfo(const Model::EntityNode* i_entity, const Assets::Texture* i_sprite, float i_size, Color& i_tintColor, bool i_applyTint) :
        entity(i_entity),
        sprite(i_sprite),
        size(i_size),
        tintColor(i_tintColor),
        applyTint(i_applyTint) {}

    EntitySpriteRenderer::EntitySpriteRenderer(Logger& /*logger*/, const Model::EditorContext& editorContext) :
        m_entitiesListChanged(false),
        m_maxViewDistance(DefaultMaxViewDistance),
        m_minZoomFactor(DefaultMinZoomFactor),
        m_editorContext(editorContext),
        m_applyTinting(false),
        m_showHiddenEntities(false) {
        // Create sprite quad
        const float size = 0.5f;

        m_vertexArray = VertexArray::move(std::vector<SpriteVertex>({
            SpriteVertex { vm::vec3f(-size,  size, 0.0f), vm::vec2f(0.0f, 0.0f) }, // Top left
            SpriteVertex { vm::vec3f( size,  size, 0.0f), vm::vec2f(1.0f, 0.0f) }, // Top right
            SpriteVertex { vm::vec3f( size, -size, 0.0f), vm::vec2f(1.0f, 1.0f) }, // Bottom right
            SpriteVertex { vm::vec3f(-size, -size, 0.0f), vm::vec2f(0.0f, 1.0f) }, // Bottom left
        }));
    }

    EntitySpriteRenderer::~EntitySpriteRenderer() {
        clear();
    }

    void EntitySpriteRenderer::addEntity(Model::EntityNode* entity) {
        if (entity->hasPointEntitySprite()) {
            m_entities.insert(std::make_pair(entity, createEntityInfo(entity)));
            m_entitiesListChanged = true;
        }
    }

    void EntitySpriteRenderer::updateEntity(Model::EntityNode* entity) {
        const auto it = m_entities.find(entity);

        if (it == std::end(m_entities)) { // No such entity in collection?
            if (entity->hasPointEntitySprite()) { // Add it
                m_entities.insert(std::make_pair(entity, createEntityInfo(entity)));
                m_entitiesListChanged = true;
            }
        } else { // Entity found
            if (!entity->hasPointEntitySprite()) { // Sprite is unused. Remove from collection...
                m_entities.erase(it);
                m_entitiesListChanged = true;
            } else { // Update EntityInfo?
                const auto info = it->second;
                const auto newInfo = createEntityInfo(entity);

                if (newInfo.size != info.size || newInfo.applyTint != info.applyTint || newInfo.tintColor != info.tintColor || newInfo.sprite != info.sprite) {
                    it->second = newInfo;
                    m_entitiesListChanged = true;
                }
            }
        }
    }

    EntitySpriteRenderer::EntityInfo EntitySpriteRenderer::createEntityInfo(const Model::EntityNode* entity) const {
        const auto size = static_cast<float>(std::min(entity->definitionBounds().size().x(), entity->definitionBounds().size().y())) * 0.9f; // Scale down a bit so the sprite looks better
        const auto colorAttributeList = entity->attributeWithName("_color");
        Color tintColor;
        bool applyTint = false;

        if (!colorAttributeList.empty()) {
            for (const auto& attribute : colorAttributeList) {
                if (Color::canParse(attribute.value())) {
                    tintColor = Color(Color::parse(attribute.value()), 0.5f);
                    applyTint = true;

                    // Convert from [0..255] to [0.0f..1.0f]?
                    if (tintColor.r() > 1.0f || tintColor.g() > 1.0f || tintColor.b() > 1.0f) {
                        for (size_t i = 0; i < 3; i++) {
                            tintColor.v[i] /= 255.0f;
                        }
                    }
                    
                    break;
                }
            }
        }

        const EntityInfo info(entity, entity->sprite(), size, tintColor, applyTint);
        return info;
    }

    void EntitySpriteRenderer::updateEntityByTextureList() {
        if (m_entitiesListChanged) {
            m_entitiesByTexture.clear();

            for (const auto& source : m_entities) {
                const auto target = m_entitiesByTexture.find(source.second.sprite);

                if (target == std::end(m_entitiesByTexture)) {
                    std::vector<const EntityInfo*> infos;
                    m_entitiesByTexture.insert(std::make_pair(source.second.sprite, infos));
                }

                m_entitiesByTexture[source.second.sprite].push_back(&source.second);
            }

            m_entitiesListChanged = false;
        }
    }

    bool EntitySpriteRenderer::isVisible(const Model::EntityNode* entity, RenderContext& renderContext) const {
        if (renderContext.render2D() && renderContext.camera().zoom() < m_minZoomFactor) {
            return false;
        }

        const Camera& camera = renderContext.camera();
        const float distance = camera.perpendicularDistanceTo(vm::vec3f(entity->origin()));

        if (renderContext.render3D() && distance > m_maxViewDistance) {
            return false;
        }

        return true;
    }

    void EntitySpriteRenderer::doRender(RenderContext& renderContext) {
        // Entity list needs updating?
        updateEntityByTextureList();

        auto& prefs = PreferenceManager::instance();
        const auto bc = prefs.get(Preferences::SoftMapBoundsColor);

        ActiveShader shader(renderContext.shaderManager(), Shaders::EntitySpriteShader);
        shader.set("Brightness", prefs.get(Preferences::Brightness));
        shader.set("ApplyTinting", m_applyTinting);
        shader.set("TintColor", m_tintColor);
        shader.set("Texture", 0);
        shader.set("ShowSoftMapBounds", !renderContext.softMapBounds().is_empty());
        shader.set("SoftMapBoundsMin", renderContext.softMapBounds().min);
        shader.set("SoftMapBoundsMax", renderContext.softMapBounds().max);
        shader.set("SoftMapBoundsColor", vm::vec4f(bc.r(), bc.g(), bc.b(), 0.1f));

        glAssert(glEnable(GL_TEXTURE_2D));
        glAssert(glActiveTexture(GL_TEXTURE0));

        for (const auto& entry : m_entitiesByTexture) {
            // Set texture...
            entry.first->activate();

            for (const auto& info: entry.second) {
                const auto* entity = info->entity;
                if (!m_showHiddenEntities && (!m_editorContext.visible(entity) || !isVisible(entity, renderContext))) {
                    continue;
                }

                const auto transformation = entity->modelTransformation();
                MultiplyModelMatrix multMatrix(renderContext.transformation(), vm::mat4x4f(transformation));

                shader.set("ModelMatrix", vm::mat4x4f(transformation));
                shader.set("Scale", info->size);

                // Set custom tint color?
                if (info->applyTint && !m_applyTinting) {
                    shader.set("ApplyTinting", true);
                    shader.set("TintColor", info->tintColor);
                }

                // Draw the sprite
                if (m_vertexArray.setup()) {
                    m_vertexArray.render(PrimType::Quads);
                    m_vertexArray.cleanup();
                }

                // Reset custom tint color?
                if (info->applyTint && !m_applyTinting) {
                    shader.set("ApplyTinting", false);
                }
            }
        }
    }

    void EntitySpriteRenderer::clear() {
        m_entities.clear();
        m_entitiesByTexture.clear();
        m_entitiesListChanged = false;
    }

    bool EntitySpriteRenderer::applyTinting() const {
        return m_applyTinting;
    }

    void EntitySpriteRenderer::setApplyTinting(const bool applyTinting) {
        m_applyTinting = applyTinting;
    }

    const Color& EntitySpriteRenderer::tintColor() const {
        return m_tintColor;
    }

    void EntitySpriteRenderer::setTintColor(const Color& tintColor) {
        m_tintColor = tintColor;
    }

    bool EntitySpriteRenderer::showHiddenEntities() const {
        return m_showHiddenEntities;
    }

    void EntitySpriteRenderer::setShowHiddenEntities(const bool showHiddenEntities) {
        m_showHiddenEntities = showHiddenEntities;
    }

    void EntitySpriteRenderer::render(RenderBatch& renderBatch) {
        renderBatch.add(this);
    }

    void EntitySpriteRenderer::doPrepareVertices(VboManager& vboManager) {
        m_vertexArray.prepare(vboManager);
    }
}
