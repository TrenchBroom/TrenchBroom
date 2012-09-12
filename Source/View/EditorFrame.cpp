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

#include "Model/MapDocument.h"
#include "Utility/Console.h"
#include "View/EditorView.h"
#include "View/Inspector.h"
#include "View/MapGLCanvas.h"
#include "View/MenuCommandIds.h"

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
        void EditorFrame::CreateGui(Model::MapDocument& document, EditorView& view) {
            wxSplitterWindow* logSplitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3DSASH | wxSP_LIVE_UPDATE);
            logSplitter->SetSashGravity(1.0f);
            logSplitter->SetMinimumPaneSize(0);
            
            wxSplitterWindow* inspectorSplitter = new wxSplitterWindow(logSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3DSASH | wxSP_LIVE_UPDATE);
            inspectorSplitter->SetSashGravity(1.0f);
            inspectorSplitter->SetMinimumPaneSize(350);
            
            m_logView = new wxTextCtrl(logSplitter, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP | wxTE_RICH2);
            m_logView->SetDefaultStyle(wxTextAttr(*wxLIGHT_GREY, *wxBLACK));
            m_logView->SetBackgroundColour(*wxBLACK);
            
            m_mapCanvas = new MapGLCanvas(inspectorSplitter, document, view);
            Inspector* inspector = new Inspector(inspectorSplitter);
            
            inspectorSplitter->SplitVertically(m_mapCanvas, inspector, 0);
            logSplitter->SplitHorizontally(inspectorSplitter, m_logView);
            
            wxSizer* logSplitterSizer = new wxBoxSizer(wxVERTICAL);
            logSplitterSizer->Add(logSplitter, 1, wxEXPAND);
            SetSizer(logSplitterSizer);
            
            SetSize(800, 600);
            inspectorSplitter->SetSashPosition(GetSize().x - 350);
            logSplitter->SetSashPosition(GetSize().y - 150);
            Layout();
        }
        
        wxMenu* EditorFrame::CreateFileMenu(Model::MapDocument& document, EditorView& view) {
            wxMenu* fileMenu = new wxMenu();
            fileMenu->Append(wxID_NEW, wxT("New\tCtrl-N"));
            fileMenu->Append(wxID_OPEN, wxT("Open...\tCtrl-O"));
            fileMenu->AppendSeparator();
            fileMenu->Append(wxID_CLOSE, wxT("Close\tCtrl-W"));
            fileMenu->Append(wxID_SAVE, wxT("Save\tCtrl-S"));
            fileMenu->Append(wxID_SAVEAS, wxT("Save as...\tCtrl-Shift-S"));
            return fileMenu;
        }
        
        wxMenu* EditorFrame::CreateEditMenu(Model::MapDocument& document, EditorView& view) {
            wxMenu* editMenu = new wxMenu();
            editMenu->Append(wxID_UNDO, wxT("Undo\tCtrl-Z"));
            editMenu->Append(wxID_REDO, wxT("Redo\tCtrl-Shift-Z"));
            editMenu->AppendSeparator();
            editMenu->Append(wxID_CUT, wxT("Cut\tCtrl+X"));
            editMenu->Append(wxID_COPY, wxT("Copy\tCtrl+C"));
            editMenu->Append(wxID_PASTE, wxT("Paste\tCtrl+V"));
            editMenu->AppendSeparator();
            editMenu->Append(MenuCommandIds::tbID_EDIT_SELECT_ALL, wxT("Select All\tCtrl+A"));
            editMenu->Append(MenuCommandIds::tbID_EDIT_SELECT_SIBLINGS, wxT("Select Siblings\tCtrl+Alt+A"));
            editMenu->Append(MenuCommandIds::tbID_EDIT_SELECT_TOUCHING, wxT("Select Touching\tCtrl+T"));
            editMenu->Append(MenuCommandIds::tbID_EDIT_SELECT_NONE, wxT("Select None\tCtrl+Shift+A"));
            editMenu->AppendSeparator();
            editMenu->Append(MenuCommandIds::tbID_EDIT_HIDE_SELECTED, wxT("Hide Selected\tCtrl+H"));
            editMenu->Append(MenuCommandIds::tbID_EDIT_HIDE_UNSELECTED, wxT("Hide Unselected\tCtrl+Alt+H"));
            editMenu->Append(MenuCommandIds::tbID_EDIT_UNHIDE_ALL, wxT("Unhide All\tCtrl+Shift+H"));
            editMenu->AppendSeparator();
            editMenu->Append(MenuCommandIds::tbID_EDIT_LOCK_SELECTED, wxT("Lock Selected\tCtrl+L"));
            editMenu->Append(MenuCommandIds::tbID_EDIT_LOCK_UNSELECTED, wxT("Lock Unselected\tCtrl+Alt+L"));
            editMenu->Append(MenuCommandIds::tbID_EDIT_UNLOCK_ALL, wxT("Unlock All\tCtrl+Shift+L"));
            return editMenu;
        }
        
        wxMenu* EditorFrame::CreateViewMenu(Model::MapDocument& document, EditorView& view) {
            wxMenu* viewMenu = new wxMenu();
            return viewMenu;
        }
        
        wxMenu* EditorFrame::CreateHelpMenu(Model::MapDocument& document, EditorView& view) {
            wxMenu* helpMenu = new wxMenu();
            return helpMenu;
        }

        void EditorFrame::CreateMenuBar(Model::MapDocument& document, EditorView& view) {
            wxDocManager* docManager = document.GetDocumentManager();

            wxMenu* fileMenu = CreateFileMenu(document, view);
			fileMenu->SetEventHandler(docManager);
            docManager->FileHistoryUseMenu(fileMenu);
            docManager->FileHistoryLoad(*wxConfig::Get());
            
            wxMenu* editMenu = CreateEditMenu(document, view);
            editMenu->SetEventHandler(&view);
            
            wxMenu* viewMenu = CreateViewMenu(document, view);
            viewMenu->SetEventHandler(&view);
            
            wxMenu* helpMenu = CreateHelpMenu(document, view);
            helpMenu->SetEventHandler(&view);
            
            wxMenuBar* menuBar = new wxMenuBar();
            menuBar->Append(fileMenu, wxT("File"));
            menuBar->Append(editMenu, wxT("Edit"));
            menuBar->Append(viewMenu, wxT("View"));
            menuBar->Append(helpMenu, wxT("Help"));
            
			SetMenuBar(menuBar);
            
            // TODO handle events here and delegate them to the docmanager manually!
        }

        EditorFrame::EditorFrame(Model::MapDocument& document, EditorView& view) : wxFrame(NULL, wxID_ANY, wxT("TrenchBroom")) {
            CreateGui(document, view);
            CreateMenuBar(document, view);
        }
    }
}