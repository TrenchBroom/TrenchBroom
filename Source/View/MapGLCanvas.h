/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__MapGLCanvas__
#define __TrenchBroom__MapGLCanvas__

#include "Controller/Input.h"

// glew must be included before glcanvas
#include <GL/glew.h>
#include <wx/dnd.h>
#include <wx/glcanvas.h>
#include <wx/timer.h>

namespace TrenchBroom {
    namespace Controller {
        class InputController;
    }
    
    namespace Model {
        class MapDocument;
    }
    
    namespace Renderer {
        class Camera;
        class MapRenderer;
        class OverlayRenderer;
        class Vbo;
    }
    
    namespace Utility {
        class Console;
    }
    
    namespace View {
        class DocumentViewHolder;
        class EditorView;
        class MapGLCanvas;
        
        class MapGLCanvasDropTarget : public wxTextDropTarget {
        protected:
            MapGLCanvas* m_view;
            Controller::InputController& m_inputController;
        public:
            MapGLCanvasDropTarget(MapGLCanvas* view, Controller::InputController& inputController) :
            wxTextDropTarget(),
            m_view(view),
            m_inputController(inputController) {}
            
            wxDragResult OnEnter(wxCoord x, wxCoord y, wxDragResult def);
            wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult def);
            void OnLeave();
            bool OnDropText(wxCoord x, wxCoord y, const wxString& data);
        };
        
        class MapGLCanvas : public wxGLCanvas {
        protected:
            DocumentViewHolder& m_documentViewHolder;
            
            wxGLContext* m_glContext;
            Renderer::Vbo* m_vbo;
            Controller::InputController* m_inputController;
            Renderer::OverlayRenderer* m_overlayRenderer;
            
            bool m_hasFocus;
            bool m_ignoreNextClick;

            bool handleModifierKey(int keyCode, bool down);
        public:
            MapGLCanvas(wxWindow* parent, DocumentViewHolder& documentViewHolder);
            ~MapGLCanvas();
            
            inline Controller::InputController& inputController() const {
                return *m_inputController;
            }
            
            inline bool hasFocus() const {
                return m_hasFocus;
            }
            
            void ignoreNextClick();
            bool setHasFocus(bool hasFocus, bool dontIgnoreNextClick = false);
            void resetModifierKeys();
            void clearModifierKeys();
            void updateMenuBar();
            
            void OnPaint(wxPaintEvent& event);
            void OnKeyDown(wxKeyEvent& event);
            void OnKeyUp(wxKeyEvent& event);
            void OnMouseLeftDown(wxMouseEvent& event);
            void OnMouseLeftUp(wxMouseEvent& event);
            void OnMouseLeftDClick(wxMouseEvent& event);
            void OnMouseRightDown(wxMouseEvent& event);
            void OnMouseRightUp(wxMouseEvent& event);
            void OnMouseRightDClick(wxMouseEvent& event);
            void OnMouseMiddleDown(wxMouseEvent& event);
            void OnMouseMiddleUp(wxMouseEvent& event);
            void OnMouseMiddleDClick(wxMouseEvent& event);
            void OnMouseMove(wxMouseEvent& event);
            void OnMouseWheel(wxMouseEvent& event);
            void OnMouseCaptureLost(wxMouseCaptureLostEvent& event);

            DECLARE_EVENT_TABLE()
        };
    }
}

#endif /* defined(__TrenchBroom__MapGLCanvas__) */
