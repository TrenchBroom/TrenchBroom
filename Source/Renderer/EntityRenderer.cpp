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
#include "Renderer/EntityClassnameAnchor.h"
#include "Renderer/EntityClassnameFilter.h"
#include "Renderer/EntityModelRenderer.h"
#include "Renderer/EntityModelRendererManager.h"
#include "Renderer/SharedResources.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/Shader/ShaderProgram.h"
#include "Utility/Preferences.h"

#include <cassert>

namespace TrenchBroom {
    namespace Renderer {
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
                for (unsigned int i = 0; i < vertices.size(); i++) {
                    m_boundsVertexArray->addAttribute(vertices[i]);
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
                
                for (unsigned int i = 0; i < vertices.size(); i++)
                    m_boundsVertexArray->addAttribute(vertices[i]);
            }
        }

        void EntityRenderer::validateBounds(RenderContext& context) {
            m_boundsVertexArray = VertexArrayPtr(NULL);
            
            Model::EntityList entities;
            Model::EntitySet::iterator entityIt, entityEnd;
            for (entityIt = m_entities.begin(), entityEnd = m_entities.end(); entityIt != entityEnd; ++entityIt) {
                Model::Entity* entity = *entityIt;
                if (context.filter().entityVisible(*entity))
                    entities.push_back(entity);
            }
            
            if (entities.empty())
                return;
            
            m_boundsVbo.activate();
            m_boundsVbo.map();
            
            if (m_applyColor) {
                unsigned int vertexCount = 2 * 4 * 6 * static_cast<unsigned int>(entities.size());
                m_boundsVertexArray = VertexArrayPtr(new VertexArray(m_boundsVbo, GL_LINES, vertexCount, VertexAttribute(3, GL_FLOAT, VertexAttribute::Position)));
                writeBounds(context, entities);
            } else {
                unsigned int vertexCount = 2 * 4 * 6 * static_cast<unsigned int>(entities.size());
                m_boundsVertexArray = VertexArrayPtr(new VertexArray(m_boundsVbo, GL_LINES, vertexCount, VertexAttribute(3, GL_FLOAT, VertexAttribute::Position), VertexAttribute(4, GL_FLOAT, VertexAttribute::Color)));
                writeColoredBounds(context, entities);
            }
            
            m_boundsVbo.unmap();
            m_boundsVbo.deactivate();
            m_boundsValid = true;
        }
        
        void EntityRenderer::validateModels(RenderContext& context) {
            m_modelRenderers.clear();
            
            EntityModelRendererManager& modelRendererManager = m_document.sharedResources().modelRendererManager();
            Model::EntitySet::iterator entityIt, entityEnd;
            for (entityIt = m_entities.begin(), entityEnd = m_entities.end(); entityIt != entityEnd; ++entityIt) {
                Model::Entity* entity = *entityIt;
                EntityModelRenderer* renderer = modelRendererManager.modelRenderer(*entity, m_document.mods());
                if (renderer != NULL)
                    m_modelRenderers[entity] = CachedEntityModelRenderer(renderer, *entity->classname());
            }
            
            m_modelRendererCacheValid = true;
        }

