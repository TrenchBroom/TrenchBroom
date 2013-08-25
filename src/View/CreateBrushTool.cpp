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

#include "CreateBrushTool.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Model/Brush.h"
#include "Model/HitFilters.h"
#include "Model/Map.h"
#include "Model/ModelUtils.h"
#include "Model/Picker.h"
#include "Renderer/Camera.h"
#include "View/Grid.h"
#include "View/InputState.h"
#include "View/MapDocument.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        struct RendererFilter : public Renderer::BrushRenderer::Filter {
            bool operator()(const Model::Brush* brush) const { return true; }
            bool operator()(const Model::Brush* brush, const Model::BrushFace* face) const { return true; }
            bool operator()(const Model::BrushEdge* edge) const { return true; }
        };
        
        CreateBrushTool::CreateBrushTool(BaseTool* next, MapDocumentPtr document) :
        Tool(next),
        m_document(document),
        m_brushRenderer(RendererFilter()),
        m_brush(NULL) {}

        bool CreateBrushTool::doStartPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
            assert(m_brush == NULL);
            
            if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                return false;
            if (m_document->hasSelectedObjects())
                return false;
            
            Model::HitFilterChain hitFilter = Model::chainHitFilters(Model::TypedHitFilter(Model::Brush::BrushHit), Model::DefaultHitFilter(inputState.filter()));
            const Model::PickResult::FirstHit first = inputState.pickResult().firstHit(hitFilter, true);
            if (first.matches)
                initialPoint = first.hit.hitPoint();
            else
                initialPoint = inputState.camera().defaultPoint(inputState.pickRay().direction);

            plane = Plane3(initialPoint, Vec3::PosZ);
            m_initialPoint = initialPoint;
            
            const BBox3 bounds = computeBounds(m_initialPoint, m_initialPoint);
            m_brush = createBrush(bounds);
            updateBrushRenderer();

            return true;
        }
        
        void CreateBrushTool::doResetPlane(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
        }
        
        bool CreateBrushTool::doPlaneDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint) {
            assert(m_brush != NULL);
            delete m_brush;
            
            const BBox3 bounds = computeBounds(m_initialPoint, curPoint);
            m_brush = createBrush(bounds);
            updateBrushRenderer();
            
            return true;
        }
        
        void CreateBrushTool::doEndPlaneDrag(const InputState& inputState) {
            assert(m_brush != NULL);
            delete m_brush;
            m_brush = NULL;
            updateBrushRenderer();
        }
        
        void CreateBrushTool::doCancelPlaneDrag(const InputState& inputState) {
            assert(m_brush != NULL);
            delete m_brush;
            m_brush = NULL;
            updateBrushRenderer();
        }

        void CreateBrushTool::doRender(const InputState& inputState, Renderer::RenderContext& renderContext) {
            PreferenceManager& prefs = PreferenceManager::instance();
            
            m_brushRenderer.setFaceColor(prefs.getColor(Preferences::FaceColor));
            m_brushRenderer.setEdgeColor(prefs.getColor(Preferences::SelectedEdgeColor));
            m_brushRenderer.setTintFaces(true);
            m_brushRenderer.setTintColor(prefs.getColor(Preferences::SelectedFaceColor));
            m_brushRenderer.setRenderOccludedEdges(true);
            m_brushRenderer.setOccludedEdgeColor(prefs.getColor(Preferences::OccludedSelectedEdgeColor));
            
            m_brushRenderer.render(renderContext);
        }

        BBox3 CreateBrushTool::computeBounds(const Vec3& point1, const Vec3& point2) const {
            BBox3 bounds;
            for (size_t i = 0; i < 3; i++) {
                bounds.min[i] = std::min(point1[i], point2[i]);
                bounds.max[i] = std::max(point1[i], point2[i]);
            }
            
            View::Grid& grid = m_document->grid();
            bounds.min = grid.snapDown(bounds.min);
            bounds.max = grid.snapUp(bounds.max);
            
            for (size_t i = 0; i < 3; i++)
                if (bounds.max[i] <= bounds.min[i])
                    bounds.max[i] = bounds.min[i] + grid.actualSize();
            return bounds;
        }

        Model::Brush* CreateBrushTool::createBrush(const BBox3& bounds) const {
            Model::Map& map = *m_document->map();
            const BBox3& worldBounds = m_document->worldBounds();
            const String textureName = m_document->currentTextureName();
            return Model::createBrushFromBounds(map, worldBounds, bounds, textureName);
        }

        void CreateBrushTool::updateBrushRenderer() {
            Model::BrushList brushes;
            if (m_brush != NULL)
                brushes.push_back(m_brush);
            m_brushRenderer.setBrushes(brushes);
        }
    }
}
