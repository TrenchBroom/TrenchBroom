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
        ToolController::~ToolController() = default;

        bool ToolController::toolActive() const { return tool().active(); }

        void ToolController::pick(const InputState&, Model::PickResult&) {}

        void ToolController::modifierKeyChange(const InputState&) {}

        void ToolController::mouseDown(const InputState&) {}
        void ToolController::mouseUp(const InputState&) {}
        bool ToolController::mouseClick(const InputState&) { return false; }
        bool ToolController::mouseDoubleClick(const InputState&) { return false; }
        void ToolController::mouseMove(const InputState&) {}
        void ToolController::mouseScroll(const InputState&) {}

        std::unique_ptr<DragTracker> ToolController::acceptMouseDrag(const InputState&) { return nullptr;}
        bool ToolController::anyToolDragging(const InputState&) const {return false; }

        std::unique_ptr<DropTracker> acceptDrop(const InputState&, const std::string& /* payload */) { return nullptr; }

        void ToolController::setRenderOptions(const InputState&, Renderer::RenderContext&) const {}
        void ToolController::render(const InputState&, Renderer::RenderContext&, Renderer::RenderBatch&) {}

        bool ToolController::cancel() { return false; }

        void ToolController::refreshViews() { tool().refreshViews(); }

        std::unique_ptr<DropTracker> ToolController::acceptDrop(const InputState&, const std::string& /* payload */) {
            return nullptr;
        }

        ToolControllerGroup::ToolControllerGroup() = default;
        ToolControllerGroup::~ToolControllerGroup() = default;

        void ToolControllerGroup::addController(std::unique_ptr<ToolController> controller) {
            ensure(controller != nullptr, "controller is null");
            m_chain.append(std::move(controller));
        }

        void ToolControllerGroup::pick(const InputState& inputState, Model::PickResult& pickResult) {
            m_chain.pick(inputState, pickResult);
        }

        void ToolControllerGroup::modifierKeyChange(const InputState& inputState) {
            m_chain.modifierKeyChange(inputState);
        }

        void ToolControllerGroup::mouseDown(const InputState& inputState) {
            m_chain.mouseDown(inputState);
        }

        void ToolControllerGroup::mouseUp(const InputState& inputState) {
            m_chain.mouseUp(inputState);
        }

        bool ToolControllerGroup::mouseClick(const InputState& inputState) {
            return m_chain.mouseClick(inputState);
        }

        bool ToolControllerGroup::mouseDoubleClick(const InputState& inputState) {
            return m_chain.mouseDoubleClick(inputState);
        }

        void ToolControllerGroup::mouseMove(const InputState& inputState) {
            m_chain.mouseMove(inputState);
        }

        void ToolControllerGroup::mouseScroll(const InputState& inputState) {
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

        void ToolControllerGroup::setRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {
            m_chain.setRenderOptions(inputState, renderContext);
        }

        void ToolControllerGroup::render(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            m_chain.render(inputState, renderContext, renderBatch);
        }

        bool ToolControllerGroup::cancel() {
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
