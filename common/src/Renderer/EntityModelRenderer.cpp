/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "EntityModelRenderer.h"

#include "Logger.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Assets/AssetUtils.h"
#include "Assets/EntityModel.h"
#include "Assets/EntityModelManager.h"
#include "Assets/ModelDefinition.h"
#include "EL/ELExceptions.h"
#include "Model/EditorContext.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Renderer/ActiveShader.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Shaders.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/TexturedIndexRangeRenderer.h"
#include "Renderer/Transformation.h"

#include <vecmath/mat.h>

namespace TrenchBroom {
    namespace Renderer {
        EntityModelRenderer::EntityModelRenderer(Logger& logger, Assets::EntityModelManager& entityModelManager, const Model::EditorContext& editorContext) :
        m_logger(logger),
        m_entityModelManager(entityModelManager),
        m_editorContext(editorContext),
        m_applyTinting(false),
        m_showHiddenEntities(false) {}

        EntityModelRenderer::~EntityModelRenderer() {
            clear();
        }

        void EntityModelRenderer::addEntity(Model::EntityNode* entityNode) {
            const auto modelSpec = Assets::safeGetModelSpecification(m_logger, entityNode->entity().classname(), [&]() {
                return entityNode->entity().modelSpecification();
            });

            auto* renderer = m_entityModelManager.renderer(modelSpec);
            if (renderer != nullptr) {
                m_entities.insert(std::make_pair(entityNode, renderer));
            }
        }

        void EntityModelRenderer::updateEntity(Model::EntityNode* entityNode) {
            const auto modelSpec = Assets::safeGetModelSpecification(m_logger, entityNode->entity().classname(), [&]() {
                return entityNode->entity().modelSpecification();
            });

            auto* renderer = m_entityModelManager.renderer(modelSpec);
            EntityMap::iterator it = m_entities.find(entityNode);

            if (renderer == nullptr && it == std::end(m_entities)) {
                return;
            }

            if (it == std::end(m_entities)) {
                m_entities.insert(std::make_pair(entityNode, renderer));
            } else {
                if (renderer == nullptr) {
                    m_entities.erase(it);
                } else if (it->second != renderer) {
                    it->second = renderer;
                }
            }
        }

        void EntityModelRenderer::clear() {
            m_entities.clear();
        }

        bool EntityModelRenderer::applyTinting() const {
            return m_applyTinting;
        }

        void EntityModelRenderer::setApplyTinting(const bool applyTinting) {
            m_applyTinting = applyTinting;
        }

        const Color& EntityModelRenderer::tintColor() const {
            return m_tintColor;
        }

        void EntityModelRenderer::setTintColor(const Color& tintColor) {
            m_tintColor = tintColor;
        }

        bool EntityModelRenderer::showHiddenEntities() const {
            return m_showHiddenEntities;
        }

        void EntityModelRenderer::setShowHiddenEntities(const bool showHiddenEntities) {
            m_showHiddenEntities = showHiddenEntities;
        }

        void EntityModelRenderer::render(RenderBatch& renderBatch) {
            renderBatch.add(this);
        }

        void EntityModelRenderer::doPrepareVertices(VboManager& vboManager) {
            m_entityModelManager.prepare(vboManager);
        }

        void EntityModelRenderer::doRender(RenderContext& renderContext) {
            auto& prefs = PreferenceManager::instance();

            ActiveShader shader(renderContext.shaderManager(), Shaders::EntityModelShader);
            shader.set("Brightness", prefs.get(Preferences::Brightness));
            shader.set("ApplyTinting", m_applyTinting);
            shader.set("TintColor", m_tintColor);
            shader.set("GrayScale", false);
            shader.set("Texture", 0);
            shader.set("ShowSoftMapBounds", !renderContext.softMapBounds().is_empty());
            shader.set("SoftMapBoundsMin", renderContext.softMapBounds().min);
            shader.set("SoftMapBoundsMax", renderContext.softMapBounds().max);
            shader.set("SoftMapBoundsColor", vm::vec4f(prefs.get(Preferences::SoftMapBoundsColor).r(),
                                                       prefs.get(Preferences::SoftMapBoundsColor).g(),
                                                       prefs.get(Preferences::SoftMapBoundsColor).b(),
                                                       0.1f));

            glAssert(glEnable(GL_TEXTURE_2D));
            glAssert(glActiveTexture(GL_TEXTURE0));

            for (const auto& entry : m_entities) {
                auto* entityNode = entry.first;
                if (!m_showHiddenEntities && !m_editorContext.visible(entityNode)) {
                    continue;
                }

                auto* renderer = entry.second;

                const auto transformation = entityNode->entity().modelTransformation();
                MultiplyModelMatrix multMatrix(renderContext.transformation(), vm::mat4x4f(transformation));

                shader.set("ModelMatrix", vm::mat4x4f(transformation));

                renderer->render();
            }
        }
    }
}
