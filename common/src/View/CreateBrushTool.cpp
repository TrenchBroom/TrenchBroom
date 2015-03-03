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

#include "CreateBrushTool.h"
#include "Polyhedron.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/Layer.h"
#include "Model/World.h"
#include "Renderer/RenderService.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace View {
        CreateBrushTool::CreateBrushTool(MapDocumentWPtr document) :
        Tool(false),
        m_document(document) {}

        void CreateBrushTool::createBrush(const Polyhedron3& polyhedron) {
            if (polyhedron.closed()) {
                MapDocumentSPtr document = lock(m_document);
                const Model::BrushBuilder builder(document->world(), document->worldBounds());
                Model::Brush* brush = builder.createBrush(polyhedron, document->currentTextureName());
                
                const Transaction transaction(document, "Create brush");
                document->deselectAll();
                document->addNode(brush, document->currentLayer());
            }
        }

        void CreateBrushTool::render(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Polyhedron3& polyhedron) {
            if (!polyhedron.empty()) {
                Renderer::RenderService renderService(renderContext, renderBatch);
                renderService.setForegroundColor(pref(Preferences::HandleColor));
                renderService.setLineWidth(2.0f);
                
                const Polyhedron3::EdgeList& edges = polyhedron.edges();
                Polyhedron3::EdgeList::const_iterator eIt, eEnd;
                for (eIt = edges.begin(), eEnd = edges.end(); eIt != eEnd; ++eIt) {
                    const Polyhedron3::Edge* edge = *eIt;
                    renderService.renderLine(edge->firstVertex()->position(), edge->secondVertex()->position());
                }
                
                const Polyhedron3::VertexList& vertices = polyhedron.vertices();
                Polyhedron3::VertexList::const_iterator vIt, vEnd;
                for (vIt = vertices.begin(), vEnd = vertices.end(); vIt != vEnd; ++vIt) {
                    const Polyhedron3::Vertex* vertex = *vIt;
                    renderService.renderPointHandle(vertex->position());
                }
            }
        }

        bool CreateBrushTool::doActivate() {
            return true;
        }
    }
}
