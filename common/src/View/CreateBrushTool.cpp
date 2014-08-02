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

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Model/Brush.h"
#include "Model/HitAdapter.h"
#include "Model/Map.h"
#include "Model/ModelUtils.h"
#include "Renderer/Camera.h"
#include "View/ControllerFacade.h"
#include "View/Grid.h"
#include "View/InputState.h"
#include "View/MapDocument.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        struct RendererFilter : public Renderer::BrushRenderer::Filter {
            bool operator()(const Model::Brush* brush) const { return true; }
            bool operator()(const Model::BrushFace* face) const { return true; }
            bool operator()(const Model::BrushEdge* edge) const { return true; }
        };
        
        CreateBrushTool::CreateBrushTool(MapDocumentWPtr document, ControllerWPtr controller, const Renderer::Camera& camera, Renderer::TextureFont& font) :
        ToolImpl(document, controller),
        m_camera(camera),
        m_brushRenderer(RendererFilter()),
        m_guideRenderer(document, font),
        m_brush(NULL) {}

        void CreateBrushTool::doModifierKeyChange(const InputState& inputState) {
            if (dragging())
                resetPlane(inputState);
        }

        bool CreateBrushTool::doStartPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
            assert(m_brush == NULL);
            
            if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                return false;
            if (!inputState.checkModifierKeys(ModifierKeys::MKNone, ModifierKeys::MKAlt))
                return false;
            if (document()->hasSelectedObjects())
                return false;
            
            const Hit& hit = Model::findFirstHit(inputState.hits(), Model::Brush::BrushHit, document()->filter(), true);
            if (hit.isMatch())
                initialPoint = hit.hitPoint();
            else
                initialPoint = m_camera.defaultPoint(inputState.pickRay());

            plane = Plane3(initialPoint, Vec3::PosZ);
            m_initialPoint = initialPoint;
            
            const BBox3 bounds = computeBounds(m_initialPoint, m_initialPoint);
            if (!checkBounds(bounds))
                return false;

            m_brush = createBrush(bounds);
            updateBrushRenderer();
            updateGuideRenderer(bounds);

            return true;
        }
        
        void CreateBrushTool::doResetPlane(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
            const FloatType distance = plane.intersectWithRay(inputState.pickRay());
            if (Math::isnan(distance))
                return;
            initialPoint = inputState.pickRay().pointAtDistance(distance);
            
            if (inputState.modifierKeys() == ModifierKeys::MKAlt) {
                Vec3 planeNorm = inputState.pickRay().direction;
                planeNorm[2] = 0.0;
                planeNorm.normalize();
                plane = Plane3(initialPoint, planeNorm);
            } else {
                plane = horizontalDragPlane(initialPoint);
            }
        }
        
        bool CreateBrushTool::doPlaneDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint) {
            assert(m_brush != NULL);
            delete m_brush;
            
            const BBox3 bounds = computeBounds(m_initialPoint, curPoint);
            if (!checkBounds(bounds))
                return false;
            
            m_brush = createBrush(bounds);
            updateBrushRenderer();
            updateGuideRenderer(bounds);

            return true;
        }
        
        void CreateBrushTool::doEndPlaneDrag(const InputState& inputState) {
            assert(m_brush != NULL);
            addBrushToMap(m_brush);
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
            if (dragging()) {
                PreferenceManager& prefs = PreferenceManager::instance();
                
                m_brushRenderer.setFaceColor(prefs.get(Preferences::FaceColor));
                m_brushRenderer.setEdgeColor(prefs.get(Preferences::SelectedEdgeColor));
                m_brushRenderer.setTintFaces(true);
                m_brushRenderer.setTintColor(prefs.get(Preferences::SelectedFaceColor));
                m_brushRenderer.setRenderOccludedEdges(true);
                m_brushRenderer.setOccludedEdgeColor(prefs.get(Preferences::OccludedSelectedEdgeColor));
                
                m_brushRenderer.render(renderContext);
                
                m_guideRenderer.setColor(prefs.get(Preferences::HandleColor));
                m_guideRenderer.render(renderContext);
            }
        }

        BBox3 CreateBrushTool::computeBounds(const Vec3& point1, const Vec3& point2) const {
            BBox3 bounds;
            for (size_t i = 0; i < 3; i++) {
                bounds.min[i] = std::min(point1[i], point2[i]);
                bounds.max[i] = std::max(point1[i], point2[i]);
            }
            
            View::Grid& grid = document()->grid();
            bounds.min = grid.snapDown(bounds.min);
            bounds.max = grid.snapUp(bounds.max);
            
            for (size_t i = 0; i < 3; i++)
                if (bounds.max[i] <= bounds.min[i])
                    bounds.max[i] = bounds.min[i] + grid.actualSize();
            return bounds.intersectWith(document()->worldBounds());
        }

        bool CreateBrushTool::checkBounds(const BBox3& bounds) const {
            const Vec3 size = bounds.size();
            return size.x() > 0.0 && size.y() > 0.0 && size.z() > 0.0;
        }

        Model::Brush* CreateBrushTool::createBrush(const BBox3& bounds) const {
            Model::Map& map = *document()->map();
            const BBox3& worldBounds = document()->worldBounds();
            const String textureName = document()->currentTextureName();
            return Model::createBrushFromBounds(map, worldBounds, bounds, textureName);
        }

        void CreateBrushTool::updateBrushRenderer() {
            Model::BrushList brushes;
            if (m_brush != NULL)
                brushes.push_back(m_brush);
            m_brushRenderer.setBrushes(brushes);
        }

        void CreateBrushTool::updateGuideRenderer(const BBox3& bounds) {
            m_guideRenderer.setBounds(bounds);
        }

        void CreateBrushTool::addBrushToMap(Model::Brush* brush) {
            controller()->beginUndoableGroup("Create Brush");
            controller()->deselectAll();
            controller()->addBrush(brush);
            controller()->selectObject(brush);
            controller()->closeGroup();
        }
    }
}
