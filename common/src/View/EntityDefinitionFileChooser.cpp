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
#include "Assets/EntityDefinitionFileSpec.h"
#include "IO/Path.h"
#include "Model/Game.h"
#include "Model/GameFactory.h"
#include "View/BorderLine.h"
#include "View/ChoosePathTypeDialog.h"
#include "View/MapDocument.h"
#include "View/ViewConstants.h"
#include "View/ViewUtils.h"

#include <wx/button.h>
#include <wx/filedlg.h>
#include <wx/listbox.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        EntityDefinitionFileChooser::EntityDefinitionFileChooser(wxWindow* parent, MapDocumentWPtr document) :
        wxPanel(parent),
        m_document(document) {
            createGui();
            bindEvents();
            bindObservers();
        }
        
        EntityDefinitionFileChooser::~EntityDefinitionFileChooser() {
            unbindObservers();
        }

        void EntityDefinitionFileChooser::OnBuiltinSelectionChanged(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            assert(m_builtin->GetSelection() != wxNOT_FOUND);

            MapDocumentSPtr document = lock(m_document);

            Assets::EntityDefinitionFileSpec::List specs = document->allEntityDefinitionFiles();
            VectorUtils::sort(specs);
            
            const size_t index = static_cast<size_t>(m_builtin->GetSelection());
            assert(index < specs.size());
            const Assets::EntityDefinitionFileSpec& spec = specs[index];
            
            document->setEntityDefinitionFile(spec);
        }
        
        void EntityDefinitionFileChooser::OnChooseExternalClicked(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            const wxString pathWxStr = ::wxFileSelector("Load Entity Definition File",
                                                        wxEmptyString, wxEmptyString, wxEmptyString,
                                                        "Worldcraft / Hammer files (*.fgd)|*.fgd|QuakeC files (*.def)|*.def",
                                                        wxFD_OPEN | wxFD_FILE_MUST_EXIST);
            if (pathWxStr.empty())
                return;
            
            loadEntityDefinitionFile(m_document, this, pathWxStr);
        }

        void EntityDefinitionFileChooser::OnReloadExternalClicked(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            const Assets::EntityDefinitionFileSpec& spec = document->entityDefinitionFile();
            document->setEntityDefinitionFile(spec);
        }
        
        void EntityDefinitionFileChooser::OnUpdateReloadExternal(wxUpdateUIEvent& event) {
            if (IsBeingDeleted()) return;

            event.Enable(lock(m_document)->entityDefinitionFile().external());
        }

        void EntityDefinitionFileChooser::createGui() {
            static const int ListBoxMargin =
#ifdef __APPLE__
            0;
#else
            LayoutConstants::NarrowHMargin;
#endif

            wxStaticText* builtinHeader = new wxStaticText(this, wxID_ANY, "Builtin");
            builtinHeader->SetFont(builtinHeader->GetFont().Bold());
            m_builtin = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxBORDER_NONE);
            
            wxStaticText* externalHeader = new wxStaticText(this, wxID_ANY, "External");
            externalHeader->SetFont(externalHeader->GetFont().Bold());
            m_external = new wxStaticText(this, wxID_ANY, "use builtin", wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_MIDDLE);
            m_chooseExternal = new wxButton(this, wxID_ANY, "Browse...", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
            m_chooseExternal->SetToolTip("Click to browse for an entity definition file");
            m_reloadExternal = new wxButton(this, wxID_ANY, "Reload", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
            m_reloadExternal->SetToolTip("Reload the currently loaded entity definition file");

            wxSizer* externalSizer = new wxBoxSizer(wxHORIZONTAL);
            externalSizer->AddSpacer(LayoutConstants::NarrowHMargin);
            externalSizer->Add(m_external, 1, wxEXPAND | wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            externalSizer->AddSpacer(LayoutConstants::NarrowHMargin);
            externalSizer->Add(m_chooseExternal, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            externalSizer->AddSpacer(LayoutConstants::NarrowHMargin);
            externalSizer->Add(m_reloadExternal, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            externalSizer->AddSpacer(LayoutConstants::NarrowHMargin);
            
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(builtinHeader, 0, wxLEFT | wxRIGHT, LayoutConstants::NarrowHMargin);
            sizer->AddSpacer(LayoutConstants::NarrowVMargin);
            sizer->Add(m_builtin, 1, wxEXPAND | wxLEFT | wxRIGHT, ListBoxMargin);
            sizer->Add(new BorderLine(this, BorderLine::Direction_Horizontal), 0, wxEXPAND);
            sizer->Add(externalHeader, 0, wxLEFT | wxRIGHT, LayoutConstants::NarrowHMargin);
            sizer->AddSpacer(LayoutConstants::NarrowVMargin);
            sizer->Add(externalSizer, 0, wxEXPAND);
            sizer->SetItemMinSize(m_builtin, 100, 70);
            
            SetBackgroundColour(*wxWHITE);
            SetSizerAndFit(sizer);
        }
        
        void EntityDefinitionFileChooser::bindEvents() {
            m_builtin->Bind(wxEVT_LISTBOX, &EntityDefinitionFileChooser::OnBuiltinSelectionChanged, this);
            m_chooseExternal->Bind(wxEVT_BUTTON, &EntityDefinitionFileChooser::OnChooseExternalClicked, this);
            m_reloadExternal->Bind(wxEVT_BUTTON, &EntityDefinitionFileChooser::OnReloadExternalClicked, this);
            m_reloadExternal->Bind(wxEVT_UPDATE_UI, &EntityDefinitionFileChooser::OnUpdateReloadExternal, this);
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
        
        void EntityDefinitionFileChooser::documentWasNewed(MapDocument* document) {
            updateControls();
        }
        
        void EntityDefinitionFileChooser::documentWasLoaded(MapDocument* document) {
            updateControls();
        }
        
        void EntityDefinitionFileChooser::entityDefinitionsDidChange() {
            updateControls();
        }

        void EntityDefinitionFileChooser::updateControls() {
            m_builtin->Clear();
            
            MapDocumentSPtr document = lock(m_document);
            Assets::EntityDefinitionFileSpec::List specs = document->allEntityDefinitionFiles();
            VectorUtils::sort(specs);
            
            Assets::EntityDefinitionFileSpec::List::const_iterator it, end;
            for (it = specs.begin(), end = specs.end(); it != end; ++it) {
                const Assets::EntityDefinitionFileSpec& spec = *it;
                const IO::Path& path = spec.path();
                m_builtin->Append(path.lastComponent().asString());
            }
            
            const Assets::EntityDefinitionFileSpec spec = document->entityDefinitionFile();
            if (spec.builtin()) {
                const size_t index = VectorUtils::indexOf(specs, spec);
                if (index < specs.size())
                    m_builtin->SetSelection(static_cast<int>(index));
                m_external->SetLabel("use builtin");
                m_external->SetForegroundColour(Colors::disabledText());
                
                wxFont font = m_external->GetFont();
                font.SetStyle(wxFONTSTYLE_ITALIC);
                m_external->SetFont(font);
            } else {
                m_builtin->DeselectAll();
                m_external->SetLabel(spec.path().asString());
                m_external->SetForegroundColour(*wxBLACK);

                wxFont font = m_external->GetFont();
                font.SetStyle(wxFONTSTYLE_NORMAL);
                m_external->SetFont(font);
            }
        }
    }
}
