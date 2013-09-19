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

#include "Utility/Preferences.h"
#include "View/DocumentViewHolder.h"

class wxDocManager;
class wxPanel;
class wxTextCtrl;

namespace TrenchBroom {
    namespace Controller {
        class Command;
    }

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
        class NavBar;

        class EditorFrame : public wxFrame {
        public:
            static const wxEventType EVT_SET_FOCUS;
        private:
            DECLARE_DYNAMIC_CLASS(EditorFrame)
        protected:
            class MenuSelector : public Preferences::MultiMenuSelector {
            private:
                DocumentViewHolder& m_documentViewHolder;
            public:
                MenuSelector(DocumentViewHolder& documentViewHolder);
                const Preferences::Menu* select(const Preferences::MultiMenu& multiMenu) const;
            };

            DocumentViewHolder m_documentViewHolder;
            Inspector* m_inspector;
            wxPanel* m_mapCanvasContainerPanel;
            NavBar* m_navBar;
            MapGLCanvas* m_mapCanvas;
            wxTextCtrl* m_logView;
            unsigned int m_focusMapCanvasOnIdle;

            void CreateGui();
        public:
            EditorFrame();
            EditorFrame(Model::MapDocument& document, EditorView& view);
            
            void Create(Model::MapDocument& document, EditorView& view);

            inline MapGLCanvas& mapCanvas() const {
                return *m_mapCanvas;
            }

            inline Inspector& inspector() const {
                return *m_inspector;
            }

            inline wxTextCtrl* logView() const {
                return m_logView;
            }

            void update(const Controller::Command& command);
            void updateMenuBar();
            void updateNavBar();
            void disableProcessing();

            void OnActivate(wxActivateEvent& event);
            void OnChangeFocus(wxCommandEvent& event);
            void OnIdle(wxIdleEvent& event);
            void OnClose(wxCloseEvent& event);
            void OnMenuOpen(wxMenuEvent& event);

            DECLARE_EVENT_TABLE()
        };
    }
}


#endif /* defined(__TrenchBroom__EditorFrame__) */
