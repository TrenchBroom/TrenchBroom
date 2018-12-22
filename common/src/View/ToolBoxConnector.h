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

#ifndef TrenchBroom_ToolBoxConnector
#define TrenchBroom_ToolBoxConnector

#include "StringUtils.h"
#include "View/InputState.h"
#include "View/PickRequest.h"

#include <wx/gdicmn.h>
#include <wx/longlong.h>

class wxWindow;
class wxKeyEvent;
class wxFocusEvent;
class wxMouseEvent;
class wxMouseCaptureLostEvent;

namespace TrenchBroom {
    namespace Model {
        class PickResult;
    }
    
    namespace Renderer {
        class Camera;
        class RenderBatch;
        class RenderContext;
    }

    namespace View {
        class ToolController;
        class ToolBox;
        class ToolChain;

        class ToolBoxConnector {
        private:
            wxWindow* m_window;
            ToolBox* m_toolBox;
            ToolChain* m_toolChain;
            
            InputState m_inputState;
            
            wxLongLong m_clickTime;
            wxPoint m_clickPos;
            wxPoint m_lastMousePos;
            bool m_ignoreNextDrag;
        public:
            ToolBoxConnector(wxWindow* window);
            virtual ~ToolBoxConnector();
            
            const vm::ray3& pickRay() const;
            const Model::PickResult& pickResult() const;

            void updatePickResult();
            void updateLastActivation();
        protected:
            void setToolBox(ToolBox& toolBox);
            void addTool(ToolController* tool);
        public: // drag and drop
            bool dragEnter(wxCoord x, wxCoord y, const String& text);
            bool dragMove(wxCoord x, wxCoord y, const String& text);
            void dragLeave();
            bool dragDrop(wxCoord x, wxCoord y, const String& text);
        public: // cancel
            bool cancel();
        protected: // rendering
            void setRenderOptions(Renderer::RenderContext& renderContext);
            void renderTools(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
        private:
            void bindEvents();
            void unbindEvents();

            void OnKey(wxKeyEvent& event);
            void OnMouseButton(wxMouseEvent& event);
            void OnMouseDoubleClick(wxMouseEvent& event);
            void OnMouseMotion(wxMouseEvent& event);
            void OnMouseWheel(wxMouseEvent& event);
            void OnMouseCaptureLost(wxMouseCaptureLostEvent& event);
            void OnSetFocus(wxFocusEvent& event);
            void OnKillFocus(wxFocusEvent& event);
        private:
            bool isWithinClickDistance(const wxPoint& pos) const;
            
            void startDrag(wxMouseEvent& event);
            void drag(wxMouseEvent& event);
            void endDrag(wxMouseEvent& event);
        public:
            bool cancelDrag();
        private:
            void captureMouse();
            void releaseMouse();

            
            ModifierKeyState modifierKeys();
            bool setModifierKeys();
            bool clearModifierKeys();
            void updateModifierKeys();
            
            MouseButtonState mouseButton(wxMouseEvent& event);
            void mouseMoved(const wxPoint& position);

            void showPopupMenu();
        private:
            virtual PickRequest doGetPickRequest(int x, int y) const = 0;
            virtual Model::PickResult doPick(const vm::ray3& pickRay) const = 0;
            virtual void doShowPopupMenu();
        };
    }
}

#endif /* defined(TrenchBroom_ToolBoxConnector) */
