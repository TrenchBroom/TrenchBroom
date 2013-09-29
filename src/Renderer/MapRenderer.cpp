/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "MapRenderer.h"

#include "Color.h"
#include "Preferences.h"
#include "CastIterator.h"
#include "FilterIterator.h"
#include "Controller/AddRemoveObjectsCommand.h"
#include "Controller/EntityPropertyCommand.h"
#include "Controller/FaceAttributeCommand.h"
#include "Controller/NewDocumentCommand.h"
#include "Controller/OpenDocumentCommand.h"
#include "Controller/ResizeBrushesCommand.h"
#include "Controller/SelectionCommand.h"
#include "GL/GL.h"
#include "Model/Brush.h"
#include "Model/BrushEdge.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceGeometry.h"
#include "Model/ModelFilter.h"
#include "Model/Map.h"
#include "Model/ModelUtils.h"
#include "Model/Object.h"
#include "Model/SelectionResult.h"
#include "Renderer/Camera.h"
#include "Renderer/Mesh.h"
#include "Renderer/RenderContext.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/VertexSpec.h"
#include "Renderer/VertexArray.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace Renderer {
        struct UnselectedBrushRendererFilter : public BrushRenderer::Filter {
        private:
            const Model::ModelFilter& m_filter;
        public:
            UnselectedBrushRendererFilter(const Model::ModelFilter& filter) :
            m_filter(filter) {}
            
            bool operator()(const Model::Brush* brush) const {
                return !brush->selected() && m_filter.visible(brush);
            }
            
            bool operator()(const Model::BrushFace* face) const {
                return !face->selected() && m_filter.visible(face);
            }
            
            bool operator()(const Model::BrushEdge* edge) const {
                const Model::BrushFace* left = edge->left()->face();
                const Model::BrushFace* right = edge->right()->face();
                const Model::Brush* brush = left->parent();
                return (!brush->selected() ||
                        (!left->selected() && !right->selected()));
            }
        };
        
        struct SelectedBrushRendererFilter : public BrushRenderer::Filter {
            const Model::ModelFilter& m_filter;
        public:
            SelectedBrushRendererFilter(const Model::ModelFilter& filter) :
            m_filter(filter) {}
            
            bool operator()(const Model::Brush* brush) const {
                return (brush->selected() || brush->partiallySelected()) && m_filter.visible(brush);
            }
            
            bool operator()(const Model::BrushFace* face) const {
                return (face->parent()->selected() || face->selected()) && m_filter.visible(face);
            }
            
            bool operator()(const Model::BrushEdge* edge) const {
                const Model::BrushFace* left = edge->left()->face();
                const Model::BrushFace* right = edge->right()->face();
                const Model::Brush* brush = left->parent();
                return (brush->selected() ||
                        left->selected() || right->selected());
            }
        };
        
        MapRenderer::MapRenderer(View::MapDocumentPtr document, FontManager& fontManager) :
        m_document(document),
        m_fontManager(fontManager),
        m_unselectedBrushRenderer(UnselectedBrushRendererFilter(m_document->filter())),
        m_selectedBrushRenderer(SelectedBrushRendererFilter(m_document->filter())),
        m_unselectedEntityRenderer(m_fontManager, m_document->filter()),
        m_selectedEntityRenderer(m_fontManager, m_document->filter()) {}
        
        void MapRenderer::render(RenderContext& context) {
            setupGL(context);
            
            renderGeometry(context);
            renderEntities(context);
        }
        
        void MapRenderer::commandDone(Controller::Command::Ptr command) {
            using namespace Controller;
            if (command->type() == NewDocumentCommand::Type) {
                clearState();
                loadMap(*m_document->map());
            } else if (command->type() == OpenDocumentCommand::Type) {
                clearState();
                loadMap(*m_document->map());
            } else if (command->type() == SelectionCommand::Type) {
                updateSelection(command);
            } else if (command->type() == AddRemoveObjectsCommand::Type) {
                addRemoveObjects(command);
            } else if (command->type() == EntityPropertyCommand::Type) {
                updateEntities(command);
            } else if (command->type() == FaceAttributeCommand::Type) {
                updateBrushes(command);
            } else if (command->type() == ResizeBrushesCommand::Type) {
                updateBrushes(command);
            }
        }
        
        void MapRenderer::commandUndone(Controller::Command::Ptr command) {
            using namespace Controller;
            if (command->type() == SelectionCommand::Type) {
                updateSelection(command);
            } else if (command->type() == AddRemoveObjectsCommand::Type) {
                addRemoveObjects(command);
            } else if (command->type() == EntityPropertyCommand::Type) {
                updateEntities(command);
            } else if (command->type() == ResizeBrushesCommand::Type) {
                updateBrushes(command);
            }
        }

        void MapRenderer::setupGL(RenderContext& context) {
            glDisableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_COLOR_ARRAY);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            glBindTexture(GL_TEXTURE_2D, 0);
            glDisable(GL_TEXTURE_2D);
            glFrontFace(GL_CW);
            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
            glResetEdgeOffset();
        }

        void MapRenderer::renderGeometry(RenderContext& context) {
            PreferenceManager& prefs = PreferenceManager::instance();

            m_unselectedBrushRenderer.setFaceColor(prefs.getColor(Preferences::FaceColor));
            m_unselectedBrushRenderer.setEdgeColor(prefs.getColor(Preferences::EdgeColor));
            m_unselectedBrushRenderer.render(context);
            
            if (!context.hideSelection()) {
                m_selectedBrushRenderer.setFaceColor(prefs.getColor(Preferences::FaceColor));
                m_selectedBrushRenderer.setEdgeColor(prefs.getColor(Preferences::SelectedEdgeColor));
                m_selectedBrushRenderer.setTintFaces(true);
                m_selectedBrushRenderer.setTintColor(prefs.getColor(Preferences::SelectedFaceColor));
                m_selectedBrushRenderer.setRenderOccludedEdges(true);
                m_selectedBrushRenderer.setOccludedEdgeColor(prefs.getColor(Preferences::OccludedSelectedEdgeColor));
                m_selectedBrushRenderer.render(context);
            }
        }

        void MapRenderer::renderEntities(RenderContext& context) {
            PreferenceManager& prefs = PreferenceManager::instance();
            
            m_unselectedEntityRenderer.setOverlayTextColor(prefs.getColor(Preferences::InfoOverlayTextColor));
            m_unselectedEntityRenderer.setOverlayBackgroundColor(prefs.getColor(Preferences::InfoOverlayBackgroundColor));
            m_unselectedEntityRenderer.setApplyTinting(false);
            m_unselectedEntityRenderer.render(context);
            
            if (!context.hideSelection()) {
                m_selectedEntityRenderer.setOverlayTextColor(prefs.getColor(Preferences::SelectedInfoOverlayTextColor));
                m_selectedEntityRenderer.setOverlayBackgroundColor(prefs.getColor(Preferences::SelectedInfoOverlayBackgroundColor));
                m_selectedEntityRenderer.setOverrideBoundsColor(true);
                m_selectedEntityRenderer.setBoundsColor(prefs.getColor(Preferences::SelectedEdgeColor));
                m_selectedEntityRenderer.setRenderOccludedBounds(true);
                m_selectedEntityRenderer.setOccludedBoundsColor(prefs.getColor(Preferences::OccludedSelectedEdgeColor));
                m_selectedEntityRenderer.setApplyTinting(true);
                m_selectedEntityRenderer.setTintColor(prefs.getColor(Preferences::SelectedFaceColor));
                m_selectedEntityRenderer.render(context);
            }
        }

        void MapRenderer::clearState() {
            m_unselectedBrushRenderer.clear();
            m_selectedBrushRenderer.clear();
            m_unselectedEntityRenderer.clear();
            m_selectedEntityRenderer.clear();
        }

        void MapRenderer::loadMap(Model::Map& map) {
            m_unselectedBrushRenderer.setBrushes(map.brushes());
            
            m_unselectedEntityRenderer.addEntities(map.entities().begin(),
                                                   map.entities().end());
        }

        void MapRenderer::updateSelection(Controller::Command::Ptr command) {
            using namespace Controller;
            SelectionCommand::Ptr selectionCommand = Command::cast<SelectionCommand>(command);

            m_unselectedBrushRenderer.setBrushes(m_document->unselectedBrushes());
            m_selectedBrushRenderer.setBrushes(m_document->allSelectedBrushes());
            
            const Model::SelectionResult& result = selectionCommand->lastResult();
            m_unselectedEntityRenderer.removeEntities(Model::entityIterator(result.selectedObjects().begin(), result.selectedObjects().end()),
                                                      Model::entityIterator(result.selectedObjects().end()));
            m_unselectedEntityRenderer.addEntities(Model::entityIterator(result.deselectedObjects().begin(), result.deselectedObjects().end()),
                                                   Model::entityIterator(result.deselectedObjects().end()));
            m_selectedEntityRenderer.removeEntities(Model::entityIterator(result.deselectedObjects().begin(), result.deselectedObjects().end()),
                                                    Model::entityIterator(result.deselectedObjects().end()));
            m_selectedEntityRenderer.addEntities(Model::entityIterator(result.selectedObjects().begin(), result.selectedObjects().end()),
                                                 Model::entityIterator(result.selectedObjects().end()));
        }
        
        void MapRenderer::addRemoveObjects(Controller::Command::Ptr command) {
            using namespace Controller;
            AddRemoveObjectsCommand::Ptr addRemoveCommand = Command::cast<AddRemoveObjectsCommand>(command);

            const Model::ObjectList& addObjects = addRemoveCommand->addedObjects();
            m_unselectedBrushRenderer.addBrushes(Model::brushIterator(addObjects.begin(), addObjects.end()),
                                                 Model::brushIterator(addObjects.end()));

            const Model::ObjectList& removedObjects = addRemoveCommand->removedObjects();
            m_unselectedBrushRenderer.removeBrushes(Model::brushIterator(removedObjects.begin(), removedObjects.end()),
                                                    Model::brushIterator(removedObjects.end()));
        }

        void MapRenderer::updateEntities(Controller::Command::Ptr command) {
            m_selectedEntityRenderer.updateEntities(Model::entityIterator(command->affectedEntities().begin(), command->affectedEntities().end()),
                                                    Model::entityIterator(command->affectedEntities().end()));
        }

        void MapRenderer::updateBrushes(Controller::Command::Ptr command) {
            m_selectedBrushRenderer.setBrushes(m_document->allSelectedBrushes());
        }
    }
}
