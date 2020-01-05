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

#include "Ensure.h"
#include "Renderer/Camera.h"
#include "View/CreateSimpleBrushTool.h"
#include "View/Grid.h"
#include "View/InputState.h"
#include "View/MapDocument.h"

#include <kdl/memory_utils.h>

#include <vecmath/intersection.h>
#include <vecmath/scalar.h>

namespace TrenchBroom {
    namespace View {
        CreateSimpleBrushToolController2D::CreateSimpleBrushToolController2D(CreateSimpleBrushTool* tool, std::weak_ptr<MapDocument> document) :
        m_tool(tool),
        m_document(document) {
            ensure(m_tool != nullptr, "tool is null");
        }

        Tool* CreateSimpleBrushToolController2D::doGetTool() {
            return m_tool;
        }

        const Tool* CreateSimpleBrushToolController2D::doGetTool() const {
            return m_tool;
        }

        RestrictedDragPolicy::DragInfo CreateSimpleBrushToolController2D::doStartDrag(const InputState& inputState) {
            if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft)) {
                return DragInfo();
            }
            if (!inputState.modifierKeysPressed(ModifierKeys::MKNone)) {
                return DragInfo();
            }

            auto document = kdl::mem_lock(m_document);
            if (document->hasSelection()) {
                return DragInfo();
            }

            const auto& bounds = document->referenceBounds();
            const auto& camera = inputState.camera();
            const vm::plane3 plane(bounds.min, vm::vec3(vm::get_abs_max_component_axis(camera.direction())));

            const auto distance = vm::intersect_ray_plane(inputState.pickRay(), plane);
            if (vm::is_nan(distance)) {
                return DragInfo();
            }

            m_initialPoint = vm::point_at_distance(inputState.pickRay(), distance);
            if (updateBounds(inputState, m_initialPoint)) {
                m_tool->refreshViews();
            }

            return DragInfo(new PlaneDragRestricter(plane), new NoDragSnapper(), m_initialPoint);
        }

        RestrictedDragPolicy::DragResult CreateSimpleBrushToolController2D::doDrag(const InputState& inputState, const vm::vec3& /* lastHandlePosition */, const vm::vec3& nextHandlePosition) {
            if (updateBounds(inputState, nextHandlePosition)) {
                m_tool->refreshViews();
                return DR_Continue;
            }
            return DR_Deny;
        }

        void CreateSimpleBrushToolController2D::doEndDrag(const InputState&) {
            if (!m_bounds.is_empty())
                m_tool->createBrush();
        }

        void CreateSimpleBrushToolController2D::doCancelDrag() {
            m_tool->cancel();
        }

        void CreateSimpleBrushToolController2D::doSetRenderOptions(const InputState&, Renderer::RenderContext&) const {}

        void CreateSimpleBrushToolController2D::doRender(const InputState&, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            m_tool->render(renderContext, renderBatch);
        }

        bool CreateSimpleBrushToolController2D::doCancel() {
            return false;
        }

        bool CreateSimpleBrushToolController2D::updateBounds(const InputState& inputState, const vm::vec3& currentPoint) {
            vm::bbox3 bounds(m_initialPoint, m_initialPoint);
            bounds = merge(bounds, currentPoint);
            snapBounds(inputState, bounds);

            auto document = kdl::mem_lock(m_document);
            bounds = vm::intersect(bounds, document->worldBounds());

            if (bounds.is_empty() || bounds == m_bounds)
                return false;

            using std::swap;
            swap(m_bounds, bounds);
            m_tool->update(m_bounds);

            return true;
        }

        void CreateSimpleBrushToolController2D::snapBounds(const InputState& inputState, vm::bbox3& bounds) {
            auto document = kdl::mem_lock(m_document);
            const auto& grid = document->grid();
            auto min = grid.snapDown(bounds.min);
            auto max = grid.snapUp(bounds.max);

            const auto& camera = inputState.camera();
            const auto& refBounds = document->referenceBounds();
            const auto factors = vm::vec3(abs(vm::get_abs_max_component_axis(camera.direction())));
            min = vm::mix(min, refBounds.min, factors);
            max = vm::mix(max, refBounds.max, factors);

            bounds = vm::bbox3(min, max);
        }
    }
}
