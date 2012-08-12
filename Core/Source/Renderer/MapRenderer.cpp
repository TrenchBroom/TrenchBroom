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

#include "MapRenderer.h"
#include <set>
#include <algorithm>
#include <cassert>

#include "Model/Map/Brush.h"
#include "Model/Map/BrushGeometry.h"
#include "Model/Map/Entity.h"
#include "Model/Map/EntityDefinition.h"
#include "Model/Map/Map.h"
#include "Model/Preferences.h"
#include "Model/Selection.h"
#include "Controller/Camera.h"
#include "Controller/Editor.h"
#include "Controller/Grid.h"
#include "Controller/Options.h"
#include "Renderer/EntityRendererManager.h"
#include "Renderer/EntityRenderer.h"
#include "Renderer/EntityClassnameAnchor.h"
#include "Renderer/EntityClassnameFilter.h"
#include "Renderer/Figures/Figure.h"
#include "Renderer/Figures/SizeGuideFigure.h"
#include "Renderer/FontManager.h"
#include "Renderer/GridRenderer.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/TextRenderer.h"
#include "Renderer/Vbo.h"
#include "Utilities/Filter.h"

namespace TrenchBroom {
    namespace Renderer {
        static const int IndexSize = sizeof(GLuint);
        static const int VertexSize = 3 * sizeof(GLfloat);
        static const int ColorSize = 4;
        static const int TexCoordSize = 2 * sizeof(GLfloat);
        static const int FaceVertexSize = TexCoordSize + TexCoordSize + VertexSize;
        static const int EdgeVertexSize = VertexSize;
        static const int EntityBoundsVertexSize = ColorSize + VertexSize;

        MapRenderer::EdgeRenderInfo::EdgeRenderInfo(GLuint offset, GLuint vertexCount) : offset(offset), vertexCount(vertexCount) {}
        
        MapRenderer::TexturedTriangleRenderInfo::TexturedTriangleRenderInfo(Model::Assets::Texture* texture, GLuint offset, GLuint vertexCount) : texture(texture), offset(offset), vertexCount(vertexCount) {}
        
        void MapRenderer::writeFaceData(RenderContext& context, FacesByTexture& facesByTexture, FaceRenderInfos& renderInfos, VboBlock& block) {
            if (facesByTexture.empty())
                return;
            
            unsigned int address = block.address;
            unsigned int offset = 0;
            
            FacesByTexture::iterator textureIt;
            for (textureIt = facesByTexture.begin(); textureIt != facesByTexture.end(); ++textureIt) {
                Model::Assets::Texture* texture = textureIt->first;
                Model::FaceList& faces = textureIt->second;

                unsigned int vertexCount = 0;

                for (unsigned int i = 0; i < faces.size(); i++) {
                    Model::Face* face = faces[i];
                    const std::vector<Model::Vertex*>& vertices = face->side->vertices;
                    const std::vector<Vec2f>& texCoords = face->texCoords();
                    const std::vector<Vec2f>& gridCoords = face->gridCoords();
                    for (unsigned int j = 1; j < vertices.size() - 1; j++) {
                        offset = block.writeVec(gridCoords[0], offset);
                        offset = block.writeVec(texCoords[0], offset);
                        offset = block.writeVec(vertices[0]->position, offset);
                        
                        offset = block.writeVec(gridCoords[j], offset);
                        offset = block.writeVec(texCoords[j], offset);
                        offset = block.writeVec(vertices[j]->position, offset);
                        
                        offset = block.writeVec(gridCoords[j + 1], offset);
                        offset = block.writeVec(texCoords[j + 1], offset);
                        offset = block.writeVec(vertices[j + 1]->position, offset);
                    }
                    
                    vertexCount += (3 * vertices.size() - 6);
                }
                
                renderInfos.push_back(TexturedTriangleRenderInfo(texture, address, vertexCount));
                address = block.address + offset;
            }
        }
        
        void MapRenderer::writeEdgeData(RenderContext& context, Model::BrushList& brushes, Model::FaceList& faces, EdgeRenderInfo& renderInfo, VboBlock& block) {
            if (brushes.empty() && faces.empty())
                return;
            
            unsigned int offset = 0;
            unsigned int vertexCount = 0;
            
            for (unsigned int i = 0; i < brushes.size(); i++) {
                Model::Brush* brush = brushes[i];
                const std::vector<Model::Edge*>& edges = brush->geometry->edges;
                for (unsigned int i = 0; i < edges.size(); i++) {
                    Model::Edge* edge = edges[i];
                    offset = block.writeVec(edge->start->position, offset);
                    offset = block.writeVec(edge->end->position, offset);
                }
                vertexCount += (2 * edges.size());
            }
            
            for (unsigned int i = 0; i < faces.size(); i++) {
                Model::Face* face = faces[i];
                const std::vector<Model::Edge*>& edges = face->side->edges;
                for (unsigned int i = 0; i < edges.size(); i++) {
                    Model::Edge* edge = edges[i];
                    offset = block.writeVec(edge->start->position, offset);
                    offset = block.writeVec(edge->end->position, offset);
                }
                vertexCount += (2 * edges.size());
            }
            
            renderInfo = EdgeRenderInfo(block.address, vertexCount);
        }

