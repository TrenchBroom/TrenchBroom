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
#include "Renderer/EntityRenderer.h"
#include "Renderer/RenderContext.h"
#include "Renderer/SharedResources.h"
#include "Renderer/TextureRenderer.h"
#include "Renderer/TextureRendererManager.h"
#include "Renderer/Shader/Shader.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/Shader/ShaderProgram.h"
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
            TextureRendererManager& textureRendererManager = m_document.sharedResources().textureRendererManager();
            const Color& faceColor = prefs.getColor(Preferences::FaceColor);

            FaceCollectionMap::const_iterator it, end;
            for (it = faceCollectionMap.begin(), end = faceCollectionMap.end(); it != end; ++it) {
                Model::Texture* texture = it->first;
                TextureRenderer& textureRenderer = textureRendererManager.renderer(texture);
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
                    const Color& color = texture != NULL ? textureRenderer.averageColor() : faceColor;
                    
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
                
                vertexArrays.push_back(TextureVertexArray(&textureRenderer, vertexArray));
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
            const Model::EntityList& entities = m_document.map().entities();
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
                            Model::Texture* texture = face->texture();
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

            ShaderManager& shaderManager = m_document.sharedResources().shaderManager();
            ShaderProgram& faceProgram = shaderManager.shaderProgram(Shaders::FaceShader);
            ShaderProgram& coloredEdgeProgram = shaderManager.shaderProgram(Shaders::ColoredEdgeShader);
            ShaderProgram& edgeProgram = shaderManager.shaderProgram(Shaders::EdgeShader);
            
            // write face triangles
            m_faceVbo->activate();
            m_faceVbo->map();
            if (!m_geometryDataValid && !unselectedFaceSorter.empty())
                writeFaceData(context, unselectedFaceSorter.collections(), m_faceVertexArrays, faceProgram);
            if (!m_selectedGeometryDataValid && !selectedFaceSorter.empty())
                writeFaceData(context, selectedFaceSorter.collections(), m_selectedFaceVertexArrays, faceProgram);
            if (!m_lockedGeometryDataValid && !lockedFaceSorter.empty())
                writeFaceData(context, lockedFaceSorter.collections(), m_lockedFaceVertexArrays, faceProgram);
            
            m_faceVbo->unmap();
            m_faceVbo->deactivate();
            
            // write edges
            m_edgeVbo->activate();
            m_edgeVbo->map();
            
            if (!m_geometryDataValid && !unselectedBrushes.empty()) {
                m_edgeVertexArray = VertexArrayPtr(new VertexArray(*m_edgeVbo, GL_LINES, totalUnselectedEdgeVertexCount, VertexAttribute(3, GL_FLOAT, VertexAttribute::Position), VertexAttribute(4, GL_FLOAT, VertexAttribute::Color)));
                m_edgeVertexArray->bindAttributes(coloredEdgeProgram);
                writeColoredEdgeData(context, unselectedBrushes, Model::EmptyFaceList, *m_edgeVertexArray);
            }
            
            if (!m_selectedGeometryDataValid && (!selectedBrushes.empty() || !partiallySelectedBrushFaces.empty())) {
                m_selectedEdgeVertexArray = VertexArrayPtr(new VertexArray(*m_edgeVbo, GL_LINES, totalSelectedEdgeVertexCount, VertexAttribute(3, GL_FLOAT, VertexAttribute::Position)));
                m_selectedEdgeVertexArray->bindAttributes(edgeProgram);
                writeEdgeData(context, selectedBrushes, partiallySelectedBrushFaces, *m_selectedEdgeVertexArray);
            }
            
            if (!m_lockedGeometryDataValid && !lockedBrushes.empty()) {
                m_lockedEdgeVertexArray = VertexArrayPtr(new VertexArray(*m_edgeVbo, GL_LINES, totalLockedEdgeVertexCount, VertexAttribute(3, GL_FLOAT, VertexAttribute::Position)));
                m_lockedEdgeVertexArray->bindAttributes(edgeProgram);
                writeEdgeData(context, lockedBrushes, Model::EmptyFaceList, *m_lockedEdgeVertexArray);
            }
            
            m_edgeVbo->unmap();
            m_edgeVbo->deactivate();
            
            m_geometryDataValid = true;
            m_selectedGeometryDataValid = true;
            m_lockedGeometryDataValid = true;
        }
        
        void MapRenderer::validate(RenderContext& context) {
            if (!m_geometryDataValid || !m_selectedGeometryDataValid || !m_lockedGeometryDataValid)
                rebuildGeometryData(context);
        }
        
        void MapRenderer::renderFaces(RenderContext& context) {
            if (m_faceVertexArrays.empty() && m_selectedFaceVertexArrays.empty() && m_lockedFaceVertexArrays.empty())
                return;
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            Utility::Grid& grid = m_document.grid();
            
            ShaderManager& shaderManager = m_document.sharedResources().shaderManager();
            ShaderProgram& faceProgram = shaderManager.shaderProgram(Shaders::FaceShader);

            m_faceVbo->activate();
            if (faceProgram.activate()) {
                glActiveTexture(GL_TEXTURE0);
                bool applyTexture = context.viewOptions().faceRenderMode() == View::ViewOptions::Textured;
                faceProgram.setUniformVariable("Brightness", prefs.getFloat(Preferences::RendererBrightness));
                faceProgram.setUniformVariable("RenderGrid", grid.visible());
                faceProgram.setUniformVariable("GridSize", static_cast<float>(grid.actualSize()));
                faceProgram.setUniformVariable("GridColor", prefs.getColor(Preferences::GridColor));
                faceProgram.setUniformVariable("ApplyTexture", applyTexture);
                if (!m_faceVertexArrays.empty()) {
                    faceProgram.setUniformVariable("ApplyTinting", false);
                    faceProgram.setUniformVariable("GrayScale", false);
                    for (unsigned int i = 0; i < m_faceVertexArrays.size(); i++) {
                        TextureVertexArray& textureVertexArray = m_faceVertexArrays[i];
                        textureVertexArray.texture->activate();
                        faceProgram.setUniformVariable("FaceTexture", 0);
                        textureVertexArray.vertexArray->render();
                        textureVertexArray.texture->deactivate();
                    }
                }
                if (!m_selectedFaceVertexArrays.empty()) {
                    faceProgram.setUniformVariable("ApplyTinting", true);
                    faceProgram.setUniformVariable("TintColor", prefs.getColor(Preferences::SelectedFaceColor));
                    faceProgram.setUniformVariable("GrayScale", false);
                    for (unsigned int i = 0; i < m_selectedFaceVertexArrays.size(); i++) {
                        TextureVertexArray& textureVertexArray = m_selectedFaceVertexArrays[i];
                        textureVertexArray.texture->activate();
                        faceProgram.setUniformVariable("FaceTexture", 0);
                        textureVertexArray.vertexArray->render();
                        textureVertexArray.texture->deactivate();
                    }
                }
                if (!m_lockedFaceVertexArrays.empty()) {
                    faceProgram.setUniformVariable("ApplyTinting", true);
                    faceProgram.setUniformVariable("TintColor", prefs.getColor(Preferences::LockedFaceColor));
                    faceProgram.setUniformVariable("GrayScale", true);
                    for (unsigned int i = 0; i < m_lockedFaceVertexArrays.size(); i++) {
                        TextureVertexArray& textureVertexArray = m_lockedFaceVertexArrays[i];
                        textureVertexArray.texture->activate();
                        faceProgram.setUniformVariable("FaceTexture", 0);
                        textureVertexArray.vertexArray->render();
                        textureVertexArray.texture->deactivate();
                    }
                }
                faceProgram.deactivate();
            }
            m_faceVbo->deactivate();
        }
        
        void MapRenderer::renderEdges(RenderContext& context) {
            if (m_edgeVertexArray.get() == NULL && m_selectedEdgeVertexArray.get() == NULL && m_lockedEdgeVertexArray.get() == NULL)
                return;
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();

            ShaderManager& shaderManager = m_document.sharedResources().shaderManager();
            ShaderProgram& coloredEdgeProgram = shaderManager.shaderProgram(Shaders::ColoredEdgeShader);
            ShaderProgram& edgeProgram = shaderManager.shaderProgram(Shaders::EdgeShader);

            m_edgeVbo->activate();
            if (m_edgeVertexArray.get() != NULL && coloredEdgeProgram.activate()) {
                glSetEdgeOffset(0.02f);
                m_edgeVertexArray->render();
                coloredEdgeProgram.deactivate();
            }
            if (edgeProgram.activate()) {
                if (m_lockedEdgeVertexArray.get() != NULL) {
                    edgeProgram.setUniformVariable("Color", prefs.getColor(Preferences::LockedEdgeColor));
                    glSetEdgeOffset(0.02f);
                    m_lockedEdgeVertexArray->render();
                }
                if (m_selectedEdgeVertexArray.get() != NULL) {
                    glDisable(GL_DEPTH_TEST);
                    edgeProgram.setUniformVariable("Color", prefs.getColor(Preferences::OccludedSelectedEdgeColor));
                    glSetEdgeOffset(0.02f);
                    m_selectedEdgeVertexArray->render();
                    glEnable(GL_DEPTH_TEST);
                    edgeProgram.setUniformVariable("Color", prefs.getColor(Preferences::SelectedEdgeColor));
                    glSetEdgeOffset(0.025f);
                    m_selectedEdgeVertexArray->render();
                }
                edgeProgram.deactivate();
            }
            m_edgeVbo->deactivate();
            glResetEdgeOffset();
        }
        
        MapRenderer::MapRenderer(Model::MapDocument& document) :
        m_document(document) {
            m_rendering = false;

            m_faceVbo = VboPtr(new Vbo(GL_ARRAY_BUFFER, 0xFFFF));
            m_edgeVbo = VboPtr(new Vbo(GL_ARRAY_BUFFER, 0xFFFF));
            m_entityVbo = VboPtr(new Vbo(GL_ARRAY_BUFFER, 0xFFFF));
            
            m_geometryDataValid = false;
            m_selectedGeometryDataValid = false;
            m_lockedGeometryDataValid = false;

            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            float infoOverlayFadeDistance = prefs.getFloat(Preferences::InfoOverlayFadeDistance);
            float selectedInfoOverlayFadeDistance = prefs.getFloat(Preferences::SelectedInfoOverlayFadeDistance);
            const Color& selectedEntityColor = prefs.getColor(Preferences::SelectedEntityBoundsColor);
            const Color& occludedEntityColor = prefs.getColor(Preferences::OccludedSelectedEntityBoundsColor);
            const Color& lockedEntityColor = prefs.getColor(Preferences::LockedEntityBoundsColor);
            
            m_entityRenderer = EntityRendererPtr(new EntityRenderer(*m_entityVbo, m_document, infoOverlayFadeDistance));
            m_selectedEntityRenderer = EntityRendererPtr(new EntityRenderer(*m_entityVbo, m_document, selectedInfoOverlayFadeDistance, selectedEntityColor, occludedEntityColor));
            m_lockedEntityRenderer = EntityRendererPtr(new EntityRenderer(*m_entityVbo, m_document, infoOverlayFadeDistance, lockedEntityColor));
        }
        
        MapRenderer::~MapRenderer() {
        }

        void MapRenderer::addEntities(const Model::EntityList& entities) {
            m_entityRenderer->addEntities(entities);
        }

        void MapRenderer::removeEntities(const Model::EntityList& entities) {
            m_entityRenderer->removeEntities(entities);
        }
        
        void MapRenderer::changeEditState(const Model::EditStateChangeSet& changeSet) {
            m_entityRenderer->addEntities(changeSet.entitiesTo(Model::EditState::Default));
            m_entityRenderer->removeEntities(changeSet.entitiesFrom(Model::EditState::Default));
            m_selectedEntityRenderer->addEntities(changeSet.entitiesTo(Model::EditState::Selected));
            m_selectedEntityRenderer->removeEntities(changeSet.entitiesFrom(Model::EditState::Selected));
            m_lockedEntityRenderer->addEntities(changeSet.entitiesTo(Model::EditState::Locked));
            m_lockedEntityRenderer->removeEntities(changeSet.entitiesFrom(Model::EditState::Locked));
            
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
                    if (!entity->worldspawn() && entity->partiallySelected()) {
                        m_entityRenderer->removeEntity(*entity);
                        m_selectedEntityRenderer->addEntity(*entity);
                    }
                }

                const Model::BrushList& deselectedBrushes = changeSet.brushesFrom(Model::EditState::Selected);
                for (unsigned int i = 0; i < deselectedBrushes.size(); i++) {
                    Model::Brush* brush = deselectedBrushes[i];
                    Model::Entity* entity = brush->entity();
                    if (!entity->worldspawn() && !entity->partiallySelected()) {
                        m_selectedEntityRenderer->removeEntity(*entity);
                        m_entityRenderer->addEntity(*entity);
                    }
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
            addEntities(m_document.map().entities());
        }
        
        void MapRenderer::clearMap() {
			m_faceVertexArrays.clear();
			m_selectedFaceVertexArrays.clear();
			m_lockedFaceVertexArrays.clear();
			m_edgeVertexArray = VertexArrayPtr();
			m_selectedEdgeVertexArray = VertexArrayPtr();
			m_lockedEdgeVertexArray = VertexArrayPtr();
            m_entityRenderer->clear();
            m_selectedEntityRenderer->clear();
            m_lockedEntityRenderer->clear();

            invalidateAll();
            invalidateEntityModelRendererCache();
        }

        void MapRenderer::invalidateEntities() {
            m_entityRenderer->invalidateBounds();
            m_selectedEntityRenderer->invalidateBounds();
            m_lockedEntityRenderer->invalidateBounds();
        }

        void MapRenderer::invalidateBrushes() {
            m_geometryDataValid = false;
            m_selectedGeometryDataValid = false;
            m_lockedGeometryDataValid = false;
        }

        void MapRenderer::invalidateSelectedBrushes() {
            m_selectedGeometryDataValid = false;
        }

        void MapRenderer::invalidateAll() {
            invalidateEntities();
            invalidateBrushes();
        }

        void MapRenderer::invalidateEntityModelRendererCache() {
            m_entityRenderer->invalidateModels();
            m_selectedEntityRenderer->invalidateModels();
            m_lockedEntityRenderer->invalidateModels();
        }

        void MapRenderer::render(RenderContext& context) {
            if (m_rendering)
                return;
            m_rendering = true;
            
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
                    m_entityRenderer->render(context);
                if (context.viewOptions().showEntityBounds())
                    m_selectedEntityRenderer->render(context);
                if (context.viewOptions().showEntityClassnames())
                    m_lockedEntityRenderer->render(context);
            }
            
            m_rendering = false;
        }
    }
}