        void EntityRenderer::renderBounds(RenderContext& context) {
            if (m_boundsVertexArray.get() == NULL)
                return;
            
            ShaderManager& shaderManager = m_document.sharedResources().shaderManager();

            m_boundsVbo.activate();
            if (m_applyColor) {
                ShaderProgram& edgeProgram = shaderManager.shaderProgram(Shaders::EdgeShader);
                if (edgeProgram.activate()) {
                    if (m_renderOcclusion) {
                        glDisable(GL_DEPTH_TEST);
                        edgeProgram.setUniformVariable("Color", m_occlusionColor);
                        m_boundsVertexArray->render();
                        glEnable(GL_DEPTH_TEST);
                    }
                    edgeProgram.setUniformVariable("Color", m_color);
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
            if (m_applyColor) {
                if (m_renderOcclusion) {
                    glDisable(GL_DEPTH_TEST);
                    m_classnameRenderer->render(context, classnameFilter, textProgram,
                                                m_classnameColor, textBackgroundProgram,
                                                m_occlusionColor);
                    glEnable(GL_DEPTH_TEST);
                }
                m_classnameRenderer->render(context, classnameFilter, textProgram,
                                            m_classnameColor, textBackgroundProgram,
                                            m_color);
            } else {
                m_classnameRenderer->render(context, classnameFilter, textProgram,
                                            m_classnameColor, textBackgroundProgram,
                                            m_classnameBackgroundColor);
            }
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
                entityModelProgram.setUniformVariable("ApplyTinting", m_applyColor);
                entityModelProgram.setUniformVariable("TintColor", m_color);
                entityModelProgram.setUniformVariable("GrayScale", false);
                
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

        EntityRenderer::EntityRenderer(Vbo& boundsVbo, Model::MapDocument& document, float classnameFadeDistance) :
        m_boundsVbo(boundsVbo),
        m_document(document),
        m_modelRendererCacheValid(true),
        m_boundsValid(true),
        m_classnameColor(1.0f, 1.0f, 1.0f, 1.0f),
        m_classnameBackgroundColor(0.0f, 0.0f, 0.0f, 0.6f),
        m_applyColor(false),
        m_renderOcclusion(false) {
            Text::StringManager& stringManager = m_document.sharedResources().stringManager();
            m_classnameRenderer = EntityClassnameRendererPtr(new EntityClassnameRenderer(stringManager, classnameFadeDistance));
        }

        EntityRenderer::EntityRenderer(Vbo& boundsVbo, Model::MapDocument& document, float classnameFadeDistance, const Color& color) :
        m_boundsVbo(boundsVbo),
        m_document(document),
        m_modelRendererCacheValid(true),
        m_boundsValid(true),
        m_classnameColor(1.0f, 1.0f, 1.0f, 1.0f),
        m_classnameBackgroundColor(0.0f, 0.0f, 0.0f, 0.6f),
        m_applyColor(true),
        m_color(color),
        m_renderOcclusion(false) {
            Text::StringManager& stringManager = m_document.sharedResources().stringManager();
            m_classnameRenderer = EntityClassnameRendererPtr(new EntityClassnameRenderer(stringManager, classnameFadeDistance));
        }

        EntityRenderer::EntityRenderer(Vbo& boundsVbo, Model::MapDocument& document, float classnameFadeDistance, const Color& color, const Color& occlusionColor) :
        m_boundsVbo(boundsVbo),
        m_document(document),
        m_modelRendererCacheValid(true),
        m_boundsValid(true),
        m_classnameColor(1.0f, 1.0f, 1.0f, 1.0f),
        m_classnameBackgroundColor(0.0f, 0.0f, 0.0f, 0.6f),
        m_applyColor(true),
        m_color(color),
        m_renderOcclusion(true),
        m_occlusionColor(occlusionColor) {
            Text::StringManager& stringManager = m_document.sharedResources().stringManager();
            m_classnameRenderer = EntityClassnameRendererPtr(new EntityClassnameRenderer(stringManager, classnameFadeDistance));
        }
        
        void EntityRenderer::addEntity(Model::Entity& entity) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            EntityModelRendererManager& modelRendererManager = m_document.sharedResources().modelRendererManager();
            
            const String& fontName = prefs.getString(Preferences::RendererFontName);
            int fontSize = prefs.getInt(Preferences::RendererFontSize);
            Text::FontDescriptor fontDescriptor(fontName, fontSize);
            
            const Model::PropertyValue& classname = *entity.classname();
            EntityModelRenderer* renderer = modelRendererManager.modelRenderer(entity, m_document.mods());
            if (renderer != NULL)
                m_modelRenderers[&entity] = CachedEntityModelRenderer(renderer, classname);
            
            EntityClassnameAnchor* anchor = new EntityClassnameAnchor(entity);
            m_classnameRenderer->addString(&entity, fontDescriptor, classname, anchor);
            
            m_entities.insert(&entity);
            m_boundsValid = false;
        }
        
        void EntityRenderer::addEntities(const Model::EntityList& entities) {
            if (entities.empty())
                return;
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            EntityModelRendererManager& modelRendererManager = m_document.sharedResources().modelRendererManager();
            
            const String& fontName = prefs.getString(Preferences::RendererFontName);
            int fontSize = prefs.getInt(Preferences::RendererFontSize);
            Text::FontDescriptor fontDescriptor(fontName, fontSize);
            
            for (unsigned int i = 0; i < entities.size(); i++) {
                Model::Entity* entity = entities[i];
                const Model::PropertyValue& classname = *entity->classname();
                EntityModelRenderer* renderer = modelRendererManager.modelRenderer(*entity, m_document.mods());
                if (renderer != NULL)
                    m_modelRenderers[entity] = CachedEntityModelRenderer(renderer, classname);
                
                EntityClassnameAnchor* anchor = new EntityClassnameAnchor(*entity);
                m_classnameRenderer->addString(entity, fontDescriptor, classname, anchor);
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