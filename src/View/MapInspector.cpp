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

#include "MapInspector.h"

#include "View/LayoutConstants.h"
#include "View/MapTreeView.h"
#include "View/ModEditor.h"

#include <wx/collpane.h>
#include <wx/notebook.h>
#include <wx/settings.h>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        MapInspector::MapInspector(wxWindow* parent, MapDocumentPtr document, ControllerPtr controller) :
        wxPanel(parent) {
            createGui(document, controller);
        }

        void MapInspector::OnPaneChanged(wxCollapsiblePaneEvent& event) {
            Layout();
        }

        void MapInspector::createGui(MapDocumentPtr document, ControllerPtr controller) {
            wxWindow* mapTree = createMapTree(this, document, controller);
            wxWindow* modEditor = createModEditor(this, document, controller);
            
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(mapTree, 1, wxEXPAND | wxTOP | wxLEFT | wxRIGHT, LayoutConstants::NotebookPageInnerMargin);
            sizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
            sizer->Add(modEditor, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, LayoutConstants::NotebookPageInnerMargin);
            
            SetSizerAndFit(sizer);
        }

        wxWindow* MapInspector::createMapTree(wxWindow* parent, MapDocumentPtr document, ControllerPtr controller) {
            return new MapTreeView(parent, document, controller);
        }
        
        wxWindow* MapInspector::createModEditor(wxWindow* parent, MapDocumentPtr document, ControllerPtr controller) {
            wxCollapsiblePane* collPane = new wxCollapsiblePane(parent, wxID_ANY, _("Mods"), wxDefaultPosition, wxDefaultSize, wxCP_NO_TLW_RESIZE | wxTAB_TRAVERSAL | wxBORDER_NONE);
            
#if defined _WIN32
            // this is a hack to prevent the pane having the wrong background color on Windows 7
            wxNotebook* book = static_cast<wxNotebook*>(GetParent());
            wxColour col = book->GetThemeBackgroundColour();
            if (col.IsOk()) {
                collPane->SetBackgroundColour(col);
                collPane->GetPane()->SetBackgroundColour(col);
            }
#endif
            
            ModEditor* modEditor = new ModEditor(collPane->GetPane(), document, controller);

            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(modEditor, 1, wxEXPAND);
            collPane->GetPane()->SetSizerAndFit(sizer);
            
            collPane->Bind(wxEVT_COLLAPSIBLEPANE_CHANGED, &MapInspector::OnPaneChanged, this);
            return collPane;
        }
    }
}
