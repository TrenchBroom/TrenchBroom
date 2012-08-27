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

#include "View/Inspector.h"
#include "View/MapGLCanvas.h"

#include <wx/colour.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/splitter.h>
#include <wx/textctrl.h>

namespace TrenchBroom {
    namespace View {
        EditorFrame::EditorFrame() : wxFrame(NULL, wxID_ANY, wxT("TrenchBroom")) {
            wxSplitterWindow* logSplitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3DSASH | wxSP_LIVE_UPDATE);
            logSplitter->SetSashGravity(1.0f);
            logSplitter->SetMinimumPaneSize(0);
            logSplitter->Connect(wxEVT_IDLE, wxIdleEventHandler(EditorFrame::logSplitterOnIdle), NULL, this);
            
            wxSplitterWindow* inspectorSplitter = new wxSplitterWindow(logSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3DSASH | wxSP_LIVE_UPDATE);
            inspectorSplitter->SetSashGravity(1.0f);
            inspectorSplitter->SetMinimumPaneSize(300);
            inspectorSplitter->Connect(wxEVT_IDLE, wxIdleEventHandler( EditorFrame::inspectorSplitterOnIdle ), NULL, this);

            m_mapCanvas = new MapGLCanvas(inspectorSplitter);
            Inspector* inspector = new Inspector(inspectorSplitter);
            inspectorSplitter->SplitVertically(m_mapCanvas, inspector, 0);
            
            m_logView = new wxTextCtrl(logSplitter, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP | wxTE_RICH2);
            m_logView->SetDefaultStyle(wxTextAttr(*wxLIGHT_GREY, *wxBLACK));
            m_logView->SetBackgroundColour(*wxBLACK);
            logSplitter->SplitHorizontally(inspectorSplitter, m_logView);
            
            wxSizer* logSplitterSizer = new wxBoxSizer(wxVERTICAL);
            logSplitterSizer->Add(logSplitter, 1, wxEXPAND);
            SetSizer(logSplitterSizer);

            SetSize(800, 600);
            Layout();
        }
        
        EditorFrame::~EditorFrame() {
        }
        
        void EditorFrame::logSplitterOnIdle(wxIdleEvent& event) {
            wxSplitterWindow* splitterWindow = dynamic_cast<wxSplitterWindow*>(event.GetEventObject());
            if (splitterWindow != NULL) {
                splitterWindow->SetSashPosition(GetSize().y - 150);
                splitterWindow->Disconnect(wxEVT_IDLE, wxIdleEventHandler(EditorFrame::logSplitterOnIdle ), NULL, this);
            }
        }
        
        void EditorFrame::inspectorSplitterOnIdle(wxIdleEvent& event) {
            wxSplitterWindow* splitterWindow = dynamic_cast<wxSplitterWindow*>(event.GetEventObject());
            if (splitterWindow != NULL) {
                splitterWindow->SetSashPosition(GetSize().x - 300);
                splitterWindow->Disconnect(wxEVT_IDLE, wxIdleEventHandler(EditorFrame::inspectorSplitterOnIdle ), NULL, this);
            }
        }
    }
}