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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.C
 */

#include "MapRenderer.h"

#include "IO/FileManager.h"
#include "Model/Brush.h"
#include "Model/BrushGeometry.h"
#include "Model/EditStateManager.h"
#include "Model/Entity.h"
#include "Model/EntityDefinition.h"
#include "Model/Filter.h"
#include "Model/Map.h"
#include "Model/MapDocument.h"
#include "Renderer/EntityClassnameAnchor.h"
#include "Renderer/EntityClassnameFilter.h"
#include "Renderer/EntityRendererManager.h"
#include "Renderer/EntityRenderer.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Shader/Shader.h"
#include "Renderer/Vbo.h"
#include "Renderer/Text/FontDescriptor.h"
#include "Renderer/Text/StringManager.h"
#include "Utility/Grid.h"
#include "Utility/Preferences.h"

namespace TrenchBroom {
    namespace Renderer {
        static const int IndexSize = sizeof(GLuint);
        static const int VertexSize = 3 * sizeof(GLfloat);
        static const int ColorSize = 4;
        static const int TexCoordSize = 2 * sizeof(GLfloat);
        static const int FaceVertexSize = TexCoordSize + TexCoordSize + VertexSize;
        static const int EdgeVertexSize = ColorSize + VertexSize;
        static const int EntityBoundsVertexSize = ColorSize + VertexSize;

        void MapRenderer::writeFaceData(RenderContext& context, const FaceCollectionMap& faceCollectionMap, TextureVertexArrayList& vertexArrays, ShaderProgram& shaderProgram) {
            if (faceCollectionMap.empty())
                return;
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            const Color& faceColor = prefs.getColor(Preferences::FaceColor);

            FaceCollectionMap::const_iterator it, end;
            for (it = faceCollectionMap.begin(), end = faceCollectionMap.end(); it != end; ++it) {
                Model::Texture* texture = it->first;
                const FaceCollection& faceCollection = it->second;
                const Model::FaceList& faces = faceCollection.polygons();
                unsigned int vertexCount = static_cast<unsigned int>(3 * faceCollection.vertexCount() - 2 * faces.size());
                VertexArrayPtr vertexArray = VertexArrayPtr(new VertexArray(*m_faceVbo, GL_TRIANGLES, vertexCount,
                                                                            VertexAttribute(3, GL_FLOAT, VertexAttribute::Position),
                                                                            VertexAttribute(3, GL_FLOAT, VertexAttribute::Normal),
                                                                            VertexAttribute(2, GL_FLOAT, VertexAttribute::TexCoord0),
                                                                            VertexAttribute(4, GL_FLOAT, VertexAttribute::Color)));

                for (unsigned int i = 0; i < faces.size(); i++) {
                    Model::Face* face = faces[i];
                    const Model::VertexList& vertices = face->vertices();
                    const Vec2f::List& texCoords = face->texCoords();
                    Model::Texture* texture = face->texture();
                    const Color& color = texture != NULL ? texture->averageColor() : faceColor;
                    
                    for (unsigned int j = 1; j < vertices.size() - 1; j++) {
                        vertexArray->addAttribute(vertices[0]->position);
                        vertexArray->addAttribute(face->boundary().normal);
                        vertexArray->addAttribute(texCoords[0]);
                        vertexArray->addAttribute(color);
                        vertexArray->addAttribute(vertices[j]->position);
                        vertexArray->addAttribute(face->boundary().normal);
                        vertexArray->addAttribute(texCoords[j]);
                        vertexArray->addAttribute(color);
                        vertexArray->addAttribute(vertices[j + 1]->position);
                        vertexArray->addAttribute(face->boundary().normal);
                        vertexArray->addAttribute(texCoords[j + 1]);
                        vertexArray->addAttribute(color);
                    }
                }
                
                vertexArrays.push_back(TextureVertexArray(texture, vertexArray));
            }
        }
        
        void MapRenderer::writeColoredEdgeData(RenderContext& context, const Model::BrushList& brushes, const Model::FaceList& faces, VertexArray& vertexArray) {
            if (brushes.empty() && faces.empty())
                return;
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            const Color& worldColor = prefs.getColor(Preferences::EdgeColor);
            
            for (unsigned int i = 0; i < brushes.size(); i++) {
                Model::Brush* brush = brushes[i];
                Model::Entity* entity = brush->entity();
                Model::EntityDefinition* definition = entity->definition();
                const Color& color = (!entity->worldspawn() && definition != NULL && definition->type() == Model::EntityDefinition::BrushEntity) ? definition->color() : worldColor;
                
                const Model::EdgeList& edges = brush->edges();
                for (unsigned int j = 0; j < edges.size(); j++) {
                    Model::Edge* edge = edges[j];
                    vertexArray.addAttribute(edge->start->position);
                    vertexArray.addAttribute(color);
                    vertexArray.addAttribute(edge->end->position);
                    vertexArray.addAttribute(color);
                }
            }
            
            for (unsigned int i = 0; i < faces.size(); i++) {
                Model::Face* face = faces[i];
                Model::Brush* brush = face->brush();
                Model::Entity* entity = brush->entity();
                Model::EntityDefinition* definition = entity->definition();
                const Color& color = (!entity->worldspawn() && definition != NULL && definition->type() == Model::EntityDefinition::BrushEntity) ? definition->color() : worldColor;
                
                const Model::EdgeList& edges = face->edges();
                for (unsigned int j = 0; j < edges.size(); j++) {
                    Model::Edge* edge = edges[j];
                    vertexArray.addAttribute(edge->start->position);
                    vertexArray.addAttribute(color);
                    vertexArray.addAttribute(edge->end->position);
                    vertexArray.addAttribute(color);
                }
            }
        }
        
