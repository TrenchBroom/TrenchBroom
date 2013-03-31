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

#include "EntityRenderer.h"

#include "Model/MapDocument.h"
#include "Renderer/EntityModelRenderer.h"
#include "Renderer/EntityModelRendererManager.h"
#include "Renderer/SharedResources.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/Shader/ShaderProgram.h"
#include "Renderer/Text/FontManager.h"
#include "Utility/Preferences.h"

#include <cassert>

namespace TrenchBroom {
    namespace Renderer {
        EntityRenderer::EntityClassnameAnchor::EntityClassnameAnchor(Model::Entity& entity) :
        m_entity(&entity) {}
        
        const Vec3f EntityRenderer::EntityClassnameAnchor::position() const {
            Vec3f position = m_entity->center();
            position.z = m_entity->bounds().max.z + 1.0f;
            return position;
        }
        
        const Text::Alignment::Type EntityRenderer::EntityClassnameAnchor::alignment() const {
            return Text::Alignment::Bottom;
        }

        bool EntityRenderer::EntityClassnameFilter::stringVisible(RenderContext& context, const EntityKey& entity) const {
            return context.filter().entityVisible(*entity);
        }

        void EntityRenderer::writeColoredBounds(RenderContext& context, const Model::EntityList& entities) {
            if (entities.empty())
                return;

            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            Vec3f::List vertices(24);

            for (unsigned int i = 0; i < entities.size(); i++) {
                Model::Entity* entity = entities[i];
                const BBox& bounds = entity->bounds();
                const Model::EntityDefinition* definition = entity->definition();
                Color entityColor;
                if (definition != NULL) {
                    entityColor = definition->color();
                    entityColor.w = prefs.getColor(Preferences::EntityBoundsColor).w;
                } else {
                    entityColor = prefs.getColor(Preferences::EntityBoundsColor);
                }

                bounds.vertices(vertices);
                for (unsigned int j = 0; j < vertices.size(); j++) {
                    m_boundsVertexArray->addAttribute(vertices[j]);
                    m_boundsVertexArray->addAttribute(entityColor);
                }
            }
        }

        void EntityRenderer::writeBounds(RenderContext& context, const Model::EntityList& entities) {
            if (entities.empty())
                return;

            Vec3f::List vertices(24);
            for (unsigned int i = 0; i < entities.size(); i++) {
                Model::Entity* entity = entities[i];
                const BBox& bounds = entity->bounds();
                bounds.vertices(vertices);

                for (unsigned int j = 0; j < vertices.size(); j++)
                    m_boundsVertexArray->addAttribute(vertices[j]);
            }
        }

        void EntityRenderer::validateBounds(RenderContext& context) {
            delete m_boundsVertexArray;
            m_boundsVertexArray = NULL;

            Model::EntityList entities;
            Model::EntitySet::iterator entityIt, entityEnd;
            for (entityIt = m_entities.begin(), entityEnd = m_entities.end(); entityIt != entityEnd; ++entityIt) {
                Model::Entity* entity = *entityIt;
                if (context.filter().entityVisible(*entity))
                    entities.push_back(entity);
            }

            if (entities.empty())
                return;

            SetVboState mapVbo(m_boundsVbo, Vbo::VboMapped);

            if (m_overrideBoundsColor) {
                unsigned int vertexCount = 2 * 4 * 6 * static_cast<unsigned int>(entities.size());
                m_boundsVertexArray = new VertexArray(m_boundsVbo, GL_LINES, vertexCount,
                                                      Attribute::position3f());
                writeBounds(context, entities);
            } else {
                unsigned int vertexCount = 2 * 4 * 6 * static_cast<unsigned int>(entities.size());
                m_boundsVertexArray = new VertexArray(m_boundsVbo, GL_LINES, vertexCount,
                                                      Attribute::position3f(),
                                                      Attribute::color4f());
                writeColoredBounds(context, entities);
            }

            m_boundsValid = true;
        }

        void EntityRenderer::validateModels(RenderContext& context) {
            m_modelRenderers.clear();

            EntityModelRendererManager& modelRendererManager = m_document.sharedResources().modelRendererManager();
            Model::EntitySet::iterator entityIt, entityEnd;
            for (entityIt = m_entities.begin(), entityEnd = m_entities.end(); entityIt != entityEnd; ++entityIt) {
                Model::Entity* entity = *entityIt;
                const String* classname = entity->classname();
                if (classname != NULL) {
                    EntityModelRenderer* renderer = modelRendererManager.modelRenderer(*entity, m_document.searchPaths());
                    if (renderer != NULL)
                        m_modelRenderers[entity] = CachedEntityModelRenderer(renderer, *classname);
                }
            }

            m_modelRendererCacheValid = true;
        }

