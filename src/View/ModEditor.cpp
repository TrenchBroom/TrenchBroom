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

#include "ModEditor.h"

#include "Notifier.h"
#include "Model/Entity.h"
#include "Model/Object.h"
#include "IO/Path.h"
#include "IO/ResourceUtils.h"
#include "View/LayoutConstants.h"
#include "View/MapDocument.h"

#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>

namespace TrenchBroom {
    namespace View {
        ModEditor::ModEditor(wxWindow* parent, MapDocumentPtr document, ControllerPtr controller) :
        wxPanel(parent),
        m_document(document),
        m_controller(controller) {
            createGui();
            bindEvents();
            bindObservers();
        }

        ModEditor::~ModEditor() {
            unbindObservers();
        }

        void ModEditor::createGui() {
            wxStaticText* label = new wxStaticText(this, wxID_ANY, _("Mods: "));
            m_modList = new wxStaticText(this, wxID_ANY, _("none"));
            m_modList->SetToolTip("The currently used mods");
            
            const wxBitmap editImage = IO::loadImageResource(IO::Path("images/EditMods.png"));
            m_editModsButton = new wxBitmapButton(this, wxID_ANY, editImage, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN);
            m_editModsButton->SetToolTip(_("Edit the mods used by this map"));
            
            wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
            sizer->Add(label, 0, wxEXPAND);
            sizer->Add(m_modList, 1, wxEXPAND);
            sizer->AddSpacer(LayoutConstants::ControlHorizontalMargin);
            sizer->Add(m_editModsButton, 0, wxEXPAND);
#if defined __APPLE__
            sizer->SetItemMinSize(m_editModsButton, wxDefaultSize.x, 22);
#endif
            SetSizerAndFit(sizer);
        }
        
        void ModEditor::bindEvents() {
        }

        void ModEditor::bindObservers() {
            m_document->documentWasNewedNotifier.addObserver(this, &ModEditor::documentWasNewed);
            m_document->documentWasLoadedNotifier.addObserver(this, &ModEditor::documentWasLoaded);
        }
        
        void ModEditor::unbindObservers() {
            m_document->documentWasNewedNotifier.removeObserver(this, &ModEditor::documentWasNewed);
            m_document->documentWasLoadedNotifier.removeObserver(this, &ModEditor::documentWasLoaded);
        }
        
        void ModEditor::documentWasNewed() {
            updateMods();
        }
        
        void ModEditor::documentWasLoaded() {
            updateMods();
        }
        
        void ModEditor::objectDidChange(Model::Object* object) {
            if (object->type() == Model::Object::OTEntity) {
                const Model::Entity* entity = static_cast<Model::Entity*>(object);
                if (entity->worldspawn())
                    updateMods();
            }
        }

        void ModEditor::updateMods() {
            const StringList mods = m_document->mods();
            if (mods.empty())
                m_modList->SetLabel(_("none"));
            else
                m_modList->SetLabel(StringUtils::join(mods, ", "));
        }
    }
}