        void MapRenderer::rebuildGeometryData(RenderContext& context) {
            if (!m_geometryDataValid) {
                if (m_faceBlock != NULL) {
                    m_faceBlock->freeBlock();
                    m_faceBlock = NULL;
                }
                if (m_edgeBlock != NULL) {
                    m_edgeBlock->freeBlock();
                    m_edgeBlock = NULL;
                }
                m_faceRenderInfos.clear();
                m_edgeRenderInfo = EdgeRenderInfo(0, 0);
            }
            
            if (!m_selectedGeometryDataValid) {
                if (m_selectedFaceBlock != NULL) {
                    m_selectedFaceBlock->freeBlock();
                    m_selectedFaceBlock = NULL;
                }
                if (m_selectedEdgeBlock != NULL) {
                    m_selectedEdgeBlock->freeBlock();
                    m_selectedEdgeBlock = NULL;
                }
                m_selectedFaceRenderInfos.clear();
                m_selectedEdgeRenderInfo = EdgeRenderInfo(0, 0);
            }

            FacesByTexture unselectedFaces;
            FacesByTexture selectedFaces;
            unsigned int totalUnselectedFaceVertexCount = 0;
            unsigned int totalSelectedFaceVertexCount = 0;
            
            Model::BrushList unselectedBrushes;
            Model::BrushList selectedBrushes;
            Model::FaceList partiallySelectedBrushFaces;
            unsigned int totalUnselectedEdgeVertexCount = 0;
            unsigned int totalSelectedEdgeVertexCount = 0;
            
            // collect all visible faces and brushes
            const std::vector<Model::Entity*>& entities = m_editor.map().entities();
            for (unsigned int i = 0; i < entities.size(); i++) {
                Model::Entity* entity = entities[i];
                const std::vector<Model::Brush*>& brushes = entity->brushes();
                for (unsigned int j = 0; j < brushes.size(); j++) {
                    Model::Brush* brush = brushes[j];
					assert(brush->geometry->edges.size() >= 6);
                    if (context.filter.brushVisible(*brush)) {
                        if (entity->selected() || brush->selected) {
                            selectedBrushes.push_back(brush);
                            totalSelectedEdgeVertexCount += (2 * brush->geometry->edges.size());
                        } else {
                            unselectedBrushes.push_back(brush);
                            totalUnselectedEdgeVertexCount += (2 * brush->geometry->edges.size());
                            if (brush->partiallySelected()) {
                                for (unsigned int k = 0; k < brush->faces.size(); k++) {
                                    Model::Face* face = brush->faces[k];
                                    if (face->selected()) {
                                        partiallySelectedBrushFaces.push_back(face);
                                        totalSelectedEdgeVertexCount += (2 * face->side->edges.size());
                                    }
                                }
                            }
                        }
                        
                        const std::vector<Model::Face*>& faces = brush->faces;
                        for (unsigned int k = 0; k < faces.size(); k++) {
                            Model::Face* face = faces[k];
							assert(face->side->vertices.size() >= 3);
                            
                            Model::Assets::Texture* texture = face->texture != NULL ? face->texture : m_dummyTexture;
                            if (entity->selected() || brush->selected || face->selected()) {
                                selectedFaces[texture].push_back(face);
                                totalSelectedFaceVertexCount += (3 * face->side->vertices.size() - 6);
                            } else {
                                unselectedFaces[texture].push_back(face);
                                totalUnselectedFaceVertexCount += (3 * face->side->vertices.size() - 6);
                            }
                        }
                    }
                }
            }
            
            // write face triangles
            m_faceVbo->activate();
            m_faceVbo->map();

            if (!m_geometryDataValid && !unselectedFaces.empty())
                m_faceBlock = m_faceVbo->allocBlock(totalUnselectedFaceVertexCount * FaceVertexSize);
            if (!m_selectedGeometryDataValid && !selectedFaces.empty())
                m_selectedFaceBlock = m_faceVbo->allocBlock(totalSelectedFaceVertexCount * FaceVertexSize);
            
            if (!m_geometryDataValid && !unselectedFaces.empty())
                writeFaceData(context, unselectedFaces, m_faceRenderInfos, *m_faceBlock);
            if (!m_selectedGeometryDataValid && !selectedFaces.empty())
                writeFaceData(context, selectedFaces, m_selectedFaceRenderInfos, *m_selectedFaceBlock);
            
            m_faceVbo->unmap();
            m_faceVbo->deactivate();
            
            // write edges
            m_edgeVbo->activate();
            m_edgeVbo->map();

            if (!m_geometryDataValid && !unselectedBrushes.empty())
                m_edgeBlock = m_edgeVbo->allocBlock(totalUnselectedEdgeVertexCount * EdgeVertexSize);
            if (!m_selectedGeometryDataValid && (!selectedBrushes.empty() || !partiallySelectedBrushFaces.empty()))
                m_selectedEdgeBlock = m_edgeVbo->allocBlock(totalSelectedEdgeVertexCount * EdgeVertexSize);
            
            if (!m_geometryDataValid && !unselectedBrushes.empty()) {
                Model::FaceList temp;
                writeEdgeData(context, unselectedBrushes, temp, m_edgeRenderInfo, *m_edgeBlock);
            }
            if (!m_selectedGeometryDataValid && (!selectedBrushes.empty() || !partiallySelectedBrushFaces.empty()))
                writeEdgeData(context, selectedBrushes, partiallySelectedBrushFaces, m_selectedEdgeRenderInfo, *m_selectedEdgeBlock);
            
            m_edgeVbo->unmap();
            m_edgeVbo->deactivate();
            
            m_geometryDataValid = true;
            m_selectedGeometryDataValid = true;
        }

