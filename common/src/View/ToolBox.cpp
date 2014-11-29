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

#include "ToolBox.h"
#include "SetBool.h"
#include "View/InputState.h"
#include "View/Tool.h"
#include "View/ToolChain.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        ToolBox::ToolBox() :
        m_toolChain(NULL),
        m_dragReceiver(NULL),
        m_modalReceiver(NULL),
        m_dropReceiver(NULL),
        m_savedDropReceiver(NULL),
        m_enabled(true) {}

        void ToolBox::pick(const InputState& inputState, Hits& hits) {
            if (m_toolChain != NULL)
                m_toolChain->pick(inputState, hits);
        }

        bool ToolBox::dragEnter(const InputState& inputState, const String& text) {
            assert(m_dropReceiver == NULL);
            
            if (!m_enabled || m_toolChain == NULL)
                return false;
            
            deactivateAllTools();
            m_dropReceiver = m_toolChain->dragEnter(inputState, text);
            return m_dropReceiver != NULL;
        }
        
        bool ToolBox::dragMove(const InputState& inputState, const String& text) {
            if (m_dropReceiver == NULL)
                return false;
            
            if (!m_enabled || m_toolChain == NULL)
                return false;
            
            m_dropReceiver->dragMove(inputState);
            return true;
        }
        
        void ToolBox::dragLeave(const InputState& inputState) {
            if (m_dropReceiver == NULL)
                return;
            if (!m_enabled || m_toolChain == NULL)
                return;
            
            // This is a workaround for a bug in wxWidgets 3.0.0 on GTK2, where a drag leave event
            // is sent right before the drop event. So we save the drag receiver in an instance variable
            // and if dragDrop() is called, it can use that variable to find out who the drop receiver is.
            m_savedDropReceiver = m_dropReceiver;
            
            m_dropReceiver->dragLeave(inputState);
            m_dropReceiver = NULL;
        }
        
        bool ToolBox::dragDrop(const InputState& inputState, const String& text) {
            if (m_dropReceiver == NULL && m_savedDropReceiver == NULL)
                return false;
            
            if (!m_enabled || m_toolChain == NULL)
                return false;
            
            if (m_dropReceiver == NULL) {
                m_dropReceiver = m_savedDropReceiver;
                m_dropReceiver->activate(); // GTK2 fix: has been deactivated by dragLeave()
                m_dropReceiver->dragEnter(inputState, text);
            }
            
            const bool result = m_dropReceiver->dragDrop(inputState);
            m_dropReceiver = NULL;
            m_savedDropReceiver = NULL;
            return result;
        }
        
        void ToolBox::modifierKeyChange(const InputState& inputState) {
            if (m_enabled && m_toolChain != NULL)
                m_toolChain->modifierKeyChange(inputState);
        }
        
        void ToolBox::mouseDown(const InputState& inputState) {
            if (m_enabled && m_toolChain != NULL)
                m_toolChain->mouseDown(inputState);
        }

        bool ToolBox::mouseUp(const InputState& inputState) {
            if (!m_enabled || m_toolChain == NULL)
                return false;
            return m_toolChain->mouseUp(inputState);
        }

        void ToolBox::mouseDoubleClick(const InputState& inputState) {
            if (m_enabled && m_toolChain != NULL)
                m_toolChain->mouseDoubleClick(inputState);
        }
        
        void ToolBox::mouseMove(const InputState& inputState) {
            if (m_enabled && m_toolChain != NULL)
                m_toolChain->mouseMove(inputState);
        }
        
        bool ToolBox::dragging() const {
            return m_dragReceiver != NULL;
        }

        bool ToolBox::startMouseDrag(const InputState& inputState) {
            if (!m_enabled || m_toolChain == NULL)
                return false;
            m_dragReceiver = m_toolChain->startMouseDrag(inputState);
            return m_dragReceiver != NULL;
        }

        bool ToolBox::mouseDrag(const InputState& inputState) {
            assert(enabled() && dragging());
            return m_dragReceiver->mouseDrag(inputState);
        }

        void ToolBox::endMouseDrag(const InputState& inputState) {
            assert(enabled() && dragging());
            m_dragReceiver->endMouseDrag(inputState);
            m_dragReceiver = NULL;
        }
        
        void ToolBox::cancelDrag() {
            assert(dragging());
            m_dragReceiver->cancelMouseDrag();
            m_dragReceiver = NULL;
        }
        
        void ToolBox::mouseWheel(const InputState& inputState) {
            if (m_enabled && m_toolChain != NULL)
                m_toolChain->scroll(inputState);
        }
        
        bool ToolBox::cancel() {
            if (dragging()) {
                cancelDrag();
                return true;
            }
            
            if (m_toolChain == NULL || m_toolChain->cancel())
                return true;
            
            if (anyToolActive()) {
                deactivateAllTools();
                return true;
            }
            
            return false;
        }
        
        void ToolBox::addTool(Tool* tool) {
            if (m_toolChain == NULL)
                m_toolChain = new ToolChain(tool);
            else
                m_toolChain->append(new ToolChain(tool));
        }
        
        void ToolBox::deactivateWhen(Tool* master, Tool* slave) {
            assert(master != NULL);
            assert(slave != NULL);
            assert(master != slave);
            m_deactivateWhen[master].push_back(slave);
        }

        bool ToolBox::anyToolActive() const {
            return m_modalReceiver != NULL;
        }
        
        bool ToolBox::toolActive(const Tool* tool) const {
            return m_modalReceiver == tool;
        }
        
        void ToolBox::toggleTool(Tool* tool) {
            if (tool == NULL) {
                if (m_modalReceiver != NULL) {
                    deactivateTool(m_modalReceiver);
                    m_modalReceiver = NULL;
                }
            } else {
                if (m_modalReceiver == tool) {
                    deactivateTool(m_modalReceiver);
                    m_modalReceiver = NULL;
                } else {
                    if (m_modalReceiver != NULL) {
                        deactivateTool(m_modalReceiver);
                        m_modalReceiver = NULL;
                    }
                    if (activateTool(tool))
                        m_modalReceiver = tool;
                }
            }
        }
        
        void ToolBox::deactivateAllTools() {
            toggleTool(NULL);
        }
        
        bool ToolBox::enabled() const {
            return m_enabled;
        }
        
        void ToolBox::enable() {
            m_enabled = true;
        }
        
        void ToolBox::disable() {
            if (dragging())
                cancelDrag();
            m_enabled = false;
        }
        
        void ToolBox::setRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) {
            if (m_toolChain != NULL)
                m_toolChain->setRenderOptions(inputState, renderContext);
        }
        
        void ToolBox::renderTools(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            /* if (m_modalReceiver != NULL)
                m_modalReceiver->renderOnly(m_inputState, renderContext);
            else */
            if (m_toolChain != NULL)
                m_toolChain->render(inputState, renderContext, renderBatch);
        }
        
        bool ToolBox::activateTool(Tool* tool) {
            if (!tool->activate())
                return false;
            
            ToolMap::iterator mapIt = m_deactivateWhen.find(tool);
            if (mapIt != m_deactivateWhen.end()) {
                const ToolList& slaves = mapIt->second;
                ToolList::const_iterator listIt, listEnd;
                for (listIt = slaves.begin(), listEnd = slaves.end(); listIt != listEnd; ++listIt) {
                    Tool* slave = *listIt;
                    
                    slave->deactivate();
                    toolDeactivatedNotifier(slave);
                }
            }
        
            toolActivatedNotifier(tool);
            return true;
        }
        
        void ToolBox::deactivateTool(Tool* tool) {
            tool->deactivate();
            toolDeactivatedNotifier(tool);

            ToolMap::iterator mapIt = m_deactivateWhen.find(tool);
            if (mapIt != m_deactivateWhen.end()) {
                const ToolList& slaves = mapIt->second;
                ToolList::const_iterator listIt, listEnd;
                for (listIt = slaves.begin(), listEnd = slaves.end(); listIt != listEnd; ++listIt) {
                    Tool* slave = *listIt;
                    
                    slave->activate();
                    toolActivatedNotifier(slave);
                }
            }
        }
    }
}
