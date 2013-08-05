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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "MapRenderer.h"

#include "Color.h"
#include "Preferences.h"
#include "Controller/NewDocumentCommand.h"
#include "Controller/OpenDocumentCommand.h"
#include "Controller/SelectionCommand.h"
#include "GL/GL.h"
#include "Model/Brush.h"
#include "Model/BrushEdge.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceGeometry.h"
#include "Model/Filter.h"
#include "Model/Map.h"
#include "Model/ModelUtils.h"
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
            const Model::Filter& m_filter;
        public:
            UnselectedBrushRendererFilter(const Model::Filter& filter) :
            m_filter(filter) {}
            
            bool operator()(const Model::Brush* brush) const {
                return !brush->selected() && m_filter.visible(brush);
            }
            
            bool operator()(const Model::Brush* brush, const Model::BrushFace* face) const {
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
            const Model::Filter& m_filter;
        public:
            SelectedBrushRendererFilter(const Model::Filter& filter) :
            m_filter(filter) {}
            
            bool operator()(const Model::Brush* brush) const {
                return brush->selected() && m_filter.visible(brush);
            }
            
            bool operator()(const Model::Brush* brush, const Model::BrushFace* face) const {
                return (brush->selected() || face->selected()) && m_filter.visible(face);
            }
            
            bool operator()(const Model::BrushEdge* edge) const {
                const Model::BrushFace* left = edge->left()->face();
                const Model::BrushFace* right = edge->right()->face();
                const Model::Brush* brush = left->parent();
                return (brush->selected() ||
                        left->selected() || right->selected());
            }
        };
        
        MapRenderer::MapRenderer(FontManager& fontManager, const Model::Filter& filter) :
        m_fontManager(fontManager),
        m_filter(filter),
        m_auxVbo(0xFFFFF),
        m_unselectedBrushRenderer(UnselectedBrushRendererFilter(m_filter)),
        m_selectedBrushRenderer(SelectedBrushRendererFilter(m_filter)),
        m_unselectedEntityRenderer(m_fontManager, m_filter),
        m_selectedEntityRenderer(m_fontManager, m_filter) {}
        
        void MapRenderer::render(RenderContext& context) {
            setupGL(context);
            
            clearBackground(context);
            renderCoordinateSystem(context);
            renderGeometry(context);
            renderEntities(context);
        }

        void MapRenderer::commandDone(Controller::Command::Ptr command) {
            if (command->type() == Controller::NewDocumentCommand::Type) {
                clearState();
                Controller::NewDocumentCommand::Ptr newDocumentCommand = Controller::Command::cast<Controller::NewDocumentCommand>(command);
                Model::Map* map = newDocumentCommand->map();
                loadMap(*map);
            } else if (command->type() == Controller::OpenDocumentCommand::Type) {
                clearState();
                Controller::OpenDocumentCommand::Ptr openDocumentCommand = Controller::Command::cast<Controller::OpenDocumentCommand>(command);
                Model::Map* map = openDocumentCommand->map();
                loadMap(*map);
            } else if (command->type() == Controller::SelectionCommand::Type) {
                updateSelection(command);
            }
        }
        
        void MapRenderer::commandUndone(Controller::Command::Ptr command) {
            if (command->type() == Controller::SelectionCommand::Type) {
                updateSelection(command);
            }
        }

        void MapRenderer::setupGL(RenderContext& context) {
            const Renderer::Camera::Viewport& viewport = context.camera().viewport();
            glViewport(viewport.x, viewport.y, viewport.width, viewport.height);
            
            glDisableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_COLOR_ARRAY);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            glBindTexture(GL_TEXTURE_2D, 0);
            glDisable(GL_TEXTURE_2D);
            
            glEnable(GL_MULTISAMPLE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glFrontFace(GL_CW);
            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
            glShadeModel(GL_SMOOTH);
            // glResetEdgeOffset();
        }

        void MapRenderer::clearBackground(RenderContext& context) {
            PreferenceManager& prefs = PreferenceManager::instance();
            const Color& backgroundColor = prefs.getColor(Preferences::BackgroundColor);
            glClearColor(backgroundColor.r(), backgroundColor.g(), backgroundColor.b(), backgroundColor.a());
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }
        
        void MapRenderer::renderCoordinateSystem(RenderContext& context) {
            PreferenceManager& prefs = PreferenceManager::instance();
            const Color& xAxisColor = prefs.getColor(Preferences::XAxisColor);
            const Color& yAxisColor = prefs.getColor(Preferences::YAxisColor);
            const Color& zAxisColor = prefs.getColor(Preferences::ZAxisColor);

            typedef VertexSpecs::P3C4::Vertex Vertex;
            Vertex::List vertices;
            
            vertices.push_back(Vertex(Vec3f(-128.0f, 0.0f, 0.0f), xAxisColor));
            vertices.push_back(Vertex(Vec3f( 128.0f, 0.0f, 0.0f), xAxisColor));
            vertices.push_back(Vertex(Vec3f(0.0f, -128.0f, 0.0f), yAxisColor));
            vertices.push_back(Vertex(Vec3f(0.0f,  128.0f, 0.0f), yAxisColor));
            vertices.push_back(Vertex(Vec3f(0.0f, 0.0f, -128.0f), zAxisColor));
            vertices.push_back(Vertex(Vec3f(0.0f, 0.0f,  128.0f), zAxisColor));
            
            VertexArray array(m_auxVbo, GL_LINES, vertices);
            
            SetVboState setVboState(m_auxVbo);
            setVboState.active();
            array.render();
        }

        void MapRenderer::renderGeometry(RenderContext& context) {
            PreferenceManager& prefs = PreferenceManager::instance();

            m_unselectedBrushRenderer.setFaceColor(prefs.getColor(Preferences::FaceColor));
            m_unselectedBrushRenderer.setEdgeColor(prefs.getColor(Preferences::EdgeColor));
            
            m_selectedBrushRenderer.setFaceColor(prefs.getColor(Preferences::FaceColor));
            m_selectedBrushRenderer.setEdgeColor(prefs.getColor(Preferences::SelectedEdgeColor));
            m_selectedBrushRenderer.setTintFaces(true);
            m_selectedBrushRenderer.setTintColor(prefs.getColor(Preferences::SelectedFaceColor));
            m_selectedBrushRenderer.setRenderOccludedEdges(true);
            m_selectedBrushRenderer.setOccludedEdgeColor(prefs.getColor(Preferences::OccludedSelectedEdgeColor));

            m_unselectedBrushRenderer.render(context);
            m_selectedBrushRenderer.render(context);
        }

        void MapRenderer::renderEntities(RenderContext& context) {
            PreferenceManager& prefs = PreferenceManager::instance();
            
            m_unselectedEntityRenderer.setOverlayTextColor(prefs.getColor(Preferences::InfoOverlayTextColor));
            m_unselectedEntityRenderer.setOverlayBackgroundColor(prefs.getColor(Preferences::InfoOverlayBackgroundColor));
            
            m_selectedEntityRenderer.setOverlayTextColor(prefs.getColor(Preferences::SelectedInfoOverlayTextColor));
            m_selectedEntityRenderer.setOverlayBackgroundColor(prefs.getColor(Preferences::SelectedInfoOverlayBackgroundColor));
            m_selectedEntityRenderer.setOverrideBoundsColor(true);
            m_selectedEntityRenderer.setBoundsColor(prefs.getColor(Preferences::SelectedEdgeColor));
            m_selectedEntityRenderer.setRenderOccludedBounds(true);
            m_selectedEntityRenderer.setOccludedBoundsColor(prefs.getColor(Preferences::OccludedSelectedEdgeColor));
            
            m_unselectedEntityRenderer.render(context);
            m_selectedEntityRenderer.render(context);
        }

        void MapRenderer::clearState() {
            m_unselectedBrushRenderer.clear();
            m_selectedBrushRenderer.clear();
            m_unselectedEntityRenderer.clear();
            m_selectedEntityRenderer.clear();
        }

        void MapRenderer::loadMap(Model::Map& map) {
            m_unselectedBrushRenderer.setBrushes(map.brushes());
            m_unselectedEntityRenderer.addEntities(map.entities());
        }

        void MapRenderer::updateSelection(Controller::Command::Ptr command) {
            Controller::SelectionCommand::Ptr selectionCommand = Controller::Command::cast<Controller::SelectionCommand>(command);
            View::MapDocumentPtr document = selectionCommand->document();
            m_unselectedBrushRenderer.setBrushes(document->unselectedBrushes());
            m_selectedBrushRenderer.setBrushes(document->selectedBrushes());
            
            const Model::SelectionResult& result = selectionCommand->lastResult();
            m_unselectedEntityRenderer.removeEntities(Model::extractEntities<Model::EntitySet>(result.selectedObjects()));
            m_unselectedEntityRenderer.addEntities(Model::extractEntities<Model::EntitySet>(result.deselectedObjects()));
            m_selectedEntityRenderer.removeEntities(Model::extractEntities<Model::EntitySet>(result.deselectedObjects()));
            m_selectedEntityRenderer.addEntities(Model::extractEntities<Model::EntitySet>(result.selectedObjects()));
        }
    }
}
