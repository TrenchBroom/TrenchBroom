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

#include "CreateSimpleBrushToolController3D.h"

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
        CreateSimpleBrushToolController3D::CreateSimpleBrushToolController3D(CreateSimpleBrushTool* tool, MapDocumentWPtr document) :
        m_tool(tool),
        m_document(document) {
            ensure(tool != NULL, "tool is null");
        }

        Tool* CreateSimpleBrushToolController3D::doGetTool() {
            return m_tool;
        }

        void CreateSimpleBrushToolController3D::doModifierKeyChange(const InputState& inputState) {
            if (thisToolDragging()) {
                if (inputState.modifierKeys() == ModifierKeys::MKAlt) {
                    setRestricter(inputState, new LineDragRestricter(Line3(curPoint(), Vec3::PosZ)), true);
                } else {
                    setRestricter(inputState, new PlaneDragRestricter(horizontalDragPlane(curPoint())), true);
                }
            }
        }

        RestrictedDragPolicy::DragInfo CreateSimpleBrushToolController3D::doStartDrag(const InputState& inputState) {
            if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                return DragInfo();
            if (!inputState.modifierKeysPressed(ModifierKeys::MKNone))
                return DragInfo();
            
            MapDocumentSPtr document = lock(m_document);
            if (document->hasSelection())
                return DragInfo();
            
            const Model::PickResult& pickResult = inputState.pickResult();
            const Model::Hit& hit = pickResult.query().pickable().type(Model::Brush::BrushHit).occluded().first();
            if (hit.isMatch())
                m_initialPoint = hit.hitPoint();
            else
                m_initialPoint = inputState.defaultPointUnderMouse();
            
            updateBounds(m_initialPoint, Vec3(inputState.camera().position()));
            refreshViews();
                
            
            const Plane3 plane = Plane3(m_initialPoint, Vec3::PosZ);
            return DragInfo(new PlaneDragRestricter(plane), new NoDragSnapper(), m_initialPoint);
        }
        
        RestrictedDragPolicy::DragResult CreateSimpleBrushToolController3D::doDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint) {
            updateBounds(curPoint, Vec3(inputState.camera().position()));
            refreshViews();
            return DR_Continue;
        }
        
        void CreateSimpleBrushToolController3D::doEndDrag(const InputState& inputState) {
            m_tool->createBrush();
        }
        
        void CreateSimpleBrushToolController3D::doCancelDrag() {
            m_tool->cancel();
        }

        void CreateSimpleBrushToolController3D::doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {}

        void CreateSimpleBrushToolController3D::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            m_tool->render(renderContext, renderBatch);
        }

        bool CreateSimpleBrushToolController3D::doCancel() {
            return false;
        }

        void CreateSimpleBrushToolController3D::updateBounds(const Vec3& point, const Vec3 cameraPosition) {
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
            
            for (size_t i = 0; i < 3; i++) {
                if (Math::lte(bounds.max[i], bounds.min[i])) {
                    if (bounds.min[i] < cameraPosition[i])
                        bounds.max[i] = bounds.min[i] + grid.actualSize();
                    else
                        bounds.min[i] = bounds.max[i] - grid.actualSize();
                }
            }

            bounds.intersectWith(document->worldBounds());
            m_tool->update(bounds);
        }
    }
}