        void MapRenderer::writeEntityBounds(RenderContext& context, const std::vector<Model::Entity*>& entities, EdgeRenderInfo& renderInfo, VboBlock& block) {
            if (entities.empty())
                return;
            
            unsigned int offset = 0;
            unsigned int vertexCount = 0;
            
            for (unsigned int i = 0; i < entities.size(); i++) {
                Model::Entity* entity = entities[i];
                const BBox& bounds = entity->bounds();
                const Model::EntityDefinitionPtr definition = entity->entityDefinition();
                Vec4f entityColor;
                if (definition != NULL) {
                    entityColor = definition->color;
                    entityColor.w = context.preferences.entityBoundsColor().w;
                } else {
                    entityColor = context.preferences.entityBoundsColor();
                }

                std::vector<Vec3f> vertices = bboxEdgeVertices(bounds);
                for (unsigned int i = 0; i < vertices.size(); i++) {
                    offset = block.writeColor(entityColor, offset);
                    offset = block.writeVec(vertices[i], offset);
                }
                
                vertexCount += vertices.size();
            }
            
            renderInfo = EdgeRenderInfo(block.address, vertexCount);
        }
        
        void MapRenderer::rebuildEntityData(RenderContext& context) {
            if (!m_entityDataValid) {
                if (m_entityBoundsBlock != NULL) {
                    m_entityBoundsBlock->freeBlock();
                    m_entityBoundsBlock = NULL;
                }
                m_entityBoundsRenderInfo = EdgeRenderInfo(0, 0);
            }

            if (!m_selectedEntityDataValid) {
                if (m_selectedEntityBoundsBlock != NULL) {
                    m_selectedEntityBoundsBlock->freeBlock();
                    m_selectedEntityBoundsBlock = NULL;
                }
                m_selectedEntityBoundsRenderInfo = EdgeRenderInfo(0, 0);
            }
            
            // collect all model entities
            std::vector<Model::Entity*> allEntities;
            std::vector<Model::Entity*> allSelectedEntities;
            const std::vector<Model::Entity*> entities = m_editor.map().entities();
            for (unsigned int i = 0; i < entities.size(); i++) {
                Model::Entity* entity = entities[i];
                if (context.filter.entityVisible(*entity)) {
                    if (entity->selected())
                        allSelectedEntities.push_back(entity);
                    else
                        allEntities.push_back(entity);
                }
            }
            
            m_entityBoundsVbo->activate();
            m_entityBoundsVbo->map();

            if (!m_entityDataValid && !allEntities.empty()) {
                unsigned int entityBoundsVertexCount = 2 * 4 * 6 * static_cast<unsigned int>(allEntities.size());
                m_entityBoundsBlock = m_entityBoundsVbo->allocBlock(entityBoundsVertexCount * EntityBoundsVertexSize);
            }
            
            if (!m_selectedEntityDataValid && !allSelectedEntities.empty()) {
                unsigned int selectedEntityBoundsVertexCount = 2 * 4 * 6 * static_cast<unsigned int>(allSelectedEntities.size());
                m_selectedEntityBoundsBlock = m_entityBoundsVbo->allocBlock(selectedEntityBoundsVertexCount * EntityBoundsVertexSize);
            }
            
            if (!m_entityDataValid && !allEntities.empty())
                writeEntityBounds(context, allEntities, m_entityBoundsRenderInfo, *m_entityBoundsBlock);
            if (!m_selectedEntityDataValid && !allSelectedEntities.empty())
                writeEntityBounds(context, allSelectedEntities, m_selectedEntityBoundsRenderInfo, *m_selectedEntityBoundsBlock);

            m_entityBoundsVbo->unmap();
            m_entityBoundsVbo->deactivate();
            
            m_entityDataValid = true;
            m_selectedEntityDataValid = true;
        }

        bool MapRenderer::reloadEntityModel(const Model::Entity& entity, CachedEntityRenderer& cachedRenderer) {
            EntityRenderer* renderer = m_entityRendererManager->entityRenderer(entity, m_editor.map().mods());
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
            
            const Model::EntityList& entities = m_editor.map().entities();
            for (unsigned int i = 0; i < entities.size(); i++) {
                Model::Entity* entity = entities[i];
                EntityRenderer* renderer = m_entityRendererManager->entityRenderer(*entity, m_editor.map().mods());
                if (renderer != NULL) {
                    if (entity->selected())
                        m_selectedEntityRenderers[entity] = CachedEntityRenderer(renderer, *entity->classname());
                    else
                        m_entityRenderers[entity] = CachedEntityRenderer(renderer, *entity->classname());
                }
            }
            
            m_entityRendererCacheValid = true;
        }
        
        void MapRenderer::entitiesWereAdded(const std::vector<Model::Entity*>& entities) {
            Model::Preferences& prefs = *Model::Preferences::sharedPreferences;
            const std::string& fontName = prefs.rendererFontName();
            int fontSize = prefs.rendererFontSize();
            FontDescriptor descriptor(fontName, fontSize);
            
            for (unsigned int i = 0; i < entities.size(); i++) {
                Model::Entity* entity = entities[i];
                EntityRenderer* renderer = m_entityRendererManager->entityRenderer(*entity, m_editor.map().mods());
                if (renderer != NULL)
                    m_entityRenderers[entity] = CachedEntityRenderer(renderer, *entity->classname());
                
                const Model::PropertyValue& classname = *entity->classname();
                EntityClassnameAnchor* anchor = new EntityClassnameAnchor(*entity);
                TextAnchorPtr anchorPtr(anchor);
                m_classnameRenderer->addString(entity, classname, descriptor, anchorPtr);
            }

            m_entityDataValid = false;
            rendererChanged(*this);
        }