        void MapRenderer::writeEdgeData(RenderContext& context, const Model::BrushList& brushes, const Model::FaceList& faces, VertexArray& vertexArray) {
            if (brushes.empty() && faces.empty())
                return;
            
            for (unsigned int i = 0; i < brushes.size(); i++) {
                Model::Brush* brush = brushes[i];
                
                const Model::EdgeList& edges = brush->edges();
                for (unsigned int j = 0; j < edges.size(); j++) {
                    Model::Edge* edge = edges[j];
                    vertexArray.addAttribute(edge->start->position);
                    vertexArray.addAttribute(edge->end->position);
                }
            }
            
            for (unsigned int i = 0; i < faces.size(); i++) {
                Model::Face* face = faces[i];
                
                const Model::EdgeList& edges = face->edges();
                for (unsigned int j = 0; j < edges.size(); j++) {
                    Model::Edge* edge = edges[j];
                    vertexArray.addAttribute(edge->start->position);
                    vertexArray.addAttribute(edge->end->position);
                }
            }
        }
        
        void MapRenderer::rebuildGeometryData(RenderContext& context) {
            if (!m_geometryDataValid) {
                m_edgeVertexArray = VertexArrayPtr(NULL);
                m_faceVertexArrays.clear();
            }
            if (!m_selectedGeometryDataValid) {
                m_selectedEdgeVertexArray = VertexArrayPtr(NULL);
                m_selectedFaceVertexArrays.clear();
            }
            if (!m_lockedGeometryDataValid) {
                m_lockedEdgeVertexArray = VertexArrayPtr(NULL);
                m_lockedFaceVertexArrays.clear();
            }
            
            FaceSorter unselectedFaceSorter;
            FaceSorter selectedFaceSorter;
            FaceSorter lockedFaceSorter;
            
            Model::BrushList unselectedWorldBrushes;
            Model::BrushList unselectedEntityBrushes;
            Model::BrushList selectedBrushes;
            Model::BrushList lockedBrushes;
            Model::FaceList partiallySelectedBrushFaces;
            unsigned int totalUnselectedEdgeVertexCount = 0;
            unsigned int totalSelectedEdgeVertexCount = 0;
            unsigned int totalLockedEdgeVertexCount = 0;
            
            // collect all visible faces and brushes
            const Model::EntityList& entities = m_document.Map().entities();
            for (unsigned int i = 0; i < entities.size(); i++) {
                Model::Entity* entity = entities[i];
                const Model::BrushList& brushes = entity->brushes();
                for (unsigned int j = 0; j < brushes.size(); j++) {
                    Model::Brush* brush = brushes[j];
                    if (context.filter().brushVisible(*brush)) {
                        if (entity->selected() || brush->selected()) {
                            selectedBrushes.push_back(brush);
                            totalSelectedEdgeVertexCount += (2 * brush->edges().size());
                        } else if (entity->locked() || brush->locked()) {
                            lockedBrushes.push_back(brush);
                            totalLockedEdgeVertexCount+= (2 * brush->edges().size());
                        } else {
                            if (entity->worldspawn())
                                unselectedWorldBrushes.push_back(brush);
                            else
                                unselectedEntityBrushes.push_back(brush);
                            totalUnselectedEdgeVertexCount += (2 * brush->edges().size());
                            if (brush->partiallySelected()) {
                                const Model::FaceList& faces = brush->faces();
                                for (unsigned int k = 0; k < faces.size(); k++) {
                                    Model::Face* face = faces[k];
                                    if (face->selected()) {
                                        partiallySelectedBrushFaces.push_back(face);
                                        totalSelectedEdgeVertexCount += (2 * face->edges().size());
                                    }
                                }
                            }
                        }
                        
                        const Model::FaceList& faces = brush->faces();
                        for (unsigned int k = 0; k < faces.size(); k++) {
                            Model::Face* face = faces[k];
                            Model::Texture* texture = face->texture() != NULL ? face->texture() : m_dummyTexture.get();
                            if (entity->selected() || brush->selected() || face->selected())
                                selectedFaceSorter.addPolygon(texture, face, face->vertices().size());
                            else if (entity->locked() || brush->locked())
                                lockedFaceSorter.addPolygon(texture, face, face->vertices().size());
                            else
                                unselectedFaceSorter.addPolygon(texture, face, face->vertices().size());
                        }
                    }
                }
            }
            
            // merge the collected brushes
            Model::BrushList unselectedBrushes(unselectedWorldBrushes);
            unselectedBrushes.insert(unselectedBrushes.end(), unselectedEntityBrushes.begin(), unselectedEntityBrushes.end());
            
            // write face triangles
            m_faceVbo->activate();
            m_faceVbo->map();
            if (!m_geometryDataValid && !unselectedFaceSorter.empty())
                writeFaceData(context, unselectedFaceSorter.collections(), m_faceVertexArrays, *m_faceProgram.get());
            if (!m_selectedGeometryDataValid && !selectedFaceSorter.empty())
                writeFaceData(context, selectedFaceSorter.collections(), m_selectedFaceVertexArrays, *m_faceProgram.get());
            if (!m_lockedGeometryDataValid && !lockedFaceSorter.empty())
                writeFaceData(context, lockedFaceSorter.collections(), m_lockedFaceVertexArrays, *m_faceProgram.get());
            
            m_faceVbo->unmap();
            m_faceVbo->deactivate();
            
            // write edges
            m_edgeVbo->activate();
            m_edgeVbo->map();
            
            if (!m_geometryDataValid && !unselectedBrushes.empty()) {
                m_edgeVertexArray = VertexArrayPtr(new VertexArray(*m_edgeVbo, GL_LINES, totalUnselectedEdgeVertexCount, VertexAttribute(3, GL_FLOAT, VertexAttribute::Position), VertexAttribute(4, GL_FLOAT, VertexAttribute::Color)));
                m_edgeVertexArray->bindAttributes(*m_coloredEdgeProgram);
                writeColoredEdgeData(context, unselectedBrushes, Model::EmptyFaceList, *m_edgeVertexArray);
            }
            
            if (!m_selectedGeometryDataValid && (!selectedBrushes.empty() || !partiallySelectedBrushFaces.empty())) {
                m_selectedEdgeVertexArray = VertexArrayPtr(new VertexArray(*m_edgeVbo, GL_LINES, totalSelectedEdgeVertexCount, VertexAttribute(3, GL_FLOAT, VertexAttribute::Position)));
                m_selectedEdgeVertexArray->bindAttributes(*m_edgeProgram);
                writeEdgeData(context, selectedBrushes, partiallySelectedBrushFaces, *m_selectedEdgeVertexArray);
            }
            
            if (!m_lockedGeometryDataValid && !lockedBrushes.empty()) {
                m_lockedEdgeVertexArray = VertexArrayPtr(new VertexArray(*m_edgeVbo, GL_LINES, totalLockedEdgeVertexCount, VertexAttribute(3, GL_FLOAT, VertexAttribute::Position)));
                m_lockedEdgeVertexArray->bindAttributes(*m_edgeProgram);
                writeEdgeData(context, lockedBrushes, Model::EmptyFaceList, *m_lockedEdgeVertexArray);
            }
            
            m_edgeVbo->unmap();
            m_edgeVbo->deactivate();
            
            m_geometryDataValid = true;
            m_selectedGeometryDataValid = true;
            m_lockedGeometryDataValid = true;
        }
        
