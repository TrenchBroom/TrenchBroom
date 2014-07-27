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

#ifndef __TrenchBroom__ToolBox__
#define __TrenchBroom__ToolBox__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "StringUtils.h"
#include "Hit.h"
#include "Notifier.h"
#include "View/InputState.h"

#include <wx/wx.h>

namespace TrenchBroom {
    namespace Renderer {
        class RenderContext;
    }
    
    namespace View {
        class Tool;
        
        class ToolBoxHelper {
        public:
            virtual ~ToolBoxHelper();
            
            Ray3 pickRay(int x, int y) const;
            Hits pick(const Ray3& pickRay) const;
            
            void showPopupMenu();
        private:
            virtual Ray3 doGetPickRay(int x, int y) const = 0;
            virtual Hits doPick(const Ray3& pickRay) const = 0;
            virtual void doShowPopupMenu();
        };
        
        class ToolBox {
        private:
            wxWindow* m_window;
            ToolBoxHelper* m_helper;
            
            InputState m_inputState;

            Tool* m_toolChain;
            Tool* m_dragReceiver;
            Tool* m_modalReceiver;
            Tool* m_dropReceiver;
            Tool* m_savedDropReceiver;
            
            wxPoint m_clickDelta;
            wxPoint m_lastMousePos;
            bool m_ignoreNextDrag;
            bool m_clickToActivate;
            bool m_ignoreNextClick;
            wxDateTime m_lastActivation;
            
            bool m_enabled;
            bool m_ignoreMotionEvents;
        public:
            Notifier1<Tool*> toolActivatedNotifier;
            Notifier1<Tool*> toolDeactivatedNotifier;
        public:
            ToolBox(wxWindow* window, ToolBoxHelper* helper);
            ~ToolBox();
            
            void setClickToActivate(bool clickToActivate);
            
            const Ray3& pickRay() const;
            const Hits& hits() const;
            
            void updateHits();
            void updateLastActivation();

            bool dragEnter(wxCoord x, wxCoord y, const String& text);
            bool dragMove(wxCoord x, wxCoord y, const String& text);
            void dragLeave();
            bool dragDrop(wxCoord x, wxCoord y, const String& text);

            void OnKey(wxKeyEvent& event);
            void OnMouseButton(wxMouseEvent& event);
            void OnMouseDoubleClick(wxMouseEvent& event);
            void OnMouseMotion(wxMouseEvent& event);
            void OnMouseWheel(wxMouseEvent& event);
            void OnMouseCaptureLost(wxMouseCaptureLostEvent& event);
            void OnSetFocus(wxFocusEvent& event);
            void OnKillFocus(wxFocusEvent& event);
            
            void addTool(Tool* tool);
            
            bool anyToolActive() const;
            bool toolActive(const Tool* tool) const;
            void toggleTool(Tool* tool);
            void deactivateAllTools();

            bool enabled() const;
            void enable();
            void disable();
            
            void setRenderOptions(Renderer::RenderContext& renderContext);
            void renderTools(Renderer::RenderContext& renderContext);
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

            void bindEvents();
            void unbindEvents();
        };
    }
}

#endif /* defined(__TrenchBroom__ToolBox__) */