        void MapRenderer::entitiesWillBeRemoved(const std::vector<Model::Entity*>& entities) {
            for (unsigned int i = 0; i < entities.size(); i++) {
                Model::Entity* entity = entities[i];
                m_entityRenderers.erase(entity);
                m_classnameRenderer->removeString(entity);
            }
            m_entityDataValid = false;
            rendererChanged(*this);
        }

        void MapRenderer::propertiesDidChange(const std::vector<Model::Entity*>& entities) {
            m_selectedEntityDataValid = false;
            Model::Entity* worldspawn = m_editor.map().worldspawn(false);
            if (worldspawn != NULL && find(entities.begin(), entities.end(), worldspawn) != entities.end()) {
                m_entityRendererCacheValid = false;
            } else {
                // reload entity renderers if the classname has changed
                for (unsigned int i = 0; i < entities.size(); i++) {
                    Model::Entity* entity = entities[i];
                    if (entity->selected()) {
                        EntityRenderers::iterator it = m_selectedEntityRenderers.find(entity);
                        if (it != m_selectedEntityRenderers.end()) {
                            CachedEntityRenderer& cachedRenderer = it->second;
                            if (entity->classname() == NULL)
                                m_selectedEntityRenderers.erase(it);
                            else if (*entity->classname() != cachedRenderer.classname && !reloadEntityModel(*entity, cachedRenderer))
                                m_selectedEntityRenderers.erase(it);
                        } else {
                            EntityRenderer* renderer = m_entityRendererManager->entityRenderer(*entity, m_editor.map().mods());
                            if (renderer != NULL)
                                m_selectedEntityRenderers[entity] = CachedEntityRenderer(renderer, *entity->classname());
                        }

                        if (entity->classname() != NULL)
                            m_selectedClassnameRenderer->updateString(entity, *entity->classname());
                        else
                            m_selectedClassnameRenderer->removeString(entity);
                    }
                }
            }
            
            rendererChanged(*this);
        }

        void MapRenderer::brushesWereAdded(const std::vector<Model::Brush*>& brushes) {
            m_entityDataValid = false;
            m_geometryDataValid = false;
            rendererChanged(*this);
        }

        void MapRenderer::brushesWillBeRemoved(const std::vector<Model::Brush*>& brushes) {
            m_entityDataValid = false;
            m_geometryDataValid = false;
            rendererChanged(*this);
        }

        void MapRenderer::brushesDidChange(const std::vector<Model::Brush*>& brushes) {
            m_selectedEntityDataValid = false;
            m_selectedGeometryDataValid = false;
            
            if (m_sizeGuideFigure != NULL)
                m_sizeGuideFigure->setBounds(m_editor.map().selection().bounds());

            rendererChanged(*this);
        }

        void MapRenderer::facesDidChange(const std::vector<Model::Face*>& faces) {
            m_selectedGeometryDataValid = false;
            rendererChanged(*this);
        }

        void MapRenderer::mapLoaded(Model::Map& map) {
            entitiesWereAdded(map.entities());

            m_entityDataValid = false;
            m_selectedEntityDataValid = false;
            m_geometryDataValid = false;
            m_selectedGeometryDataValid = false;
            rendererChanged(*this);
        }

        void MapRenderer::mapCleared(Model::Map& map) {
            m_entityRenderers.clear();
            m_selectedEntityRenderers.clear();
			m_classnameRenderer->clear();
			m_selectedClassnameRenderer->clear();
            m_entityDataValid = false;
            m_selectedEntityDataValid = false;
            m_geometryDataValid = false;
            m_selectedGeometryDataValid = false;
            rendererChanged(*this);
        }

        void MapRenderer::selectionAdded(const Model::SelectionEventData& event) {
            if (!event.entities.empty()) {
                for (unsigned int i = 0; i < event.entities.size(); i++) {
                    Model::Entity* entity = event.entities[i];
                    EntityRenderers::iterator it = m_entityRenderers.find(entity);
                    if (it != m_entityRenderers.end()) {
                        m_selectedEntityRenderers[entity] = it->second;
                        m_entityRenderers.erase(it);
                    }
                    
                    m_classnameRenderer->transferString(entity, *m_selectedClassnameRenderer);
                }
                m_entityDataValid = false;
                m_selectedEntityDataValid = false;
            }
            
            if (!event.brushes.empty() || !event.faces.empty()) {
                m_geometryDataValid = false;
                m_selectedGeometryDataValid = false;
            }

            Model::Selection& selection = m_editor.map().selection();
            if (selection.selectionMode() == Model::TB_SM_BRUSHES || selection.selectionMode() == Model::TB_SM_BRUSHES_ENTITIES) {
                if (m_sizeGuideFigure == NULL) {
                    Model::Preferences& prefs = *Model::Preferences::sharedPreferences;
                    m_sizeGuideFigure = new SizeGuideFigure(m_fontManager, FontDescriptor(prefs.rendererFontName(), prefs.rendererFontSize()));
                    m_sizeGuideFigure->setColor(prefs.selectionGuideColor());
                    addFigure(*m_sizeGuideFigure);
                }
                m_sizeGuideFigure->setBounds(selection.bounds());
            }
            
            rendererChanged(*this);
        }

