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

#include "EditorFrame.h"

#include "Controller/Command.h"
#include "Controller/EntityPropertyCommand.h"
#include "Controller/PreferenceChangeEvent.h"
#include "Controller/InputController.h"
#include "Model/Brush.h"
#include "Model/EditStateManager.h"
#include "Model/Entity.h"
#include "Model/MapDocument.h"
#include "Utility/Console.h"
#include "Utility/List.h"
#include "View/CommandIds.h"
#include "View/EditorView.h"
#include "View/Inspector.h"
#include "View/MapGLCanvas.h"
#include "View/NavBar.h"
#include "TrenchBroomApp.h"

#include <wx/colour.h>
#include <wx/config.h>
#include <wx/display.h>
#include <wx/docview.h>
#include <wx/menu.h>
#include <wx/panel.h>
#include <wx/srchctrl.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/splitter.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/toolbar.h>

namespace TrenchBroom {
    namespace View {
        const wxEventType EditorFrame::EVT_SET_FOCUS = wxNewEventType();

        IMPLEMENT_DYNAMIC_CLASS(EditorFrame, wxFrame)

        BEGIN_EVENT_TABLE(EditorFrame, wxFrame)
		EVT_CLOSE(EditorFrame::OnClose)
        EVT_COMMAND(wxID_ANY, EVT_SET_FOCUS, EditorFrame::OnChangeFocus)
        EVT_ACTIVATE(EditorFrame::OnActivate)
        EVT_IDLE(EditorFrame::OnIdle)
        EVT_MENU_OPEN(EditorFrame::OnMenuOpen)
		END_EVENT_TABLE()

        EditorFrame::MenuSelector::MenuSelector(DocumentViewHolder& documentViewHolder) :
        m_documentViewHolder(documentViewHolder) {}

        const Preferences::Menu* EditorFrame::MenuSelector::select(const Preferences::MultiMenu& multiMenu) const {
            if (m_documentViewHolder.valid()) {
                Controller::InputController& inputController = m_documentViewHolder.view().inputController();
                Model::EditStateManager& editStateManager = m_documentViewHolder.document().editStateManager();

                switch (multiMenu.menuId()) {
                    case View::CommandIds::Menu::EditActions:
                        if (inputController.moveVerticesToolActive())
                            return multiMenu.menuById(View::CommandIds::Menu::EditVertexActions);
                        if (inputController.clipToolActive())
                            return multiMenu.menuById(View::CommandIds::Menu::EditClipActions);
                        switch (editStateManager.selectionMode()) {
                            case Model::EditStateManager::SMFaces:
                                return multiMenu.menuById(View::CommandIds::Menu::EditFaceActions);
                            case Model::EditStateManager::SMEntities:
                            case Model::EditStateManager::SMBrushes:
                            case Model::EditStateManager::SMEntitiesAndBrushes:
                                return multiMenu.menuById(View::CommandIds::Menu::EditObjectActions);
                            default:
                                break;
                        }
                        break;
                    default:
                        break;
                }
            }

            return NULL;
        }

        void EditorFrame::CreateGui() {
            /*
             * Structure:
             * inspectorSplitter
             *   logSplitter
             *     mapCanvasContainerPanel
             *       navBar
             *       mapCanvas
             *     logView
             *   inspector
             */

            wxSplitterWindow* inspectorSplitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE);
            inspectorSplitter->SetSashGravity(1.0f);
            inspectorSplitter->SetMinimumPaneSize(350);

            wxSplitterWindow* logSplitter = new wxSplitterWindow(inspectorSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE);
            logSplitter->SetSashGravity(1.0f);
            logSplitter->SetMinimumPaneSize(0);

            m_logView = new wxTextCtrl(logSplitter, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP | wxTE_RICH2);
            m_logView->SetDefaultStyle(wxTextAttr(*wxLIGHT_GREY, *wxBLACK));
            m_logView->SetBackgroundColour(*wxBLACK);

