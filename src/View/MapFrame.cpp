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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "MapFrame.h"

#include "TrenchBroomApp.h"
#include "IO/FileSystem.h"
#include "View/Autosaver.h"
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
        const wxEventType MapFrame::EVT_REBUILD_MENUBAR = wxNewEventType();

        IMPLEMENT_DYNAMIC_CLASS(MapFrame, wxFrame)

        MapFrame::MapFrame() :
        wxFrame(NULL, wxID_ANY, wxT("")),
        m_frameManager(NULL),
        m_autosaver(NULL),
        m_autosaveTimer(NULL),
        m_console(NULL),
        m_navBar(NULL),
        m_mapView(NULL),
        m_menuNeedsRebuilding(false) {}

        MapFrame::MapFrame(FrameManager* frameManager, MapDocumentPtr document) :
        wxFrame(NULL, wxID_ANY, wxT("")),
        m_frameManager(NULL),
        m_autosaver(NULL),
        m_autosaveTimer(NULL),
        m_console(NULL),
        m_navBar(NULL),
        m_mapView(NULL),
        m_menuNeedsRebuilding(false) {
            Create(frameManager, document);
        }

        void MapFrame::Create(FrameManager* frameManager, MapDocumentPtr document) {
            m_frameManager = frameManager;
            m_document = document;
            m_controller = ControllerPtr(new ControllerFacade(m_document));
            m_autosaver = new Autosaver(m_document);
            
            createGui();
            createMenuBar(false);
            updateTitle();
            m_document->setParentLogger(m_console);

            bindEvents();
            bindObservers();

            m_autosaveTimer = new wxTimer(this);
            m_autosaveTimer->Start(1000);
        }

        MapFrame::~MapFrame() {
            unbindObservers();
            
            delete m_autosaveTimer;
            m_autosaveTimer = NULL;
            delete m_autosaver;
            m_autosaver = NULL;
            
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
            return m_controller->newDocument(MapDocument::DefaultWorldBounds, game);
        }
        
        bool MapFrame::openDocument(Model::GamePtr game, const IO::Path& path) {
            if (!confirmOrDiscardChanges())
                return false;
            return m_controller->openDocument(MapDocument::DefaultWorldBounds, game, path);
        }

        void MapFrame::OnClose(wxCloseEvent& event) {
            assert(m_frameManager != NULL);
            if (event.CanVeto() && !confirmOrDiscardChanges())
                event.Veto();
            else
                m_frameManager->removeAndDestroyFrame(this);
        }

        void MapFrame::OnFileSave(wxCommandEvent& event) {
            saveDocument();
        }
        
        void MapFrame::OnFileSaveAs(wxCommandEvent& event) {
            saveDocumentAs();
        }

        void MapFrame::OnFileClose(wxCommandEvent& event) {
            Close();
        }

        void MapFrame::OnEditUndo(wxCommandEvent& event) {
            m_controller->undoLastCommand();
        }
        
        void MapFrame::OnEditRedo(wxCommandEvent& event) {
            m_controller->redoNextCommand();
        }

        void MapFrame::OnEditDeleteObjects(wxCommandEvent& event) {
            const Model::ObjectList objects = m_document->selectedObjects();
            assert(!objects.empty());
            
            m_controller->beginUndoableGroup(String("Delete ") + String(objects.size() == 1 ? "object" : "objects"));
            m_controller->deselectAll();
            m_controller->removeObjects(objects);
            m_controller->closeGroup();
        }

        void MapFrame::OnEditToggleClipTool(wxCommandEvent& event) {
            m_mapView->toggleClipTool();
            updateMenuBar(m_mapView->HasFocus());
        }

        void MapFrame::OnEditToggleClipSide(wxCommandEvent& event) {
            m_mapView->toggleClipSide();
        }
        
        void MapFrame::OnEditPerformClip(wxCommandEvent& event) {
            m_mapView->performClip();
        }

        void MapFrame::OnEditToggleRotateObjectsTool(wxCommandEvent& event) {
            m_mapView->toggleRotateObjectsTool();
            updateMenuBar(m_mapView->HasFocus());
        }

        void MapFrame::OnEditToggleMovementRestriction(wxCommandEvent& event) {
            m_mapView->toggleMovementRestriction();
        }

        void MapFrame::OnUpdateUI(wxUpdateUIEvent& event) {
            switch (event.GetId()) {
                case wxID_OPEN:
                case wxID_SAVE:
                case wxID_SAVEAS:
                case wxID_CLOSE:
                    event.Enable(true);
                    break;
                case wxID_UNDO:
                    if (m_controller->hasLastCommand()) {
                        event.Enable(true);
                        event.SetText(Menu::undoShortcut().menuText(m_controller->lastCommandName()));
                    } else {
                        event.Enable(false);
                        event.SetText(Menu::undoShortcut().menuText());
                    }
                    break;
                case wxID_REDO:
                    if (m_controller->hasNextCommand()) {
                        event.Enable(true);
                        event.SetText(Menu::redoShortcut().menuText(m_controller->nextCommandName()));
                    } else {
                        event.Enable(false);
                        event.SetText(Menu::redoShortcut().menuText());
                    }
                    break;
                case wxID_DELETE:
                    event.Enable(m_document->hasSelectedObjects());
                    break;
                case CommandIds::Menu::EditToggleClipTool:
                    event.Enable(m_document->hasSelectedBrushes());
                    event.Check(m_mapView->clipToolActive());
                    break;
                case CommandIds::Menu::EditActions:
                    event.Enable(m_mapView->clipToolActive() ||
                                 m_document->hasSelectedObjects());
                    break;
                case CommandIds::Menu::EditToggleClipSide:
                    event.Enable(m_mapView->clipToolActive() && m_mapView->canToggleClipSide());
                    break;
                case CommandIds::Menu::EditPerformClip:
                    event.Enable(m_mapView->clipToolActive() && m_mapView->canPerformClip());
                    break;
                case CommandIds::Menu::EditToggleRotateObjectsTool:
                    event.Enable(m_document->hasSelectedObjects());
                    event.Check(m_mapView->rotateObjectsToolActive());
                    break;
                case CommandIds::Menu::EditToggleMovementRestriction:
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

        void MapFrame::OnRebuildMenuBar(wxEvent& event) {
            if (m_menuNeedsRebuilding)
                rebuildMenuBar();
        }

        void MapFrame::OnAutosaveTimer(wxTimerEvent& event) {
            m_autosaver->triggerAutosave(m_console);
        }

        void MapFrame::bindObservers() {
            m_document->selectionDidChangeNotifier.addObserver(this, &MapFrame::selectionDidChange);
            m_controller->commandDoneNotifier.addObserver(this, &MapFrame::commandDone);
            m_controller->commandUndoneNotifier.addObserver(this, &MapFrame::commandUndone);
        }
        
        void MapFrame::unbindObservers() {
            m_document->selectionDidChangeNotifier.removeObserver(this, &MapFrame::selectionDidChange);
            m_controller->commandDoneNotifier.removeObserver(this, &MapFrame::commandDone);
            m_controller->commandUndoneNotifier.removeObserver(this, &MapFrame::commandUndone);
        }
        
        void MapFrame::selectionDidChange(const Model::SelectionResult& result) {
            rebuildMenuBar();
        }

        void MapFrame::commandDone(Controller::Command::Ptr command) {
            if (command->modifiesDocument()) {
                m_document->incModificationCount();
                m_autosaver->updateLastModificationTime();
            }
            updateTitle();
        }
        
        void MapFrame::commandUndone(Controller::Command::Ptr command) {
            if (command->modifiesDocument())
                m_document->decModificationCount();
            updateTitle();
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
            m_inspector = new Inspector(inspectorSplitter, m_document, m_controller, m_mapView->renderResources());
            inspectorSplitter->SplitVertically(consoleSplitter, m_inspector, -350);
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(inspectorSplitter, 1, wxEXPAND);
            SetSizer(outerSizer);
        }
        
        void MapFrame::bindEvents() {
            Bind(wxEVT_CLOSE_WINDOW, &MapFrame::OnClose, this);
            
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnFileSave, this, wxID_SAVE);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnFileSaveAs, this, wxID_SAVEAS);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnFileClose, this, wxID_CLOSE);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditUndo, this, wxID_UNDO);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditRedo, this, wxID_REDO);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditDeleteObjects, this, wxID_DELETE);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditToggleClipTool, this, CommandIds::Menu::EditToggleClipTool);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditToggleClipSide, this, CommandIds::Menu::EditToggleClipSide);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditPerformClip, this, CommandIds::Menu::EditPerformClip);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditToggleRotateObjectsTool, this, CommandIds::Menu::EditToggleRotateObjectsTool);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditToggleMovementRestriction, this, CommandIds::Menu::EditToggleMovementRestriction);
            
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
            Bind(EVT_REBUILD_MENUBAR, &MapFrame::OnRebuildMenuBar, this);
            Bind(wxEVT_TIMER, &MapFrame::OnAutosaveTimer, this);
            
            m_mapView->Bind(wxEVT_SET_FOCUS, &MapFrame::OnMapViewSetFocus, this);
            m_mapView->Bind(wxEVT_KILL_FOCUS, &MapFrame::OnMapViewKillFocus, this);
        }
        
        class FrameMenuSelector : public TrenchBroom::View::MultiMenuSelector {
        private:
            const MapView* m_mapView;
            const MapDocumentPtr m_document;
        public:
            FrameMenuSelector(const MapView* mapView, const MapDocumentPtr document) :
            m_mapView(mapView),
            m_document(document) {}
            
            const Menu* select(const MultiMenu& multiMenu) const {
                if (m_mapView->clipToolActive())
                    return multiMenu.menuById(CommandIds::Menu::EditClipActions);
                if (m_document->hasSelectedObjects())
                    return multiMenu.menuById(CommandIds::Menu::EditObjectActions);
                
                return NULL;
            }
        };
        
        void MapFrame::rebuildMenuBar() {
            updateMenuBar(m_mapView->HasFocus());
            m_menuNeedsRebuilding = false;
        }
        
        void MapFrame::createMenuBar(const bool showModifiers) {
            wxMenuBar* menuBar = Menu::createMenuBar(FrameMenuSelector(m_mapView, m_document), showModifiers);
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
#ifdef __APPLE__
            SetTitle(m_document->filename());
            OSXSetModified(m_document->modified());
#else
            SetTitle(wxString(m_document->filename()) + wxString(m_document->modified() ? "*" : ""));
#endif
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
            try {
                IO::FileSystem fs;
                if (fs.exists(m_document->path())) {
                    m_document->saveDocument();
                    updateTitle();
                    logger()->info("Saved " + m_document->path().asString());
                    return true;
                }
                return saveDocumentAs();
            } catch (FileSystemException e) {
                ::wxMessageBox(e.what(), "", wxOK | wxICON_ERROR, this);
                return false;
            } catch (...) {
                ::wxMessageBox("Unknown error while saving " + m_document->path().asString(), "", wxOK | wxICON_ERROR, this);
                return false;
            }
        }
        
        bool MapFrame::saveDocumentAs() {
            try {
                wxFileDialog saveDialog(this, _("Save map file"), "", "", "Map files (*.map)|*.map", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
                if (saveDialog.ShowModal() == wxID_CANCEL)
                    return false;
                
                const IO::Path path(saveDialog.GetPath().ToStdString());
                m_document->saveDocumentAs(path);
                updateTitle();
                logger()->info("Saved " + m_document->path().asString());
                return true;
            } catch (FileSystemException e) {
                ::wxMessageBox(e.what(), "", wxOK | wxICON_ERROR, this);
                return false;
            } catch (...) {
                ::wxMessageBox("Unknown error while saving " + m_document->filename(), "", wxOK | wxICON_ERROR, this);
                return false;
            }
        }
    }
}
