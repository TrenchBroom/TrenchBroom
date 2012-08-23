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

#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/splitter.h>

namespace TrenchBroom {
    namespace View {
        EditorFrame::EditorFrame() : wxFrame(NULL, wxID_ANY, wxT("TrenchBroom")) {
            wxSizer* mainSplitterSizer = new wxBoxSizer(wxVERTICAL);
            wxSplitterWindow* mainSplitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3DSASH | wxSP_LIVE_UPDATE);
            mainSplitter->SetSashGravity(1.0f);
            mainSplitter->SetMinimumPaneSize(300);
            mainSplitter->Connect(wxEVT_IDLE, wxIdleEventHandler( EditorFrame::mainSplitterOnIdle ), NULL, this);

            MapGLCanvas* mapCanvas = new MapGLCanvas(mainSplitter);
            Inspector* inspector = new Inspector(mainSplitter);
            mainSplitter->SplitVertically(mapCanvas, inspector, 0);
            mainSplitterSizer->Add(mainSplitter, 1, wxEXPAND, 5);
            
            SetSizer(mainSplitterSizer);

            SetSize(800, 600);
            Layout();
        }
        
        EditorFrame::~EditorFrame() {
        }
        
        void EditorFrame::mainSplitterOnIdle(wxIdleEvent& event) {
            wxSplitterWindow* splitterWindow = dynamic_cast<wxSplitterWindow*>(event.GetEventObject());
            if (splitterWindow != NULL) {
                splitterWindow->SetSashPosition(GetSize().x - 300);
                splitterWindow->Disconnect(wxEVT_IDLE, wxIdleEventHandler(EditorFrame::mainSplitterOnIdle ), NULL, this);
            }
        }
    }
}