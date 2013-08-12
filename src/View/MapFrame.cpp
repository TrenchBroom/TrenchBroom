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
#include "Controller/Command.h"
#include "Controller/NewDocumentCommand.h"
#include "Controller/OpenDocumentCommand.h"
#include "View/CommandIds.h"
#include "View/Console.h"
#include "View/FrameManager.h"
#include "View/Inspector.h"
#include "View/MapDocument.h"
#include "View/MapView.h"
#include "View/Menu.h"
#include "View/NavBar.h"

#include <wx/display.h>
#include <wx/sizer.h>
#include <wx/splitter.h>
#include <wx/textctrl.h>

namespace TrenchBroom {
    namespace View {
        const wxEventType MapFrame::EVT_REBUILD_MENU = wxNewEventType();

        IMPLEMENT_DYNAMIC_CLASS(MapFrame, wxFrame)

        MapFrame::MapFrame() :
        wxFrame(NULL, wxID_ANY, wxT("")),
        m_frameManager(NULL),
        m_console(NULL),
        m_navBar(NULL),
        m_mapView(NULL),
        m_menuNeedsRebuilding(false) {}

        MapFrame::MapFrame(FrameManager* frameManager, MapDocumentPtr document) :
        wxFrame(NULL, wxID_ANY, wxT("")),
        m_frameManager(NULL),
        m_console(NULL),
        m_navBar(NULL),
        m_mapView(NULL),
        m_menuNeedsRebuilding(false) {
            Create(frameManager, document);
        }

        void MapFrame::Create(FrameManager* frameManager, MapDocumentPtr document) {
            m_frameManager = frameManager;
            m_document = document;
            m_controller.setDocument(m_document);
            m_controller.addCommandListener(this);
            
            createGui();
            createMenuBar(false);
            updateTitle();
            m_document->setParentLogger(m_console);

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
            Bind(EVT_REBUILD_MENU, &MapFrame::OnRebuildMenu, this);
            
            m_mapView->Bind(wxEVT_SET_FOCUS, &MapFrame::OnMapViewSetFocus, this);
            m_mapView->Bind(wxEVT_KILL_FOCUS, &MapFrame::OnMapViewKillFocus, this);
        }

        MapFrame::~MapFrame() {
            m_controller.removeCommandListener(this);
            View::TrenchBroomApp* app = static_cast<View::TrenchBroomApp*>(wxTheApp);
            if (app != NULL)
                app->removeRecentDocumentMenu(Menu::findRecentDocumentsMenu(GetMenuBar()));
        }

        Logger* MapFrame::logger() const {
            return m_console;
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

        bool MapFrame::newDocument(Model::GamePtr game) {
            if (!confirmOrDiscardChanges())
                return false;
            return m_controller.newDocument(MapDocument::DefaultWorldBounds, game);
        }
        
        bool MapFrame::openDocument(Model::GamePtr game, const IO::Path& path) {
            if (!confirmOrDiscardChanges())
                return false;
            return m_controller.openDocument(MapDocument::DefaultWorldBounds, game, path);
        }

        void MapFrame::OnClose(wxCloseEvent& event) {
            assert(m_frameManager != NULL);
            if (event.CanVeto() && !confirmOrDiscardChanges())
                event.Veto();
            else
                m_frameManager->removeAndDestroyFrame(this);
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

        void MapFrame::OnMapViewSetFocus(wxFocusEvent& event) {
            m_menuNeedsRebuilding = true;
            event.Skip();
        }
        
        void MapFrame::OnMapViewKillFocus(wxFocusEvent& event) {
            m_menuNeedsRebuilding = true;
            event.Skip();
        }

        void MapFrame::OnRebuildMenu(wxEvent& event) {
            if (m_menuNeedsRebuilding) {
                updateMenuBar(m_mapView->HasFocus());
                m_menuNeedsRebuilding = false;
            }
        }

        void MapFrame::commandDo(Controller::Command::Ptr command) {
            m_document->commandDo(command);
            m_mapView->commandDo(command);
        }

        void MapFrame::commandDone(Controller::Command::Ptr command) {
            m_document->commandDone(command);
            m_mapView->commandDone(command);
            m_inspector->update(command);

            if (command->type() == Controller::NewDocumentCommand::Type ||
                command->type() == Controller::OpenDocumentCommand::Type)
                updateTitle();
        }
        
        void MapFrame::commandDoFailed(Controller::Command::Ptr command) {
            m_document->commandDoFailed(command);
            m_mapView->commandDoFailed(command);
        }
        
        void MapFrame::commandUndo(Controller::Command::Ptr command) {
            m_document->commandUndo(command);
            m_mapView->commandUndo(command);
        }

        void MapFrame::commandUndone(Controller::Command::Ptr command) {
            m_document->commandUndone(command);
            m_mapView->commandUndone(command);
            m_inspector->update(command);
        }

        void MapFrame::commandUndoFailed(Controller::Command::Ptr command) {
            m_document->commandUndoFailed(command);
            m_mapView->commandUndoFailed(command);
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
            m_mapView = new MapView(container, m_console, m_document, m_controller);

            wxSizer* containerSizer = new wxBoxSizer(wxVERTICAL);
            containerSizer->Add(m_navBar, 0, wxEXPAND);
            containerSizer->Add(m_mapView, 1, wxEXPAND);
            container->SetSizer(containerSizer);
            
            consoleSplitter->SplitHorizontally(container, m_console, -100);
            m_inspector = new Inspector(inspectorSplitter, m_document, m_controller);
            inspectorSplitter->SplitVertically(consoleSplitter, m_inspector, -350);
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(inspectorSplitter, 1, wxEXPAND);
            SetSizer(outerSizer);
        }
        
        void MapFrame::createMenuBar(const bool showModifiers) {
            wxMenuBar* menuBar = Menu::createMenuBar(TrenchBroom::View::NullMenuSelector(), showModifiers);
            SetMenuBar(menuBar);
            
            View::TrenchBroomApp* app = static_cast<View::TrenchBroomApp*>(wxTheApp);
            if (app != NULL)
                app->addRecentDocumentMenu(Menu::findRecentDocumentsMenu(menuBar));
        }
        
        void MapFrame::updateMenuBar(const bool showModifiers) {
            wxMenuBar* oldMenuBar = GetMenuBar();
            View::TrenchBroomApp* app = static_cast<View::TrenchBroomApp*>(wxTheApp);
            if (app != NULL)
                app->removeRecentDocumentMenu(Menu::findRecentDocumentsMenu(oldMenuBar));
            createMenuBar(showModifiers);
            delete oldMenuBar;
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
