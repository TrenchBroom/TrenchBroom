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

#include "ToolChain.h"

#include "View/Tool.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        ToolChain::ToolChain(Tool* tool) :
        m_tool(tool),
        m_suffix(NULL) {
            assert(m_tool != NULL);
        }
        
        ToolChain::~ToolChain() {
            delete m_suffix;
        }

        void ToolChain::append(ToolChain* suffix) {
            assert(suffix != NULL);
            if (m_suffix == NULL)
                m_suffix = suffix;
            else
                m_suffix->append(suffix);
        }
        
        bool ToolChain::cancel() {
            if (m_tool->cancel())
                return true;
            if (m_suffix != NULL)
                return m_suffix->cancel();
            return false;
        }
        
        void ToolChain::pick(const InputState& inputState, Hits& hits) {
            m_tool->pick(inputState, hits);
            if (m_suffix != NULL)
                m_suffix->pick(inputState, hits);
        }
        
        void ToolChain::modifierKeyChange(const InputState& inputState) {
            m_tool->modifierKeyChange(inputState);
            if (m_suffix != NULL)
                m_suffix->modifierKeyChange(inputState);
        }
        
        bool ToolChain::mouseDown(const InputState& inputState) {
            if (m_tool->mouseDown(inputState))
                return true;
            if (m_suffix != NULL)
                return m_suffix->mouseDown(inputState);
            return false;
        }
        
        bool ToolChain::mouseUp(const InputState& inputState) {
            if (m_tool->mouseUp(inputState))
                return true;
            if (m_suffix != NULL)
                return m_suffix->mouseUp(inputState);
            return false;
        }
        
        bool ToolChain::mouseDoubleClick(const InputState& inputState) {
            if (m_tool->mouseDoubleClick(inputState))
                return true;
            if (m_suffix != NULL)
                return m_suffix->mouseDoubleClick(inputState);
            return false;
        }
        
        void ToolChain::scroll(const InputState& inputState) {
            m_tool->scroll(inputState);
            if (m_suffix != NULL)
                m_suffix->scroll(inputState);
        }
        
        void ToolChain::mouseMove(const InputState& inputState) {
            m_tool->mouseMove(inputState);
            if (m_suffix != NULL)
                m_suffix->mouseMove(inputState);
        }
        
        Tool* ToolChain::startMouseDrag(const InputState& inputState) {
            if (m_tool->startMouseDrag(inputState))
                return m_tool;
            if (m_suffix != NULL)
                return m_suffix->startMouseDrag(inputState);
            return NULL;
        }
        
        Tool* ToolChain::dragEnter(const InputState& inputState, const String& payload) {
            if (m_tool->dragEnter(inputState, payload))
                return m_tool;
            if (m_suffix)
                return m_suffix->dragEnter(inputState, payload);
            return NULL;
        }
        
        void ToolChain::setRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {
            m_tool->setRenderOptions(inputState, renderContext);
            if (m_suffix)
                m_suffix->setRenderOptions(inputState, renderContext);
        }
        
        void ToolChain::render(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            m_tool->render(inputState, renderContext, renderBatch);
            if (m_suffix)
                m_suffix->render(inputState, renderContext, renderBatch);
        }
    }
}
