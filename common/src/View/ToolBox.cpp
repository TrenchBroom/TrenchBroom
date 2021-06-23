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

#include "ToolBox.h"

#include "Ensure.h"
#include "View/DragTracker.h"
#include "View/DropTracker.h"
#include "View/InputState.h"
#include "View/Tool.h"
#include "View/ToolController.h"
#include "View/ToolChain.h"

#include <cassert>
#include <string>
#include <utility>

#include <QDateTime>
#include <QDebug>

namespace TrenchBroom {
    namespace View {
        ToolBox::ToolBox() :
        m_modalTool(nullptr),
        m_enabled(true) {}

        ToolBox::~ToolBox() = default;

        void ToolBox::addTool(Tool* tool) {
            ensure(tool != nullptr, "tool is null");
            m_notifierConnection += tool->refreshViewsNotifier.connect(refreshViewsNotifier);
            m_notifierConnection += tool->toolHandleSelectionChangedNotifier.connect(toolHandleSelectionChangedNotifier);
        }

        void ToolBox::pick(ToolChain* chain, const InputState& inputState, Model::PickResult& pickResult) {
            chain->pick(inputState, pickResult);
        }

        bool ToolBox::dragEnter(ToolChain* chain, const InputState& inputState, const std::string& text) {
            if (!m_enabled) {
                return false;
            }

            if (m_dropTracker) {
                dragLeave(chain, inputState);
            }

            deactivateAllTools();
            m_dropTracker = chain->dragEnter(inputState, text);
            return m_dropTracker != nullptr;
        }

        bool ToolBox::dragMove(ToolChain* /* chain */, const InputState& inputState, const std::string& /* text */) {
            if (!m_enabled || m_dropTracker == nullptr) {
                return false;
            }

            m_dropTracker->move(inputState);
            return true;
        }

        void ToolBox::dragLeave(ToolChain* /* chain */, const InputState& inputState) {
            if (!m_enabled || m_dropTracker == nullptr) {
                return;
            }

            m_dropTracker->leave(inputState);
            m_dropTracker = nullptr;
        }

        bool ToolBox::dragDrop(ToolChain* /* chain */, const InputState& inputState, const std::string& /* text */) {
            if (!m_enabled || m_dropTracker == nullptr) {
                return false;
            }

            const auto result = m_dropTracker->drop(inputState);
            m_dropTracker = nullptr;
            return result;
        }

        void ToolBox::modifierKeyChange(ToolChain* chain, const InputState& inputState) {
            if (!m_enabled) {
                return;
            }
            chain->modifierKeyChange(inputState);
            if (m_dragTracker) {
                m_dragTracker->modifierKeyChange(inputState);
            }
        }

        void ToolBox::mouseDown(ToolChain* chain, const InputState& inputState) {
            if (!m_enabled) {
                return;
            }
            chain->mouseDown(inputState);
        }

        void ToolBox::mouseUp(ToolChain* chain, const InputState& inputState) {
            if (!m_enabled) {
                return;
            }
            chain->mouseUp(inputState);
        }

        bool ToolBox::mouseClick(ToolChain* chain, const InputState& inputState) {
            if (!m_enabled) {
                return false;
            }
            return chain->mouseClick(inputState);
        }

        void ToolBox::mouseDoubleClick(ToolChain* chain, const InputState& inputState) {
            if (!m_enabled) {
                return;
            }
            chain->mouseDoubleClick(inputState);
        }

        void ToolBox::mouseMove(ToolChain* chain, const InputState& inputState) {
            if (!m_enabled) {
                return;
            }
            chain->mouseMove(inputState);
        }

        bool ToolBox::dragging() const {
            return m_dragTracker != nullptr;
        }

        bool ToolBox::startMouseDrag(ToolChain* chain, const InputState& inputState) {
            if (!m_enabled) {
                return false;
            }
            m_dragTracker = chain->startMouseDrag(inputState);
            return m_dragTracker != nullptr;
        }

        bool ToolBox::mouseDrag(const InputState& inputState) {
            assert(enabled() && dragging());
            return m_dragTracker->drag(inputState);
        }

