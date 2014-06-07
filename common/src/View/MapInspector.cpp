/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "View/BorderLine.h"
#include "View/CollapsibleTitledPanel.h"
#include "View/MapTreeView.h"
#include "View/MiniMap.h"
#include "View/ModEditor.h"
#include "View/TitledPanel.h"
#include "View/ViewConstants.h"

#include <wx/notebook.h>
#include <wx/settings.h>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        MapInspector::MapInspector(wxWindow* parent, GLContextHolder::Ptr sharedContext, MapDocumentWPtr document, ControllerWPtr controller, Renderer::Camera& camera) :
        TabBookPage(parent) {
            createGui(sharedContext, document, controller, camera);
        }

        void MapInspector::OnPaneChanged(wxCollapsiblePaneEvent& event) {
            Layout();
        }

        void MapInspector::createGui(GLContextHolder::Ptr sharedContext, MapDocumentWPtr document, ControllerWPtr controller, Renderer::Camera& camera) {
            wxWindow* miniMap = createMiniMap(this, sharedContext, document, camera);
            wxWindow* mapTree = createMapTree(this, document, controller);
            wxWindow* modEditor = createModEditor(this, document, controller);
            
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(miniMap, 0, wxEXPAND);
            sizer->Add(new BorderLine(this, BorderLine::Direction_Horizontal), 0, wxEXPAND);
            sizer->Add(mapTree, 1, wxEXPAND);
            sizer->Add(new BorderLine(this, BorderLine::Direction_Horizontal), 0, wxEXPAND);
            sizer->Add(modEditor, 0, wxEXPAND);
            
            sizer->SetItemMinSize(miniMap, wxDefaultSize.x, 180);
            SetSizer(sizer);
        }

        wxWindow* MapInspector::createMiniMap(wxWindow* parent, GLContextHolder::Ptr sharedContext, MapDocumentWPtr document, Renderer::Camera& camera) {
            return new MiniMap(parent, sharedContext, document, camera);
        }

        wxWindow* MapInspector::createMapTree(wxWindow* parent, MapDocumentWPtr document, ControllerWPtr controller) {
            TitledPanel* titledPanel = new TitledPanel(parent, "Map Structure");
            MapTreeView* treeView = new MapTreeView(titledPanel->getPanel(), document, controller);
            
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(treeView, 1, wxEXPAND);
            titledPanel->getPanel()->SetSizer(sizer);
            
            return titledPanel;
        }
        
        wxWindow* MapInspector::createModEditor(wxWindow* parent, MapDocumentWPtr document, ControllerWPtr controller) {
            CollapsibleTitledPanel* titledPanel = new CollapsibleTitledPanel(parent, "Mods", false);

			/*
#if defined _WIN32
            // this is a hack to prevent the pane having the wrong background color on Windows 7
            wxNotebook* book = static_cast<wxNotebook*>(GetParent());
            wxColour col = book->GetThemeBackgroundColour();
            if (col.IsOk()) {
                collPane->SetBackgroundColour(col);
                collPane->GetPane()->SetBackgroundColour(col);
            }
#endif
			*/
            
            ModEditor* modEditor = new ModEditor(titledPanel->getPanel(), document, controller);

            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(modEditor, 1, wxEXPAND);
            titledPanel->getPanel()->SetSizerAndFit(sizer);
            
            return titledPanel;
        }
    }
}
