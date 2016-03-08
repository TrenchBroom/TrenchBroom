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
#include "View/CollapsibleTitledPanel.h"
#include "View/EntityBrowser.h"
#include "View/EntityDefinitionFileChooser.h"
#include "View/EntityAttributeEditor.h"
#include "View/SplitterWindow2.h"
#include "View/TitledPanel.h"
#include "View/ViewConstants.h"
#include "View/MapDocument.h"

#include <wx/event.h>
#include <wx/notebook.h>
#include <wx/persist.h>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        EntityInspector::EntityInspector(wxWindow* parent, MapDocumentWPtr document, GLContextManager& contextManager) :
        TabBookPage(parent) {
#if defined __APPLE__
            SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#endif
            createGui(document, contextManager);
        }

        void EntityInspector::createGui(MapDocumentWPtr document, GLContextManager& contextManager) {
            SplitterWindow2* splitter = new SplitterWindow2(this);
            splitter->setSashGravity(0.0);
            splitter->SetName("EntityInspectorSplitter");
            
            splitter->splitHorizontally(createAttributeEditor(splitter, document),
                                        createEntityBrowser(splitter, document, contextManager),
                                        wxSize(100, 150), wxSize(100, 150));
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(splitter, 1, wxEXPAND);
            outerSizer->Add(new BorderLine(this, BorderLine::Direction_Horizontal), 0, wxEXPAND);
            outerSizer->Add(createEntityDefinitionFileChooser(this, document), 0, wxEXPAND);
            SetSizer(outerSizer);
            
            wxPersistenceManager::Get().RegisterAndRestore(splitter);
        }
        
        wxWindow* EntityInspector::createAttributeEditor(wxWindow* parent, MapDocumentWPtr document) {
            m_attributeEditor = new EntityAttributeEditor(parent, document);
            return m_attributeEditor;
        }
        
        wxWindow* EntityInspector::createEntityBrowser(wxWindow* parent, MapDocumentWPtr document, GLContextManager& contextManager) {
            TitledPanel* panel = new TitledPanel(parent, "Entity Browser");
            m_entityBrowser = new EntityBrowser(panel->getPanel(), document, contextManager);
            
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_entityBrowser, 1, wxEXPAND);
            panel->getPanel()->SetSizer(sizer);
            
            return panel;
        }
        
        wxWindow* EntityInspector::createEntityDefinitionFileChooser(wxWindow* parent, MapDocumentWPtr document) {
            CollapsibleTitledPanel* panel = new CollapsibleTitledPanel(parent, "Entity Definitions", false);
            m_entityDefinitionFileChooser = new EntityDefinitionFileChooser(panel->getPanel(), document);

            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_entityDefinitionFileChooser, 1, wxEXPAND);
            panel->getPanel()->SetSizer(sizer);

            return panel;
        }
    }
}
