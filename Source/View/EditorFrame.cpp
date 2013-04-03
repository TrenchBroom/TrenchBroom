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

#include "Controller/InputController.h"
#include "Model/EditStateManager.h"
#include "Model/MapDocument.h"
#include "Utility/Console.h"
#include "View/EditorView.h"
#include "View/Inspector.h"
#include "View/MapGLCanvas.h"
#include "View/CommandIds.h"
#include "TrenchBroomApp.h"

#include <wx/colour.h>
#include <wx/config.h>
#include <wx/docview.h>
#include <wx/menu.h>
#include <wx/panel.h>
#include <wx/srchctrl.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/splitter.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/toolbar.h>

namespace TrenchBroom {
    namespace View {
        const wxEventType EditorFrame::EVT_SET_FOCUS = wxNewEventType();
        
        BEGIN_EVENT_TABLE(EditorFrame, wxFrame)
		EVT_CLOSE(EditorFrame::OnClose)
        EVT_MENU_OPEN(EditorFrame::OnMenuOpen)
        EVT_COMMAND(wxID_ANY, EVT_SET_FOCUS, EditorFrame::OnSetFocus)
        EVT_IDLE(EditorFrame::OnIdle)
		END_EVENT_TABLE()

        void EditorFrame::PaintNavContainer(wxPaintEvent& event) {
            wxPaintDC dc(m_navContainerPanel);
            wxRect rect = m_navContainerPanel->GetClientRect();
            rect.height -= 1;
            dc.GradientFillLinear(m_navContainerPanel->GetClientRect(), wxColour(211, 211, 211), wxColour(174, 174, 174), wxDOWN);
            dc.SetPen(wxPen(wxColour(67, 67, 67)));
            dc.DrawLine(0, rect.height, rect.width, rect.height);
            dc.DrawLine(rect.width - 1, 0, rect.width - 1, rect.height);
        }

        void EditorFrame::CreateGui() {
            wxSplitterWindow* inspectorSplitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE);
            inspectorSplitter->SetSashGravity(1.0f);
            inspectorSplitter->SetMinimumPaneSize(350);

            wxSplitterWindow* logSplitter = new wxSplitterWindow(inspectorSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE);
            logSplitter->SetSashGravity(1.0f);
            logSplitter->SetMinimumPaneSize(0);

            m_logView = new wxTextCtrl(logSplitter, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP | wxTE_RICH2);
            m_logView->SetDefaultStyle(wxTextAttr(*wxLIGHT_GREY, *wxBLACK));
            m_logView->SetBackgroundColour(*wxBLACK);
            m_inspector = new Inspector(inspectorSplitter, m_documentViewHolder);
            
            m_mapCanvasContainerPanel = new wxPanel(logSplitter, wxID_ANY);
            m_navContainerPanel = new wxPanel(m_mapCanvasContainerPanel, wxID_ANY);
            m_navPanel = new wxPanel(m_navContainerPanel, wxID_ANY);
            
            wxSearchCtrl* searchBox = new wxSearchCtrl(m_navContainerPanel, wxID_ANY);
#if defined __APPLE__
            searchBox->SetFont(*wxSMALL_FONT);
            m_navContainerPanel->SetBackgroundStyle(wxBG_STYLE_PAINT);
            m_navContainerPanel->Bind(wxEVT_PAINT, &EditorFrame::PaintNavContainer, this);
#endif
            
            wxSizer* navContainerInnerSizer = new wxBoxSizer(wxHORIZONTAL);
            navContainerInnerSizer->AddSpacer(4);
            navContainerInnerSizer->Add(m_navPanel, 1, wxEXPAND | wxALIGN_CENTRE_VERTICAL);
            navContainerInnerSizer->Add(searchBox, 0, wxEXPAND | wxALIGN_RIGHT);
#if defined __APPLE__
            navContainerInnerSizer->AddSpacer(4);
#endif
            navContainerInnerSizer->SetItemMinSize(searchBox, 200, wxDefaultSize.y);
            
            wxSizer* navContainerOuterSizer = new wxBoxSizer(wxVERTICAL);
            navContainerOuterSizer->AddSpacer(2);
            navContainerOuterSizer->Add(navContainerInnerSizer, 1, wxEXPAND);
            navContainerOuterSizer->AddSpacer(2);
            m_navContainerPanel->SetSizer(navContainerOuterSizer);
            
