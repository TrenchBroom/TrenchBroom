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
#include <wx/sizer.h>
#include <wx/splitter.h>
#include <wx/textctrl.h>

namespace TrenchBroom {
    namespace View {
        BEGIN_EVENT_TABLE(EditorFrame, wxFrame)
		EVT_CLOSE(EditorFrame::OnClose)
        EVT_MENU_OPEN(EditorFrame::OnMenuOpen)
        EVT_IDLE(EditorFrame::OnIdle)
		END_EVENT_TABLE()

        void EditorFrame::CreateGui() {
            wxSplitterWindow* logSplitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3DSASH | wxSP_LIVE_UPDATE);
            logSplitter->SetSashGravity(1.0f);
            logSplitter->SetMinimumPaneSize(0);
            
            wxSplitterWindow* inspectorSplitter = new wxSplitterWindow(logSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3DSASH | wxSP_LIVE_UPDATE);
            inspectorSplitter->SetSashGravity(1.0f);
            inspectorSplitter->SetMinimumPaneSize(350);
            
            m_logView = new wxTextCtrl(logSplitter, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP | wxTE_RICH2);
            m_logView->SetDefaultStyle(wxTextAttr(*wxLIGHT_GREY, *wxBLACK));
            m_logView->SetBackgroundColour(*wxBLACK);
            
            m_mapCanvas = new MapGLCanvas(inspectorSplitter, m_documentViewHolder);
            m_inspector = new Inspector(inspectorSplitter, m_documentViewHolder);
            
            inspectorSplitter->SplitVertically(m_mapCanvas, m_inspector, 0);
            logSplitter->SplitHorizontally(inspectorSplitter, m_logView);
            
            wxSizer* logSplitterSizer = new wxBoxSizer(wxVERTICAL);
            logSplitterSizer->Add(logSplitter, 1, wxEXPAND);
            SetSizer(logSplitterSizer);
            
            SetSize(1024, 768);
            inspectorSplitter->SetSashPosition(GetSize().x - 350);
            logSplitter->SetSashPosition(GetSize().y - 150);
            Layout();
            
            m_mapCanvas->Bind(wxEVT_SET_FOCUS, &EditorFrame::OnMapCanvasSetFocus, this);
            m_mapCanvas->Bind(wxEVT_KILL_FOCUS, &EditorFrame::OnMapCanvasKillFocus, this);
        }
        
        EditorFrame::EditorFrame(Model::MapDocument& document, EditorView& view) :
        wxFrame(NULL, wxID_ANY, wxT("TrenchBroom")),
        m_documentViewHolder(DocumentViewHolder(&document, &view)) {
            CreateGui();
            updateMenuBar();
            m_mapCanvasHasFocus = FindFocus() == m_mapCanvas;
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
            if (oldMenuBar != NULL) {
                int fileMenuIndex = oldMenuBar->FindMenu(wxT("File"));
                assert(fileMenuIndex != wxNOT_FOUND);
                wxMenu* fileMenu = oldMenuBar->GetMenu(static_cast<size_t>(fileMenuIndex));
                int fileHistoryMenuIndex = fileMenu->FindItem(wxT("Open Recent"));
                assert(fileHistoryMenuIndex != wxNOT_FOUND);
                wxMenuItem* fileHistoryMenuItem = fileMenu->FindItem(fileHistoryMenuIndex);
                assert(fileHistoryMenuItem != NULL);
                wxMenu* fileHistoryMenu = fileHistoryMenuItem->GetSubMenu();
                assert(fileHistoryMenu != NULL);
                m_documentViewHolder.document().GetDocumentManager()->FileHistoryRemoveMenu(fileHistoryMenu);
            }
            
            SetMenuBar(menuBar);
            delete oldMenuBar;
        }

        void EditorFrame::disableProcessing() {
            m_documentViewHolder.invalidate();
        }

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
        }

        void EditorFrame::OnIdle(wxIdleEvent& event) {
            // this is a fix for Mac OS X, where the kill focus event is not properly sent
            // FIXME: remove this as soon as this bug is fixed in wxWidgets 2.9.5
            
#ifdef __APPLE__
            if (m_documentViewHolder.valid()) {
                if (m_mapCanvasHasFocus && FindFocus() != m_mapCanvas) {
                    m_mapCanvasHasFocus = false;
                    
                    updateMenuBar();
                    
                    wxMenuBar* menuBar = GetMenuBar();
                    size_t menuCount = menuBar->GetMenuCount();
                    for (size_t i = 0; i < menuCount; i++) {
                        wxMenu* menu = menuBar->GetMenu(i);
                        menu->UpdateUI(&m_documentViewHolder.view());
                    }
                }
                
                m_mapCanvas->Refresh();
            }
#endif
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
        }
        
    }
}