            m_mapCanvasContainerPanel = new wxPanel(logSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize,
#ifdef _WIN32
                                                    wxBORDER_SUNKEN
#else
                                                    wxBORDER_NONE
#endif
                                                    );
            m_navBar = new NavBar(m_mapCanvasContainerPanel, m_documentViewHolder);
            m_mapCanvas = new MapGLCanvas(m_mapCanvasContainerPanel, m_documentViewHolder);

            wxSizer* mapCanvasContainerSizer = new wxBoxSizer(wxVERTICAL);
            mapCanvasContainerSizer->Add(m_navBar, 0, wxEXPAND);
            mapCanvasContainerSizer->Add(m_mapCanvas, 1, wxEXPAND);
            m_mapCanvasContainerPanel->SetSizer(mapCanvasContainerSizer);

            logSplitter->SplitHorizontally(m_mapCanvasContainerPanel, m_logView, -100);
            m_inspector = new Inspector(inspectorSplitter, m_documentViewHolder);
            inspectorSplitter->SplitVertically(logSplitter, m_inspector, -350);

            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(inspectorSplitter, 1, wxEXPAND);
            SetSizer(outerSizer);
        }

        EditorFrame::EditorFrame() :
        wxFrame(NULL, wxID_ANY, wxT("")),
        m_inspector(NULL),
        m_mapCanvasContainerPanel(NULL),
        m_navBar(NULL),
        m_mapCanvas(NULL),
        m_logView(NULL),
        m_focusMapCanvasOnIdle(2) {}

        EditorFrame::EditorFrame(Model::MapDocument& document, EditorView& view) :
        wxFrame(NULL, wxID_ANY, wxT("")),
        m_documentViewHolder(DocumentViewHolder(&document, &view)),
        m_inspector(NULL),
        m_mapCanvasContainerPanel(NULL),
        m_navBar(NULL),
        m_mapCanvas(NULL),
        m_logView(NULL),
        m_focusMapCanvasOnIdle(2) {
            Create(document, view);
        }

        void EditorFrame::Create(Model::MapDocument& document, EditorView& view) {
#ifdef _WIN32
            SetIcon(wxICON(APPICON));
#endif
            CreateGui();
            
            const wxDisplay display;
            const wxRect displaySize = display.GetClientArea();
            SetSize(std::min(displaySize.width, 1024), std::min(displaySize.height, 768));
            EditorFrame* topWindow = wxDynamicCast(wxTheApp->GetTopWindow(), EditorFrame);
            if (topWindow != NULL) {
                wxPoint position = topWindow->GetPosition();
#ifdef __APPLE__
                position.x += 23;
                position.y += 23;
#elif defined _WIN32
                position.x += 23;
                position.y += 23;
#else
                position.x += 23;
                position.y += 23;
#endif
                SetPosition(position);
            } else {
                Center();
            }
            Raise();
        }

        void EditorFrame::update(const Controller::Command& command) {
            switch (command.type()) {
                case Controller::Command::LoadMap:
                case Controller::Command::ClearMap:
                case Controller::Command::ChangeEditState:
                    updateMenuBar();
                case Controller::Command::MoveVertices:
                case Controller::Command::ReparentBrushes:
                case Controller::Command::ClipToolChange:
                case Controller::Command::MoveVerticesToolChange:
                case Controller::Command::SetEntityDefinitionFile:
                    updateNavBar();
                    break;
                case Controller::Command::SetEntityPropertyKey:
                case Controller::Command::SetEntityPropertyValue:
                case Controller::Command::RemoveEntityProperty: {
                    const Controller::EntityPropertyCommand& entityPropertyCommand = static_cast<const Controller::EntityPropertyCommand&>(command);
                    if (entityPropertyCommand.isPropertyAffected(Model::Entity::ModKey))
                        updateNavBar();
                    break;
                }
                case Controller::Command::PreferenceChange: {
                    const Controller::PreferenceChangeEvent& preferenceChangeEvent = static_cast<const Controller::PreferenceChangeEvent&>(command);
                    if (preferenceChangeEvent.menuHasChanged())
                        updateMenuBar();
                    break;
                }
                default:
                    break;
            }
        }

