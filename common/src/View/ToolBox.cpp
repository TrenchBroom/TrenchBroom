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
        m_dragReceiver(nullptr),
        m_dropReceiver(nullptr),
        m_modalTool(nullptr),
        m_enabled(true) {}

        void ToolBox::addTool(Tool* tool) {
            ensure(tool != nullptr, "tool is null");
            tool->refreshViewsNotifier.addObserver(refreshViewsNotifier);
            tool->toolHandleSelectionChangedNotifier.addObserver(toolHandleSelectionChangedNotifier);
        }

        void ToolBox::pick(ToolChain* chain, const InputState& inputState, Model::PickResult& pickResult) {
            chain->pick(inputState, pickResult);
        }

        bool ToolBox::dragEnter(ToolChain* chain, const InputState& inputState, const std::string& text) {
            if (!m_enabled) {
                return false;
            }

            if (m_dropReceiver != nullptr) {
                dragLeave(chain, inputState);
            }

            deactivateAllTools();
            m_dropReceiver = chain->dragEnter(inputState, text);
            return m_dropReceiver != nullptr;
        }

        bool ToolBox::dragMove(ToolChain* /* chain */, const InputState& inputState, const std::string& /* text */) {
            if (!m_enabled || m_dropReceiver == nullptr) {
                return false;
            }

            m_dropReceiver->dragMove(inputState);
            return true;
        }

        void ToolBox::dragLeave(ToolChain* /* chain */, const InputState& inputState) {
            if (!m_enabled || m_dropReceiver == nullptr) {
                return;
            }

            m_dropReceiver->dragLeave(inputState);
            m_dropReceiver = nullptr;
        }

        bool ToolBox::dragDrop(ToolChain* /* chain */, const InputState& inputState, const std::string& /* text */) {
            if (!m_enabled || m_dropReceiver == nullptr) {
                return false;
            }

            const auto result = m_dropReceiver->dragDrop(inputState);
            m_dropReceiver = nullptr;
            return result;
        }

        void ToolBox::modifierKeyChange(ToolChain* chain, const InputState& inputState) {
            if (!m_enabled) {
                return;
            }
            chain->modifierKeyChange(inputState);
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
            return m_dragReceiver != nullptr;
        }

        bool ToolBox::startMouseDrag(ToolChain* chain, const InputState& inputState) {
            if (!m_enabled) {
                return false;
            }
            m_dragReceiver = chain->startMouseDrag(inputState);
            return m_dragReceiver != nullptr;
        }

        bool ToolBox::mouseDrag(const InputState& inputState) {
            assert(enabled() && dragging());
            return m_dragReceiver->mouseDrag(inputState);
        }

        void ToolBox::endMouseDrag(const InputState& inputState) {
            assert(enabled() && dragging());
            m_dragReceiver->endMouseDrag(inputState);
            m_dragReceiver = nullptr;
        }

        void ToolBox::cancelMouseDrag() {
            assert(dragging());
            m_dragReceiver->cancelMouseDrag();
            m_dragReceiver = nullptr;
        }

        void ToolBox::mouseScroll(ToolChain* chain, const InputState& inputState) {
            if (!m_enabled) {
                return;
            }
            chain->mouseScroll(inputState);
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

        void ToolBox::deactivateWhen(Tool* master, Tool* slave) {
            ensure(master != nullptr, "master is null");
            ensure(slave != nullptr, "slave is null");
            assert(master != slave);
            m_deactivateWhen[master].push_back(slave);
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
        }

        void ToolBox::renderTools(ToolChain* chain, const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            /* if (m_modalTool != nullptr)
                m_modalTool->renderOnly(m_inputState, renderContext);
            else */
            chain->render(inputState, renderContext, renderBatch);
        }

        bool ToolBox::activateTool(Tool* tool) {
            if (!tool->activate()) {
                return false;
            }

            auto it = m_deactivateWhen.find(tool);
            if (it != std::end(m_deactivateWhen)) {
                const auto& slaves = it->second;
                for (Tool* slave : slaves) {
                    slave->deactivate();
                    toolDeactivatedNotifier(slave);
                }
            }

            toolActivatedNotifier(tool);
            return true;
        }

        void ToolBox::deactivateTool(Tool* tool) {
            if (dragging()) {
                cancelMouseDrag();
            }

            auto it = m_deactivateWhen.find(tool);
            if (it != std::end(m_deactivateWhen)) {
                const auto& slaves = it->second;
                for (Tool* slave : slaves) {
                    slave->activate();
                    toolActivatedNotifier(slave);
                }
            }

            tool->deactivate();
            toolDeactivatedNotifier(tool);
        }
    }
}
