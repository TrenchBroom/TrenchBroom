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

#include "EntityDefinitionFileChooser.h"

#include "CollectionUtils.h"
#include "Notifier.h"
#include "IO/Path.h"
#include "View/LayoutConstants.h"
#include "View/MapDocument.h"

#include <wx/button.h>
#include <wx/listbox.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

namespace TrenchBroom {
    namespace View {
        EntityDefinitionFileChooser::EntityDefinitionFileChooser(wxWindow* parent, MapDocumentPtr document, ControllerPtr controller) :
        wxPanel(parent),
        m_document(document),
        m_controller(controller) {
            createGui();
            bindEvents();
            bindObservers();
        }
        
        EntityDefinitionFileChooser::~EntityDefinitionFileChooser() {
            unbindObservers();
        }

        void EntityDefinitionFileChooser::OnBuiltinSelectionChanged(wxCommandEvent& event) {
        }
        
        void EntityDefinitionFileChooser::OnChooseExternalClicked(wxCommandEvent& event) {
        }

        void EntityDefinitionFileChooser::createGui() {
            wxStaticText* builtinHeader = new wxStaticText(this, wxID_ANY, _("Builtin"));
            m_builtin = new wxListBox(this, wxID_ANY);
            
            wxStaticText* externalHeader = new wxStaticText(this, wxID_ANY, _("External"));
            m_external = new wxStaticText(this, wxID_ANY, _("not set"));
            m_chooseExternal = new wxButton(this, wxID_ANY, _("Browse..."));

            wxSizer* externalSizer = new wxBoxSizer(wxHORIZONTAL);
            externalSizer->Add(m_external, 1, wxEXPAND);
            externalSizer->AddSpacer(LayoutConstants::ControlHorizontalMargin);
#if defined __APPLE__
            externalSizer->Add(m_chooseExternal, 0, wxALL, 1);
#else
            externalSizer->Add(m_chooseExternal);
#endif
            
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(builtinHeader);
            sizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
            sizer->Add(m_builtin, 1, wxEXPAND);
            sizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
            sizer->Add(externalHeader);
            sizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
            sizer->Add(externalSizer, 0, wxEXPAND);
            sizer->SetItemMinSize(m_builtin, 100, 70);
            
            SetSizerAndFit(sizer);
        }
        
        void EntityDefinitionFileChooser::bindEvents() {
            m_builtin->Bind(wxEVT_LISTBOX, &EntityDefinitionFileChooser::OnBuiltinSelectionChanged, this);
            m_chooseExternal->Bind(wxEVT_BUTTON, &EntityDefinitionFileChooser::OnChooseExternalClicked, this);
        }

        void EntityDefinitionFileChooser::bindObservers() {
            m_document->documentWasNewedNotifier.addObserver(this, &EntityDefinitionFileChooser::documentWasNewed);
            m_document->documentWasLoadedNotifier.addObserver(this, &EntityDefinitionFileChooser::documentWasLoaded);
        }
        
        void EntityDefinitionFileChooser::unbindObservers() {
            m_document->documentWasNewedNotifier.removeObserver(this, &EntityDefinitionFileChooser::documentWasNewed);
            m_document->documentWasLoadedNotifier.removeObserver(this, &EntityDefinitionFileChooser::documentWasLoaded);
        }
        
        void EntityDefinitionFileChooser::documentWasNewed() {
            updateControls();
        }
        
        void EntityDefinitionFileChooser::documentWasLoaded() {
            updateControls();
        }
        
        void EntityDefinitionFileChooser::updateControls() {
            m_builtin->Clear();
            
            IO::Path::List paths = m_document->definitionFiles();
            std::sort(paths.begin(), paths.end());
            
            IO::Path::List::const_iterator it, end;
            for (it = paths.begin(), end = paths.end(); it != end; ++it) {
                const IO::Path& path = *it;
                m_builtin->Append(path.lastComponent().asString());
            }
            
            m_external->SetLabel(_("not set"));
        }
    }
}
