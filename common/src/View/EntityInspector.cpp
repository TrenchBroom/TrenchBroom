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

#include "EntityInspector.h"

#include "StringUtils.h"
#include "View/BorderLine.h"
#include "View/EntityBrowser.h"
#include "View/EntityDefinitionFileChooser.h"
#include "View/EntityPropertyEditor.h"
#include "View/ViewConstants.h"
#include "View/MapDocument.h"

#include <wx/collpane.h>
#include <wx/event.h>
#include <wx/notebook.h>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        EntityInspector::EntityInspector(wxWindow* parent, GLContextHolder::Ptr sharedContext, MapDocumentWPtr document, ControllerWPtr controller) :
        TabBookPage(parent),
        m_document(document),
        m_controller(controller) {
            createGui(parent, sharedContext, m_document, m_controller);
        }
        
        void EntityInspector::OnEntityDefinitionFileChooserPaneChanged(wxCollapsiblePaneEvent& event) {
            Layout();
        }

        void EntityInspector::createGui(wxWindow* parent, GLContextHolder::Ptr sharedContext, MapDocumentWPtr document, ControllerWPtr controller) {
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(createPropertyEditor(this), 0, wxEXPAND);
            outerSizer->Add(new BorderLine(this, BorderLine::Direction_Horizontal), 0, wxEXPAND);
            outerSizer->Add(createEntityBrowser(this, sharedContext), 1, wxEXPAND);
            outerSizer->Add(new BorderLine(this, BorderLine::Direction_Horizontal), 0, wxEXPAND);
            outerSizer->Add(createEntityDefinitionFileChooser(this, document, controller), 0, wxEXPAND);
            SetSizerAndFit(outerSizer);
        }
        
        wxWindow* EntityInspector::createPropertyEditor(wxWindow* parent) {
            m_propertyEditor = new EntityPropertyEditor(parent, m_document, m_controller);
            return m_propertyEditor;
        }
        
        wxWindow* EntityInspector::createEntityBrowser(wxWindow* parent, GLContextHolder::Ptr sharedContext) {
            m_entityBrowser = new EntityBrowser(parent, sharedContext, m_document);
            return m_entityBrowser;
        }
        
        wxWindow* EntityInspector::createEntityDefinitionFileChooser(wxWindow* parent, MapDocumentWPtr document, ControllerWPtr controller) {
            wxCollapsiblePane* collPane = new wxCollapsiblePane(parent, wxID_ANY, "Entity Definitions", wxDefaultPosition, wxDefaultSize, wxCP_NO_TLW_RESIZE | wxTAB_TRAVERSAL | wxBORDER_NONE);

#if defined _WIN32
            // this is a hack to prevent the pane having the wrong background color on Windows 7
            wxNotebook* book = static_cast<wxNotebook*>(GetParent());
            wxColour col = book->GetThemeBackgroundColour();
            if (col.IsOk()) {
                collPane->SetBackgroundColour(col);
                collPane->GetPane()->SetBackgroundColour(col);
            }
#endif

            m_entityDefinitionFileChooser = new EntityDefinitionFileChooser(collPane->GetPane(), document, controller);

            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_entityDefinitionFileChooser, 1, wxEXPAND);
            collPane->GetPane()->SetSizerAndFit(sizer);
            
            collPane->Bind(wxEVT_COLLAPSIBLEPANE_CHANGED, &EntityInspector::OnEntityDefinitionFileChooserPaneChanged, this);
            return collPane;
        }
    }
}
