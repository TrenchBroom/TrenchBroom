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

#ifndef __TrenchBroom__ToolBoxConnector__
#define __TrenchBroom__ToolBoxConnector__

#include "View/InputState.h"
#include "View/PickRequest.h"

#include <wx/wx.h>

namespace TrenchBroom {
    namespace Renderer {
        class Camera;
        class RenderBatch;
        class RenderContext;
    }

    namespace View {
        class ToolBox;

        class ToolBoxConnector {
        private:
            wxWindow* m_window;
            ToolBox& m_toolBox;
            
            InputState m_inputState;
            
            wxPoint m_clickPos;
            wxPoint m_lastMousePos;
            bool m_ignoreNextDrag;
            bool m_clickToActivate;
            bool m_ignoreNextClick;
            wxDateTime m_lastActivation;
        public:
            ToolBoxConnector(wxWindow* window, ToolBox& toolBox, InputSource inputSource);
            virtual ~ToolBoxConnector();
            
            void setClickToActivate(bool clickToActivate);
            
            const Ray3& pickRay() const;
            const Hits& hits() const;

            void updateHits();
            void updateLastActivation();
        public: // drag and drop
            bool dragEnter(wxCoord x, wxCoord y, const String& text);
            bool dragMove(wxCoord x, wxCoord y, const String& text);
            void dragLeave();
            bool dragDrop(wxCoord x, wxCoord y, const String& text);
            
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
            void captureMouse();
            void releaseMouse();
            void cancelDrag();
            
            ModifierKeyState modifierKeys();
            bool updateModifierKeys();
            bool clearModifierKeys();
            MouseButtonState mouseButton(wxMouseEvent& event);
            void mouseMoved(const wxPoint& position);

            void showPopupMenu();
        private:
            virtual PickRequest doGetPickRequest(int x, int y) const = 0;
            virtual Hits doPick(const Ray3& pickRay) const = 0;
            virtual void doShowPopupMenu();
        };
    }
}

#endif /* defined(__TrenchBroom__ToolBoxConnector__) */
