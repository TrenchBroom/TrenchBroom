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
#include "Renderer/RenderUtils.h"
#include "Renderer/Shader/EdgeShader.h"
#include "Renderer/Shader/FaceShader.h"
#include "Renderer/Shader/Shader.h"
#include "Renderer/Vbo.h"
#include "Renderer/Text/FontDescriptor.h"
#include "Renderer/Text/StringManager.h"
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

        void MapRenderer::writeFaceData(RenderContext& context, const FacesByTexture& facesByTexture, FaceRenderInfoList& renderInfos, ShaderProgram& shaderProgram) {
            if (facesByTexture.empty())
                return;
            
            FacesByTexture::const_iterator it, end;
            for (it = facesByTexture.begin(), end = facesByTexture.end(); it != end; ++it) {
                Model::Texture* texture = it->first;
                const TextureFaceList& textureFaceList = it->second;
                const Model::FaceList& faces = textureFaceList.faces();
                unsigned int vertexCount = static_cast<unsigned int>(textureFaceList.vertexCount());
                VertexArrayPtr vertexArray = VertexArrayPtr(new VertexArray(*m_faceVbo, GL_TRIANGLES, vertexCount, VertexAttribute(3, GL_FLOAT, VertexAttribute::Position), VertexAttribute(2, GL_FLOAT, VertexAttribute::TexCoord0)));

                for (unsigned int i = 0; i < faces.size(); i++) {
                    Model::Face* face = faces[i];
                    const Model::VertexList& vertices = face->vertices();
                    const Vec2f::List& texCoords = face->texCoords();

                    for (unsigned int j = 1; j < vertices.size() - 1; j++) {
                        vertexArray->addAttribute(vertices[0]->position);
                        vertexArray->addAttribute(texCoords[0]);
                        vertexArray->addAttribute(vertices[j]->position);
                        vertexArray->addAttribute(texCoords[j]);
                        vertexArray->addAttribute(vertices[j + 1]->position);
                        vertexArray->addAttribute(texCoords[j + 1]);
                    }
                }
                
                renderInfos.push_back(FaceRenderInfo(texture, vertexArray));
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
                m_faceRenderInfos.clear();
            }
            if (!m_selectedGeometryDataValid) {
                m_selectedEdgeVertexArray = VertexArrayPtr(NULL);
                m_selectedFaceRenderInfos.clear();
            }
            if (!m_lockedGeometryDataValid) {
                m_lockedEdgeVertexArray = VertexArrayPtr(NULL);
                m_lockedFaceRenderInfos.clear();
            }
            
            FacesByTexture unselectedFaces;
            FacesByTexture selectedFaces;
            FacesByTexture lockedFaces;
            
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
							assert(face->vertices().size() >= 3);
                            
                            Model::Texture* texture = face->texture() != NULL ? face->texture() : m_dummyTexture;
                            
                            // because it is often the case that there are subsequences of faces with the same texture,
                            // we always keep an iterator to the texture / face list that was last used
                            FacesByTexture::iterator lastUnselectedTextureIt = unselectedFaces.end();
                            FacesByTexture::iterator lastSelectedTextureIt = selectedFaces.end();
                            FacesByTexture::iterator lastLockedTextureIt = lockedFaces.end();
                            
                            if (entity->selected() || brush->selected() || face->selected()) {
                                if (lastSelectedTextureIt == selectedFaces.end() || lastSelectedTextureIt->first != texture)
                                    lastSelectedTextureIt = selectedFaces.insert(std::pair<Model::Texture*, TextureFaceList>(texture, TextureFaceList())).first;
                                
                                TextureFaceList& textureFaceList = lastSelectedTextureIt->second;
                                textureFaceList.add(*face);
                            } else if (entity->locked() || brush->locked()) {
                                if (lastLockedTextureIt == lockedFaces.end() || lastLockedTextureIt->first != texture)
                                    lastLockedTextureIt = lockedFaces.insert(std::pair<Model::Texture*, TextureFaceList>(texture, TextureFaceList())).first;
                                
                                TextureFaceList& textureFaceList = lastLockedTextureIt->second;
                                textureFaceList.add(*face);
                            } else {
                                if (lastUnselectedTextureIt == unselectedFaces.end() || lastUnselectedTextureIt->first != texture)
                                    lastUnselectedTextureIt = unselectedFaces.insert(std::pair<Model::Texture*, TextureFaceList>(texture, TextureFaceList())).first;

                                TextureFaceList& textureFaceList = lastUnselectedTextureIt->second;
                                textureFaceList.add(*face);
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
                writeFaceData(context, unselectedFaces, m_faceRenderInfos, *m_faceProgram.get());
            if (!m_selectedGeometryDataValid && !selectedFaces.empty())
                writeFaceData(context, selectedFaces, m_selectedFaceRenderInfos, *m_faceProgram.get());
            if (!m_lockedGeometryDataValid && !lockedFaces.empty())
                writeFaceData(context, lockedFaces, m_lockedFaceRenderInfos, *m_faceProgram.get());
            
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
            
            if (!m_geometryDataValid && (!selectedBrushes.empty() || !partiallySelectedBrushFaces.empty())) {
                m_selectedEdgeVertexArray = VertexArrayPtr(new VertexArray(*m_edgeVbo, GL_LINES, totalSelectedEdgeVertexCount, VertexAttribute(3, GL_FLOAT, VertexAttribute::Position)));
                m_selectedEdgeVertexArray->bindAttributes(*m_constantColoredEdgeProgram);
                writeEdgeData(context, selectedBrushes, partiallySelectedBrushFaces, *m_selectedEdgeVertexArray);
            }
            
            m_edgeVbo->unmap();
            m_edgeVbo->deactivate();
            
            m_geometryDataValid = true;
            m_selectedGeometryDataValid = true;
            m_lockedGeometryDataValid = true;
        }
        
        /*
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
         */
        
        void MapRenderer::rebuildEntityData(RenderContext& context) {
            /*
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
            
            if (!m_lockedEntityDataValid) {
                if (m_lockedEntityBoundsBlock != NULL) {
                    m_lockedEntityBoundsBlock->freeBlock();
                    m_lockedEntityBoundsBlock = NULL;
                }
                m_lockedEntityBoundsRenderInfo = EdgeRenderInfo(0, 0);
            }
            
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
                m_entityBoundsBlock = m_entityBoundsVbo->allocBlock(entityBoundsVertexCount * EntityBoundsVertexSize);
            }
            
            if (!m_selectedEntityDataValid && !allSelectedEntities.empty()) {
                unsigned int selectedEntityBoundsVertexCount = 2 * 4 * 6 * static_cast<unsigned int>(allSelectedEntities.size());
                m_selectedEntityBoundsBlock = m_entityBoundsVbo->allocBlock(selectedEntityBoundsVertexCount * EntityBoundsVertexSize);
            }
            
            if (!m_lockedEntityDataValid && !allLockedEntities.empty()) {
                unsigned int lockedEntityBoundsVertexCount = 2 * 4 * 6 * static_cast<unsigned int>(allLockedEntities.size());
                m_lockedEntityBoundsBlock = m_entityBoundsVbo->allocBlock(lockedEntityBoundsVertexCount * EntityBoundsVertexSize);
            }
            
            if (!m_entityDataValid && !allEntities.empty())
                writeEntityBounds(context, allEntities, m_entityBoundsRenderInfo, *m_entityBoundsBlock);
            if (!m_selectedEntityDataValid && !allSelectedEntities.empty())
                writeEntityBounds(context, allSelectedEntities, m_selectedEntityBoundsRenderInfo, *m_selectedEntityBoundsBlock);
            if (!m_lockedEntityDataValid && !allLockedEntities.empty())
                writeEntityBounds(context, allLockedEntities, m_lockedEntityBoundsRenderInfo, *m_lockedEntityBoundsBlock);
            
            m_entityBoundsVbo->unmap();
            m_entityBoundsVbo->deactivate();
            */
            
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
        
        void MapRenderer::createShaders() {
            assert(m_constantColoredEdgeVertexShader.get() == NULL);
            assert(m_coloredEdgeVertexShader.get() == NULL);
            assert(m_edgeFragmentShader.get() == NULL);
            assert(m_faceVertexShader.get() == NULL);
            assert(m_faceFragmentShader.get() == NULL);
            assert(m_constantColoredEdgeProgram.get() == NULL);
            assert(m_coloredEdgeProgram.get() == NULL);
            assert(m_faceProgram.get() == NULL);
            
            m_constantColoredEdgeVertexShader = ShaderPtr(new Shader("constant colored edge vertex shader", GL_VERTEX_SHADER, Shaders::ConstantColoredEdgeVertexShader, m_document.Console()));
            m_constantColoredEdgeVertexShader->createShader();
            
            m_coloredEdgeVertexShader = ShaderPtr(new Shader("colored edge vertex shader", GL_VERTEX_SHADER, Shaders::ColoredEdgeVertexShader, m_document.Console()));
            m_coloredEdgeVertexShader->createShader();

            m_edgeFragmentShader = ShaderPtr(new Shader("edge fragment shader", GL_FRAGMENT_SHADER, Shaders::EdgeFragmentShader, m_document.Console()));
            m_edgeFragmentShader->createShader();
            
            m_faceVertexShader = ShaderPtr(new Shader("face vertex shader", GL_VERTEX_SHADER, Shaders::FaceVertexShader, m_document.Console()));
            m_faceVertexShader->createShader();
            
            m_faceFragmentShader = ShaderPtr(new Shader("face fragment shader", GL_FRAGMENT_SHADER, Shaders::FaceFragmentShader, m_document.Console()));
            m_faceFragmentShader->createShader();
            
            m_constantColoredEdgeProgram = ShaderProgramPtr(new ShaderProgram("constant colored edge shader program", m_document.Console(), "Color"));
            m_constantColoredEdgeProgram->createProgram();
            m_constantColoredEdgeProgram->attachShader(*m_constantColoredEdgeVertexShader);
            m_constantColoredEdgeProgram->attachShader(*m_edgeFragmentShader);
            
            m_coloredEdgeProgram = ShaderProgramPtr(new ShaderProgram("colored edge shader program", m_document.Console()));
            m_coloredEdgeProgram->createProgram();
            m_coloredEdgeProgram->attachShader(*m_coloredEdgeVertexShader);
            m_coloredEdgeProgram->attachShader(*m_edgeFragmentShader);
            
            m_faceProgram = ShaderProgramPtr(new ShaderProgram("face shader program", m_document.Console(), "FaceTexture"));
            m_faceProgram->createProgram();
            m_faceProgram->attachShader(*m_faceVertexShader);
            m_faceProgram->attachShader(*m_faceFragmentShader);
            
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

        /*
        void MapRenderer::renderEntityBounds(RenderContext& context, const EdgeRenderInfo& renderInfo, const Color* color) {
			if (renderInfo.vertexCount == 0)
				return;
            
            glSetEdgeOffset(0.01f);
            
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
        
        void MapRenderer::renderEntityModels(RenderContext& context, EntityRenderers& entities) {
			if (entities.empty())
				return;
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();

            glPushAttrib(GL_TEXTURE_BIT);
            glPolygonMode(GL_FRONT, GL_FILL);
            glEnable(GL_TEXTURE_2D);
            
            glSetBrightness(prefs.getFloat(Preferences::RendererBrightness), false);
            m_entityRendererManager->activate();
            
            glMatrixMode(GL_MODELVIEW);
            EntityRenderers::iterator it;
            for (it = entities.begin(); it != entities.end(); ++it) {
                Model::Entity* entity = it->first;
                if (context.filter().entityVisible(*entity)) {
                    EntityRenderer* renderer = it->second.renderer;
                    renderer->render(context.transformation(), *entity);
                }
            }
            
            m_entityRendererManager->deactivate();
            glPopAttrib();
        }
         */
         
        void MapRenderer::renderEdges(RenderContext& context, VertexArray* edgeVertexArray, const Color* color) {
            if (edgeVertexArray == NULL)
                return;
            glDisable(GL_TEXTURE_2D);
            edgeVertexArray->render();
        }
        
        /*
        void MapRenderer::renderFaces(RenderContext& context, bool textured, bool selected, bool locked, const FaceRenderInfos& renderInfos) {
            if (renderInfos.empty())
                return;
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();

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
                const Color selectedFaceColor = prefs.getColor(Preferences::SelectedFaceColor);
                GLfloat color[4] = {selectedFaceColor.x, selectedFaceColor.y, selectedFaceColor.z, selectedFaceColor.w};
                
                glActiveTexture(GL_TEXTURE1);
                glEnable(GL_TEXTURE_2D);
                m_dummyTexture->activate();
                glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
                glTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
                glTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
                glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, color);
                glTexEnvi (GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PREVIOUS);
                glTexEnvi (GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PREVIOUS);
                glTexEnvi (GL_TEXTURE_ENV, GL_SRC1_RGB, GL_CONSTANT);
                glTexEnvf (GL_TEXTURE_ENV, GL_RGB_SCALE, 2);
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
    */
        
        MapRenderer::MapRenderer(Model::MapDocument& document) :
        m_document(document) {
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

            m_entityRendererManager = new EntityRendererManager(prefs.getString(Preferences::QuakePath), document.Palette(), document.Console());
            m_entityRendererCacheValid = true;
            
            m_stringManager = new Text::StringManager(document.Console());
            
            float infoOverlayFadeDistance = prefs.getFloat(Preferences::InfoOverlayFadeDistance);
            float selectedInfoOverlayFadeDistance = prefs.getFloat(Preferences::SelectedInfoOverlayFadeDistance);
            
            m_classnameRenderer = new Text::TextRenderer<Model::Entity*>(*m_stringManager, infoOverlayFadeDistance);
            m_selectedClassnameRenderer = new Text::TextRenderer<Model::Entity*>(*m_stringManager, selectedInfoOverlayFadeDistance);
            m_lockedClassnameRenderer = new Text::TextRenderer<Model::Entity*>(*m_stringManager, infoOverlayFadeDistance);
            
            /*
            m_figureVbo = new Vbo(GL_ARRAY_BUFFER, 0xFFFF);

            m_gridRenderer = new Renderer::GridRenderer();
            m_gridRenderer->setColor(prefs.gridColor());
             */
            
            m_dummyTexture = new Model::Texture("dummy");
            
            /*
            m_grayScaleShader = glCreateShader(GL_FRAGMENT_SHADER);
            const char* str = Shader::GrayScale.c_str();
            glShaderSource(m_grayScaleShader, 1, &str, NULL);
            glCompileShader(m_grayScaleShader);
             */
        }
        
        MapRenderer::~MapRenderer() {
//            m_figures.clear();
            
            if (m_entityRendererManager != NULL) {
                delete m_entityRendererManager;
                m_entityRendererManager = NULL;
            }
            
            if (m_classnameRenderer != NULL) {
                delete m_classnameRenderer;
                m_classnameRenderer = NULL;
            }
            
            if (m_selectedClassnameRenderer != NULL) {
                delete m_selectedClassnameRenderer;
                m_selectedClassnameRenderer = NULL;
            }
            
            if (m_lockedClassnameRenderer != NULL) {
                delete m_lockedClassnameRenderer;
                m_lockedClassnameRenderer = NULL;
            }
            
            if (m_stringManager != NULL) {
                delete m_stringManager;
                m_stringManager = NULL;
            }
            
            if (m_dummyTexture != NULL) {
                delete m_dummyTexture;
                m_dummyTexture = NULL;
            }
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
                    m_classnameRenderer->transferString(entity, *m_selectedClassnameRenderer);
                }
                
                const Model::EntityList& deselectedEntities = changeSet.entitiesFrom(Model::EditState::Selected);
                for (unsigned int i = 0; i < deselectedEntities.size(); i++) {
                    Model::Entity* entity = deselectedEntities[i];
                    m_selectedClassnameRenderer->transferString(entity, *m_classnameRenderer);
                }
            }
            
            if (changeSet.entityStateChangedFrom(Model::EditState::Locked) ||
                changeSet.entityStateChangedTo(Model::EditState::Locked)) {
                m_lockedEntityDataValid = false;
                
                const Model::EntityList& lockedEntities = changeSet.entitiesTo(Model::EditState::Locked);
                for (unsigned int i = 0; i < lockedEntities.size(); i++) {
                    Model::Entity* entity = lockedEntities[i];
                    m_classnameRenderer->transferString(entity, *m_lockedClassnameRenderer);
                }
                
                const Model::EntityList& unlockedEntities = changeSet.entitiesFrom(Model::EditState::Locked);
                for (unsigned int i = 0; i < unlockedEntities.size(); i++) {
                    Model::Entity* entity = unlockedEntities[i];
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
            }
            
            if (changeSet.brushStateChangedFrom(Model::EditState::Locked) ||
                changeSet.brushStateChangedTo(Model::EditState::Locked) ||
                changeSet.faceSelectionChanged()) {
                m_lockedGeometryDataValid = false;
            }
        }

        void MapRenderer::loadMap() {
            addEntities(m_document.Map().entities());
            
            m_geometryDataValid = false;
            m_selectedGeometryDataValid = false;
            m_lockedGeometryDataValid = false;
            m_entityDataValid = false;
            m_selectedEntityDataValid = false;
            m_lockedEntityDataValid = false;
        }
        
        void MapRenderer::clearMap() {
            m_entityRenderers.clear();
            m_selectedEntityRenderers.clear();
			m_classnameRenderer->clear();
			m_selectedClassnameRenderer->clear();

            m_geometryDataValid = false;
            m_selectedGeometryDataValid = false;
            m_lockedGeometryDataValid = false;
            m_entityDataValid = false;
            m_selectedEntityDataValid = false;
            m_lockedEntityDataValid = false;
        }

        void MapRenderer::render(RenderContext& context) {
            if (m_rendering)
                return;
            m_rendering = true;
            
            if (!m_shadersCreated)
                createShaders();
            validate(context);
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glFrontFace(GL_CW);
            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
            glShadeModel(GL_SMOOTH);
            glResetEdgeOffset();
            
            glDisable(GL_TEXTURE_2D);
            m_edgeVbo->activate();
            if (m_edgeVertexArray.get() != NULL && m_coloredEdgeProgram->activate()) {
                glSetEdgeOffset(0.01f);
                m_edgeVertexArray->render();
                m_coloredEdgeProgram->deactivate();
            }
            if (m_selectedEdgeVertexArray.get() != NULL && m_constantColoredEdgeProgram->activate()) {
                m_constantColoredEdgeProgram->setUniformVariable("Color", prefs.getColor(Preferences::SelectedEdgeColor));
                glSetEdgeOffset(0.015f);
                m_selectedEdgeVertexArray->render();
                m_constantColoredEdgeProgram->deactivate();
            }
            m_edgeVbo->deactivate();
            glResetEdgeOffset();
            
            glPolygonMode(GL_FRONT, GL_FILL);
            m_faceVbo->activate();
            if (!m_faceRenderInfos.empty() && m_faceProgram->activate()) {
                for (unsigned int i = 0; i < m_faceRenderInfos.size(); i++) {
                    FaceRenderInfo& renderInfo = m_faceRenderInfos[i];
                    glActiveTexture(GL_TEXTURE0);
                    renderInfo.texture->activate();
                    m_faceProgram->setUniformVariable("FaceTexture", renderInfo.texture);
                    renderInfo.vertexArray->render();
                    renderInfo.texture->deactivate();
                }
                m_faceProgram->deactivate();
            }
            m_faceVbo->deactivate();
            
            /*
            // render geometry faces
            m_faceVbo->activate();
            glEnableClientState(GL_VERTEX_ARRAY);
            renderFaces(context, true, false, false, m_faceRenderInfos);
            renderFaces(context, true, true, false, m_selectedFaceRenderInfos);
            renderFaces(context, false, false, true, m_lockedFaceRenderInfos);
            glDisableClientState(GL_VERTEX_ARRAY);
            m_faceVbo->deactivate();
            
            // render geometry edges
            m_edgeVbo->activate();
            glEnableClientState(GL_VERTEX_ARRAY);
            glSetEdgeOffset(0.01f);
            renderEdges(context, m_edgeRenderInfo, NULL);
            renderEdges(context, m_lockedEdgeRenderInfo, &prefs.getColor(Preferences::LockedEdgeColor));
            glSetEdgeOffset(0.02f);
            glDisable(GL_DEPTH_TEST);
            renderEdges(context, m_selectedEdgeRenderInfo, &prefs.getColor(Preferences::OccludedSelectedEdgeColor));
            glEnable(GL_DEPTH_TEST);
            renderEdges(context, m_selectedEdgeRenderInfo, &prefs.getColor(Preferences::SelectedEdgeColor));
            glResetEdgeOffset();
            glDisableClientState(GL_VERTEX_ARRAY);
            m_edgeVbo->deactivate();

            // render entity bounds
            m_entityBoundsVbo->activate();
            glEnableClientState(GL_VERTEX_ARRAY);
            renderEntityBounds(context, m_entityBoundsRenderInfo, NULL);
            renderEntityBounds(context, m_lockedEntityBoundsRenderInfo, &prefs.getColor(Preferences::LockedEntityBoundsColor));
            glDisable(GL_DEPTH_TEST);
            renderEntityBounds(context, m_selectedEntityBoundsRenderInfo, &prefs.getColor(Preferences::OccludedSelectedEntityBoundsColor));
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
            renderEntityBounds(context, m_selectedEntityBoundsRenderInfo, &prefs.getColor(Preferences::SelectedEntityBoundsColor));
            glDepthFunc(GL_LESS);
            glDisableClientState(GL_VERTEX_ARRAY);
            m_entityBoundsVbo->deactivate();

            // render entity models
            renderEntityModels(context, m_entityRenderers);
            renderEntityModels(context, m_selectedEntityRenderers);

            // render classnames
            EntityClassnameFilter classnameFilter;
            m_stringManager->activate();
            m_classnameRenderer->render(context, classnameFilter, prefs.getColor(Preferences::InfoOverlayColor));

            glDisable(GL_DEPTH_TEST);
            m_selectedClassnameRenderer->render(context, classnameFilter, prefs.getColor(Preferences::OccludedSelectedInfoOverlayColor));
            glEnable(GL_DEPTH_TEST);

             m_selectedClassnameRenderer->render(context, classnameFilter, prefs.getColor(Preferences::SelectedInfoOverlayColor));
            m_stringManager->deactivate();
             */
            m_rendering = false;
        }
    }
}