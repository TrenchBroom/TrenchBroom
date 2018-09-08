/*
 Copyright (C) 2010-2017 Kristian Duske
 
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
            ensure(m_tool != nullptr, "tool is null");
        }

        Tool* CreateSimpleBrushToolController2D::doGetTool() {
            return m_tool;
        }
        
        RestrictedDragPolicy::DragInfo CreateSimpleBrushToolController2D::doStartDrag(const InputState& inputState) {
            if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft)) {
                return DragInfo();
            }
            if (!inputState.modifierKeysPressed(ModifierKeys::MKNone)) {
                return DragInfo();
            }

            auto document = lock(m_document);
            if (document->hasSelection()) {
                return DragInfo();
            }

            const auto& bounds = document->referenceBounds();
            const auto& camera = inputState.camera();
            const vm::plane3 plane(bounds.min, vm::vec3(firstAxis(camera.direction())));
            
            const auto distance = intersect(inputState.pickRay(), plane);
            if (vm::isnan(distance)) {
                return DragInfo();
            }

            m_initialPoint = inputState.pickRay().pointAtDistance(distance);
            if (updateBounds(inputState, m_initialPoint)) {
                m_tool->refreshViews();
            }

            return DragInfo(new PlaneDragRestricter(plane), new NoDragSnapper(), m_initialPoint);
        }
        
        RestrictedDragPolicy::DragResult CreateSimpleBrushToolController2D::doDrag(const InputState& inputState, const vm::vec3& lastHandlePosition, const vm::vec3& nextHandlePosition) {
            if (updateBounds(inputState, nextHandlePosition)) {
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

        bool CreateSimpleBrushToolController2D::updateBounds(const InputState& inputState, const vm::vec3& currentPoint) {
            vm::bbox3 bounds(m_initialPoint, m_initialPoint);
            bounds = merge(bounds, currentPoint);
            snapBounds(inputState, bounds);

            MapDocumentSPtr document = lock(m_document);
            bounds = intersect(bounds, document->worldBounds());
            
            if (bounds.empty() || bounds == m_bounds)
                return false;
            
            using std::swap;
            swap(m_bounds, bounds);
            m_tool->update(m_bounds);
            
            return true;
        }

        void CreateSimpleBrushToolController2D::snapBounds(const InputState& inputState, vm::bbox3& bounds) {
            auto document = lock(m_document);
            const auto& grid = document->grid();
            auto min = grid.snapDown(bounds.min);
            auto max = grid.snapUp(bounds.max);
            
            const auto& camera = inputState.camera();
            const auto& refBounds = document->referenceBounds();
            const auto factors = vm::vec3(abs(firstAxis(camera.direction())));
            min = mix(min, refBounds.min, factors);
            max = mix(max, refBounds.max, factors);

            bounds = vm::bbox3(min, max);
        }
    }
}