        void ToolBox::endMouseDrag(const InputState& inputState) {
            assert(enabled() && dragging());
            m_dragTracker->end(inputState);
            m_dragTracker = nullptr;
        }

        void ToolBox::cancelMouseDrag() {
            assert(dragging());
            m_dragTracker->cancel();
            m_dragTracker = nullptr;
        }

        void ToolBox::mouseScroll(ToolChain* chain, const InputState& inputState) {
            if (!m_enabled) {
                return;
            }
            if (m_dragTracker) {
                m_dragTracker->mouseScroll(inputState);
            } else {
                chain->mouseScroll(inputState);
            }
        }

        bool ToolBox::cancel(ToolChain* chain) {
            if (dragging()) {
                cancelMouseDrag();
                return true;
            }

            if (chain->cancel())
                return true;

            if (anyToolActive()) {
                deactivateAllTools();
                return true;
            }

            return false;
        }

        void ToolBox::suppressWhileActive(Tool* suppressedTool, Tool* primaryTool) {
            ensure(primaryTool != nullptr, "primary is null");
            ensure(suppressedTool != nullptr, "supressed is null");
            assert(primaryTool != suppressedTool);
            m_suppressedTools[primaryTool].push_back(suppressedTool);
        }

        bool ToolBox::anyToolActive() const {
            return m_modalTool != nullptr;
        }

        bool ToolBox::toolActive(const Tool* tool) const {
            if (tool == nullptr) {
                return false;
            } else {
                return tool->active();
            }
        }

        void ToolBox::toggleTool(Tool* tool) {
            if (tool == nullptr) {
                if (m_modalTool != nullptr) {
                    Tool* previousModalTool = std::exchange(m_modalTool, nullptr);
                    deactivateTool(previousModalTool);
                }
            } else {
                if (m_modalTool == tool) {
                    Tool* previousModalTool = std::exchange(m_modalTool, nullptr);
                    deactivateTool(previousModalTool);
                } else {
                    if (m_modalTool != nullptr) {
                        Tool* previousModalTool = std::exchange(m_modalTool, nullptr);
                        deactivateTool(previousModalTool);
                    }
                    if (activateTool(tool)) {
                        m_modalTool = tool;
                    }
                }
            }
        }

        void ToolBox::deactivateAllTools() {
            toggleTool(nullptr);
        }

        bool ToolBox::enabled() const {
            return m_enabled;
        }

        void ToolBox::enable() {
            m_enabled = true;
        }

        void ToolBox::disable() {
            assert(!dragging());
            m_enabled = false;
        }

        void ToolBox::setRenderOptions(ToolChain* chain, const InputState& inputState, Renderer::RenderContext& renderContext) {
            chain->setRenderOptions(inputState, renderContext);
            if (m_dragTracker) {
                m_dragTracker->setRenderOptions(inputState, renderContext);
            }
        }

        void ToolBox::renderTools(ToolChain* chain, const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            /* if (m_modalTool != nullptr)
                m_modalTool->renderOnly(m_inputState, renderContext);
            else */
            chain->render(inputState, renderContext, renderBatch);
            if (m_dragTracker) {
                m_dragTracker->render(inputState, renderContext, renderBatch);
            }
        }

        bool ToolBox::activateTool(Tool* tool) {
            if (!tool->activate()) {
                return false;
            }

            auto it = m_suppressedTools.find(tool);
            if (it != std::end(m_suppressedTools)) {
                for (Tool* suppress : it->second) {
                    suppress->deactivate();
                    toolDeactivatedNotifier(suppress);
                }
            }

            toolActivatedNotifier(tool);
            return true;
        }

        void ToolBox::deactivateTool(Tool* tool) {
            if (dragging()) {
                cancelMouseDrag();
            }

            auto it = m_suppressedTools.find(tool);
            if (it != std::end(m_suppressedTools)) {
                for (Tool* suppress : it->second) {
                    suppress->activate();
                    toolActivatedNotifier(suppress);
                }
            }

            tool->deactivate();
            toolDeactivatedNotifier(tool);
        }
    }
}
