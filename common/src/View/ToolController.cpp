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

#include "ToolController.h"

#include "Ensure.h"
#include "FloatType.h"
#include "Model/HitType.h"
#include "View/DragTracker.h"
#include "View/DropTracker.h"
#include "View/Grid.h"
#include "View/Tool.h"

#include <vecmath/distance.h>
#include <vecmath/intersection.h>
#include <vecmath/line.h>
#include <vecmath/vec.h>

namespace TrenchBroom {
    namespace View {
        PickingPolicy::~PickingPolicy() = default;

        NoPickingPolicy::~NoPickingPolicy() = default;
        void NoPickingPolicy::doPick(const InputState&, Model::PickResult&) {}

        KeyPolicy::~KeyPolicy() = default;

        NoKeyPolicy::~NoKeyPolicy() = default;
        void NoKeyPolicy::doModifierKeyChange(const InputState&) {}

        MousePolicy::~MousePolicy() = default;

        void MousePolicy::doMouseDown(const InputState&) {}
        void MousePolicy::doMouseUp(const InputState&) {}
        bool MousePolicy::doMouseClick(const InputState&) { return false; }
        bool MousePolicy::doMouseDoubleClick(const InputState&) { return false; }
        void MousePolicy::doMouseMove(const InputState&) {}
        void MousePolicy::doMouseScroll(const InputState&) {}

        RenderPolicy::~RenderPolicy() = default;
        void RenderPolicy::doSetRenderOptions(const InputState&, Renderer::RenderContext&) const {}
        void RenderPolicy::doRender(const InputState&, Renderer::RenderContext&, Renderer::RenderBatch&) {}

        ToolController::~ToolController() = default;
        Tool* ToolController::tool() { return doGetTool(); }
        const Tool* ToolController::tool() const { return doGetTool(); }
        bool ToolController::toolActive() const { return tool()->active(); }
        void ToolController::refreshViews() { tool()->refreshViews(); }

        std::unique_ptr<DragTracker> ToolController::acceptMouseDrag(const InputState&) {
            return nullptr;
        }

        std::unique_ptr<DropTracker> ToolController::acceptDrop(const InputState&, const std::string& /* payload */) {
            return nullptr;
        }

        ToolControllerGroup::ToolControllerGroup() = default;
        ToolControllerGroup::~ToolControllerGroup() = default;

        void ToolControllerGroup::addController(std::unique_ptr<ToolController> controller) {
            ensure(controller != nullptr, "controller is null");
            m_chain.append(std::move(controller));
        }

        void ToolControllerGroup::doPick(const InputState& inputState, Model::PickResult& pickResult) {
            m_chain.pick(inputState, pickResult);
        }

        void ToolControllerGroup::doModifierKeyChange(const InputState& inputState) {
            m_chain.modifierKeyChange(inputState);
        }

        void ToolControllerGroup::doMouseDown(const InputState& inputState) {
            m_chain.mouseDown(inputState);
        }

        void ToolControllerGroup::doMouseUp(const InputState& inputState) {
            m_chain.mouseUp(inputState);
        }

        bool ToolControllerGroup::doMouseClick(const InputState& inputState) {
            return m_chain.mouseClick(inputState);
        }

        bool ToolControllerGroup::doMouseDoubleClick(const InputState& inputState) {
            return m_chain.mouseDoubleClick(inputState);
        }

        void ToolControllerGroup::doMouseMove(const InputState& inputState) {
            m_chain.mouseMove(inputState);
        }

        void ToolControllerGroup::doMouseScroll(const InputState& inputState) {
            m_chain.mouseScroll(inputState);
        }

        std::unique_ptr<DragTracker> ToolControllerGroup::acceptMouseDrag(const InputState& inputState) {
            if (!doShouldHandleMouseDrag(inputState)) {
                return nullptr;
            }

            return m_chain.startMouseDrag(inputState);
        }

        std::unique_ptr<DropTracker> ToolControllerGroup::acceptDrop(const InputState& inputState, const std::string& payload) {
            if (!doShouldHandleDrop(inputState, payload)) {
                return nullptr;
            }
            return m_chain.dragEnter(inputState, payload);
        }

        void ToolControllerGroup::doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {
            m_chain.setRenderOptions(inputState, renderContext);
        }

        void ToolControllerGroup::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            m_chain.render(inputState, renderContext, renderBatch);
        }

        bool ToolControllerGroup::doCancel() {
            return m_chain.cancel();
        }

        bool ToolControllerGroup::doShouldHandleMouseDrag(const InputState&) const {
            return true;
        }

        bool ToolControllerGroup::doShouldHandleDrop(const InputState&, const std::string& /* payload */) const {
            return true;
        }
    }
}
