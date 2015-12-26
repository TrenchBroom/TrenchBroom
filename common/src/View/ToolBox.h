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

#ifndef TrenchBroom_ToolBox
#define TrenchBroom_ToolBox

#include "TrenchBroom.h"
#include "VecMath.h"
#include "StringUtils.h"
#include "Notifier.h"

#include <wx/wx.h>

#include <map>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class PickResult;
    }
    
    namespace Renderer {
        class RenderBatch;
        class RenderContext;
    }
    
    namespace View {
        class InputState;
        class Tool;
        class ToolAdapter;
        class ToolChain;
        
        class ToolBox {
        private:
            ToolAdapter* m_dragReceiver;
            ToolAdapter* m_dropReceiver;
            ToolAdapter* m_savedDropReceiver;
            Tool* m_modalTool;
            
            typedef std::vector<Tool*> ToolList;
            typedef std::map<Tool*, ToolList> ToolMap;
            ToolMap m_deactivateWhen;
            
            typedef std::vector<wxWindow*> WindowList;
            WindowList m_focusGroup;
            
            bool m_clickToActivate;
            bool m_ignoreNextClick;
            wxDateTime m_lastActivation;
            
            bool m_enabled;
        public:
            Notifier1<Tool*> toolActivatedNotifier;
            Notifier1<Tool*> toolDeactivatedNotifier;
            Notifier1<Tool*> refreshViewsNotifier;
        public:
            ToolBox();
        public: // focus window management
            void addWindow(wxWindow* window);
            void removeWindow(wxWindow* window);
        private:
            void OnSetFocus(wxFocusEvent& event);
            void OnKillFocus(wxFocusEvent& event);
            void OnEnterWindow(wxMouseEvent& event);
            void OnLeaveWindow(wxMouseEvent& event);
            void setFocusCursor();
            void clearFocusCursor();
        protected:
            void addTool(Tool* tool);
        public: // picking
            void pick(ToolChain* chain, const InputState& inputState, Model::PickResult& pickResult);
        public: // event handling
            bool clickToActivate() const;
            void setClickToActivate(bool clickToActivate);
            void updateLastActivation();
            
            bool ignoreNextClick() const;
            void clearIgnoreNextClick();
            
            bool dragEnter(ToolChain* chain, const InputState& inputState, const String& text);
            bool dragMove(ToolChain* chain, const InputState& inputState, const String& text);
            void dragLeave(ToolChain* chain, const InputState& inputState);
            bool dragDrop(ToolChain* chain, const InputState& inputState, const String& text);

            void modifierKeyChange(ToolChain* chain, const InputState& inputState);
            void mouseDown(ToolChain* chain, const InputState& inputState);
            void mouseUp(ToolChain* chain, const InputState& inputState);
            bool mouseClick(ToolChain* chain, const InputState& inputState);
            void mouseDoubleClick(ToolChain* chain, const InputState& inputState);
            void mouseMove(ToolChain* chain, const InputState& inputState);
            
            bool dragging() const;
            bool startMouseDrag(ToolChain* chain, const InputState& inputState);
            bool mouseDrag(const InputState& inputState);
            void endMouseDrag(const InputState& inputState);
            void cancelDrag();
            
            void mouseScroll(ToolChain* chain, const InputState& inputState);

            bool cancel(ToolChain* chain);
        public: // tool management
            void deactivateWhen(Tool* master, Tool* slave);
            
            bool anyToolActive() const;
            bool toolActive(const Tool* tool) const;
            void toggleTool(Tool* tool);
            void deactivateAllTools();

            bool enabled() const;
            void enable();
            void disable();
        public: // rendering
            void setRenderOptions(ToolChain* chain, const InputState& inputState, Renderer::RenderContext& renderContext);
            void renderTools(ToolChain* chain, const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
        private:
            bool activateTool(Tool* tool);
            void deactivateTool(Tool* tool);
        };
    }
}

#endif /* defined(TrenchBroom_ToolBox) */
