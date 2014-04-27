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

#include "EntityDefinitionFileChooser.h"

#include "CollectionUtils.h"
#include "Notifier.h"
#include "IO/Path.h"
#include "Model/EntityDefinitionFileSpec.h"
#include "View/ControllerFacade.h"
#include "View/ViewConstants.h"
#include "View/MapDocument.h"

#include <wx/button.h>
#include <wx/filedlg.h>
#include <wx/listbox.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        EntityDefinitionFileChooser::EntityDefinitionFileChooser(wxWindow* parent, MapDocumentWPtr document, ControllerWPtr controller) :
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
            assert(m_builtin->GetSelection() != wxNOT_FOUND);

            MapDocumentSPtr document = lock(m_document);
            ControllerSPtr controller = lock(m_controller);

            IO::Path::List paths = document->entityDefinitionFiles();
            std::sort(paths.begin(), paths.end());
            
            const size_t index = static_cast<size_t>(m_builtin->GetSelection());
            assert(index < paths.size());
            const IO::Path& path = paths[index];
            
            assert(!path.isAbsolute());
            controller->setEntityDefinitionFile(path);
        }
        
        void EntityDefinitionFileChooser::OnChooseExternalClicked(wxCommandEvent& event) {
            const wxString pathWxStr = ::wxFileSelector("Load Entity Definition File",
                                                        wxEmptyString, wxEmptyString, wxEmptyString,
                                                        "Worldcraft / Hammer files (*.fgd)|*.fgd|QuakeC files (*.def)|*.def",
                                                        wxFD_OPEN | wxFD_FILE_MUST_EXIST);
            if (pathWxStr.empty())
                return;
            
            const IO::Path path(pathWxStr.ToStdString());
            assert(path.isAbsolute());

            ControllerSPtr controller = lock(m_controller);
            controller->setEntityDefinitionFile(path);
        }

        void EntityDefinitionFileChooser::createGui() {
            wxStaticText* builtinHeader = new wxStaticText(this, wxID_ANY, "Builtin");
            m_builtin = new wxListBox(this, wxID_ANY);
            
            wxStaticText* externalHeader = new wxStaticText(this, wxID_ANY, "External");
            m_external = new wxStaticText(this, wxID_ANY, "use builtin", wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_MIDDLE);
            m_chooseExternal = new wxButton(this, wxID_ANY, "Browse...");

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
            sizer->AddSpacer(LayoutConstants::ControlVerticalMargin / 2);
            sizer->Add(m_builtin, 1, wxEXPAND);
            sizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
            sizer->Add(externalHeader);
            sizer->AddSpacer(LayoutConstants::ControlVerticalMargin / 2);
            sizer->Add(externalSizer, 0, wxEXPAND);
            sizer->SetItemMinSize(m_builtin, 100, 70);
            
            SetSizerAndFit(sizer);
        }
        
        void EntityDefinitionFileChooser::bindEvents() {
            m_builtin->Bind(wxEVT_LISTBOX, &EntityDefinitionFileChooser::OnBuiltinSelectionChanged, this);
            m_chooseExternal->Bind(wxEVT_BUTTON, &EntityDefinitionFileChooser::OnChooseExternalClicked, this);
        }

        void EntityDefinitionFileChooser::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->documentWasNewedNotifier.addObserver(this, &EntityDefinitionFileChooser::documentWasNewed);
            document->documentWasLoadedNotifier.addObserver(this, &EntityDefinitionFileChooser::documentWasLoaded);
            document->entityDefinitionsDidChangeNotifier.addObserver(this, &EntityDefinitionFileChooser::entityDefinitionsDidChange);
        }
        
        void EntityDefinitionFileChooser::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->documentWasNewedNotifier.removeObserver(this, &EntityDefinitionFileChooser::documentWasNewed);
                document->documentWasLoadedNotifier.removeObserver(this, &EntityDefinitionFileChooser::documentWasLoaded);
                document->entityDefinitionsDidChangeNotifier.removeObserver(this, &EntityDefinitionFileChooser::entityDefinitionsDidChange);
            }
        }
        
        void EntityDefinitionFileChooser::documentWasNewed() {
            updateControls();
        }
        
        void EntityDefinitionFileChooser::documentWasLoaded() {
            updateControls();
        }
        
        void EntityDefinitionFileChooser::entityDefinitionsDidChange() {
            updateControls();
        }

        void EntityDefinitionFileChooser::updateControls() {
            m_builtin->Clear();
            
            MapDocumentSPtr document = lock(m_document);
            IO::Path::List paths = document->entityDefinitionFiles();
            std::sort(paths.begin(), paths.end());
            
            IO::Path::List::const_iterator it, end;
            for (it = paths.begin(), end = paths.end(); it != end; ++it) {
                const IO::Path& path = *it;
                m_builtin->Append(path.lastComponent().asString());
            }
            
            const Model::EntityDefinitionFileSpec spec = document->entityDefinitionFile();
            if (spec.builtin()) {
                const size_t index = VectorUtils::indexOf(paths, spec.path());
                if (index < paths.size())
                    m_builtin->SetSelection(static_cast<int>(index));
                m_external->SetLabel("use builtin");
            } else {
                m_builtin->DeselectAll();
                m_external->SetLabel(spec.fullPath().asString());
            }
        }
    }
}
