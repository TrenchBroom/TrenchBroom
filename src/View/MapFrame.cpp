/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "MapFrame.h"

#include "TrenchBroomApp.h"
#include "View/CommandIds.h"
#include "View/Console.h"
#include "View/FrameManager.h"
#include "View/MapView.h"
#include "View/Menu.h"
#include "View/NavBar.h"

#include <wx/display.h>
#include <wx/sizer.h>
#include <wx/splitter.h>
#include <wx/textctrl.h>

namespace TrenchBroom {
    namespace View {
        IMPLEMENT_DYNAMIC_CLASS(MapFrame, wxFrame)

        MapFrame::MapFrame() :
        wxFrame(NULL, wxID_ANY, wxT("unnamed.map")),
        m_frameManager(NULL),
        m_console(NULL),
        m_navBar(NULL),
        m_mapView(NULL) {}

        MapFrame::MapFrame(FrameManager* frameManager, MapDocument::Ptr document) :
        wxFrame(NULL, wxID_ANY, wxT("unnamed.map")),
        m_frameManager(NULL),
        m_console(NULL),
        m_navBar(NULL),
        m_mapView(NULL) {
            Create(frameManager, document);
        }

        void MapFrame::Create(FrameManager* frameManager, MapDocument::Ptr document) {
            m_frameManager = frameManager;
            m_document = document;
            
            createMenuBar();
            createGui();
            updateTitle();

            Bind(wxEVT_CLOSE_WINDOW, &MapFrame::OnClose, this);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnFileClose, this, wxID_CLOSE);

            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, wxID_SAVE);
            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, wxID_SAVEAS);
            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, wxID_CLOSE);
            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, wxID_UNDO);
            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, wxID_REDO);
            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, wxID_CUT);
            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, wxID_COPY);
            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, wxID_PASTE);
            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, wxID_DELETE);
            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, CommandIds::Menu::Lowest, CommandIds::Menu::Highest);
        }

        void MapFrame::positionOnScreen(wxFrame* reference) {
            const wxDisplay display;
            const wxRect displaySize = display.GetClientArea();
            if (reference == NULL) {
                SetSize(std::min(displaySize.width, 1024), std::min(displaySize.height, 768));
                CenterOnScreen();
            } else {
                wxPoint position = reference->GetPosition();
                position.x += 23;
                position.y += 23;
                
                if (displaySize.GetBottom() - position.x < 100 ||
                    displaySize.GetRight() - position.y < 70)
                    position = displaySize.GetTopLeft();
                
                SetPosition(position);
                SetSize(std::min(displaySize.GetRight() - position.x, 1024), std::min(displaySize.GetBottom() - position.y, 768));
            }
        }

        bool MapFrame::newDocument(Model::Game::Ptr game) {
            if (!confirmOrDiscardChanges())
                return false;
            m_console->info("Creating new document");
            m_mapView->makeCurrent();
            return m_controller.newDocument(m_document, game);
        }
        
        bool MapFrame::openDocument(Model::Game::Ptr game, const IO::Path& path) {
            if (!confirmOrDiscardChanges())
                return false;
            m_console->info("Opening document " + path.asString());
            m_mapView->makeCurrent();
            return m_controller.openDocument(m_document, game, path);
        }

        void MapFrame::OnClose(wxCloseEvent& event) {
            assert(m_frameManager != NULL);
            m_frameManager->removeAndDestroyFrame(this);
        }

        Console* MapFrame::console() const {
            return m_console;
        }

        void MapFrame::OnFileClose(wxCommandEvent& event) {
            Close();
        }

        void MapFrame::OnUpdateUI(wxUpdateUIEvent& event) {
            switch (event.GetId()) {
                case wxID_CLOSE:
                    event.Enable(true);
                    break;
                default:
                    event.Enable(false);
                    break;
            }
        }

        void MapFrame::createGui() {
            wxSplitterWindow* inspectorSplitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE);
            inspectorSplitter->SetSashGravity(1.0f);
            inspectorSplitter->SetMinimumPaneSize(350);
            
            wxSplitterWindow* consoleSplitter = new wxSplitterWindow(inspectorSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE);
            consoleSplitter->SetSashGravity(1.0f);
            consoleSplitter->SetMinimumPaneSize(0);
            
            m_console = new Console(consoleSplitter);

            wxPanel* container = new wxPanel(consoleSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize,
#ifdef _WIN32
                                             wxBORDER_SUNKEN
#else
                                             wxBORDER_NONE
#endif
                                             );
            
            m_navBar = new NavBar(container);
            m_mapView = new MapView(container, *m_console);

            wxSizer* containerSizer = new wxBoxSizer(wxVERTICAL);
            containerSizer->Add(m_navBar, 0, wxEXPAND);
            containerSizer->Add(m_mapView, 1, wxEXPAND);
            container->SetSizer(containerSizer);
            
            consoleSplitter->SplitHorizontally(container, m_console, -100);
            wxPanel* inspector = new wxPanel(inspectorSplitter);
            // m_inspector = new Inspector(inspectorSplitter, m_documentViewHolder);
            inspectorSplitter->SplitVertically(consoleSplitter, inspector, -350);
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(inspectorSplitter, 1, wxEXPAND);
            SetSizer(outerSizer);
        }
        
        void MapFrame::createMenuBar() {
            wxMenuBar* menuBar = Menu::createMenuBar(TrenchBroom::View::NullMenuSelector(), false);
            SetMenuBar(menuBar);
        }

        void MapFrame::updateTitle() {
            SetTitle(m_document->filename());
        }

        bool MapFrame::confirmOrDiscardChanges() {
            if (!m_document->modified())
                return true;
            const int result = ::wxMessageBox(m_document->filename() + " has been modified. Do you want to save the changes?", "", wxYES_NO | wxCANCEL, this);
            switch (result) {
                case wxYES:
                    return saveDocument();
                case wxNO:
                    return true;
                default:
                    return false;
            }
        }

        bool MapFrame::saveDocument() {
            return true;
        }
        
        bool MapFrame::saveDocumentAs(const IO::Path& path) {
            return true;
        }
    }
}