        void MapRenderer::selectionRemoved(const Model::SelectionEventData& event) {
            if (!event.entities.empty()) {
                for (unsigned int i = 0; i < event.entities.size(); i++) {
                    Model::Entity* entity = event.entities[i];
                    EntityRenderers::iterator it = m_selectedEntityRenderers.find(entity);
                    if (it != m_selectedEntityRenderers.end()) {
                        m_entityRenderers[entity] = it->second;
                        m_selectedEntityRenderers.erase(it);
                    }
                    
                    m_selectedClassnameRenderer->transferString(entity, *m_classnameRenderer);
                }
                m_entityDataValid = false;
                m_selectedEntityDataValid = false;
            }

            if (!event.brushes.empty() || !event.faces.empty()) {
                m_geometryDataValid = false;
                m_selectedGeometryDataValid = false;
            }
            
            Model::Selection& selection = m_editor.map().selection();
            if (selection.selectionMode() == Model::TB_SM_BRUSHES || selection.selectionMode() == Model::TB_SM_BRUSHES_ENTITIES) {
                m_sizeGuideFigure->setBounds(selection.bounds());
            } else if (m_sizeGuideFigure != NULL) {
                removeFigure(*m_sizeGuideFigure);
                delete m_sizeGuideFigure;
                m_sizeGuideFigure = NULL;
            }

            rendererChanged(*this);
        }

        void MapRenderer::textureManagerDidChange(Model::Assets::TextureManager& textureManager) {
            m_geometryDataValid = false;
            m_selectedGeometryDataValid = false;
            rendererChanged(*this);
        }

        void MapRenderer::cameraDidChange(Controller::Camera& camera) {
            rendererChanged(*this);
        }

        void MapRenderer::gridDidChange(Controller::Grid& grid) {
            rendererChanged(*this);
        }

        void MapRenderer::preferencesDidChange(const std::string& key) {
            if (key == Model::Preferences::QuakePath) {
                Model::Preferences& prefs = *Model::Preferences::sharedPreferences;
                m_entityRendererCacheValid = false;
                m_entityRendererManager->setQuakePath(prefs.quakePath());
            } else if (key == Model::Preferences::GridColor) {
                Model::Preferences& prefs = *Model::Preferences::sharedPreferences;
                m_gridRenderer->setColor(prefs.gridColor());
            }
            rendererChanged(*this);
        }

        void MapRenderer::optionsDidChange(const Controller::TransientOptions& options) {
            rendererChanged(*this);
        }

        void MapRenderer::validate(RenderContext& context) {
            if (!m_entityRendererCacheValid)
                reloadEntityModels(context);
            if (!m_geometryDataValid || !m_selectedGeometryDataValid)
                rebuildGeometryData(context);
            if (!m_entityDataValid || !m_selectedEntityDataValid)
                rebuildEntityData(context);
        }

        void MapRenderer::renderEntityBounds(RenderContext& context, const EdgeRenderInfo& renderInfo, const Vec4f* color) {
			if (renderInfo.vertexCount == 0)
				return;

            glSetEdgeOffset(0.5f);

            glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
            if (color != NULL) {
                glColorV4f(*color);
                glVertexPointer(3, GL_FLOAT, EntityBoundsVertexSize, (const GLvoid *)(long)ColorSize);
            } else {
                glInterleavedArrays(GL_C4UB_V3F, EntityBoundsVertexSize, 0);
            }

            glDrawArrays(GL_LINES, renderInfo.offset / EntityBoundsVertexSize, renderInfo.vertexCount);

            glPopClientAttrib();
            glResetEdgeOffset();
        }

        void MapRenderer::renderEntityModels(RenderContext& context, EntityRenderers& entities) {
			if (entities.empty())
				return;

            glPushAttrib(GL_TEXTURE_BIT);
            glPolygonMode(GL_FRONT, GL_FILL);
            glEnable(GL_TEXTURE_2D);
            
            glSetBrightness(context.preferences.brightness(), false);
            m_entityRendererManager->activate();

            glMatrixMode(GL_MODELVIEW);
            EntityRenderers::iterator it;
            for (it = entities.begin(); it != entities.end(); ++it) {
                Model::Entity* entity = it->first;
                if (context.filter.entityVisible(*entity)) {
                    EntityRenderer* renderer = it->second.renderer;
                    renderer->render(*entity);
                }
            }

            m_entityRendererManager->deactivate();
            glPopAttrib();
        }

        void MapRenderer::renderEdges(RenderContext& context, const EdgeRenderInfo& renderInfo, const Vec4f& color) {
            if (renderInfo.vertexCount == 0)
                return;
            
            glDisable(GL_TEXTURE_2D);
            glColorV4f(color);

            glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
            glVertexPointer(3, GL_FLOAT, EdgeVertexSize, (const GLvoid *)0L);
            glDrawArrays(GL_LINES, renderInfo.offset / EdgeVertexSize, renderInfo.vertexCount);
            glPopClientAttrib();
        }

        void MapRenderer::renderFaces(RenderContext& context, bool textured, bool selected, const FaceRenderInfos& renderInfos) {
            if (renderInfos.empty())
                return;

            glPolygonMode(GL_FRONT, GL_FILL);
            glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);

            if (m_editor.grid().visible()) {
                glActiveTexture(GL_TEXTURE2);
                glEnable(GL_TEXTURE_2D);
                context.gridRenderer.activate(context.grid);
                glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
                
                glClientActiveTexture(GL_TEXTURE2);
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                glTexCoordPointer(2, GL_FLOAT, FaceVertexSize, (const GLvoid *)0L);
            }

