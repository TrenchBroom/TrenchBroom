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

#include "Utility/Console.h"
#include "View/Inspector.h"
#include "View/MapGLCanvas.h"

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
        void EditorFrame::CreateGui(Renderer::Camera& camera, Renderer::MapRenderer& renderer) {
            wxSplitterWindow* logSplitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3DSASH | wxSP_LIVE_UPDATE);
            logSplitter->SetSashGravity(1.0f);
            logSplitter->SetMinimumPaneSize(0);
            
            wxSplitterWindow* inspectorSplitter = new wxSplitterWindow(logSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3DSASH | wxSP_LIVE_UPDATE);
            inspectorSplitter->SetSashGravity(1.0f);
            inspectorSplitter->SetMinimumPaneSize(300);
            
            wxTextCtrl* logView = new wxTextCtrl(logSplitter, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP | wxTE_RICH2);
            logView->SetDefaultStyle(wxTextAttr(*wxLIGHT_GREY, *wxBLACK));
            logView->SetBackgroundColour(*wxBLACK);
            m_console = new Utility::Console(logView);
            
            m_mapCanvas = new MapGLCanvas(inspectorSplitter, *m_console, camera, renderer);
            Inspector* inspector = new Inspector(inspectorSplitter);
            
            inspectorSplitter->SplitVertically(m_mapCanvas, inspector, 0);
            logSplitter->SplitHorizontally(inspectorSplitter, logView);
            
            wxSizer* logSplitterSizer = new wxBoxSizer(wxVERTICAL);
            logSplitterSizer->Add(logSplitter, 1, wxEXPAND);
            SetSizer(logSplitterSizer);
            
            SetSize(800, 600);
            inspectorSplitter->SetSashPosition(GetSize().x - 300);
            logSplitter->SetSashPosition(GetSize().y - 150);
            Layout();
        }
        
        void EditorFrame::CreateMenuBar(wxDocManager& docManager) {
            wxMenuBar* menuBar = new wxMenuBar();
            wxMenu* fileMenu = new wxMenu();
            fileMenu->Append(wxID_NEW, wxT("New\tCtrl-N"));
            fileMenu->Append(wxID_OPEN, wxT("Open...\tCtrl-O"));
            fileMenu->AppendSeparator();
            fileMenu->Append(wxID_CLOSE, wxT("Close\tCtrl-W"));
            fileMenu->Append(wxID_SAVE, wxT("Save\tCtrl-S"));
            fileMenu->Append(wxID_SAVEAS, wxT("Save as...\tCtrl-Shift-S"));

            docManager.FileHistoryUseMenu(fileMenu);
            docManager.FileHistoryLoad(*wxConfig::Get());
            
            menuBar->Append(fileMenu, wxT("File"));
			SetMenuBar(menuBar);
            
            // TODO handle events here and delegate them to the docmanager manually!
        }

        EditorFrame::EditorFrame(wxDocManager& docManager, Renderer::Camera& camera, Renderer::MapRenderer& renderer) : wxFrame(NULL, wxID_ANY, wxT("TrenchBroom")) {
            CreateGui(camera, renderer);
            CreateMenuBar(docManager);
        }
        
        EditorFrame::~EditorFrame() {
            delete m_console;
            m_console = NULL;
        }
    }
}