/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "CreateSimpleBrushToolController2D.h"

#include "Polyhedron.h"
#include "Renderer/Camera.h"
#include "View/CreateSimpleBrushTool.h"
#include "View/Grid.h"
#include "View/InputState.h"
#include "View/MapDocument.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        CreateSimpleBrushToolController2D::CreateSimpleBrushToolController2D(CreateSimpleBrushTool* tool, MapDocumentWPtr document) :
        m_tool(tool),
        m_document(document) {
            assert(m_tool != NULL);
        }

        Tool* CreateSimpleBrushToolController2D::doGetTool() {
            return m_tool;
        }
        
        RestrictedDragPolicy::DragInfo CreateSimpleBrushToolController2D::doStartDrag(const InputState& inputState) {
            if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                return DragInfo();
            if (!inputState.modifierKeysPressed(ModifierKeys::MKNone))
                return DragInfo();
            
            MapDocumentSPtr document = lock(m_document);
            if (document->hasSelection())
                return DragInfo();
            
            const BBox3& bounds = document->referenceBounds();
            const Renderer::Camera& camera = inputState.camera();
            const Plane3 plane(bounds.min, camera.direction().firstAxis());
            
            const FloatType distance = plane.intersectWithRay(inputState.pickRay());
            if (Math::isnan(distance))
                return DragInfo();
            
            m_initialPoint = inputState.pickRay().pointAtDistance(distance);
            if (updateBounds(inputState, m_initialPoint))
                m_tool->refreshViews();

            return DragInfo(new PlaneDragRestricter(plane), new NoDragSnapper(), m_initialPoint);
        }
        
        RestrictedDragPolicy::DragResult CreateSimpleBrushToolController2D::doDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint) {
            if (updateBounds(inputState, curPoint)) {
                m_tool->refreshViews();
                return DR_Continue;
            }
            return DR_Deny;
        }
        
        void CreateSimpleBrushToolController2D::doEndDrag(const InputState& inputState) {
            if (!m_bounds.empty())
                m_tool->createBrush();
        }
        
        void CreateSimpleBrushToolController2D::doCancelDrag() {
            m_tool->cancel();
        }

        void CreateSimpleBrushToolController2D::doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {}
        
        void CreateSimpleBrushToolController2D::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            m_tool->render(renderContext, renderBatch);
        }

        bool CreateSimpleBrushToolController2D::doCancel() {
            return false;
        }

        bool CreateSimpleBrushToolController2D::updateBounds(const InputState& inputState, const Vec3& currentPoint) {
            BBox3 bounds(m_initialPoint, m_initialPoint);
            bounds.mergeWith(currentPoint);
            snapBounds(inputState, bounds);

            MapDocumentSPtr document = lock(m_document);
            bounds.intersectWith(document->worldBounds());
            
            if (bounds.empty() || bounds == m_bounds)
                return false;
            
            using std::swap;
            swap(m_bounds, bounds);
            m_tool->update(m_bounds);
            
            return true;
        }

        void CreateSimpleBrushToolController2D::snapBounds(const InputState& inputState, BBox3& bounds) {
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