            if (selected) {
                const Vec4f& selectedFaceColor = context.preferences.selectedFaceColor();
                GLfloat color[4] = {selectedFaceColor.x, selectedFaceColor.y, selectedFaceColor.z, selectedFaceColor.w};

                glActiveTexture(GL_TEXTURE1);
                glEnable(GL_TEXTURE_2D);
                m_dummyTexture->activate();
                glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
                glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
                glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
                glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, color);
                glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PREVIOUS);
                glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PREVIOUS);
                glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_CONSTANT);
                glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 2);
            }

            bool textureActive = textured;
            glActiveTexture(GL_TEXTURE0);
            if (textured) {
                glEnable(GL_TEXTURE_2D);
                glSetBrightness(context.preferences.brightness(), false);

                glClientActiveTexture(GL_TEXTURE0);
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                glTexCoordPointer(2, GL_FLOAT, FaceVertexSize, (const GLvoid *)(long)TexCoordSize);
            } else {
                glDisable(GL_TEXTURE_2D);
            }

            glVertexPointer(3, GL_FLOAT, FaceVertexSize, (const GLvoid *)(long)(TexCoordSize + TexCoordSize));

            for (unsigned int i = 0; i < renderInfos.size(); i++) {
                const TexturedTriangleRenderInfo& renderInfo = renderInfos[i];
                if (textured) {
                    if (renderInfo.texture == NULL && textureActive)
                        glDisable(GL_TEXTURE_2D);
                    else
                        glEnable(GL_TEXTURE_2D);
                    if (renderInfo.texture != NULL)
                        renderInfo.texture->activate();
                    else
                        glColorV4f(context.preferences.faceColor());
                } else {
                    if (renderInfo.texture != NULL)
                        glColorV4f(renderInfo.texture->averageColor);
                    else
                        glColorV4f(context.preferences.faceColor());
                }
                
                glDrawArrays(GL_TRIANGLES, renderInfo.offset / FaceVertexSize, renderInfo.vertexCount);
                if (renderInfo.texture && renderInfo.texture != NULL)
                    renderInfo.texture->deactivate();
            }
            
            if (textured && textureActive)
                glDisable(GL_TEXTURE_2D);

            if (selected) {
                glActiveTexture(GL_TEXTURE1);
                m_dummyTexture->deactivate();
                glDisable(GL_TEXTURE_2D);
                glActiveTexture(GL_TEXTURE0);
            }

            if (m_editor.grid().visible()) {
                glActiveTexture(GL_TEXTURE2);
                context.gridRenderer.deactivate();
                glDisable(GL_TEXTURE_2D);
                glActiveTexture(GL_TEXTURE0);
            }

            glPopClientAttrib();
        }

        void MapRenderer::renderFigures(RenderContext& context) {
            m_figureVbo->activate();
            for (unsigned int i = 0; i < m_figures.size(); i++)
                m_figures[i]->render(context, *m_figureVbo);
            m_figureVbo->deactivate();
        }

        MapRenderer::MapRenderer(Controller::Editor& editor, FontManager& fontManager) : m_editor(editor), m_fontManager(fontManager), m_geometryDataValid(false), m_entityDataValid(false), m_sizeGuideFigure(NULL) {
            Model::Preferences& prefs = *Model::Preferences::sharedPreferences;

            m_faceVbo = new Vbo(GL_ARRAY_BUFFER, 0xFFFF);
            m_faceBlock = NULL;
            m_selectedFaceBlock = NULL;
            
            m_edgeVbo = new Vbo(GL_ARRAY_BUFFER, 0xFFFF);
            m_edgeBlock = NULL;
            m_selectedEdgeBlock = NULL;
            
            m_entityBoundsVbo = new Vbo(GL_ARRAY_BUFFER, 0xFFFF);
            m_entityBoundsBlock = NULL;
            m_selectedEntityBoundsBlock = NULL;
            
            m_entityRendererManager = new EntityRendererManager(prefs.quakePath(), m_editor.palette());
            m_entityRendererCacheValid = true;

            m_classnameRenderer = new TextRenderer<Model::Entity*>(m_fontManager, prefs.infoOverlayFadeDistance());
            m_selectedClassnameRenderer = new TextRenderer<Model::Entity*>(m_fontManager, prefs.selectedInfoOverlayFadeDistance());

            m_figureVbo = new Vbo(GL_ARRAY_BUFFER, 0xFFFF);
            
            m_dummyTexture = new Model::Assets::Texture("dummy");
            m_gridRenderer = new Renderer::GridRenderer();
            m_gridRenderer->setColor(prefs.gridColor());

            m_editor.setRenderer(this);

            Controller::Camera& camera = m_editor.camera();
            Controller::Grid& grid = m_editor.grid();
            Controller::TransientOptions& options = m_editor.options();
            Model::Map& map = m_editor.map();
            Model::Selection& selection = map.selection();
            Model::Assets::TextureManager& textureManager = m_editor.textureManager();

            map.mapLoaded                           += new Model::Map::MapEvent::Listener<MapRenderer>(this, &MapRenderer::mapLoaded);
            map.mapCleared                          += new Model::Map::MapEvent::Listener<MapRenderer>(this, &MapRenderer::mapCleared);
            map.entitiesWereAdded                   += new Model::Map::EntityEvent::Listener<MapRenderer>(this, &MapRenderer::entitiesWereAdded);
            map.propertiesDidChange                 += new Model::Map::EntityEvent::Listener<MapRenderer>(this, &MapRenderer::propertiesDidChange);
            map.entitiesWillBeRemoved               += new Model::Map::EntityEvent::Listener<MapRenderer>(this, &MapRenderer::entitiesWillBeRemoved);
            map.brushesDidChange                    += new Model::Map::BrushEvent::Listener<MapRenderer>(this, &MapRenderer::brushesDidChange);
            map.facesDidChange                      += new Model::Map::FaceEvent::Listener<MapRenderer>(this, &MapRenderer::facesDidChange);
            selection.selectionAdded                += new Model::Selection::SelectionEvent::Listener<MapRenderer>(this, &MapRenderer::selectionAdded);
            selection.selectionRemoved              += new Model::Selection::SelectionEvent::Listener<MapRenderer>(this, &MapRenderer::selectionRemoved);
            textureManager.textureManagerDidChange  += new Model::Assets::TextureManager::TextureManagerEvent::Listener<MapRenderer>(this, &MapRenderer::textureManagerDidChange);
            camera.cameraDidChange                  += new Controller::Camera::CameraEvent::Listener<MapRenderer>(this, &MapRenderer::cameraDidChange);
            grid.gridDidChange                      += new Controller::Grid::GridEvent::Listener<MapRenderer>(this, &MapRenderer::gridDidChange);
            prefs.preferencesDidChange              += new Model::Preferences::PreferencesEvent::Listener<MapRenderer>(this, &MapRenderer::preferencesDidChange);
            options.optionsDidChange                += new Controller::TransientOptions::OptionsEvent::Listener<MapRenderer>(this, &MapRenderer::optionsDidChange);
            
            mapLoaded(map);
        }

        MapRenderer::~MapRenderer() {
            m_editor.setRenderer(NULL);
            
            Model::Preferences& prefs = *Model::Preferences::sharedPreferences;
            Controller::Camera& camera = m_editor.camera();
            Controller::Grid& grid = m_editor.grid();
            Controller::TransientOptions& options = m_editor.options();
            Model::Map& map = m_editor.map();
            Model::Selection& selection = map.selection();
            Model::Assets::TextureManager& textureManager = m_editor.textureManager();
            
            map.mapLoaded                           -= new Model::Map::MapEvent::Listener<MapRenderer>(this, &MapRenderer::mapLoaded);
            map.mapCleared                          -= new Model::Map::MapEvent::Listener<MapRenderer>(this, &MapRenderer::mapCleared);
            map.entitiesWereAdded                   -= new Model::Map::EntityEvent::Listener<MapRenderer>(this, &MapRenderer::entitiesWereAdded);
            map.propertiesDidChange                 -= new Model::Map::EntityEvent::Listener<MapRenderer>(this, &MapRenderer::propertiesDidChange);
            map.entitiesWillBeRemoved               -= new Model::Map::EntityEvent::Listener<MapRenderer>(this, &MapRenderer::entitiesWillBeRemoved);
            map.brushesDidChange                    -= new Model::Map::BrushEvent::Listener<MapRenderer>(this, &MapRenderer::brushesDidChange);
            map.facesDidChange                      -= new Model::Map::FaceEvent::Listener<MapRenderer>(this, &MapRenderer::facesDidChange);
            selection.selectionAdded                -= new Model::Selection::SelectionEvent::Listener<MapRenderer>(this, &MapRenderer::selectionAdded);
            selection.selectionRemoved              -= new Model::Selection::SelectionEvent::Listener<MapRenderer>(this, &MapRenderer::selectionRemoved);
            textureManager.textureManagerDidChange  -= new Model::Assets::TextureManager::TextureManagerEvent::Listener<MapRenderer>(this, &MapRenderer::textureManagerDidChange);
            camera.cameraDidChange                  -= new Controller::Camera::CameraEvent::Listener<MapRenderer>(this, &MapRenderer::cameraDidChange);
            grid.gridDidChange                      -= new Controller::Grid::GridEvent::Listener<MapRenderer>(this, &MapRenderer::gridDidChange);
            prefs.preferencesDidChange              -= new Model::Preferences::PreferencesEvent::Listener<MapRenderer>(this, &MapRenderer::preferencesDidChange);
            options.optionsDidChange                -= new Controller::TransientOptions::OptionsEvent::Listener<MapRenderer>(this, &MapRenderer::optionsDidChange);

            m_figures.clear();

            delete m_faceVbo;
            delete m_edgeVbo;
            delete m_entityBoundsVbo;
            
            delete m_entityRendererManager;

            delete m_classnameRenderer;
            delete m_selectedClassnameRenderer;

            delete m_figureVbo;
            
            if (m_dummyTexture != NULL)
                delete m_dummyTexture;
        }

        void MapRenderer::addFigure(Figure& figure) {
            m_figures.push_back(&figure);
            rendererChanged(*this);
        }
        
        void MapRenderer::removeFigure(Figure& figure) {
            std::vector<Figure*>::iterator it = find(m_figures.begin(), m_figures.end(), &figure);
            if (it != m_figures.end()) {
                m_figures.erase(it);
                rendererChanged(*this);
            }
        }

        void MapRenderer::render() {
            Renderer::RenderContext context(m_editor.camera(), m_editor.filter(), m_editor.grid(), m_editor.options(), *m_gridRenderer);
            validate(context);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glFrontFace(GL_CW);
            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
            glShadeModel(GL_SMOOTH);
            glResetEdgeOffset();

            if (context.options.renderOrigin()) {
                glDisable(GL_TEXTURE_2D);
                glBegin(GL_LINES);
                glColor4f(1, 0, 0, 0.5f);
                glVertex3f(-context.options.originAxisLength(), 0, 0);
                glVertex3f(context.options.originAxisLength(), 0, 0);
                glColor4f(0, 1, 0, 0.5f);
                glVertex3f(0, -context.options.originAxisLength(), 0);
                glVertex3f(0, context.options.originAxisLength(), 0);
                glColor4f(0, 0, 1, 0.5f);
                glVertex3f(0, 0, -context.options.originAxisLength());
                glVertex3f(0, 0, context.options.originAxisLength());
                glEnd();
            }

//            glColor4f(1, 1, 1, 1);
//
//            glMatrixMode(GL_MODELVIEW);
//            glPushMatrix();
//            m_editor.camera().setBillboard();
//
//            FontDescriptor descriptor("Arial", 13);
//            StringRenderer& renderer = m_fontManager.createStringRenderer(descriptor, "test");
//            m_fontManager.activate();
//            glTranslatef(100, 200, 0);
//            renderer.render();
//            m_fontManager.deactivate();
//            m_fontManager.destroyStringRenderer(renderer);
//            glPopMatrix();

            if (context.options.renderBrushes()) {
                m_faceVbo->activate();
                glEnableClientState(GL_VERTEX_ARRAY);

                switch (context.options.renderMode()) {
                    case Controller::TB_RM_TEXTURED:
                        if (context.options.isolationMode() == Controller::IM_NONE)
                            renderFaces(context, true, false, m_faceRenderInfos);
                        if (!m_editor.map().selection().empty())
                            renderFaces(context, true, true, m_selectedFaceRenderInfos);
                        break;
                    case Controller::TB_RM_FLAT:
                        if (context.options.isolationMode() == Controller::IM_NONE)
                            renderFaces(context, false, false, m_faceRenderInfos);
                        if (!m_editor.map().selection().empty())
                            renderFaces(context, false, true, m_selectedFaceRenderInfos);
                        break;
                    case Controller::TB_RM_WIREFRAME:
                        break;
                }

                glDisableClientState(GL_VERTEX_ARRAY);
                m_faceVbo->deactivate();
                
                m_edgeVbo->activate();
                glEnableClientState(GL_VERTEX_ARRAY);
                
                if (context.options.isolationMode() != Controller::IM_DISCARD) {
                    glSetEdgeOffset(0.1f);
                    renderEdges(context, m_edgeRenderInfo, context.preferences.edgeColor());
                    glResetEdgeOffset();
                }

                if (!m_editor.map().selection().empty()) {
                    glDisable(GL_DEPTH_TEST);
                    renderEdges(context, m_selectedEdgeRenderInfo, context.preferences.hiddenSelectedEdgeColor());
                    glEnable(GL_DEPTH_TEST);

                    glSetEdgeOffset(0.2f);
                    glDepthFunc(GL_LEQUAL);
                    renderEdges(context, m_selectedEdgeRenderInfo, context.preferences.selectedEdgeColor());
                    glDepthFunc(GL_LESS);
                    glResetEdgeOffset();
                }

                m_edgeVbo->deactivate();
                glDisableClientState(GL_VERTEX_ARRAY);
            }

            if (context.options.renderEntities()) {
                EntityClassnameFilter classnameFilter;
                
                if (context.options.isolationMode() == Controller::IM_NONE) {
                    m_entityBoundsVbo->activate();
                    glEnableClientState(GL_VERTEX_ARRAY);
                    renderEntityBounds(context, m_entityBoundsRenderInfo, NULL);
                    glDisableClientState(GL_VERTEX_ARRAY);
                    m_entityBoundsVbo->deactivate();

                    renderEntityModels(context, m_entityRenderers);

                    if (context.options.renderEntityClassnames()) {
                        m_fontManager.activate();
                        m_classnameRenderer->render(context, classnameFilter, context.preferences.infoOverlayColor());
                        m_fontManager.deactivate();
                    }
                } else if (context.options.isolationMode() == Controller::IM_WIREFRAME) {
                    m_entityBoundsVbo->activate();
                    glEnableClientState(GL_VERTEX_ARRAY);
                    renderEntityBounds(context, m_entityBoundsRenderInfo, &context.preferences.entityBoundsWireframeColor());
                    glDisableClientState(GL_VERTEX_ARRAY);
                    m_entityBoundsVbo->deactivate();
                }

                if (!m_editor.map().selection().selectedEntities().empty()) {
                    if (context.options.renderEntityClassnames()) {
                        m_fontManager.activate();
                        m_selectedClassnameRenderer->render(context, classnameFilter, context.preferences.selectedInfoOverlayColor());
                        m_fontManager.deactivate();
                    }

                    m_entityBoundsVbo->activate();
                    glEnableClientState(GL_VERTEX_ARRAY);

                    glDisable(GL_CULL_FACE);
                    glDisable(GL_DEPTH_TEST);
                    renderEntityBounds(context, m_selectedEntityBoundsRenderInfo, &context.preferences.hiddenSelectedEntityBoundsColor());
                    glEnable(GL_DEPTH_TEST);
                    glDepthFunc(GL_LEQUAL);
                    renderEntityBounds(context, m_selectedEntityBoundsRenderInfo, &context.preferences.selectedEntityBoundsColor());
                    glDepthFunc(GL_LESS);
                    glEnable(GL_CULL_FACE);

                    glDisableClientState(GL_VERTEX_ARRAY);
                    m_entityBoundsVbo->deactivate();

                    renderEntityModels(context, m_selectedEntityRenderers);
                }
            }
            
            renderFigures(context);
        }
        
        EntityRendererManager& MapRenderer::entityRendererManager() {
            return *m_entityRendererManager;
        }
        
        FontManager& MapRenderer::fontManager() {
            return m_fontManager;
        }

    }
}
