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
#include "Renderer/BrushRenderer.h"
#include "Renderer/RenderService.h"
#include "Renderer/SelectionBoundsRenderer.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace View {
        CreateBrushTool::CreateBrushTool(MapDocumentWPtr document) :
        Tool(false),
        m_document(document),
        m_brush(NULL),
        m_brushRenderer(new Renderer::BrushRenderer(false)) {}

        CreateBrushTool::~CreateBrushTool() {
            delete m_brushRenderer;
            delete m_brush;
        }
        
        void CreateBrushTool::updateBrush(const BBox3& bounds) {
            updateBrush(Polyhedron3(bounds));
        }
        
        void CreateBrushTool::updateBrush(const Polyhedron3& polyhedron) {
            delete m_brush;
            m_brush = NULL;
            
            if (polyhedron.closed()) {
                MapDocumentSPtr document = lock(m_document);
                const Model::BrushBuilder builder(document->world(), document->worldBounds());
                m_brush = builder.createBrush(polyhedron, document->currentTextureName());
            }
        }

        void CreateBrushTool::createBrush() {
            if (m_brush != NULL) {
                MapDocumentSPtr document = lock(m_document);
                const Transaction transaction(document, "Create brush");
                document->deselectAll();
                document->addNode(m_brush, document->currentParent());
                document->select(m_brush);
                m_brush = NULL;
            }
        }

        void CreateBrushTool::cancel() {
            delete m_brush;
            m_brush = NULL;
        }

        void CreateBrushTool::render(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            if (m_brush != NULL)
                renderBrush(renderContext, renderBatch);
            
        }
        
        void CreateBrushTool::renderBrush(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            assert(m_brush != NULL);
            
            m_brushRenderer->setFaceColor(pref(Preferences::FaceColor));
            m_brushRenderer->setEdgeColor(pref(Preferences::SelectedEdgeColor));
            m_brushRenderer->setShowEdges(true);
            m_brushRenderer->setShowOccludedEdges(true);
            m_brushRenderer->setOccludedEdgeColor(pref(Preferences::OccludedSelectedEdgeColor));
            m_brushRenderer->setTint(true);
            m_brushRenderer->setTintColor(pref(Preferences::SelectedFaceColor));
            m_brushRenderer->render(renderContext, renderBatch);
            
            m_brushRenderer->setBrushes(Model::BrushList(1, m_brush));
            m_brushRenderer->render(renderContext, renderBatch);
            
            Renderer::SelectionBoundsRenderer boundsRenderer(m_brush->bounds());
            boundsRenderer.render(renderContext, renderBatch);
        }

        bool CreateBrushTool::doActivate() {
            return true;
        }
        
        String CreateBrushTool::doGetIconName() const {
            return "BrushTool.png";
        }
    }
}
