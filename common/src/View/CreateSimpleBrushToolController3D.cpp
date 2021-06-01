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

#include "CreateSimpleBrushToolController3D.h"

#include "FloatType.h"
#include "PreferenceManager.h"
#include "Model/BrushNode.h"
#include "Model/BrushFace.h"
#include "Model/Hit.h"
#include "Model/HitQuery.h"
#include "Model/HitFilter.h"
#include "Model/PickResult.h"
#include "Renderer/Camera.h"
#include "View/CreateSimpleBrushTool.h"
#include "View/Grid.h"
#include "View/InputState.h"
#include "View/MapDocument.h"

#include <kdl/memory_utils.h>

#include <vecmath/vec.h>
#include <vecmath/line.h>
#include <vecmath/plane.h>
#include <vecmath/bbox.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        CreateSimpleBrushToolController3D::CreateSimpleBrushToolController3D(CreateSimpleBrushTool* tool, std::weak_ptr<MapDocument> document) :
        m_tool(tool),
        m_document(document) {
            ensure(tool != nullptr, "tool is null");
        }

        Tool* CreateSimpleBrushToolController3D::doGetTool() {
            return m_tool;
        }

        const Tool* CreateSimpleBrushToolController3D::doGetTool() const {
            return m_tool;
        }

        void CreateSimpleBrushToolController3D::doModifierKeyChange(const InputState& inputState) {
            if (thisToolDragging()) {
                if (inputState.modifierKeys() == ModifierKeys::MKAlt) {
                    setRestricter(inputState, new LineDragRestricter(vm::line3(currentHandlePosition(), vm::vec3::pos_z())), true);
                } else {
                    setRestricter(inputState, new PlaneDragRestricter(vm::horizontal_plane(currentHandlePosition())), true);
                }
            }
        }

        RestrictedDragPolicy::DragInfo CreateSimpleBrushToolController3D::doStartDrag(const InputState& inputState) {
            using namespace Model::HitFilters;

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

            const Model::Hit& hit = inputState.pickResult().first(type(Model::BrushNode::BrushHitType));
            if (hit.isMatch()) {
                m_initialPoint = hit.hitPoint();
            } else {
                m_initialPoint = inputState.defaultPointUnderMouse();
            }

            updateBounds(m_initialPoint, vm::vec3(inputState.camera().position()));
            refreshViews();


            const vm::plane3 plane = vm::plane3(m_initialPoint, vm::vec3::pos_z());
            return DragInfo(new PlaneDragRestricter(plane), new NoDragSnapper(), m_initialPoint);
        }

        RestrictedDragPolicy::DragResult CreateSimpleBrushToolController3D::doDrag(const InputState& inputState, const vm::vec3& /* lastHandlePosition */, const vm::vec3& nextHandlePosition) {
            updateBounds(nextHandlePosition, vm::vec3(inputState.camera().position()));
            refreshViews();
            return DR_Continue;
        }

        void CreateSimpleBrushToolController3D::doEndDrag(const InputState&) {
            m_tool->createBrush();
        }

        void CreateSimpleBrushToolController3D::doCancelDrag() {
            m_tool->cancel();
        }

        void CreateSimpleBrushToolController3D::doSetRenderOptions(const InputState&, Renderer::RenderContext&) const {}

        void CreateSimpleBrushToolController3D::doRender(const InputState&, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            m_tool->render(renderContext, renderBatch);
        }

        bool CreateSimpleBrushToolController3D::doCancel() {
            return false;
        }

        void CreateSimpleBrushToolController3D::updateBounds(const vm::vec3& point, const vm::vec3& cameraPosition) {
            vm::bbox3 bounds;

            bounds.min = min(m_initialPoint, point);
            bounds.max = max(m_initialPoint, point);

            auto document = kdl::mem_lock(m_document);
            const auto& grid = document->grid();

            // prevent flickering due to very small rounding errors
            bounds.min = correct(bounds.min);
            bounds.max = correct(bounds.max);

            bounds.min = grid.snapDown(bounds.min);
            bounds.max = grid.snapUp(bounds.max);

            for (size_t i = 0; i < 3; i++) {
                if (bounds.max[i] <= bounds.min[i]) {
                    if (bounds.min[i] < cameraPosition[i]) {
                        bounds.max[i] = bounds.min[i] + grid.actualSize();
                    } else {
                        bounds.min[i] = bounds.max[i] - grid.actualSize();
                    }
                }
            }

            bounds = intersect(bounds, document->worldBounds());
            if (!bounds.is_empty()) {
                m_tool->update(bounds);
            }
        }
    }
}
