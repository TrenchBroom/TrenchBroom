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
#include "Model/Map/Face.h"
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
#include "Renderer/Figures/Figure.h"
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

        EdgeRenderInfo::EdgeRenderInfo(GLuint offset, GLuint vertexCount) : offset(offset), vertexCount(vertexCount) {}

        TexturedTriangleRenderInfo::TexturedTriangleRenderInfo(Model::Assets::Texture* texture, GLuint offset, GLuint vertexCount) : texture(texture), offset(offset), vertexCount(vertexCount) {}

        bool compareFacesByTexture(const Model::Face* left, const Model::Face* right) {
            if (right->texture == NULL)
                return false;
            if (left->texture == NULL)
                return true;
            
            return left->texture->uniqueId < right->texture->uniqueId;
        }

        void MapRenderer::writeFaceData(RenderContext& context, std::vector<Model::Face*>& faces, FaceRenderInfos& renderInfos, VboBlock& block) {
            if (faces.empty())
                return;
            
            unsigned int offset = 0;
            unsigned int address = block.address;
            unsigned int vertexCount = 0;
            Model::Assets::Texture* texture = faces[0]->texture;
            unsigned int width = texture != NULL ? texture->width : 1;
            unsigned int height = texture != NULL ? texture->height : 1;
            
            for (unsigned int i = 0; i < faces.size(); i++) {
                Model::Face* face = faces[i];
                if (texture != face->texture) {
                    renderInfos.push_back(TexturedTriangleRenderInfo(texture, address, vertexCount));
                    
                    texture = face->texture;
                    if (texture != NULL) {
                        width = texture->width;
                        height = texture->height;
                    } else {
                        width = height = 1;
                    }
                    
                    vertexCount = 0;
                    address = block.address + offset;
                }
                
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
        }
        
        void MapRenderer::writeEdgeData(RenderContext& context, std::vector<Model::Brush*>& brushes, std::vector<Model::Face*>& faces, EdgeRenderInfo& renderInfo, VboBlock& block) {
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
            
            std::vector<Model::Face*> allFaces;
            std::vector<Model::Face*> allSelectedFaces;
            unsigned int totalFaceVertexCount = 0;
            unsigned int totalSelectedFaceVertexCount = 0;
            
            std::vector<Model::Brush*> allBrushes;
            std::vector<Model::Brush*> allSelectedBrushes;
            std::vector<Model::Face*> allPartialBrushFaces;
            unsigned int totalEdgeVertexCount = 0;
            unsigned int totalSelectedEdgeVertexCount = 0;
            
            // collect all visible faces and brushes
            const std::vector<Model::Entity*>& entities = m_editor.map().entities();
            for (unsigned int i = 0; i < entities.size(); i++) {
                Model::Entity* entity = entities[i];
                const std::vector<Model::Brush*>& brushes = entity->brushes();
                for (unsigned int j = 0; j < brushes.size(); j++) {
                    Model::Brush* brush = brushes[j];
                    if (context.filter.brushVisible(*brush)) {
                        if (entity->selected() || brush->selected) {
                            allSelectedBrushes.push_back(brush);
                            totalSelectedEdgeVertexCount += (2 * brush->geometry->edges.size());
                        } else {
                            allBrushes.push_back(brush);
                            totalEdgeVertexCount += (2 * brush->geometry->edges.size());
                            if (brush->partiallySelected) {
                                for (unsigned int k = 0; k < brush->faces.size(); k++) {
                                    Model::Face* face = brush->faces[k];
                                    if (face->selected) {
                                        allPartialBrushFaces.push_back(face);
                                        totalSelectedEdgeVertexCount += (2 * face->side->edges.size());
                                    }
                                }
                            }
                        }
                        
                        const std::vector<Model::Face*>& faces = brush->faces;
                        for (unsigned int k = 0; k < faces.size(); k++) {
                            Model::Face* face = faces[k];
                            if (entity->selected() || brush->selected || face->selected) {
                                allSelectedFaces.push_back(face);
                                totalSelectedFaceVertexCount += (3 * face->side->vertices.size() - 6);
                            } else {
                                allFaces.push_back(face);
                                totalFaceVertexCount += (3 * face->side->vertices.size() - 6);
                            }
                        }
                    }
                }
            }
            
            // sort the faces by their texture
            std::sort(allFaces.begin(), allFaces.end(), compareFacesByTexture);
            std::sort(allSelectedFaces.begin(), allSelectedFaces.end(), compareFacesByTexture);
            
            // write face triangles
            m_faceVbo->activate();
            m_faceVbo->map();

            if (!m_geometryDataValid && !allFaces.empty())
                m_faceBlock = m_faceVbo->allocBlock(totalFaceVertexCount * FaceVertexSize);
            if (!m_selectedGeometryDataValid && !allSelectedFaces.empty())
                m_selectedFaceBlock = m_faceVbo->allocBlock(totalSelectedFaceVertexCount * FaceVertexSize);
            
            if (!m_geometryDataValid && !allFaces.empty())
                writeFaceData(context, allFaces, m_faceRenderInfos, *m_faceBlock);
            if (!m_selectedGeometryDataValid && !allSelectedFaces.empty())
                writeFaceData(context, allSelectedFaces, m_selectedFaceRenderInfos, *m_selectedFaceBlock);
            
            m_faceVbo->unmap();
            m_faceVbo->deactivate();
            
            // write edges
            m_edgeVbo->activate();
            m_edgeVbo->map();

            if (!m_geometryDataValid && !allBrushes.empty())
                m_edgeBlock = m_edgeVbo->allocBlock(totalEdgeVertexCount * EdgeVertexSize);
            if (!m_selectedGeometryDataValid && (!allSelectedBrushes.empty() || !allPartialBrushFaces.empty()))
                m_selectedEdgeBlock = m_edgeVbo->allocBlock(totalSelectedEdgeVertexCount * EdgeVertexSize);
            
            if (!m_geometryDataValid && !allBrushes.empty()) {
                std::vector<Model::Face*> temp;
                writeEdgeData(context, allBrushes, temp, m_edgeRenderInfo, *m_edgeBlock);
            }
            if (!m_selectedGeometryDataValid && (!allSelectedBrushes.empty() || !allPartialBrushFaces.empty()))
                writeEdgeData(context, allSelectedBrushes, allPartialBrushFaces, m_selectedEdgeRenderInfo, *m_selectedEdgeBlock);
            
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
                unsigned int entityBoundsVertexCount = 2 * 4 * 6 * allEntities.size();
                m_entityBoundsBlock = m_entityBoundsVbo->allocBlock(entityBoundsVertexCount * EntityBoundsVertexSize);
            }
            
            if (!m_selectedEntityDataValid && !allSelectedEntities.empty()) {
                unsigned int selectedEntityBoundsVertexCount = 2 * 4 * 6 * allSelectedEntities.size();
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

        void MapRenderer::entitiesWereAdded(const std::vector<Model::Entity*>& entities) {
            Model::Preferences& prefs = *Model::Preferences::sharedPreferences;
            const std::string& fontName = prefs.rendererFontName();
            int fontSize = prefs.rendererFontSize();
            FontDescriptor descriptor(fontName, fontSize);
            
            for (unsigned int i = 0; i < entities.size(); i++) {
                Model::Entity* entity = entities[i];
                EntityRenderer* renderer = m_entityRendererManager->entityRenderer(*entity, m_editor.map().mods());
                if (renderer != NULL)
                    m_entityRenderers[entity] = renderer;
                
                const Model::PropertyValue& classname = *entity->classname();
                EntityClassnameAnchor* anchor = new EntityClassnameAnchor(*entity);
                TextRenderer::AnchorPtr anchorPtr(anchor);
                m_classnameRenderer->addString(entity->uniqueId(), classname, descriptor, anchorPtr);
            }

            m_entityDataValid = false;
            rendererChanged(*this);
        }

        void MapRenderer::entitiesWillBeRemoved(const std::vector<Model::Entity*>& entities) {
            for (unsigned int i = 0; i < entities.size(); i++) {
                Model::Entity* entity = entities[i];
                m_entityRenderers.erase(entity);
                m_classnameRenderer->removeString(entity->uniqueId());
            }
            m_entityDataValid = false;
            rendererChanged(*this);
        }

        void MapRenderer::propertiesDidChange(const std::vector<Model::Entity*>& entities) {
            m_selectedEntityDataValid = false;
            Model::Entity* worldspawn = m_editor.map().worldspawn(true);
            if (find(entities.begin(), entities.end(), worldspawn) != entities.end()) {
                // if mods changed, invalidate renderer cache here
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
                    
                    m_classnameRenderer->transferString(entity->uniqueId(), *m_selectedClassnameRenderer);
                }
                m_entityDataValid = false;
                m_selectedEntityDataValid = false;
            }
            
            if (!event.brushes.empty() || !event.faces.empty()) {
                m_geometryDataValid = false;
                m_selectedGeometryDataValid = false;
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
                    
                    m_selectedClassnameRenderer->transferString(entity->uniqueId(), *m_classnameRenderer);
                }
                m_entityDataValid = false;
                m_selectedEntityDataValid = false;
            }

            if (!event.brushes.empty() || !event.faces.empty()) {
                m_geometryDataValid = false;
                m_selectedGeometryDataValid = false;
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
            rendererChanged(*this);
        }

        void MapRenderer::validate(RenderContext& context) {
            if (!m_geometryDataValid || !m_selectedGeometryDataValid)
                rebuildGeometryData(context);
            if (!m_entityDataValid || !m_selectedEntityDataValid)
                rebuildEntityData(context);
        }

        void MapRenderer::renderSelectionGuides(RenderContext& context, const Vec4f& color) {
            FontManager& fontManager = m_fontManager;

            const Vec3f cameraPos = context.camera.position();
            Vec3f center = m_selectionBounds.center();
            Vec3f size = m_selectionBounds.size();
            Vec3f diff = center - cameraPos;

            unsigned int maxi = 3;
            Vec3f gv[3][4];
            // X guide
            if (diff.y >= 0) {
                gv[0][0] = m_selectionBounds.min;
                gv[0][0].y -= 5;
                gv[0][1] = gv[0][0];
                gv[0][1].y -= 5;
                gv[0][2] = gv[0][1];
                gv[0][2].x = m_selectionBounds.max.x;
                gv[0][3] = gv[0][0];
                gv[0][3].x = m_selectionBounds.max.x;
            } else {
                gv[0][0] = m_selectionBounds.min;
                gv[0][0].y = m_selectionBounds.max.y + 5;
                gv[0][1] = gv[0][0];
                gv[0][1].y += 5;
                gv[0][2] = gv[0][1];
                gv[0][2].x = m_selectionBounds.max.x;
                gv[0][3] = gv[0][0];
                gv[0][3].x = m_selectionBounds.max.x;
            }

            // Y guide
            if (diff.x >= 0) {
                gv[1][0] = m_selectionBounds.min;
                gv[1][0].x -= 5;
                gv[1][1] = gv[1][0];
                gv[1][1].x -= 5;
                gv[1][2] = gv[1][1];
                gv[1][2].y = m_selectionBounds.max.y;
                gv[1][3] = gv[1][0];
                gv[1][3].y = m_selectionBounds.max.y;
            } else {
                gv[1][0] = m_selectionBounds.min;
                gv[1][0].x = m_selectionBounds.max.x + 5;
                gv[1][1] = gv[1][0];
                gv[1][1].x += 5;
                gv[1][2] = gv[1][1];
                gv[1][2].y = m_selectionBounds.max.y;
                gv[1][3] = gv[1][0];
                gv[1][3].y = m_selectionBounds.max.y;
            }

            if (diff.z >= 0)
                for (unsigned int i = 0; i < 2; i++)
                    for (unsigned int j = 0; j < 4; j++)
                        gv[i][j].z = m_selectionBounds.max.z;

            // Z Guide
            if (cameraPos.x <= m_selectionBounds.min.x && cameraPos.y <= m_selectionBounds.max.y) {
                gv[2][0] = m_selectionBounds.min;
                gv[2][0].x -= 3.5f;
                gv[2][0].y = m_selectionBounds.max.y + 3.5f;
                gv[2][1] = gv[2][0];
                gv[2][1].x -= 3.5f;
                gv[2][1].y += 3.5f;
                gv[2][2] = gv[2][1];
                gv[2][2].z = m_selectionBounds.max.z;
                gv[2][3] = gv[2][0];
                gv[2][3].z = m_selectionBounds.max.z;
            } else if (cameraPos.x <= m_selectionBounds.max.x && cameraPos.y >= m_selectionBounds.max.y) {
                gv[2][0] = m_selectionBounds.max;
                gv[2][0].x += 3.5f;
                gv[2][0].y += 3.5f;
                gv[2][1] = gv[2][0];
                gv[2][1].x += 3.5f;
                gv[2][1].y += 3.5f;
                gv[2][2] = gv[2][1];
                gv[2][2].z = m_selectionBounds.min.z;
                gv[2][3] = gv[2][0];
                gv[2][3].z = m_selectionBounds.min.z;
            } else if (cameraPos.x >= m_selectionBounds.max.x && cameraPos.y >= m_selectionBounds.min.y) {
                gv[2][0] = m_selectionBounds.max;
                gv[2][0].y = m_selectionBounds.min.y;
                gv[2][0].x += 3.5f;
                gv[2][0].y -= 3.5f;
                gv[2][1] = gv[2][0];
                gv[2][1].x += 3.5f;
                gv[2][1].y -= 3.5f;
                gv[2][2] = gv[2][1];
                gv[2][2].z = m_selectionBounds.min.z;
                gv[2][3] = gv[2][0];
                gv[2][3].z = m_selectionBounds.min.z;
            } else if (cameraPos.x >= m_selectionBounds.min.x && cameraPos.y <= m_selectionBounds.min.y) {
                gv[2][0] = m_selectionBounds.min;
                gv[2][0].x -= 3.5f;
                gv[2][0].y -= 3.5f;
                gv[2][1] = gv[2][0];
                gv[2][1].x -= 3.5f;
                gv[2][1].y -= 3.5f;
                gv[2][2] = gv[2][1];
                gv[2][2].z = m_selectionBounds.max.z;
                gv[2][3] = gv[2][0];
                gv[2][3].z = m_selectionBounds.max.z;
            } else {
                // above, inside or below, don't render Z guide
                maxi = 2;
            }

            // initialize the stencil buffer to cancel out the guides in those areas where the strings will be rendered
            glPolygonMode(GL_FRONT, GL_FILL);
            glClear(GL_STENCIL_BUFFER_BIT);
            glColorMask(false, false, false, false);
            glEnable(GL_STENCIL_TEST);
            glStencilFunc(GL_ALWAYS, 1, 1);
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

			bool depth = glIsEnabled(GL_DEPTH_TEST) == GL_TRUE;
            if (depth)
                glDisable(GL_DEPTH_TEST);

            Vec3f points[3];
            for (unsigned int i = 0; i < maxi; i++) {
                points[i] = (gv[i][2] - gv[i][1]) / 2 + gv[i][1];

                /*
                
                float dist = context.camera.distanceTo(points[i]);
                float factor = dist / 300;
                
                
                float width = m_guideStrings[i]->width;
                float height = m_guideStrings[i]->height;

                glPushMatrix();
                glTranslatef(points[i].x, points[i].y, points[i].z);
                context.camera.setBillboard();
                glScalef(factor, factor, 0);
                glTranslatef(-width / 2, -height / 2, 0);
                m_guideStrings[i]->renderBackground(1, 1);
                glPopMatrix();
                 */
            }

            glColorMask(true, true, true, true);
            glStencilFunc(GL_NOTEQUAL, 1, 1);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

            if (depth)
                glEnable(GL_DEPTH_TEST);

            for (unsigned int i = 0; i < 3; i++) {
                glColorV4f(color);

                glBegin(GL_LINE_STRIP);
                for (unsigned int j = 0; j < 4; j++)
                    glVertexV3f(gv[i][j]);
                glEnd();
            }

            glDisable(GL_STENCIL_TEST);

            /*
            fontManager.activate();
            for (unsigned int i = 0; i < maxi; i++) {
                glColorV4f(color);

                float dist = context.camera.distanceTo(points[i]);
                float factor = dist / 300;
                float width = m_guideStrings[i]->width;
                float height = m_guideStrings[i]->height;

                glPushMatrix();
                glTranslatef(points[i].x, points[i].y, points[i].z);
                context.camera.setBillboard();
                glScalef(factor, factor, 0);
                glTranslatef(-width / 2, -height / 2, 0);
                m_guideStrings[i]->render();
                glPopMatrix();
            }
            fontManager.deactivate();
             */
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
                EntityRenderer* renderer = it->second;
                renderer->render(*entity);
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

            if (context.options.renderGrid) {
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

            if (context.options.renderGrid) {
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

        MapRenderer::MapRenderer(Controller::Editor& editor, FontManager& fontManager) : m_editor(editor), m_fontManager(fontManager), m_geometryDataValid(false), m_entityDataValid(false) {
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

            m_classnameRenderer = new TextRenderer(m_fontManager, prefs.infoOverlayFadeDistance());
            m_selectedClassnameRenderer = new TextRenderer(m_fontManager, prefs.selectedInfoOverlayFadeDistance());

            m_figureVbo = new Vbo(GL_ARRAY_BUFFER, 0xFFFF);
            
            m_dummyTexture = new Model::Assets::Texture("dummy");

            m_editor.setRenderer(this);

            Controller::Camera& camera = m_editor.camera();
            Controller::Grid& grid = m_editor.grid();
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
            
            mapLoaded(map);
        }

        MapRenderer::~MapRenderer() {
            m_editor.setRenderer(NULL);
            
            Model::Preferences& prefs = *Model::Preferences::sharedPreferences;
            Controller::Camera& camera = m_editor.camera();
            Controller::Grid& grid = m_editor.grid();
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

            while (!m_figures.empty()) delete m_figures.back(), m_figures.pop_back();
            
            delete m_faceVbo;
            delete m_edgeVbo;
            delete m_entityBoundsVbo;
            
            delete m_entityRendererManager;

            delete m_classnameRenderer;
            delete m_selectedClassnameRenderer;

            m_figures.clear();
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

            delete &figure;
        }

        void MapRenderer::render(RenderContext& context) {
            validate(context);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glFrontFace(GL_CW);
            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
            glShadeModel(GL_SMOOTH);
            glResetEdgeOffset();

            if (context.options.renderOrigin) {
                glDisable(GL_TEXTURE_2D);
                glBegin(GL_LINES);
                glColor4f(1, 0, 0, 0.5f);
                glVertex3f(-context.options.originAxisLength, 0, 0);
                glVertex3f(context.options.originAxisLength, 0, 0);
                glColor4f(0, 1, 0, 0.5f);
                glVertex3f(0, -context.options.originAxisLength, 0);
                glVertex3f(0, context.options.originAxisLength, 0);
                glColor4f(0, 0, 1, 0.5f);
                glVertex3f(0, 0, -context.options.originAxisLength);
                glVertex3f(0, 0, context.options.originAxisLength);
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

            if (context.options.renderBrushes) {
                m_faceVbo->activate();
                glEnableClientState(GL_VERTEX_ARRAY);

                switch (context.options.renderMode) {
                    case Controller::TB_RM_TEXTURED:
                        if (context.options.isolationMode == Controller::IM_NONE)
                            renderFaces(context, true, false, m_faceRenderInfos);
                        if (!m_editor.map().selection().empty())
                            renderFaces(context, true, true, m_selectedFaceRenderInfos);
                        break;
                    case Controller::TB_RM_FLAT:
                        if (context.options.isolationMode == Controller::IM_NONE)
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
                
                if (context.options.isolationMode != Controller::IM_DISCARD) {
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

            if (context.options.renderEntities) {
                if (context.options.isolationMode == Controller::IM_NONE) {
                    m_entityBoundsVbo->activate();
                    glEnableClientState(GL_VERTEX_ARRAY);
                    renderEntityBounds(context, m_entityBoundsRenderInfo, NULL);
                    glDisableClientState(GL_VERTEX_ARRAY);
                    m_entityBoundsVbo->deactivate();

                    renderEntityModels(context, m_entityRenderers);

                    if (context.options.renderEntityClassnames) {
                        m_fontManager.activate();
                        m_classnameRenderer->render(context, context.preferences.infoOverlayColor());
                        m_fontManager.deactivate();
                    }
                } else if (context.options.isolationMode == Controller::IM_WIREFRAME) {
                    m_entityBoundsVbo->activate();
                    glEnableClientState(GL_VERTEX_ARRAY);
                    renderEntityBounds(context, m_entityBoundsRenderInfo, &context.preferences.entityBoundsWireframeColor());
                    glDisableClientState(GL_VERTEX_ARRAY);
                    m_entityBoundsVbo->deactivate();
                }

                if (!m_editor.map().selection().entities().empty()) {
                    if (context.options.renderEntityClassnames) {
                        m_fontManager.activate();
                        m_selectedClassnameRenderer->render(context, context.preferences.selectedInfoOverlayColor());
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

                    if (context.options.renderSizeGuides) {
                        glDisable(GL_DEPTH_TEST);
                        renderSelectionGuides(context, context.preferences.selectionGuideColor());
                    }
                }
            }
            
            renderFigures(context);
        }
        
        EntityRendererManager& MapRenderer::entityRendererManager() {
            return *m_entityRendererManager;
        }

    }
}
