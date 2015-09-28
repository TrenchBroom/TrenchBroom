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

#include "CreateSimpleBrushToolAdapter3D.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/HitAdapter.h"
#include "Model/HitQuery.h"
#include "Model/PickResult.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderService.h"
#include "View/CreateSimpleBrushTool.h"
#include "View/Grid.h"
#include "View/InputState.h"
#include "View/MapDocument.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        CreateSimpleBrushToolAdapter3D::CreateSimpleBrushToolAdapter3D(CreateSimpleBrushTool* tool, MapDocumentWPtr document) :
        m_tool(tool),
        m_document(document) {
            assert(tool != NULL);
        }

        Tool* CreateSimpleBrushToolAdapter3D::doGetTool() {
            return m_tool;
        }

        void CreateSimpleBrushToolAdapter3D::doModifierKeyChange(const InputState& inputState) {
            if (dragging())
                resetPlane(inputState);
        }

        bool CreateSimpleBrushToolAdapter3D::doStartPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
            if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                return false;
            if (!inputState.modifierKeysPressed(ModifierKeys::MKNone))
                return false;
            
            MapDocumentSPtr document = lock(m_document);
            if (document->hasSelection())
                return false;

            const Model::PickResult& pickResult = inputState.pickResult();
            const Model::Hit& hit = pickResult.query().pickable().type(Model::Brush::BrushHit).occluded().first();
            if (hit.isMatch())
                m_initialPoint = initialPoint = hit.hitPoint();
            else
                m_initialPoint = initialPoint = inputState.defaultPointUnderMouse();
            
            plane = Plane3(initialPoint, Vec3::PosZ);
            
            updateBounds(m_initialPoint);
            return true;
        }

        bool CreateSimpleBrushToolAdapter3D::doPlaneDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint) {
            updateBounds(curPoint);
            return true;
        }

        void CreateSimpleBrushToolAdapter3D::doEndPlaneDrag(const InputState& inputState) {
            m_tool->createBrush();
        }

        void CreateSimpleBrushToolAdapter3D::doCancelPlaneDrag() {
            m_tool->cancel();
        }

        void CreateSimpleBrushToolAdapter3D::doResetPlane(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
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

        void CreateSimpleBrushToolAdapter3D::doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {}

        void CreateSimpleBrushToolAdapter3D::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            m_tool->render(renderContext, renderBatch);
        }

        bool CreateSimpleBrushToolAdapter3D::doCancel() {
            return false;
        }

        void CreateSimpleBrushToolAdapter3D::updateBounds(const Vec3& point) {
            BBox3 bounds;
            bounds.min = min(m_initialPoint, point);
            bounds.max = max(m_initialPoint, point);
            
            MapDocumentSPtr document = lock(m_document);
            const Grid& grid = document->grid();

            // prevent flickering due to very small rounding errors
            bounds.min.correct();
            bounds.max.correct();
            
            bounds.min = grid.snapDown(bounds.min);
            bounds.max = grid.snapUp(bounds.max);

            for (size_t i = 0; i < 3; i++)
                if (bounds.max[i] <= bounds.min[i])
                    bounds.max[i] = bounds.min[i] + grid.actualSize();

            m_tool->update(bounds);
        }
    }
}