        void EditorFrame::updateMenuBar() {
            if (!m_documentViewHolder.valid())
                return;

            TrenchBroomApp* app = static_cast<TrenchBroomApp*>(wxTheApp);
            wxMenuBar* menuBar = app->CreateMenuBar(MenuSelector(m_documentViewHolder), &m_documentViewHolder.view(), m_mapCanvas->hasFocus());
            int editMenuIndex = menuBar->FindMenu(wxT("Edit"));
            assert(editMenuIndex != wxNOT_FOUND);
            wxMenu* editMenu = menuBar->GetMenu(static_cast<size_t>(editMenuIndex));
            m_documentViewHolder.document().GetCommandProcessor()->SetEditMenu(editMenu);

            wxMenuBar* oldMenuBar = GetMenuBar();
            app->DetachFileHistoryMenu(oldMenuBar);

            SetMenuBar(menuBar);
            delete oldMenuBar;
        }

        void EditorFrame::updateNavBar() {
            if (!m_documentViewHolder.valid())
                return;
            m_navBar->updateBreadcrump();
        }

        void EditorFrame::disableProcessing() {
            wxMenuBar* oldMenuBar = GetMenuBar();
            TrenchBroomApp* app = static_cast<TrenchBroomApp*>(wxTheApp);
            app->DetachFileHistoryMenu(oldMenuBar);

            m_documentViewHolder.invalidate();
        }

        void EditorFrame::OnActivate(wxActivateEvent& event) {
            m_mapCanvas->setHasFocus(event.GetActive(), true);
            m_mapCanvas->Refresh();
            event.Skip();
        }

        void EditorFrame::OnChangeFocus(wxCommandEvent& event) {
            if (m_documentViewHolder.valid()) {
                wxWindow* focus = FindFocus();
                if (m_mapCanvas->setHasFocus(m_mapCanvas == focus))
                    updateMenuBar();
            }
        }

        void EditorFrame::OnIdle(wxIdleEvent& event) {
            if (m_focusMapCanvasOnIdle > 0) {
                m_mapCanvas->SetFocus();
                m_mapCanvas->setHasFocus(true, true);
                updateMenuBar();
                updateNavBar();
                m_focusMapCanvasOnIdle--;
            }

            // FIXME: Workaround for a bug in Ubuntu GTK where menus are not updated
            // This will be fixed in wxWidgets 2.9.5: http://trac.wxwidgets.org/ticket/14302
            // Unfortunately right now this leads to a crash after the "Navigate Up" item is invoked.
#ifdef __linux__
            wxMenuBar* menuBar = GetMenuBar();
            size_t menuCount = menuBar->GetMenuCount();
            for (size_t i = 0; i < menuCount; i++) {
                wxMenu* menu = menuBar->GetMenu(i);
                menu->UpdateUI(&m_documentViewHolder.view());
            }
#endif

            if (IsActive() && wxTheApp->GetTopWindow() != this)
                wxTheApp->SetTopWindow(this);
            event.Skip();
        }

        void EditorFrame::OnClose(wxCloseEvent& event) {
            // if the user closes the editor frame, the document must also be closed:
            assert(m_documentViewHolder.valid());

            Model::MapDocument& document = m_documentViewHolder.document();
            document.GetDocumentManager()->CloseDocument(&document);
        }

        void EditorFrame::OnMenuOpen(wxMenuEvent& event) {
            // FIXME is this still necessary?
            // How does updating menu items work in Windows, anyway? 
#ifdef _WIN32
            wxMenuBar* menuBar = GetMenuBar();
            size_t menuCount = menuBar->GetMenuCount();
            for (size_t i = 0; i < menuCount; i++) {
                wxMenu* menu = menuBar->GetMenu(i);
                menu->UpdateUI(&m_documentViewHolder.view());
            }
#endif
            event.Skip();
        }
    }
}
