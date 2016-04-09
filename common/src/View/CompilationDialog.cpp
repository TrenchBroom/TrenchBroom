/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "CompilationDialog.h"

#include "Model/Game.h"
#include "View/BorderLine.h"
#include "View/CompilationProfileManager.h"
#include "View/MapDocument.h"
#include "View/MapFrame.h"
#include "View/SplitterWindow2.h"

#include <wx/sizer.h>
#include <wx/textctrl.h>

namespace TrenchBroom {
    namespace View {
        CompilationDialog::CompilationDialog(MapFrame* mapFrame) :
        wxDialog(mapFrame, wxID_ANY, "Compile", wxDefaultPosition, wxDefaultSize, wxCAPTION | wxRESIZE_BORDER | wxCLOSE_BOX),
        m_mapFrame(mapFrame),
        m_output(NULL) {
            createGui();
            SetSize(800, 600);
            CentreOnParent();
        }
        
        void CompilationDialog::createGui() {
            MapDocumentSPtr document = m_mapFrame->document();
            Model::GamePtr game = document->game();
            Model::CompilationConfig& compilationConfig = game->compilationConfig();
            
            wxPanel* outerPanel = new wxPanel(this);
            SplitterWindow2* splitter = new SplitterWindow2(outerPanel);
            
            m_profileManager = new CompilationProfileManager(splitter, compilationConfig);
            m_output = new wxTextCtrl(splitter, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP | wxTE_RICH2);

            splitter->splitHorizontally(m_profileManager, m_output);

            wxSizer* outerPanelSizer = new wxBoxSizer(wxVERTICAL);
            outerPanelSizer->Add(splitter, 1, wxEXPAND);
            outerPanel->SetSizer(outerPanelSizer);
            
            wxSizer* dialogSizer = new wxBoxSizer(wxVERTICAL);
            dialogSizer->Add(outerPanel, 1, wxEXPAND);
            SetSizer(dialogSizer);
        }
    }
}
