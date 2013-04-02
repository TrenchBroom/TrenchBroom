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

#ifndef __TrenchBroom__EditorFrame__
#define __TrenchBroom__EditorFrame__

#include <wx/frame.h>

#include "View/DocumentViewHolder.h"

class wxDocManager;
class wxPanel;
class wxStaticText;
class wxTextCtrl;

namespace TrenchBroom {
    namespace Model {
        class MapDocument;
    }
    
    namespace Renderer {
        class Camera;
        class MapRenderer;
    }
    
    namespace View {
        class EditorView;
        class Inspector;
        class MapGLCanvas;
        
        class EditorFrame : public wxFrame {
        public:
            static const wxEventType EVT_SET_FOCUS;
        protected:
            DocumentViewHolder m_documentViewHolder;
            Inspector* m_inspector;
            wxPanel* m_canvasPanel;
            wxPanel* m_navContainerPanel;
            wxPanel* m_navPanel;
            MapGLCanvas* m_mapCanvas;
            wxTextCtrl* m_logView;
            bool m_mapCanvasHasFocus;
            bool m_focusMapCanvasOnIdle;
            
            void CreateGui();
            wxStaticText* makeBreadcrump(const wxString& text, bool link);
        public:
            EditorFrame(Model::MapDocument& document, EditorView& view);
            
            void updateNavigation();
            
            inline MapGLCanvas& mapCanvas() const {
                return *m_mapCanvas;
            }

            inline Inspector& inspector() const {
                return *m_inspector;
            }
            
            inline wxTextCtrl* logView() const {
                return m_logView;
            }
            
            void updateMenuBar();
            void disableProcessing();

            /*
            void OnMapCanvasSetFocus(wxFocusEvent& event);
            void OnMapCanvasKillFocus(wxFocusEvent& event);
             */
            void OnSetFocus(wxCommandEvent& event);
            void OnIdle(wxIdleEvent& event);

            void OnClose(wxCloseEvent& event);
            
            void OnMenuOpen(wxMenuEvent& event);            
        
            DECLARE_EVENT_TABLE()
        };
    }
}


#endif /* defined(__TrenchBroom__EditorFrame__) */
