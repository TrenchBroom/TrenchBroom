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

#include "Model/Brush.h"
#include "Model/BrushGeometry.h"
#include "Model/Entity.h"
#include "Model/EntityDefinition.h"
#include "Model/Face.h"
#include "Model/Filter.h"
#include "Model/Map.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/Vbo.h"
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

        void MapRenderer::writeFaceData(RenderContext& context, FacesByTexture& facesByTexture, FaceRenderInfos& renderInfos, VboBlock& block) {
            if (facesByTexture.empty())
                return;
            
            unsigned int address = block.address();
            unsigned int offset = 0;
            
            FacesByTexture::iterator textureIt;
            for (textureIt = facesByTexture.begin(); textureIt != facesByTexture.end(); ++textureIt) {
                Model::Texture* texture = textureIt->first;
                Model::FaceList& faces = textureIt->second;
                
                unsigned int vertexCount = 0;
                
                for (unsigned int i = 0; i < faces.size(); i++) {
                    Model::Face* face = faces[i];
                    const Model::VertexList& vertices = face->vertices();
                    const Vec2f::List& texCoords = face->texCoords();
                    const Vec2f::List& gridCoords = face->gridCoords();

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
                address = block.address() + offset;
            }
        }
        
        void MapRenderer::writeEdgeData(RenderContext& context, Model::BrushList& brushes, Model::FaceList& faces, EdgeRenderInfo& renderInfo, VboBlock& block) {
            if (brushes.empty() && faces.empty())
                return;
            
            unsigned int offset = 0;
            unsigned int vertexCount = 0;
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            const Color& worldColor = prefs.getColor(Preferences::EdgeColor);
            
            for (unsigned int i = 0; i < brushes.size(); i++) {
                Model::Brush* brush = brushes[i];
                Model::Entity* entity = brush->entity();
                Model::EntityDefinition* definition = entity->definition();
                const Color& color = (!entity->worldspawn() && definition != NULL && definition->type() == Model::EntityDefinition::Type::Brush) ? definition->color() : worldColor;
                
                const Model::EdgeList& edges = brush->edges();
                for (unsigned int i = 0; i < edges.size(); i++) {
                    Model::Edge* edge = edges[i];
                    offset = block.writeColor(color, offset);
                    offset = block.writeVec(edge->start->position, offset);
                    offset = block.writeColor(color, offset);
                    offset = block.writeVec(edge->end->position, offset);
                }
                vertexCount += (2 * edges.size());
            }
            
            for (unsigned int i = 0; i < faces.size(); i++) {
                Model::Face* face = faces[i];
                Model::Brush* brush = face->brush();
                Model::Entity* entity = brush->entity();
                Model::EntityDefinition* definition = entity->definition();
                const Color& color = (!entity->worldspawn() && definition != NULL && definition->type() == Model::EntityDefinition::Type::Brush) ? definition->color() : worldColor;
                
                const Model::EdgeList& edges = brush->edges();
                for (unsigned int i = 0; i < edges.size(); i++) {
                    Model::Edge* edge = edges[i];
                    offset = block.writeColor(color, offset);
                    offset = block.writeVec(edge->start->position, offset);
                    offset = block.writeColor(color, offset);
                    offset = block.writeVec(edge->end->position, offset);
                }
                vertexCount += (2 * edges.size());
            }
            
            renderInfo = EdgeRenderInfo(block.address(), vertexCount);
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
            
            Model::BrushList unselectedWorldBrushes;
            Model::BrushList unselectedEntityBrushes;
            Model::BrushList selectedBrushes;
            Model::FaceList partiallySelectedBrushFaces;
            unsigned int totalUnselectedEdgeVertexCount = 0;
            unsigned int totalSelectedEdgeVertexCount = 0;
            
            // collect all visible faces and brushes
            const Model::EntityList& entities = m_map.entities();
            for (unsigned int i = 0; i < entities.size(); i++) {
                Model::Entity* entity = entities[i];
                const Model::BrushList& brushes = entity->brushes();
                for (unsigned int j = 0; j < brushes.size(); j++) {
                    Model::Brush* brush = brushes[j];
                    if (context.filter().brushVisible(*brush)) {
                        if (entity->selected() || brush->selected()) {
                            selectedBrushes.push_back(brush);
                            totalSelectedEdgeVertexCount += (2 * brush->edges().size());
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
							assert(face->vertices().size() >= 3);
                            
                            Model::Texture* texture = face->texture() != NULL ? face->texture() : m_dummyTexture;
                            
                            // because it is often the case that there are subsequences of faces with the same texture,
                            // we always keep an iterator to the texture / face list that was last used
                            FacesByTexture::iterator lastSelectedTextureIt = selectedFaces.end();
                            FacesByTexture::iterator lastUnselectedTextureIt = unselectedFaces.end();
                            
                            if (entity->selected() || brush->selected() || face->selected()) {
                                if (lastSelectedTextureIt == selectedFaces.end() || lastSelectedTextureIt->first != texture)
                                    lastSelectedTextureIt = selectedFaces.insert(std::pair<Model::Texture*, Model::FaceList>(texture, Model::FaceList())).first;
                                
                                lastSelectedTextureIt->second.push_back(face);
                                totalSelectedFaceVertexCount += (3 * face->vertices().size() - 6);
                            } else {
                                if (lastUnselectedTextureIt == unselectedFaces.end() || lastUnselectedTextureIt->first != texture)
                                    lastUnselectedTextureIt = unselectedFaces.insert(std::pair<Model::Texture*, Model::FaceList>(texture, Model::FaceList())).first;
                                
                                lastUnselectedTextureIt->second.push_back(face);
                                totalUnselectedFaceVertexCount += (3 * face->vertices().size() - 6);
                            }
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
        
        void MapRenderer::writeEntityBounds(RenderContext& context, const Model::EntityList& entities, EdgeRenderInfo& renderInfo, VboBlock& block) {
            if (entities.empty())
                return;
            
            unsigned int offset = 0;
            unsigned int vertexCount = 0;
            
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
                    offset = block.writeColor(entityColor, offset);
                    offset = block.writeVec(vertices[i], offset);
                }
                
                vertexCount += vertices.size();
            }
            
            renderInfo = EdgeRenderInfo(block.address(), vertexCount);
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
            Model::EntityList allEntities;
            Model::EntityList allSelectedEntities;
            const Model::EntityList entities = m_map.entities();
            for (unsigned int i = 0; i < entities.size(); i++) {
                Model::Entity* entity = entities[i];
                if (context.filter().entityVisible(*entity)) {
                    if (entity->selected() || entity->partiallySelected())
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
        
        void MapRenderer::validate(RenderContext& context) {
            /*
            if (!m_entityRendererCacheValid)
                reloadEntityModels(context);
             */
            if (!m_geometryDataValid || !m_selectedGeometryDataValid)
                rebuildGeometryData(context);
            if (!m_entityDataValid || !m_selectedEntityDataValid)
                rebuildEntityData(context);
        }

        void MapRenderer::renderEntityBounds(RenderContext& context, const EdgeRenderInfo& renderInfo, const Color* color) {
			if (renderInfo.vertexCount == 0)
				return;
            
            glSetEdgeOffset(0.5f);
            
            glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
            if (color != NULL) {
                glColorV4f(*color);
                glVertexPointer(3, GL_FLOAT, EntityBoundsVertexSize, reinterpret_cast<const GLvoid *>(ColorSize));
            } else {
                glInterleavedArrays(GL_C4UB_V3F, EntityBoundsVertexSize, 0);
            }
            
            glDrawArrays(GL_LINES, renderInfo.offset / EntityBoundsVertexSize, renderInfo.vertexCount);
            
            glPopClientAttrib();
            glResetEdgeOffset();
        }
        
        /*
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
        */
         
        void MapRenderer::renderEdges(RenderContext& context, const EdgeRenderInfo& renderInfo, const Color* color) {
            if (renderInfo.vertexCount == 0)
                return;
            
            glDisable(GL_TEXTURE_2D);
            glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
            
            if (color != NULL) {
                glColorV4f(*color);
                glVertexPointer(3, GL_FLOAT, EdgeVertexSize, reinterpret_cast<const GLvoid *>(ColorSize));
            } else {
                glInterleavedArrays(GL_C4UB_V3F, EdgeVertexSize, 0);
            }
            
            glDrawArrays(GL_LINES, renderInfo.offset / EdgeVertexSize, renderInfo.vertexCount);
            glPopClientAttrib();
        }
        
        void MapRenderer::renderFaces(RenderContext& context, bool textured, bool selected, const FaceRenderInfos& renderInfos) {
            if (renderInfos.empty())
                return;
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();

            glPolygonMode(GL_FRONT, GL_FILL);
            glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
            
            /*
            if (m_editor.grid().visible()) {
                glActiveTexture(GL_TEXTURE2);
                glEnable(GL_TEXTURE_2D);
                context.gridRenderer.activate(context.grid);
                glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
                
                glClientActiveTexture(GL_TEXTURE2);
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                glTexCoordPointer(2, GL_FLOAT, FaceVertexSize, (const GLvoid *)0L);
            }
             */
            
            if (selected) {
                const Color selectedFaceColor = prefs.getColor(Preferences::SelectedFaceColor);
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
                glSetBrightness(prefs.getFloat(Preferences::RendererBrightness), false);
                
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
                    if (renderInfo.texture->dummy() && textureActive)
                        glDisable(GL_TEXTURE_2D);
                    else
                        glEnable(GL_TEXTURE_2D);
                    if (!renderInfo.texture->dummy())
                        renderInfo.texture->activate();
                    else
                        glColorV4f(prefs.getColor(Preferences::FaceColor));
                } else {
                    if (!renderInfo.texture->dummy())
                        glColorV4f(renderInfo.texture->averageColor());
                    else
                        glColorV4f(prefs.getColor(Preferences::FaceColor));
                }
                
                glDrawArrays(GL_TRIANGLES, renderInfo.offset / FaceVertexSize, renderInfo.vertexCount);
                if (renderInfo.texture && !renderInfo.texture->dummy())
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
            
            /*
            if (m_editor.grid().visible()) {
                glActiveTexture(GL_TEXTURE2);
                context.gridRenderer.deactivate();
                glDisable(GL_TEXTURE_2D);
                glActiveTexture(GL_TEXTURE0);
            }
             */
            
            glPopClientAttrib();
        }
        
        void MapRenderer::renderFigures(RenderContext& context) {
            /*
            m_figureVbo->activate();
            for (unsigned int i = 0; i < m_figures.size(); i++)
                m_figures[i]->render(context, *m_figureVbo);
            m_figureVbo->deactivate();
             */
        }
        
        MapRenderer::MapRenderer(Model::Map& map) : m_map(map) {
            m_faceVbo = new Vbo(GL_ARRAY_BUFFER, 0xFFFF);
            m_faceBlock = NULL;
            m_selectedFaceBlock = NULL;
            
            m_edgeVbo = new Vbo(GL_ARRAY_BUFFER, 0xFFFF);
            m_edgeBlock = NULL;
            m_selectedEdgeBlock = NULL;
            
            m_entityBoundsVbo = new Vbo(GL_ARRAY_BUFFER, 0xFFFF);
            m_entityBoundsBlock = NULL;
            m_selectedEntityBoundsBlock = NULL;
            
            m_entityDataValid = false;
            m_selectedEntityDataValid = false;
            m_geometryDataValid = false;
            m_selectedGeometryDataValid = false;

            /*
            m_entityRendererManager = new EntityRendererManager(prefs.quakePath(), m_editor.palette());
            m_entityRendererCacheValid = true;
            
            m_classnameRenderer = new TextRenderer<Model::Entity*>(m_fontManager, prefs.infoOverlayFadeDistance());
            m_selectedClassnameRenderer = new TextRenderer<Model::Entity*>(m_fontManager, prefs.selectedInfoOverlayFadeDistance());
            
            m_figureVbo = new Vbo(GL_ARRAY_BUFFER, 0xFFFF);

            m_gridRenderer = new Renderer::GridRenderer();
            m_gridRenderer->setColor(prefs.gridColor());
             */
            
            m_dummyTexture = new Model::Texture("dummy");
        }
        
        MapRenderer::~MapRenderer() {
//            m_figures.clear();
            
            delete m_faceVbo;
            delete m_edgeVbo;
            delete m_entityBoundsVbo;
            
            /*
            delete m_entityRendererManager;
            
            delete m_classnameRenderer;
            delete m_selectedClassnameRenderer;
            
            delete m_figureVbo;
             */
            
            if (m_dummyTexture != NULL)
                delete m_dummyTexture;
        }

        void MapRenderer::addEntities(const Model::EntityList& entities) {
        }

        void MapRenderer::loadMap() {
            addEntities(m_map.entities());
            
            m_entityDataValid = false;
            m_selectedEntityDataValid = false;
            m_geometryDataValid = false;
            m_selectedGeometryDataValid = false;
        }
        
        void MapRenderer::clearMap() {
            /*
            m_entityRenderers.clear();
            m_selectedEntityRenderers.clear();
			m_classnameRenderer->clear();
			m_selectedClassnameRenderer->clear();
             */
            m_entityDataValid = false;
            m_selectedEntityDataValid = false;
            m_geometryDataValid = false;
            m_selectedGeometryDataValid = false;
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
            
            m_faceVbo->activate();
            glEnableClientState(GL_VERTEX_ARRAY);
            renderFaces(context, true, false, m_faceRenderInfos);
            glDisableClientState(GL_VERTEX_ARRAY);
            m_faceVbo->deactivate();
                
            m_edgeVbo->activate();
            glEnableClientState(GL_VERTEX_ARRAY);
            glSetEdgeOffset(0.1f);
            renderEdges(context, m_edgeRenderInfo, NULL);
            glResetEdgeOffset();
            glDisableClientState(GL_VERTEX_ARRAY);
            m_edgeVbo->deactivate();

            m_entityBoundsVbo->activate();
            glEnableClientState(GL_VERTEX_ARRAY);
            renderEntityBounds(context, m_entityBoundsRenderInfo, NULL);
            glDisableClientState(GL_VERTEX_ARRAY);
            m_entityBoundsVbo->deactivate();
        }
    }
}