            m_mapCanvas = new MapGLCanvas(m_mapCanvasContainerPanel, m_documentViewHolder);
            
            wxSizer* mapCanvasContainerSizer = new wxBoxSizer(wxVERTICAL);
            mapCanvasContainerSizer->Add(m_navContainerPanel, 0, wxEXPAND);
            mapCanvasContainerSizer->Add(m_mapCanvas, 1, wxEXPAND);
            m_mapCanvasContainerPanel->SetSizer(mapCanvasContainerSizer);

            logSplitter->SplitHorizontally(m_mapCanvasContainerPanel, m_logView, -150);
            inspectorSplitter->SplitVertically(logSplitter, m_inspector, -350);

            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(inspectorSplitter, 1, wxEXPAND);
            SetSizer(outerSizer);

            updateNavigation();
            SetSize(1024, 768);

            /*
            m_mapCanvas->Bind(wxEVT_SET_FOCUS, &EditorFrame::OnMapCanvasSetFocus, this);
            m_mapCanvas->Bind(wxEVT_KILL_FOCUS, &EditorFrame::OnMapCanvasKillFocus, this);
             */
        }

        wxStaticText* EditorFrame::makeBreadcrump(const wxString& text, bool link) {
            wxStaticText* staticText = new wxStaticText(m_navPanel, wxID_ANY, text);
#if defined __APPLE__
            staticText->SetFont(*wxSMALL_FONT);
#endif
            if (link) {
                staticText->SetForegroundColour(wxColour(10, 75, 220)); //wxSystemSettings::GetColour(wxSYS_COLOUR_HOTLIGHT));
                staticText->SetCursor(wxCursor(wxCURSOR_HAND));
            }
            return staticText;
        }

        EditorFrame::EditorFrame(Model::MapDocument& document, EditorView& view) :
        wxFrame(NULL, wxID_ANY, wxT("")),
        m_documentViewHolder(DocumentViewHolder(&document, &view)),
        m_focusMapCanvasOnIdle(true) {
#if defined _WIN32
            SetIcon(wxICON(APPICON));
#endif
            CreateGui();
            updateMenuBar();

            m_mapCanvasHasFocus = false;
        }

        void EditorFrame::updateNavigation() {
            m_navPanel->DestroyChildren();
            
            wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
            sizer->Add(makeBreadcrump(wxT("func_door"), true), 0, wxALIGN_CENTRE_VERTICAL);
            sizer->AddSpacer(2);
            sizer->Add(makeBreadcrump(L"\u00BB", false), 0, wxALIGN_CENTRE_VERTICAL);
            sizer->AddSpacer(2);
            sizer->Add(makeBreadcrump(wxT("3 / 7 brushes"), false), 0, wxALIGN_CENTRE_VERTICAL);
            m_navPanel->SetSizer(sizer);

            m_navContainerPanel->Layout();
        }

        void EditorFrame::updateMenuBar() {
            if (!m_documentViewHolder.valid())
                return;

            bool mapViewFocused = FindFocus() == m_mapCanvas;
            TrenchBroomApp* app = static_cast<TrenchBroomApp*>(wxTheApp);
            wxMenu* actionMenu = NULL;
            if (wxDynamicCast(FindFocus(), wxTextCtrl) == NULL) {
                if (m_mapCanvas->inputController().moveVerticesToolActive()) {
                    actionMenu = app->CreateVertexActionMenu(mapViewFocused);
                } else if (m_mapCanvas->inputController().clipToolActive()) {
                } else {
                    Model::EditStateManager& editStateManager = m_documentViewHolder.document().editStateManager();
                    switch (editStateManager.selectionMode()) {
                        case Model::EditStateManager::SMFaces:
                            actionMenu = app->CreateTextureActionMenu(mapViewFocused);
                            break;
                        case Model::EditStateManager::SMEntities:
                        case Model::EditStateManager::SMBrushes:
                        case Model::EditStateManager::SMEntitiesAndBrushes:
                            actionMenu = app->CreateObjectActionMenu(mapViewFocused);
                            break;
                        default:
                            break;
                    }
                }
            }

            wxMenuBar* menuBar = app->CreateMenuBar(&m_documentViewHolder.view(), actionMenu, mapViewFocused);
            int editMenuIndex = menuBar->FindMenu(wxT("Edit"));
            assert(editMenuIndex != wxNOT_FOUND);
            wxMenu* editMenu = menuBar->GetMenu(static_cast<size_t>(editMenuIndex));
            m_documentViewHolder.document().GetCommandProcessor()->SetEditMenu(editMenu);

            // SetMenuBar(NULL);

            wxMenuBar* oldMenuBar = GetMenuBar();
            app->DetachFileHistoryMenu(oldMenuBar);

            SetMenuBar(menuBar);
            delete oldMenuBar;
        }

