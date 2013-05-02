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

#include "Controller/AddObjectsCommand.h"
#include "Controller/Command.h"
#include "Controller/ChangeEditStateCommand.h"
#include "Controller/PreferenceChangeEvent.h"
#include "Controller/RemoveObjectsCommand.h"
#include "IO/FileManager.h"
#include "Model/Brush.h"
#include "Model/BrushGeometry.h"
#include "Model/EditStateManager.h"
#include "Model/Entity.h"
#include "Model/EntityDefinition.h"
#include "Model/Filter.h"
#include "Model/Map.h"
#include "Model/MapDocument.h"
#include "Renderer/EdgeRenderer.h"
#include "Renderer/EntityRenderer.h"
#include "Renderer/EntityRotationDecorator.h"
#include "Renderer/EntityLinkDecorator.h"
#include "Renderer/FaceRenderer.h"
#include "Renderer/PointHandleRenderer.h"
#include "Renderer/PointTraceRenderer.h"
#include "Renderer/RenderContext.h"
#include "Renderer/SharedResources.h"
#include "Renderer/TextureRenderer.h"
#include "Renderer/TextureRendererManager.h"
#include "Renderer/Shader/Shader.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/Shader/ShaderProgram.h"
#include "Renderer/Vbo.h"
#include "Renderer/Text/FontDescriptor.h"
#include "Utility/Console.h"
#include "Utility/Grid.h"
#include "Utility/List.h"
#include "Utility/Preferences.h"

namespace TrenchBroom {
    namespace Renderer {
        static const int IndexSize = sizeof(GLuint);
        static const int VertexSize = 3 * sizeof(GLfloat);
        static const int NormalSize = 3 * sizeof(GLfloat);
        static const int ColorSize = 4;
        static const int TexCoordSize = 2 * sizeof(GLfloat);
        static const int FaceVertexSize = VertexSize + NormalSize + TexCoordSize;
        static const int EdgeVertexSize = VertexSize;
        static const int EntityBoundsVertexSize = ColorSize + VertexSize;