        void EntityRenderer::renderBounds(RenderContext& context) {
            if (m_boundsVertexArray == NULL)
                return;

            ShaderManager& shaderManager = m_document.sharedResources().shaderManager();

            m_boundsVbo.activate();
            if (m_overrideBoundsColor) {
                ShaderProgram& edgeProgram = shaderManager.shaderProgram(Shaders::EdgeShader);
                if (edgeProgram.activate()) {
                    if (m_renderOccludedBounds) {
                        glDisable(GL_DEPTH_TEST);
                        edgeProgram.setUniformVariable("Color", m_occludedBoundsColor);
                        m_boundsVertexArray->render();
                        glEnable(GL_DEPTH_TEST);
                    }
                    edgeProgram.setUniformVariable("Color", m_boundsColor);
                    m_boundsVertexArray->render();
                    edgeProgram.deactivate();
                }
            } else {
                ShaderProgram& coloredEdgeProgram = shaderManager.shaderProgram(Shaders::ColoredEdgeShader);
                if (coloredEdgeProgram.activate()) {
                    m_boundsVertexArray->render();
                    coloredEdgeProgram.deactivate();
                }
            }
            m_boundsVbo.deactivate();
        }

        void EntityRenderer::renderClassnames(RenderContext& context) {
            if (m_classnameRenderer->empty())
                return;

            ShaderManager& shaderManager = m_document.sharedResources().shaderManager();
            ShaderProgram& textProgram = shaderManager.shaderProgram(Shaders::TextShader);
            ShaderProgram& textBackgroundProgram = shaderManager.shaderProgram(Shaders::TextBackgroundShader);

            EntityClassnameFilter classnameFilter;
            /*
            if (m_renderOccludedClassnames) {
                glDisable(GL_DEPTH_TEST);
                m_classnameRenderer->render(context, classnameFilter, textProgram,
                                            m_occludedClassnameColor, textBackgroundProgram,
                                            m_occludedClassnameBackgroundColor);
                glEnable(GL_DEPTH_TEST);
            }
             */

            m_classnameRenderer->render(context, classnameFilter, textProgram,
                                        m_classnameColor, textBackgroundProgram,
                                        m_classnameBackgroundColor);

        }

        void EntityRenderer::renderModels(RenderContext& context) {
            if (m_modelRenderers.empty())
                return;

            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            EntityModelRendererManager& modelRendererManager = m_document.sharedResources().modelRendererManager();

            ShaderManager& shaderManager = m_document.sharedResources().shaderManager();
            ShaderProgram& entityModelProgram = shaderManager.shaderProgram(Shaders::EntityModelShader);

            if (entityModelProgram.activate()) {
                modelRendererManager.activate();
                entityModelProgram.setUniformVariable("Brightness", prefs.getFloat(Preferences::RendererBrightness));
                entityModelProgram.setUniformVariable("ApplyTinting", m_applyTinting);
                entityModelProgram.setUniformVariable("TintColor", m_tintColor);
                entityModelProgram.setUniformVariable("GrayScale", m_grayscale);

                EntityModelRenderers::iterator it, end;
                for (it = m_modelRenderers.begin(), end = m_modelRenderers.end(); it != end; ++it) {
                    Model::Entity* entity = it->first;
                    if (context.filter().entityVisible(*entity)) {
                        EntityModelRenderer* renderer = it->second.renderer;
                        renderer->render(entityModelProgram, context.transformation(), *entity);
                    }
                }

                modelRendererManager.deactivate();
                entityModelProgram.deactivate();
            }
        }

