/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "MiniMapRenderer.h"

#include "Preferences.h"
#include "Model/Brush.h"
#include "Model/BrushEdge.h"
#include "Model/BrushVertex.h"
#include "Model/ModelUtils.h"
#include "Renderer/Camera.h"
#include "Renderer/GL.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/ShaderManager.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace Renderer {
        void MiniMapRenderer::BuildBrushEdges::operator()(Model::Brush* brush) {
            const Model::BrushEdgeList edges = brush->edges();
            Model::BrushEdgeList::const_iterator it, end;
            for (it = edges.begin(), end = edges.end(); it != end; ++it) {
                const Model::BrushEdge* edge = *it;
                vertices.push_back(VertexSpecs::P3::Vertex(edge->start->position));
                vertices.push_back(VertexSpecs::P3::Vertex(edge->end->position));
            }
        }
        
        MiniMapRenderer::MiniMapRenderer(View::MapDocumentWPtr document) :
        m_document(document),
        m_layerObserver(m_document),
        m_vbo(0xFFFF),
        m_unselectedValid(false),
        m_selectedValid(false) {
            bindObservers();
        }
        
        MiniMapRenderer::~MiniMapRenderer() {
            unbindObservers();
        }
        
        void MiniMapRenderer::render(RenderContext& context, const BBox3f& bounds) {
            SetVboState setVboState(m_vbo);
            setVboState.active();

            setupGL(context);
            renderEdges(context, bounds);
        }
        
        void MiniMapRenderer::setupGL(RenderContext& context) {
            glDisableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_COLOR_ARRAY);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            glBindTexture(GL_TEXTURE_2D, 0);
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_DEPTH_TEST);
            glFrontFace(GL_CW);
            glDisable(GL_CULL_FACE);
            glResetEdgeOffset();
        }
        
        void MiniMapRenderer::renderEdges(RenderContext& context, const BBox3f& bounds) {
            validateEdges(context);
            
            ActiveShader shader(context.shaderManager(), Shaders::MiniMapEdgeShader);
            shader.set("BoundsMin", bounds.min);
            shader.set("BoundsMax", bounds.max);
            
            PreferenceManager& prefs = PreferenceManager::instance();
            shader.set("Color", prefs.get(Preferences::EdgeColor));
            m_unselectedEdgeArray.render();

            shader.set("Color", prefs.get(Preferences::SelectedEdgeColor));
            m_selectedEdgeArray.render();
        }

        void MiniMapRenderer::validateEdges(RenderContext& context) {
            if (!m_unselectedValid || !m_selectedValid) {
                assert(!expired(m_document));
                View::MapDocumentSPtr document = lock(m_document);
                
                SetVboState mapVbo(m_vbo);
                mapVbo.mapped();

                if (!m_unselectedValid) {
                    m_unselectedEdgeArray = buildVertexArray(document->unselectedBrushes());
                    m_unselectedEdgeArray.prepare(m_vbo);
                    m_unselectedValid = true;
                }
                if (!m_selectedValid) {
                    m_selectedEdgeArray = buildVertexArray(document->selectedBrushes());
                    m_selectedEdgeArray.prepare(m_vbo);
                    m_selectedValid = true;
                }
            }
        }
        
        VertexArray MiniMapRenderer::buildVertexArray(const Model::BrushList& brushes) const {
            const Model::ModelFilter& filter = lock(m_document)->filter();
            BuildBrushEdges buildEdges;
            Model::each(brushes.begin(), brushes.end(), buildEdges, Model::MatchVisibleObjects(filter));
            return VertexArray::swap(GL_LINES, buildEdges.vertices);
        }

        void MiniMapRenderer::bindObservers() {
            View::MapDocumentSPtr document = lock(m_document);
            document->documentWasClearedNotifier.addObserver(this, &MiniMapRenderer::documentWasCleared);
            document->documentWasNewedNotifier.addObserver(this, &MiniMapRenderer::documentWasNewedOrLoaded);
            document->documentWasLoadedNotifier.addObserver(this, &MiniMapRenderer::documentWasNewedOrLoaded);
            document->objectsWereAddedNotifier.addObserver(this, &MiniMapRenderer::objectsWereAdded);
            document->objectsWillBeRemovedNotifier.addObserver(this, &MiniMapRenderer::objectsWillBeRemoved);
            document->objectsDidChangeNotifier.addObserver(this, &MiniMapRenderer::objectsDidChange);
            document->modelFilterDidChangeNotifier.addObserver(this, &MiniMapRenderer::filterDidChange);
            document->selectionDidChangeNotifier.addObserver(this, &MiniMapRenderer::selectionDidChange);
            
            m_layerObserver.layerDidChangeNotifier.addObserver(this, &MiniMapRenderer::layerDidChange);
        }
        
        void MiniMapRenderer::unbindObservers() {
            if (!expired(m_document)) {
                View::MapDocumentSPtr document = lock(m_document);
                document->documentWasClearedNotifier.removeObserver(this, &MiniMapRenderer::documentWasCleared);
                document->documentWasNewedNotifier.removeObserver(this, &MiniMapRenderer::documentWasNewedOrLoaded);
                document->documentWasLoadedNotifier.removeObserver(this, &MiniMapRenderer::documentWasNewedOrLoaded);
                document->objectsWereAddedNotifier.removeObserver(this, &MiniMapRenderer::objectsWereAdded);
                document->objectsWillBeRemovedNotifier.removeObserver(this, &MiniMapRenderer::objectsWillBeRemoved);
                document->objectsDidChangeNotifier.removeObserver(this, &MiniMapRenderer::objectsDidChange);
                document->modelFilterDidChangeNotifier.removeObserver(this, &MiniMapRenderer::filterDidChange);
                document->selectionDidChangeNotifier.removeObserver(this, &MiniMapRenderer::selectionDidChange);
            }

            m_layerObserver.layerDidChangeNotifier.removeObserver(this, &MiniMapRenderer::layerDidChange);
        }
        
        void MiniMapRenderer::documentWasCleared() {
            m_unselectedValid = false;
            m_selectedValid = false;
        }
        
        void MiniMapRenderer::documentWasNewedOrLoaded() {
            m_unselectedValid = false;
            m_selectedValid = false;
        }
        
        void MiniMapRenderer::objectsWereAdded(const Model::ObjectList& objects) {
            m_unselectedValid = false;
        }
        
        void MiniMapRenderer::objectsWillBeRemoved(const Model::ObjectList& objects) {
            m_unselectedValid = false;
        }
        
        void MiniMapRenderer::objectsDidChange(const Model::ObjectList& objects) {
            m_selectedValid = false;
        }
        
        void MiniMapRenderer::layerDidChange(Model::Layer* layer, const Model::Layer::Attr_Type attr) {
            if ((attr & Model::Layer::Attr_Editing) != 0) {
                m_unselectedValid = false;
                m_selectedValid = false;
            }
        }

        void MiniMapRenderer::filterDidChange() {
            m_unselectedValid = false;
            m_selectedValid = false;
        }

        void MiniMapRenderer::selectionDidChange(const Model::SelectionResult& result) {
            m_unselectedValid = false;
            m_selectedValid = false;
        }
    }
}
