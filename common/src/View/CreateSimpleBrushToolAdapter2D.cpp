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

#include "CreateSimpleBrushToolAdapter2D.h"

#include "Polyhedron.h"
#include "Renderer/Camera.h"
#include "View/CreateSimpleBrushTool.h"
#include "View/Grid.h"
#include "View/InputState.h"
#include "View/MapDocument.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        CreateSimpleBrushToolAdapter2D::CreateSimpleBrushToolAdapter2D(CreateSimpleBrushTool* tool, MapDocumentWPtr document) :
        m_tool(tool),
        m_document(document) {
            assert(m_tool != NULL);
        }

        Tool* CreateSimpleBrushToolAdapter2D::doGetTool() {
            return m_tool;
        }
        
        bool CreateSimpleBrushToolAdapter2D::doStartPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
            if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                return false;
            if (!inputState.modifierKeysPressed(ModifierKeys::MKNone))
                return false;
            
            MapDocumentSPtr document = lock(m_document);
            if (document->hasSelection())
                return false;
            
            const Renderer::Camera& camera = inputState.camera();
            const Vec3 planeNorm(camera.direction().firstAxis());
            plane = Plane3(initialPoint, planeNorm);
            
            const Ray3& pickRay = inputState.pickRay();
            initialPoint = pickRay.pointAtDistance(plane.intersectWithRay(pickRay));

            m_initialPoint = initialPoint;
            updateBounds(inputState, m_initialPoint);
            m_tool->refreshViews();
            
            return true;
        }
        
        bool CreateSimpleBrushToolAdapter2D::doPlaneDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint) {
            if (updateBounds(inputState, curPoint))
                m_tool->refreshViews();
            return true;
        }
        
        void CreateSimpleBrushToolAdapter2D::doEndPlaneDrag(const InputState& inputState) {
            if (!m_bounds.empty())
                m_tool->createBrush();
        }
        
        void CreateSimpleBrushToolAdapter2D::doCancelPlaneDrag() {}
        
        void CreateSimpleBrushToolAdapter2D::doResetPlane(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {}
        
        void CreateSimpleBrushToolAdapter2D::doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {}
        
        void CreateSimpleBrushToolAdapter2D::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            m_tool->render(renderContext, renderBatch);
        }

        bool CreateSimpleBrushToolAdapter2D::doCancel() {
            return false;
        }

        bool CreateSimpleBrushToolAdapter2D::updateBounds(const InputState& inputState, const Vec3& currentPoint) {
            BBox3 bounds(m_initialPoint, m_initialPoint);
            bounds.mergeWith(currentPoint);
            snapBounds(inputState, bounds);
            
            if (bounds.empty() || bounds == m_bounds)
                return false;
            
            using std::swap;
            swap(m_bounds, bounds);
            m_tool->update(m_bounds);
            
            return true;
        }

        void CreateSimpleBrushToolAdapter2D::snapBounds(const InputState& inputState, BBox3& bounds) {
            MapDocumentSPtr document = lock(m_document);
            const Grid& grid = document->grid();
            bounds.min = grid.snapDown(bounds.min);
            bounds.max = grid.snapUp(bounds.max);
            
            const Renderer::Camera& camera = inputState.camera();
            const BBox3& refBounds = document->referenceBounds();
            bounds.mix(refBounds, camera.direction().firstAxis().absolute());
        }
    }
}