        void MapRenderer::rebuildGeometryData(RenderContext& context) {
            if (!m_geometryDataValid) {
                delete m_faceRenderer;
                m_faceRenderer = NULL;
                delete m_edgeRenderer;
                m_edgeRenderer = NULL;
            }
            if (!m_selectedGeometryDataValid) {
                delete m_selectedFaceRenderer;
                m_selectedFaceRenderer = NULL;
                delete m_selectedEdgeRenderer;
                m_selectedEdgeRenderer = NULL;
            }
            if (!m_lockedGeometryDataValid) {
                delete m_lockedFaceRenderer;
                m_lockedFaceRenderer = NULL;
                delete m_lockedEdgeRenderer;
                m_lockedEdgeRenderer = NULL;
            }
            
            FaceSorter unselectedFaceSorter;
            FaceSorter selectedFaceSorter;
            FaceSorter lockedFaceSorter;
            
            Model::BrushList unselectedWorldBrushes;
            Model::BrushList unselectedEntityBrushes;
            Model::BrushList selectedBrushes;
            Model::BrushList lockedBrushes;
            Model::FaceList partiallySelectedBrushFaces;
            
            // collect all visible faces and brushes
            const Model::EntityList& entities = m_document.map().entities();
            for (size_t i = 0; i < entities.size(); i++) {
                Model::Entity* entity = entities[i];
                const Model::BrushList& brushes = entity->brushes();
                for (size_t j = 0; j < brushes.size(); j++) {
                    Model::Brush* brush = brushes[j];
                    if (context.filter().brushVisible(*brush)) {
                        if (entity->selected() || brush->selected()) {
                            selectedBrushes.push_back(brush);
                        } else if (entity->locked() || brush->locked()) {
                            lockedBrushes.push_back(brush);
                        } else {
                            if (entity->worldspawn())
                                unselectedWorldBrushes.push_back(brush);
                            else
                                unselectedEntityBrushes.push_back(brush);
                            if (brush->partiallySelected()) {
                                const Model::FaceList& faces = brush->faces();
                                for (size_t k = 0; k < faces.size(); k++) {
                                    Model::Face* face = faces[k];
                                    if (face->selected()) {
                                        partiallySelectedBrushFaces.push_back(face);
                                    }
                                }
                            }
                        }
                        
                        const Model::FaceList& faces = brush->faces();
                        for (size_t k = 0; k < faces.size(); k++) {
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

            // write face triangles
            m_faceVbo->activate();
            m_faceVbo->map();
            
            // make sure that the VBO is sufficiently large
            size_t totalFaceVertexCount = unselectedFaceSorter.vertexCount() + selectedFaceSorter.vertexCount() + lockedFaceSorter.vertexCount();
            size_t totalPolygonCount = unselectedFaceSorter.polygonCount() + selectedFaceSorter.polygonCount() + lockedFaceSorter.polygonCount();
            size_t totalTriangleVertexCount = 3 * totalFaceVertexCount - 6 * totalPolygonCount;
            m_faceVbo->ensureFreeCapacity(static_cast<unsigned int>(totalTriangleVertexCount) * FaceVertexSize);
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            TextureRendererManager& textureRendererManager = m_document.sharedResources().textureRendererManager();
            const Color& faceColor = prefs.getColor(Preferences::FaceColor);

            if (!m_geometryDataValid && !unselectedFaceSorter.empty()) {
                assert(m_faceRenderer == NULL);
                m_faceRenderer = new FaceRenderer(*m_faceVbo, textureRendererManager, unselectedFaceSorter, faceColor);
            }
            
            if (!m_selectedGeometryDataValid && !selectedFaceSorter.empty()) {
                assert(m_selectedFaceRenderer == NULL);
                m_selectedFaceRenderer = new FaceRenderer(*m_faceVbo, textureRendererManager, selectedFaceSorter, faceColor);
            }
            
            if (!m_lockedGeometryDataValid && !lockedFaceSorter.empty()) {
                assert(m_lockedFaceRenderer == NULL);
                m_lockedFaceRenderer = new FaceRenderer(*m_faceVbo, textureRendererManager, lockedFaceSorter, faceColor);
            }
            
            m_faceVbo->unmap();
            m_faceVbo->deactivate();
            
            // write edges
            m_edgeVbo->activate();
            m_edgeVbo->map();
            
            const Color& edgeColor = prefs.getColor(Preferences::EdgeColor);

            if (!m_geometryDataValid && !unselectedBrushes.empty()) {
                assert(m_edgeRenderer == NULL);
                m_edgeRenderer = new EdgeRenderer(*m_edgeVbo, unselectedBrushes, Model::EmptyFaceList, edgeColor);
            }
            
            if (!m_selectedGeometryDataValid && (!selectedBrushes.empty() || !partiallySelectedBrushFaces.empty())) {
                assert(m_selectedEdgeRenderer == NULL);
                m_selectedEdgeRenderer = new EdgeRenderer(*m_edgeVbo, selectedBrushes, partiallySelectedBrushFaces);
            }
            
            if (!m_lockedGeometryDataValid && !lockedBrushes.empty()) {
                assert(m_lockedEdgeRenderer == NULL);
                m_lockedEdgeRenderer = new EdgeRenderer(*m_edgeVbo, lockedBrushes, Model::EmptyFaceList);
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
        
        void MapRenderer::invalidateDecorators() {
            EntityDecorator::List::const_iterator decoratorIt, decoratorEnd;
            for (decoratorIt = m_entityDecorators.begin(), decoratorEnd = m_entityDecorators.end(); decoratorIt != decoratorEnd; ++decoratorIt) {
                EntityDecorator& decorator = **decoratorIt;
                decorator.invalidate();
            }
        }

        void MapRenderer::renderFaces(RenderContext& context) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            
            m_faceVbo->activate();
            if (m_faceRenderer != NULL)
                m_faceRenderer->render(context, false);
            if (context.viewOptions().renderSelection() && m_selectedFaceRenderer != NULL) {
                const Color& color = m_overrideSelectionColors ? m_selectedFaceColor : prefs.getColor(Preferences::SelectedFaceColor);
                m_selectedFaceRenderer->render(context, false, color);
            }
            if (m_lockedFaceRenderer != NULL)
                m_lockedFaceRenderer->render(context, true, prefs.getColor(Preferences::LockedFaceColor));
            m_faceVbo->deactivate();
        }
        
        void MapRenderer::renderEdges(RenderContext& context) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            
            m_edgeVbo->activate();
            if (context.viewOptions().renderEdges()) {
                if (m_edgeRenderer != NULL) {
                    glSetEdgeOffset(0.02f);
                    m_edgeRenderer->render(context);
                }
                if (m_lockedEdgeRenderer != NULL) {
                    glSetEdgeOffset(0.02f);
                    m_lockedEdgeRenderer->render(context, prefs.getColor(Preferences::LockedEdgeColor));
                }
            }
            if (context.viewOptions().renderSelection() && m_selectedEdgeRenderer != NULL) {
                const Color& edgeColor = m_overrideSelectionColors ? m_selectedEdgeColor : prefs.getColor(Preferences::SelectedEdgeColor);
                const Color& occludedEdgeColor = m_overrideSelectionColors ? m_occludedSelectedEdgeColor : prefs.getColor(Preferences::OccludedSelectedEdgeColor);
                
                
                glDisable(GL_DEPTH_TEST);
                glSetEdgeOffset(0.02f);
                m_selectedEdgeRenderer->render(context, occludedEdgeColor);
                glEnable(GL_DEPTH_TEST);
                glSetEdgeOffset(0.025f);
                m_selectedEdgeRenderer->render(context, edgeColor);
            }
            m_edgeVbo->deactivate();
            glResetEdgeOffset();
        }
        
        void MapRenderer::renderDecorators(RenderContext& context) {
            EntityDecorator::List::const_iterator decoratorIt, decoratorEnd;
            for (decoratorIt = m_entityDecorators.begin(), decoratorEnd = m_entityDecorators.end(); decoratorIt != decoratorEnd; ++decoratorIt) {
                EntityDecorator& decorator = **decoratorIt;
                decorator.render(*m_utilityVbo, context);
            }
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
                invalidateDecorators();
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
            
            if (changeSet.entityStateChangedFrom(Model::EditState::Hidden) ||
                changeSet.entityStateChangedTo(Model::EditState::Hidden)) {
                invalidateDecorators();
            }
            
            if (changeSet.brushStateChangedFrom(Model::EditState::Hidden) ||
                changeSet.brushStateChangedTo(Model::EditState::Hidden)) {
                
                const Model::BrushList& hiddenBrushes = changeSet.brushesTo(Model::EditState::Hidden);
                for (unsigned int i = 0; i < hiddenBrushes.size(); i++) {
                    Model::Brush* brush = hiddenBrushes[i];
                    Model::Entity* entity = brush->entity();
                    if (!entity->worldspawn() && entity->fullyHidden())
                        m_entityRenderer->removeEntity(*entity);
                }
                
                const Model::BrushList& unhiddenBrushes = changeSet.brushesFrom(Model::EditState::Hidden);
                for (unsigned int i = 0; i < unhiddenBrushes.size(); i++) {
                    Model::Brush* brush = unhiddenBrushes[i];
                    Model::Entity* entity = brush->entity();
                    if (!entity->worldspawn())
                        m_entityRenderer->addEntity(*entity);
                }
                
                invalidateDecorators();
            }
            
            if (changeSet.brushStateChangedFrom(Model::EditState::Locked) ||
                changeSet.brushStateChangedTo(Model::EditState::Locked)) {
                m_lockedGeometryDataValid = false;
            }
        }
        
        void MapRenderer::invalidateEntities() {
            m_entityRenderer->invalidateBounds();
            m_selectedEntityRenderer->invalidateBounds();
            m_lockedEntityRenderer->invalidateBounds();
            invalidateDecorators();
        }
        
        void MapRenderer::invalidateSelectedEntities() {
            m_selectedEntityRenderer->invalidateBounds();
            invalidateDecorators();
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
        
        void MapRenderer::invalidateSelectedEntityModelRendererCache() {
            m_selectedEntityRenderer->invalidateModels();
        }
        
        void MapRenderer::clear() {
            delete m_faceRenderer;
            m_faceRenderer = NULL;
            delete m_selectedFaceRenderer;
            m_selectedFaceRenderer = NULL;
            delete m_lockedFaceRenderer;
            m_lockedFaceRenderer = NULL;
            
            delete m_edgeRenderer;
            m_edgeRenderer = NULL;
            delete m_selectedEdgeRenderer;
            m_selectedEdgeRenderer = NULL;
            delete m_lockedEdgeRenderer;
            m_lockedEdgeRenderer = NULL;
            
            m_entityRenderer->clear();
            m_selectedEntityRenderer->clear();
            m_lockedEntityRenderer->clear();
            
            invalidateAll();
            invalidateEntityModelRendererCache();
        }
        
        MapRenderer::MapRenderer(Model::MapDocument& document) :
        m_document(document),
        m_faceVbo(NULL),
        m_faceRenderer(NULL),
        m_selectedFaceRenderer(NULL),
        m_lockedFaceRenderer(NULL),
        m_edgeVbo(NULL),
        m_edgeRenderer(NULL),
        m_selectedEdgeRenderer(NULL),
        m_lockedEdgeRenderer(NULL),
        m_entityVbo(NULL),
        m_entityRenderer(NULL),
        m_selectedEntityRenderer(NULL),
        m_lockedEntityRenderer(NULL),
        m_utilityVbo(NULL),
        m_pointTraceRenderer(NULL),
        m_overrideSelectionColors(false),
        m_rendering(false),
        m_geometryDataValid(false),
        m_selectedGeometryDataValid(false),
        m_lockedGeometryDataValid(false) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();

            m_faceVbo = new Vbo(GL_ARRAY_BUFFER, 0xFFFF);
            m_edgeVbo = new Vbo(GL_ARRAY_BUFFER, 0xFFFF);
            m_entityVbo = new Vbo(GL_ARRAY_BUFFER, 0xFFFF);
            m_utilityVbo = new Vbo(GL_ARRAY_BUFFER, 0xFFFF);
            
            m_entityRenderer = new EntityRenderer(*m_entityVbo, m_document);
            m_entityRenderer->setClassnameFadeDistance(prefs.getFloat(Preferences::InfoOverlayFadeDistance));
            m_entityRenderer->setClassnameColor(prefs.getColor(Preferences::InfoOverlayTextColor), prefs.getColor(Preferences::InfoOverlayBackgroundColor));
            
            m_selectedEntityRenderer = new EntityRenderer(*m_entityVbo, m_document);
            m_selectedEntityRenderer->setClassnameFadeDistance(prefs.getFloat(Preferences::SelectedInfoOverlayFadeDistance));
            m_selectedEntityRenderer->setClassnameColor(prefs.getColor(Preferences::SelectedInfoOverlayTextColor), prefs.getColor(Preferences::SelectedInfoOverlayBackgroundColor));
            m_selectedEntityRenderer->setOccludedClassnameColor(prefs.getColor(Preferences::SelectedInfoOverlayTextColor), prefs.getColor(Preferences::SelectedInfoOverlayBackgroundColor));
            m_selectedEntityRenderer->setBoundsColor(prefs.getColor(Preferences::SelectedEntityBoundsColor));
            m_selectedEntityRenderer->setOccludedBoundsColor(prefs.getColor(Preferences::OccludedSelectedEntityBoundsColor));
            m_selectedEntityRenderer->setTintColor(prefs.getColor(Preferences::SelectedEntityColor));
            
            m_lockedEntityRenderer = new EntityRenderer(*m_entityVbo, m_document);
            m_lockedEntityRenderer->setClassnameFadeDistance(prefs.getFloat(Preferences::InfoOverlayFadeDistance));
            m_lockedEntityRenderer->setClassnameColor(prefs.getColor(Preferences::LockedInfoOverlayTextColor), prefs.getColor(Preferences::LockedInfoOverlayBackgroundColor));
            m_lockedEntityRenderer->setBoundsColor(prefs.getColor(Preferences::LockedEntityBoundsColor));
            m_lockedEntityRenderer->setTintColor(prefs.getColor(Preferences::LockedEntityColor));
            m_lockedEntityRenderer->setGrayscale(true);
            
            m_entityDecorators.push_back(new EntityRotationDecorator(document, prefs.getColor(Preferences::EntityRotationDecoratorFillColor), prefs.getColor(Preferences::EntityRotationDecoratorOutlineColor)));
				// TODO : give own color preferences
            m_entityDecorators.push_back(new EntityLinkDecorator(document, prefs.getColor(Preferences::EntityRotationDecoratorFillColor)));
        }
        
        MapRenderer::~MapRenderer() {
            Utility::deleteAll(m_entityDecorators);
            removePointTrace();
            
            delete m_lockedEntityRenderer;
            m_lockedEntityRenderer = NULL;
            delete m_selectedEntityRenderer;
            m_selectedEntityRenderer = NULL;
            delete m_entityRenderer;
            m_entityRenderer = NULL;
            delete m_entityVbo;
            m_entityVbo = NULL;
            delete m_lockedEdgeRenderer;
            m_lockedEdgeRenderer = NULL;
            delete m_selectedEdgeRenderer;
            m_selectedEdgeRenderer = NULL;
            delete m_edgeRenderer;
            m_edgeRenderer = NULL;
            delete m_edgeVbo;
            m_edgeVbo = NULL;
            delete m_lockedFaceRenderer;
            m_lockedFaceRenderer = NULL;
            delete m_selectedFaceRenderer;
            m_selectedFaceRenderer = NULL;
            delete m_faceRenderer;
            m_faceRenderer = NULL;
            delete m_faceVbo;
            m_faceVbo = NULL;
            delete m_utilityVbo;
            m_utilityVbo = NULL;
        }

        void MapRenderer::update(const Controller::Command& command) {
            switch (command.type()) {
                case Controller::Command::LoadMap: {
                    clear();
                    m_entityRenderer->addEntities(m_document.map().entities());
                    break;
                }
                case Controller::Command::ClearMap: {
                    clear();
                    break;
                }
                case Controller::Command::ChangeEditState: {
                    const Controller::ChangeEditStateCommand& changeEditStateCommand = static_cast<const Controller::ChangeEditStateCommand&>(command);
                    changeEditState(changeEditStateCommand.changeSet());
                    invalidateDecorators();
                    break;
                }
                case Controller::Command::ViewFilterChange: {
                    invalidateEntities();
                    invalidateBrushes();
                    break;
                }
                case Controller::Command::PreferenceChange: {
                    const Controller::PreferenceChangeEvent& preferenceChangeEvent = static_cast<const Controller::PreferenceChangeEvent&>(command);
                    if (preferenceChangeEvent.isPreferenceChanged(Preferences::QuakePath))
                        invalidateEntityModelRendererCache();
                    break;
                }
                case Controller::Command::SetFaceAttributes:
                case Controller::Command::MoveTextures:
                case Controller::Command::RotateTextures: {
                    invalidateSelectedBrushes();
                    break;
                }
                case Controller::Command::RemoveTextureCollection:
                case Controller::Command::MoveTextureCollectionUp:
                case Controller::Command::MoveTextureCollectionDown:
                case Controller::Command::AddTextureCollection: {
                    invalidateAll();
                    break;
                }
                case Controller::Command::SetEntityPropertyKey:
                case Controller::Command::SetEntityPropertyValue:
                case Controller::Command::RemoveEntityProperty: {
                    invalidateEntities();
                    invalidateSelectedEntityModelRendererCache();
                    break;
                }
                case Controller::Command::AddObjects: {
                    const Controller::AddObjectsCommand& addObjectsCommand = static_cast<const Controller::AddObjectsCommand&>(command);
                    if (addObjectsCommand.state() == Controller::Command::Doing)
                        m_entityRenderer->addEntities(addObjectsCommand.addedEntities());
                    else
                        m_entityRenderer->removeEntities(addObjectsCommand.addedEntities());
                    if (addObjectsCommand.hasAddedBrushes())
                        invalidateBrushes();
                    break;
                }
                case Controller::Command::RebuildBrushGeometry:
                case Controller::Command::MoveVertices:
                case Controller::Command::SnapVertices:
                case Controller::Command::TransformObjects:
                case Controller::Command::ResizeBrushes: {
                    invalidateSelectedBrushes();
                    invalidateSelectedEntities();
                    break;
                }
                case Controller::Command::RemoveObjects: {
                    const Controller::RemoveObjectsCommand& removeObjectsCommand = static_cast<const Controller::RemoveObjectsCommand&>(command);
                    if (removeObjectsCommand.state() == Controller::Command::Doing)
                        m_entityRenderer->removeEntities(removeObjectsCommand.removedEntities());
                    else
                        m_entityRenderer->addEntities(removeObjectsCommand.removedEntities());
                    if (!removeObjectsCommand.removedBrushes().empty())
                        invalidateBrushes();
                    break;
                }
                case Controller::Command::ReparentBrushes: {
                    invalidateSelectedBrushes();
                    invalidateEntities();
                    invalidateSelectedEntities();
                    break;
                }
                case Controller::Command::SetMod:
                case Controller::Command::SetEntityDefinitionFile: {
                    invalidateEntityModelRendererCache();
                    invalidateAll();
                }
                default:
                    break;
            }
        }

        void MapRenderer::setPointTrace(const Vec3f::List& points) {
            removePointTrace();
            m_pointTraceRenderer = new PointTraceRenderer(points);
            m_pointTraceRenderer->setColor(Color(1.0f, 1.0f, 0.0f, 1.0f));
        }
        
        void MapRenderer::removePointTrace() {
            delete m_pointTraceRenderer;
            m_pointTraceRenderer = NULL;
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
            
            if (context.viewOptions().showBrushes())
                renderEdges(context);
            
            if (context.viewOptions().showEntities()) {
                m_entityRenderer->render(context);
                if (context.viewOptions().renderSelection())
                    m_selectedEntityRenderer->render(context);
                m_lockedEntityRenderer->render(context);
                renderDecorators(context);
            }
            
            if (m_pointTraceRenderer != NULL)
                m_pointTraceRenderer->render(*m_utilityVbo, context);
            
            m_rendering = false;
        }
    }
}