        void EditorFrame::disableProcessing() {
            wxMenuBar* oldMenuBar = GetMenuBar();
            TrenchBroomApp* app = static_cast<TrenchBroomApp*>(wxTheApp);
            app->DetachFileHistoryMenu(oldMenuBar);

            m_documentViewHolder.invalidate();
        }

        /*
        void EditorFrame::OnMapCanvasSetFocus(wxFocusEvent& event) {
            m_mapCanvasHasFocus = true;

            if (m_documentViewHolder.valid()) {
                updateMenuBar();

                wxMenuBar* menuBar = GetMenuBar();
                size_t menuCount = menuBar->GetMenuCount();
                for (size_t i = 0; i < menuCount; i++) {
                    wxMenu* menu = menuBar->GetMenu(i);
                    menu->UpdateUI(&m_documentViewHolder.view());
                }

                m_mapCanvas->Refresh();
            }

            event.Skip();
        }

        void EditorFrame::OnMapCanvasKillFocus(wxFocusEvent& event) {
            m_mapCanvasHasFocus = false;

            if (m_documentViewHolder.valid()) {
                updateMenuBar();

                wxMenuBar* menuBar = GetMenuBar();
                size_t menuCount = menuBar->GetMenuCount();
                for (size_t i = 0; i < menuCount; i++) {
                    wxMenu* menu = menuBar->GetMenu(i);
                    menu->UpdateUI(&m_documentViewHolder.view());
                }

                m_mapCanvas->Refresh();
            }

            event.Skip();
        }
         */

        void EditorFrame::OnSetFocus(wxCommandEvent& event) {
            if (m_documentViewHolder.valid()) {
                wxWindow* focus = FindFocus();
                if (m_mapCanvasHasFocus != (focus == m_mapCanvas)) {
                    m_mapCanvasHasFocus = (focus == m_mapCanvas);
                    
                    updateMenuBar();
                    
                    wxMenuBar* menuBar = GetMenuBar();
                    size_t menuCount = menuBar->GetMenuCount();
                    for (size_t i = 0; i < menuCount; i++) {
                        wxMenu* menu = menuBar->GetMenu(i);
                        menu->UpdateUI(&m_documentViewHolder.view());
                    }
                    m_mapCanvas->Refresh();
                }
            }
        }

        void EditorFrame::OnIdle(wxIdleEvent& event) {
            if (m_focusMapCanvasOnIdle) {
                m_mapCanvas->SetFocus();
                m_focusMapCanvasOnIdle = false;
            }

            /*
            // this is a fix for Mac OS X, where the kill focus event is not properly sent
            // FIXME: remove this as soon as this bug is fixed in wxWidgets 2.9.5

#ifdef __APPLE__
            if (m_documentViewHolder.valid()) {
                wxWindow* focus = FindFocus();
                if (m_mapCanvasHasFocus != (focus == m_mapCanvas)) {
                    m_mapCanvasHasFocus = (focus == m_mapCanvas);

                    updateMenuBar();

                    wxMenuBar* menuBar = GetMenuBar();
                    size_t menuCount = menuBar->GetMenuCount();
                    for (size_t i = 0; i < menuCount; i++) {
                        wxMenu* menu = menuBar->GetMenu(i);
                        menu->UpdateUI(&m_documentViewHolder.view());
                    }
                    m_mapCanvas->Refresh();
                }
            }
#endif
            */
            
            // finally set the top window
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
#ifdef _WIN32
            wxMenu* menu = event.GetMenu();
			menu->UpdateUI(&m_documentViewHolder.view());
#endif
            event.Skip();
        }
    }
}
