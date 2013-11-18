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
#include "View/MapDocument.h"

#include <wx/choice.h>
#include <wx/sizer.h>

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

        void EntityDefinitionFileChooser::OnDefinitionFileChoice(wxCommandEvent& event) {
            assert(m_definitionFileChoice->GetSelection() >= 0);
            
            const size_t index = static_cast<size_t>(m_definitionFileChoice->GetSelection());
            if (index == m_definitionFileChoice->GetCount() - 1) {
                
            } else {
                IO::Path::List paths = m_document->definitionFiles();
                std::sort(paths.begin(), paths.end());
                const IO::Path& path = paths[index];
                
            }
        }

        void EntityDefinitionFileChooser::createGui() {
            m_definitionFileChoice = new wxChoice(this, wxID_ANY);
            m_definitionFileChoice->SetToolTip(_("Select one of the builtin entity definition files or browse to select an external file"));
            
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
#if defined __APPLE__
            sizer->Add(m_definitionFileChoice, 0, wxEXPAND | wxALL, 1);
#else
            sizer->Add(m_definitionFileChoice, 0, wxEXPAND);
#endif
            SetSizerAndFit(sizer);
        }
        
        void EntityDefinitionFileChooser::bindEvents() {
            m_definitionFileChoice->Bind(wxEVT_CHOICE, &EntityDefinitionFileChooser::OnDefinitionFileChoice, this);
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
            updateChoice();
        }
        
        void EntityDefinitionFileChooser::documentWasLoaded() {
            updateChoice();
        }
        
        void EntityDefinitionFileChooser::updateChoice() {
            m_definitionFileChoice->Clear();
            
            IO::Path::List paths = m_document->definitionFiles();
            std::sort(paths.begin(), paths.end());
            
            IO::Path::List::const_iterator it, end;
            for (it = paths.begin(), end = paths.end(); it != end; ++it) {
                const IO::Path& path = *it;
                m_definitionFileChoice->Append(path.lastComponent().asString());
            }
            
            m_definitionFileChoice->Append("Browse...");
        }
    }
}