        void MapRenderer::writeColoredEntityBounds(RenderContext& context, const Model::EntityList& entities, VertexArray& vertexArray) {
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
                    vertexArray.addAttribute(vertices[i]);
                    vertexArray.addAttribute(entityColor);
                }
            }
        }
        
        void MapRenderer::writeEntityBounds(RenderContext& context, const Model::EntityList& entities, VertexArray& vertexArray) {
            if (entities.empty())
                return;
            
            Vec3f::List vertices(24);
            for (unsigned int i = 0; i < entities.size(); i++) {
                Model::Entity* entity = entities[i];
                const BBox& bounds = entity->bounds();
                bounds.vertices(vertices);

                for (unsigned int i = 0; i < vertices.size(); i++)
                    vertexArray.addAttribute(vertices[i]);
            }
        }

        void MapRenderer::rebuildEntityData(RenderContext& context) {
            if (!m_entityDataValid)
                m_entityBoundsVertexArray = VertexArrayPtr(NULL);
            if (!m_selectedEntityDataValid)
                m_selectedEntityBoundsVertexArray = VertexArrayPtr(NULL);
            if (!m_lockedEntityDataValid)
                m_lockedEntityBoundsVertexArray = VertexArrayPtr(NULL);
            
            // collect all model entities
            Model::EntityList allEntities;
            Model::EntityList allSelectedEntities;
            Model::EntityList allLockedEntities;
            const Model::EntityList entities = m_document.Map().entities();
            for (unsigned int i = 0; i < entities.size(); i++) {
                Model::Entity* entity = entities[i];
                if (context.filter().entityVisible(*entity)) {
                    if (entity->selected() || entity->partiallySelected())
                        allSelectedEntities.push_back(entity);
                    else if (entity->locked())
                        allLockedEntities.push_back(entity);
                    else
                        allEntities.push_back(entity);
                }
            }
            
            m_entityBoundsVbo->activate();
            m_entityBoundsVbo->map();
            
            if (!m_entityDataValid && !allEntities.empty()) {
                unsigned int entityBoundsVertexCount = 2 * 4 * 6 * static_cast<unsigned int>(allEntities.size());
                m_entityBoundsVertexArray = VertexArrayPtr(new VertexArray(*m_entityBoundsVbo, GL_LINES, entityBoundsVertexCount, VertexAttribute(3, GL_FLOAT, VertexAttribute::Position), VertexAttribute(4, GL_FLOAT, VertexAttribute::Color)));
                writeColoredEntityBounds(context, allEntities, *m_entityBoundsVertexArray);
            }
            
            if (!m_selectedEntityDataValid && !allSelectedEntities.empty()) {
                unsigned int selectedEntityBoundsVertexCount = 2 * 4 * 6 * static_cast<unsigned int>(allSelectedEntities.size());
                m_selectedEntityBoundsVertexArray = VertexArrayPtr(new VertexArray(*m_entityBoundsVbo, GL_LINES, selectedEntityBoundsVertexCount, VertexAttribute(3, GL_FLOAT, VertexAttribute::Position)));
                writeEntityBounds(context, allSelectedEntities, *m_selectedEntityBoundsVertexArray);
            }
            
            if (!m_lockedEntityDataValid && !allLockedEntities.empty()) {
                unsigned int lockedEntityBoundsVertexCount = 2 * 4 * 6 * static_cast<unsigned int>(allLockedEntities.size());
                m_lockedEntityBoundsVertexArray = VertexArrayPtr(new VertexArray(*m_entityBoundsVbo, GL_LINES, lockedEntityBoundsVertexCount, VertexAttribute(3, GL_FLOAT, VertexAttribute::Position)));
                writeEntityBounds(context, allLockedEntities, *m_lockedEntityBoundsVertexArray);
            }
            
            m_entityBoundsVbo->unmap();
            m_entityBoundsVbo->deactivate();
            
            m_entityDataValid = true;
            m_selectedEntityDataValid = true;
            m_lockedEntityDataValid = true;
        }
        
        bool MapRenderer::reloadEntityModel(const Model::Entity& entity, CachedEntityRenderer& cachedRenderer) {
            EntityRenderer* renderer = m_entityRendererManager->entityRenderer(entity, m_document.Mods());
            if (renderer != NULL) {
                cachedRenderer = CachedEntityRenderer(renderer, *entity.classname());
                return true;
            }
            
            return false;
        }
        
        void MapRenderer::reloadEntityModels(RenderContext& context, EntityRenderers& renderers) {
            EntityRenderers::iterator it = renderers.begin();
            while (it != renderers.end()) {
                if (reloadEntityModel(*it->first, it->second))
                    ++it;
                else
                    renderers.erase(it++);
            }
        }
        
        void MapRenderer::reloadEntityModels(RenderContext& context) {
            m_entityRenderers.clear();
            m_selectedEntityRenderers.clear();
            
            const Model::EntityList& entities = m_document.Map().entities();
            for (unsigned int i = 0; i < entities.size(); i++) {
                Model::Entity* entity = entities[i];
                EntityRenderer* renderer = m_entityRendererManager->entityRenderer(*entity, m_document.Mods());
                if (renderer != NULL) {
                    if (entity->selected())
                        m_selectedEntityRenderers[entity] = CachedEntityRenderer(renderer, *entity->classname());
                    else if (entity->locked())
                        m_lockedEntityRenderers[entity] = CachedEntityRenderer(renderer, *entity->classname());
                    else
                        m_entityRenderers[entity] = CachedEntityRenderer(renderer, *entity->classname());
                }
            }
            
            m_entityRendererCacheValid = true;
        }
        
        void MapRenderer::moveEntityRenderer(Model::Entity* entity, EntityRenderers& from, EntityRenderers& to) {
            EntityRenderers::iterator it = from.find(entity);
            if (it != from.end()) {
                to[entity] = it->second;
                from.erase(it);
            }
        }

        void MapRenderer::createShaders() {
            IO::FileManager fileManager;
            String resourceDirectory = fileManager.resourceDirectory();
            
            m_coloredEdgeVertexShader = ShaderPtr(new Shader(fileManager.appendPath(resourceDirectory, "ColoredEdge.vertsh"), GL_VERTEX_SHADER, m_document.Console()));
            m_edgeVertexShader = ShaderPtr(new Shader(fileManager.appendPath(resourceDirectory, "Edge.vertsh"), GL_VERTEX_SHADER, m_document.Console()));
            m_edgeFragmentShader = ShaderPtr(new Shader(fileManager.appendPath(resourceDirectory, "Edge.fragsh"), GL_FRAGMENT_SHADER, m_document.Console()));

            m_faceVertexShader = ShaderPtr(new Shader(fileManager.appendPath(resourceDirectory, "Face.vertsh"), GL_VERTEX_SHADER, m_document.Console()));
            m_faceFragmentShader = ShaderPtr(new Shader(fileManager.appendPath(resourceDirectory, "Face.fragsh"), GL_FRAGMENT_SHADER, m_document.Console()));

            m_entityModelVertexShader = ShaderPtr(new Shader(fileManager.appendPath(resourceDirectory, "EntityModel.vertsh"), GL_VERTEX_SHADER, m_document.Console()));
            m_entityModelFragmentShader = ShaderPtr(new Shader(fileManager.appendPath(resourceDirectory, "EntityModel.fragsh"), GL_FRAGMENT_SHADER, m_document.Console()));
            
            m_textVertexShader = ShaderPtr(new Shader(fileManager.appendPath(resourceDirectory, "Text.vertsh"), GL_VERTEX_SHADER, m_document.Console()));
            m_textFragmentShader = ShaderPtr(new Shader(fileManager.appendPath(resourceDirectory, "Text.fragsh"), GL_FRAGMENT_SHADER, m_document.Console()));
            
            m_textBackgroundVertexShader = ShaderPtr(new Shader(fileManager.appendPath(resourceDirectory, "TextBackground.vertsh"), GL_VERTEX_SHADER, m_document.Console()));
            m_textBackgroundFragmentShader = ShaderPtr(new Shader(fileManager.appendPath(resourceDirectory, "TextBackground.fragsh"), GL_FRAGMENT_SHADER, m_document.Console()));

            m_edgeProgram = ShaderProgramPtr(new ShaderProgram("constant colored edge shader program", m_document.Console()));
            m_edgeProgram->attachShader(*m_edgeVertexShader);
            m_edgeProgram->attachShader(*m_edgeFragmentShader);
            
            m_coloredEdgeProgram = ShaderProgramPtr(new ShaderProgram("colored edge shader program", m_document.Console()));
            m_coloredEdgeProgram->attachShader(*m_coloredEdgeVertexShader);
            m_coloredEdgeProgram->attachShader(*m_edgeFragmentShader);
            
            m_faceProgram = ShaderProgramPtr(new ShaderProgram("face shader program", m_document.Console()));
            m_faceProgram->attachShader(*m_faceVertexShader);
            m_faceProgram->attachShader(*m_faceFragmentShader);
            
            m_entityModelProgram = ShaderProgramPtr(new ShaderProgram("entity model shader program", m_document.Console()));
            m_entityModelProgram->attachShader(*m_entityModelVertexShader);
            m_entityModelProgram->attachShader(*m_entityModelFragmentShader);
            
            m_textProgram = ShaderProgramPtr(new ShaderProgram("text shader program", m_document.Console()));
            m_textProgram->attachShader(*m_textVertexShader);
            m_textProgram->attachShader(*m_textFragmentShader);
            
            m_textBackgroundProgram = ShaderProgramPtr(new ShaderProgram("text background shader program", m_document.Console()));
            m_textBackgroundProgram->attachShader(*m_textBackgroundVertexShader);
            m_textBackgroundProgram->attachShader(*m_textBackgroundFragmentShader);
            
            m_shadersCreated = true;
        }
        
        void MapRenderer::validate(RenderContext& context) {
            if (!m_entityRendererCacheValid)
                reloadEntityModels(context);
            if (!m_geometryDataValid || !m_selectedGeometryDataValid || !m_lockedGeometryDataValid)
                rebuildGeometryData(context);
            if (!m_entityDataValid || !m_selectedEntityDataValid || !m_lockedEntityDataValid)
                rebuildEntityData(context);
        }
        
        void MapRenderer::renderFaces(RenderContext& context) {
            if (m_faceVertexArrays.empty() && m_selectedFaceVertexArrays.empty() && m_lockedFaceVertexArrays.empty())
                return;
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            Utility::Grid& grid = m_document.Grid();
            
            m_faceVbo->activate();
            if (m_faceProgram->activate()) {
                glActiveTexture(GL_TEXTURE0);
                bool applyTexture = context.viewOptions().faceRenderMode() == View::ViewOptions::Textured;
                m_faceProgram->setUniformVariable("Brightness", prefs.getFloat(Preferences::RendererBrightness));
                m_faceProgram->setUniformVariable("RenderGrid", grid.visible());
                m_faceProgram->setUniformVariable("GridSize", static_cast<float>(grid.actualSize()));
                m_faceProgram->setUniformVariable("GridColor", prefs.getColor(Preferences::GridColor));
                if (!m_faceVertexArrays.empty()) {
                    m_faceProgram->setUniformVariable("ApplyTinting", false);
                    m_faceProgram->setUniformVariable("GrayScale", false);
                    for (unsigned int i = 0; i < m_faceVertexArrays.size(); i++) {
                        TextureVertexArray& textureVertexArray = m_faceVertexArrays[i];
                        textureVertexArray.texture->activate();
                        m_faceProgram->setUniformVariable("ApplyTexture", applyTexture && textureVertexArray.texture != m_dummyTexture.get());
                        m_faceProgram->setUniformVariable("FaceTexture", 0);
                        textureVertexArray.vertexArray->render();
                        textureVertexArray.texture->deactivate();
                    }
                }
                if (!m_selectedFaceVertexArrays.empty()) {
                    m_faceProgram->setUniformVariable("ApplyTinting", true);
                    m_faceProgram->setUniformVariable("TintColor", prefs.getColor(Preferences::SelectedFaceColor));
                    m_faceProgram->setUniformVariable("GrayScale", false);
                    for (unsigned int i = 0; i < m_selectedFaceVertexArrays.size(); i++) {
                        TextureVertexArray& textureVertexArray = m_selectedFaceVertexArrays[i];
                        textureVertexArray.texture->activate();
                        m_faceProgram->setUniformVariable("ApplyTexture", applyTexture && textureVertexArray.texture != m_dummyTexture.get());
                        m_faceProgram->setUniformVariable("FaceTexture", 0);
                        textureVertexArray.vertexArray->render();
                        textureVertexArray.texture->deactivate();
                    }
                }
                if (!m_lockedFaceVertexArrays.empty()) {
                    m_faceProgram->setUniformVariable("ApplyTinting", true);
                    m_faceProgram->setUniformVariable("TintColor", prefs.getColor(Preferences::LockedFaceColor));
                    m_faceProgram->setUniformVariable("GrayScale", true);
                    for (unsigned int i = 0; i < m_lockedFaceVertexArrays.size(); i++) {
                        TextureVertexArray& textureVertexArray = m_lockedFaceVertexArrays[i];
                        textureVertexArray.texture->activate();
                        m_faceProgram->setUniformVariable("ApplyTexture", applyTexture && textureVertexArray.texture != m_dummyTexture.get());
                        m_faceProgram->setUniformVariable("FaceTexture", 0);
                        textureVertexArray.vertexArray->render();
                        textureVertexArray.texture->deactivate();
                    }
                }
                m_faceProgram->deactivate();
            }
            m_faceVbo->deactivate();
        }
        
        void MapRenderer::renderEdges(RenderContext& context) {
            if (m_edgeVertexArray.get() == NULL && m_selectedEdgeVertexArray.get() == NULL && m_lockedEdgeVertexArray.get() == NULL)
                return;
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();

            m_edgeVbo->activate();
            if (m_edgeVertexArray.get() != NULL && m_coloredEdgeProgram->activate()) {
                glSetEdgeOffset(0.02f);
                m_edgeVertexArray->render();
                m_coloredEdgeProgram->deactivate();
            }
            if (m_edgeProgram->activate()) {
                if (m_lockedEdgeVertexArray.get() != NULL) {
                    m_edgeProgram->setUniformVariable("Color", prefs.getColor(Preferences::LockedEdgeColor));
                    glSetEdgeOffset(0.02f);
                    m_lockedEdgeVertexArray->render();
                }
                if (m_selectedEdgeVertexArray.get() != NULL) {
                    glDisable(GL_DEPTH_TEST);
                    m_edgeProgram->setUniformVariable("Color", prefs.getColor(Preferences::OccludedSelectedEdgeColor));
                    glSetEdgeOffset(0.02f);
                    m_selectedEdgeVertexArray->render();
                    glEnable(GL_DEPTH_TEST);
                    m_edgeProgram->setUniformVariable("Color", prefs.getColor(Preferences::SelectedEdgeColor));
                    glSetEdgeOffset(0.025f);
                    m_selectedEdgeVertexArray->render();
                }
                m_edgeProgram->deactivate();
            }
            m_edgeVbo->deactivate();
            glResetEdgeOffset();
        }
        
        void MapRenderer::renderEntityBounds(RenderContext& context) {
            if (m_entityBoundsVertexArray.get() == NULL && m_selectedEntityBoundsVertexArray.get() == NULL && m_lockedEntityBoundsVertexArray.get() == NULL)
                return;
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            
            m_entityBoundsVbo->activate();
            if (m_entityBoundsVertexArray.get() != NULL && m_coloredEdgeProgram->activate()) {
                m_entityBoundsVertexArray->render();
                m_coloredEdgeProgram->deactivate();
            }
            if (m_edgeProgram->activate()) {
                if (m_lockedEntityBoundsVertexArray.get() != NULL) {
                    m_edgeProgram->setUniformVariable("Color", prefs.getColor(Preferences::LockedEntityBoundsColor));
                    m_lockedEntityBoundsVertexArray->render();
                }
                if (m_selectedEntityBoundsVertexArray.get() != NULL) {
                    glDisable(GL_DEPTH_TEST);
                    m_edgeProgram->setUniformVariable("Color", prefs.getColor(Preferences::OccludedSelectedEdgeColor));
                    m_selectedEntityBoundsVertexArray->render();
                    glEnable(GL_DEPTH_TEST);
                    m_edgeProgram->setUniformVariable("Color", prefs.getColor(Preferences::SelectedEdgeColor));
                    m_selectedEntityBoundsVertexArray->render();
                }
                m_edgeProgram->deactivate();
            }
            m_entityBoundsVbo->deactivate();
        }
        
        void MapRenderer::renderEntityModels(RenderContext& context) {
            if (m_entityRenderers.empty() && m_selectedEntityRenderers.empty() && m_lockedEntityRenderers.empty())
                return;
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();

            if (m_entityModelProgram->activate()) {
                EntityRenderers::iterator it, end;
                
                m_entityModelProgram->setUniformVariable("Brightness", prefs.getFloat(Preferences::RendererBrightness));
                m_entityRendererManager->activate();
                
                m_entityModelProgram->setUniformVariable("ApplyTinting", false);
                m_entityModelProgram->setUniformVariable("GrayScale", false);
                for (it = m_entityRenderers.begin(), end = m_entityRenderers.end(); it != end; ++it) {
                    Model::Entity* entity = it->first;
                    if (context.filter().entityVisible(*entity)) {
                        EntityRenderer* renderer = it->second.renderer;
                        renderer->render(*m_entityModelProgram, context.transformation(), *entity);
                    }
                }
                
                m_entityModelProgram->setUniformVariable("ApplyTinting", true);
                m_entityModelProgram->setUniformVariable("TintColor", prefs.getColor(Preferences::SelectedFaceColor));
                m_entityModelProgram->setUniformVariable("GrayScale", false);
                for (it = m_selectedEntityRenderers.begin(), end = m_selectedEntityRenderers.end(); it != end; ++it) {
                    Model::Entity* entity = it->first;
                    if (context.filter().entityVisible(*entity)) {
                        EntityRenderer* renderer = it->second.renderer;
                        renderer->render(*m_entityModelProgram, context.transformation(), *entity);
                    }
                }
                
                m_entityModelProgram->setUniformVariable("ApplyTinting", true);
                m_entityModelProgram->setUniformVariable("TintColor", prefs.getColor(Preferences::LockedFaceColor));
                m_entityModelProgram->setUniformVariable("GrayScale", true);
                for (it = m_lockedEntityRenderers.begin(), end = m_lockedEntityRenderers.end(); it != end; ++it) {
                    Model::Entity* entity = it->first;
                    if (context.filter().entityVisible(*entity)) {
                        EntityRenderer* renderer = it->second.renderer;
                        renderer->render(*m_entityModelProgram, context.transformation(), *entity);
                    }
                }
                
                m_entityRendererManager->deactivate();
                m_entityModelProgram->deactivate();
            }
        }
        
        void MapRenderer::renderEntityClassnames(RenderContext& context) {
            if (m_classnameRenderer->empty() && m_selectedClassnameRenderer->empty() && m_lockedClassnameRenderer->empty())
                return;
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();

            EntityClassnameFilter classnameFilter;
            m_classnameRenderer->render(context, classnameFilter, *m_textProgram,
                                        prefs.getColor(Preferences::InfoOverlayTextColor), *m_textBackgroundProgram,
                                        prefs.getColor(Preferences::InfoOverlayBackgroundColor));
            m_lockedClassnameRenderer->render(context, classnameFilter, *m_textProgram,
                                              prefs.getColor(Preferences::LockedInfoOverlayTextColor), *m_textBackgroundProgram,
                                              prefs.getColor(Preferences::LockedInfoOverlayBackgroundColor));
            glDisable(GL_DEPTH_TEST);
            m_selectedClassnameRenderer->render(context, classnameFilter, *m_textProgram,
                                                prefs.getColor(Preferences::OccludedSelectedInfoOverlayTextColor), *m_textBackgroundProgram,
                                                prefs.getColor(Preferences::OccludedSelectedInfoOverlayBackgroundColor));
            glEnable(GL_DEPTH_TEST);
            m_selectedClassnameRenderer->render(context, classnameFilter, *m_textProgram,
                                                prefs.getColor(Preferences::SelectedInfoOverlayTextColor), *m_textBackgroundProgram,
                                                prefs.getColor(Preferences::SelectedInfoOverlayBackgroundColor));
        }

        MapRenderer::MapRenderer(Model::MapDocument& document) :
        m_document(document) {
            m_rendering = false;
            m_shadersCreated = false;

            m_faceVbo = VboPtr(new Vbo(GL_ARRAY_BUFFER, 0xFFFF));
            m_edgeVbo = VboPtr(new Vbo(GL_ARRAY_BUFFER, 0xFFFF));
            m_entityBoundsVbo = VboPtr(new Vbo(GL_ARRAY_BUFFER, 0xFFFF));
            
            m_geometryDataValid = false;
            m_selectedGeometryDataValid = false;
            m_lockedGeometryDataValid = false;

            m_entityDataValid = false;
            m_selectedEntityDataValid = false;
            m_lockedEntityDataValid = false;

            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();

            m_entityRendererManager = EntityRendererManagerPtr(new EntityRendererManager(document.Palette(), document.Console()));
            m_entityRendererCacheValid = true;
            
            m_stringManager = StringManagerPtr(new Text::StringManager(document.Console()));
            
            float infoOverlayFadeDistance = prefs.getFloat(Preferences::InfoOverlayFadeDistance);
            float selectedInfoOverlayFadeDistance = prefs.getFloat(Preferences::SelectedInfoOverlayFadeDistance);
            
            m_classnameRenderer = EntityClassnameRendererPtr(new EntityClassnameRenderer(*m_stringManager, infoOverlayFadeDistance));
            m_selectedClassnameRenderer = EntityClassnameRendererPtr(new EntityClassnameRenderer(*m_stringManager, selectedInfoOverlayFadeDistance));
            m_lockedClassnameRenderer = EntityClassnameRendererPtr(new EntityClassnameRenderer(*m_stringManager, infoOverlayFadeDistance));
            
            m_dummyTexture = Model::TexturePtr(new Model::Texture("dummy"));
        }
        
        MapRenderer::~MapRenderer() {
        }

        void MapRenderer::addEntities(const Model::EntityList& entities) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            const String& fontName = prefs.getString(Preferences::RendererFontName);
            int fontSize = prefs.getInt(Preferences::RendererFontSize);
            Text::FontDescriptor fontDescriptor(fontName, fontSize);
             
            for (unsigned int i = 0; i < entities.size(); i++) {
                Model::Entity* entity = entities[i];
                EntityRenderer* renderer = m_entityRendererManager->entityRenderer(*entity, m_document.Mods());
                if (renderer != NULL)
                    m_entityRenderers[entity] = CachedEntityRenderer(renderer, *entity->classname());
                
                const Model::PropertyValue& classname = *entity->classname();
                EntityClassnameAnchor* anchor = new EntityClassnameAnchor(*entity);
                m_classnameRenderer->addString(entity, fontDescriptor, classname, anchor);
            }
            
            m_entityDataValid = false;
        }

        void MapRenderer::removeEntities(const Model::EntityList& entities) {
            for (unsigned int i = 0; i < entities.size(); i++) {
                Model::Entity* entity = entities[i];
                m_entityRenderers.erase(entity);
                m_classnameRenderer->removeString(entity);
            }
            m_entityDataValid = false;
        }
        
        void MapRenderer::changeEditState(const Model::EditStateChangeSet& changeSet) {
            if (changeSet.entityStateChangedFrom(Model::EditState::Default) ||
                changeSet.entityStateChangedTo(Model::EditState::Default)) {
                m_entityDataValid = false;
            }
            
            if (changeSet.entityStateChangedFrom(Model::EditState::Selected) ||
                changeSet.entityStateChangedTo(Model::EditState::Selected)) {
                m_selectedEntityDataValid = false;
                
                const Model::EntityList& selectedEntities = changeSet.entitiesTo(Model::EditState::Selected);
                for (unsigned int i = 0; i < selectedEntities.size(); i++) {
                    Model::Entity* entity = selectedEntities[i];
                    moveEntityRenderer(entity, m_entityRenderers, m_selectedEntityRenderers);
                    m_classnameRenderer->transferString(entity, *m_selectedClassnameRenderer);
                }
                
                const Model::EntityList& deselectedEntities = changeSet.entitiesFrom(Model::EditState::Selected);
                for (unsigned int i = 0; i < deselectedEntities.size(); i++) {
                    Model::Entity* entity = deselectedEntities[i];
                    moveEntityRenderer(entity, m_selectedEntityRenderers, m_entityRenderers);
                    m_selectedClassnameRenderer->transferString(entity, *m_classnameRenderer);
                }
            }
            
            if (changeSet.entityStateChangedFrom(Model::EditState::Locked) ||
                changeSet.entityStateChangedTo(Model::EditState::Locked)) {
                m_lockedEntityDataValid = false;
                
                const Model::EntityList& lockedEntities = changeSet.entitiesTo(Model::EditState::Locked);
                for (unsigned int i = 0; i < lockedEntities.size(); i++) {
                    Model::Entity* entity = lockedEntities[i];
                    moveEntityRenderer(entity, m_entityRenderers, m_lockedEntityRenderers);
                    m_classnameRenderer->transferString(entity, *m_lockedClassnameRenderer);
                }
                
                const Model::EntityList& unlockedEntities = changeSet.entitiesFrom(Model::EditState::Locked);
                for (unsigned int i = 0; i < unlockedEntities.size(); i++) {
                    Model::Entity* entity = unlockedEntities[i];
                    moveEntityRenderer(entity, m_lockedEntityRenderers, m_entityRenderers);
                    m_lockedClassnameRenderer->transferString(entity, *m_classnameRenderer);
                }
            }
            
            if (changeSet.brushStateChangedFrom(Model::EditState::Default) ||
                changeSet.brushStateChangedTo(Model::EditState::Default) ||
                changeSet.faceSelectionChanged()) {
                m_geometryDataValid = false;
            }

            if (changeSet.brushStateChangedFrom(Model::EditState::Selected) ||
                changeSet.brushStateChangedTo(Model::EditState::Selected) ||
                changeSet.faceSelectionChanged()) {
                m_selectedGeometryDataValid = false;

                const Model::BrushList& selectedBrushes = changeSet.brushesTo(Model::EditState::Selected);
                for (unsigned int i = 0; i < selectedBrushes.size(); i++) {
                    Model::Brush* brush = selectedBrushes[i];
                    Model::Entity* entity = brush->entity();
                    if (!entity->worldspawn() && entity->partiallySelected())
                        m_classnameRenderer->transferString(entity, *m_selectedClassnameRenderer);
                }

                const Model::BrushList& deselectedBrushes = changeSet.brushesFrom(Model::EditState::Selected);
                for (unsigned int i = 0; i < deselectedBrushes.size(); i++) {
                    Model::Brush* brush = deselectedBrushes[i];
                    Model::Entity* entity = brush->entity();
                    if (!entity->worldspawn() && !entity->partiallySelected())
                        m_selectedClassnameRenderer->transferString(entity, *m_classnameRenderer);
                }
            }
            
            if (changeSet.brushStateChangedFrom(Model::EditState::Locked) ||
                changeSet.brushStateChangedTo(Model::EditState::Locked) ||
                changeSet.faceSelectionChanged()) {
                m_lockedGeometryDataValid = false;
            }
        }

        void MapRenderer::loadMap() {
            clearMap();
            addEntities(m_document.Map().entities());
        }
        
        void MapRenderer::clearMap() {
			m_faceVertexArrays.clear();
			m_selectedFaceVertexArrays.clear();
			m_lockedFaceVertexArrays.clear();
			m_edgeVertexArray = VertexArrayPtr();
			m_selectedEdgeVertexArray = VertexArrayPtr();
			m_lockedEdgeVertexArray = VertexArrayPtr();
            m_entityRenderers.clear();
            m_selectedEntityRenderers.clear();
            m_lockedEntityRenderers.clear();
			m_classnameRenderer->clear();
			m_selectedClassnameRenderer->clear();
            m_lockedClassnameRenderer->clear();

            invalidateAll();
            invalidateEntityRendererCache();
        }

        void MapRenderer::invalidateEntities() {
            m_entityDataValid = false;
            m_selectedEntityDataValid = false;
            m_lockedEntityDataValid = false;
        }

        void MapRenderer::invalidateBrushes() {
            m_geometryDataValid = false;
            m_selectedGeometryDataValid = false;
            m_lockedGeometryDataValid = false;
        }

        void MapRenderer::invalidateAll() {
            invalidateEntities();
            invalidateBrushes();
        }

        void MapRenderer::invalidateEntityRendererCache() {
            m_entityRendererCacheValid = false;
        }

        void MapRenderer::render(RenderContext& context) {
            if (m_rendering)
                return;
            m_rendering = true;
            
            if (!m_shadersCreated)
                createShaders();
            validate(context);
            
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glFrontFace(GL_CW);
            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
            glShadeModel(GL_SMOOTH);
            glResetEdgeOffset();
            
            if (context.viewOptions().showBrushes() && context.viewOptions().faceRenderMode() != View::ViewOptions::Discard)
                renderFaces(context);
            if (context.viewOptions().showBrushes() && context.viewOptions().renderEdges())
                renderEdges(context);
            
            if (context.viewOptions().showEntities()) {
                if (context.viewOptions().showEntityModels())
                    renderEntityModels(context);
                if (context.viewOptions().showEntityBounds())
                    renderEntityBounds(context);
                if (context.viewOptions().showEntityClassnames())
                    renderEntityClassnames(context);
            }
            
            m_rendering = false;
        }
    }
}