        EntityRenderer::EntityRenderer(Vbo& boundsVbo, Model::MapDocument& document) :
        m_boundsVbo(boundsVbo),
        m_document(document),
        m_boundsVertexArray(NULL),
        m_boundsValid(true),
        m_modelRendererCacheValid(true),
        m_classnameRenderer(NULL),
        m_classnameColor(1.0f, 1.0f, 1.0f, 1.0f),
        m_classnameBackgroundColor(0.0f, 0.0f, 0.0f, 0.6f),
        m_renderOccludedClassnames(false),
        m_overrideBoundsColor(false),
        m_renderOccludedBounds(false),
        m_applyTinting(false),
        m_grayscale(false) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            
            const String& fontName = prefs.getString(Preferences::RendererFontName);
            int fontSize = prefs.getInt(Preferences::RendererFontSize);
            Text::FontDescriptor fontDescriptor(fontName, static_cast<unsigned int>(fontSize));

            Text::FontManager& fontManager = m_document.sharedResources().fontManager();
            Text::TexturedFont* font = fontManager.font(fontDescriptor);
            assert(font != NULL);
            
            m_classnameRenderer = new EntityClassnameRenderer(*font);
        }

        EntityRenderer::~EntityRenderer() {
            delete m_boundsVertexArray;
            m_boundsVertexArray = NULL;
            delete m_classnameRenderer;
            m_classnameRenderer = NULL;
        }

        void EntityRenderer::setClassnameFadeDistance(float classnameFadeDistance) {
            m_classnameRenderer->setFadeDistance(classnameFadeDistance);
        }

        void EntityRenderer::addEntity(Model::Entity& entity) {
            if (!m_entities.insert(&entity).second)
                return;

            EntityModelRendererManager& modelRendererManager = m_document.sharedResources().modelRendererManager();
            EntityClassnameAnchor anchor(entity);

            const String* classname = entity.classname();
            if (classname != NULL) {
                EntityModelRenderer* renderer = modelRendererManager.modelRenderer(entity, m_document.searchPaths());
                if (renderer != NULL)
                    m_modelRenderers[&entity] = CachedEntityModelRenderer(renderer, *classname);
                m_classnameRenderer->addString(&entity, *classname, anchor);
            } else {
                m_classnameRenderer->addString(&entity, Model::Entity::NoClassnameValue, anchor);
            }


            m_entities.insert(&entity);
            m_boundsValid = false;
        }

        void EntityRenderer::addEntities(const Model::EntityList& entities) {
            if (entities.empty())
                return;

            EntityModelRendererManager& modelRendererManager = m_document.sharedResources().modelRendererManager();

            for (unsigned int i = 0; i < entities.size(); i++) {
                Model::Entity* entity = entities[i];
                EntityClassnameAnchor anchor(*entity);
                const String* classname = entity->classname();
                if (classname != NULL) {
                    EntityModelRenderer* renderer = modelRendererManager.modelRenderer(*entity, m_document.searchPaths());
                    if (renderer != NULL)
                        m_modelRenderers[entity] = CachedEntityModelRenderer(renderer, *classname);
                    m_classnameRenderer->addString(entity, *classname, anchor);
                } else {
                    m_classnameRenderer->addString(entity, Model::Entity::NoClassnameValue, anchor);
                }
            }

            m_entities.insert(entities.begin(), entities.end());
            m_boundsValid = false;
        }

        void EntityRenderer::invalidateBounds() {
            m_boundsValid = false;
        }

        void EntityRenderer::invalidateModels() {
            m_modelRendererCacheValid = false;
        }

        void EntityRenderer::clear() {
            m_entities.clear();
            m_boundsValid = false;
            m_modelRenderers.clear();
            m_modelRendererCacheValid = true;
            m_classnameRenderer->clear();
        }

        void EntityRenderer::removeEntity(Model::Entity& entity) {
            m_modelRenderers.erase(&entity);
            m_classnameRenderer->removeString(&entity);
            m_entities.erase(&entity);
            m_boundsValid = false;
        }

        void EntityRenderer::removeEntities(const Model::EntityList& entities) {
            if (entities.empty())
                return;

            for (unsigned int i = 0; i < entities.size(); i++) {
                Model::Entity* entity = entities[i];
                m_modelRenderers.erase(entity);
                m_classnameRenderer->removeString(entity);
                m_entities.erase(entity);
            }
            m_boundsValid = false;
        }

        void EntityRenderer::render(RenderContext& context) {
            if (!m_boundsValid)
                validateBounds(context);
            if (!m_modelRendererCacheValid)
                validateModels(context);

            if (context.viewOptions().showEntityModels())
                renderModels(context);
            if (context.viewOptions().showEntityBounds())
                renderBounds(context);
            if (context.viewOptions().showEntityClassnames())
                renderClassnames(context);
        }
    }
}
