/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "MapPropertiesDialog.h"

#include "IO/FileManager.h"
#include "Model/Entity.h"
#include "Model/EntityDefinitionManager.h"
#include "Model/MapDocument.h"
#include "Model/TextureManager.h"
#include "Utility/Preferences.h"
#include "View/CommandIds.h"
#include "View/LayoutConstants.h"

#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/filedlg.h>
#include <wx/image.h>
#include <wx/listbox.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>


namespace TrenchBroom {
    namespace View {
        BEGIN_EVENT_TABLE(MapPropertiesDialog, wxDialog)
        EVT_CHOICE(CommandIds::MapPropertiesDialog::DefChoiceId, MapPropertiesDialog::OnDefChoiceSelected)
        EVT_CHOICE(CommandIds::MapPropertiesDialog::ModChoiceId, MapPropertiesDialog::OnModChoiceSelected)
        EVT_BUTTON(CommandIds::MapPropertiesDialog::AddWadButtonId, MapPropertiesDialog::OnAddWadClicked)
        EVT_BUTTON(CommandIds::MapPropertiesDialog::RemoveWadsButtonId, MapPropertiesDialog::OnRemoveWadsClicked)
        EVT_BUTTON(CommandIds::MapPropertiesDialog::ChangeWadPathsButtonId, MapPropertiesDialog::OnChangeWadPathsClicked)
        EVT_BUTTON(CommandIds::MapPropertiesDialog::MoveWadUpButtonId, MapPropertiesDialog::OnMoveWadUpClicked)
        EVT_BUTTON(CommandIds::MapPropertiesDialog::MoveWadDownButtonId, MapPropertiesDialog::OnMoveWadDownClicked)
        EVT_UPDATE_UI_RANGE(CommandIds::MapPropertiesDialog::AddWadButtonId, CommandIds::MapPropertiesDialog::MoveWadDownButtonId, MapPropertiesDialog::OnUpdateWadButtons)
        END_EVENT_TABLE()

        void MapPropertiesDialog::populateDefChoice(String def) {
            m_defChoice->Clear();

            int quakeDefIndex = -1;
            int selectionIndex = -1;
            const StringList builtinDefs = Model::EntityDefinitionManager::builtinDefinitionFiles();
            for (size_t i = 0; i < builtinDefs.size(); i++) {
                const String& item = builtinDefs[i];
                m_defChoice->Append(item);
                if (Utility::equalsString(item, "quake.def", false))
                    quakeDefIndex = static_cast<int>(i);
                if (Utility::startsWith(def, "builtin:") &&
                    Utility::equalsString(def.substr(8), item, false))
                    selectionIndex = static_cast<int>(i);
            }
            
            if (Utility::startsWith(def, "external:")) {
                m_defChoice->Append(def.substr(9));
                selectionIndex = static_cast<int>(m_defChoice->GetCount() - 1);
            }
            
            if (selectionIndex == -1)
                selectionIndex = quakeDefIndex;
            
            m_defChoice->Append("Choose...");
            m_defChoice->SetSelection(selectionIndex);
        }

        void MapPropertiesDialog::populateModChoice(String mod) {
            m_modChoice->Clear();

            IO::FileManager fileManager;
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            const String& quakePath = prefs.getString(Preferences::QuakePath);
            const StringList modDirs = fileManager.directoryContents(quakePath, "", true, false);
            
            int id1Index = -1;
            int selectionIndex = -1;
            for (size_t i = 0; i < modDirs.size(); i++) {
                const String& item = modDirs[i];
                m_modChoice->Append(item);
                if (Utility::equalsString(item, "id1", false))
                    id1Index = static_cast<int>(i);
                if (Utility::equalsString(mod, item, false))
                    selectionIndex = static_cast<int>(i);
            }
            
            if (selectionIndex == -1)
                selectionIndex = id1Index;
            m_modChoice->SetSelection(selectionIndex);
        }

        void MapPropertiesDialog::populateWadList() {
            m_wadList->Clear();
            
            Model::TextureManager& textureManager = m_document.textureManager();
            const Model::TextureCollectionList& collections = textureManager.collections();
            
            Model::TextureCollectionList::const_iterator it, end;
            for (it = collections.begin(), end = collections.end(); it != end; ++it) {
                const Model::TextureCollection& collection = **it;
                m_wadList->Append(collection.name());
            }
        }

        void MapPropertiesDialog::init() {
            String def = "";
            String mod = "id1";
            
            Model::Entity* worldspawn = m_document.worldspawn(false);
            if (worldspawn != NULL) {
                const Model::PropertyValue* defValue = worldspawn->propertyForKey(Model::Entity::DefKey);
                if (defValue != NULL)
                    def = *defValue;
                const Model::PropertyValue* modValue = worldspawn->propertyForKey(Model::Entity::ModKey);
                if (modValue != NULL)
                    mod = *modValue;
            }
            
            populateDefChoice(def);
            populateModChoice(mod);
            populateWadList();
        }

        MapPropertiesDialog::MapPropertiesDialog(Model::MapDocument& document) :
        wxDialog(NULL, wxID_ANY, wxT("Map Properties")),
        m_document(document) {
            wxStaticBox* modBox = new wxStaticBox(this, wxID_ANY, wxT("Entity Definitions"));
            wxStaticText* defText = new wxStaticText(modBox, wxID_ANY, wxT("Select an entity definition file for this map."));
            defText->SetFont(*wxSMALL_FONT);
            m_defChoice = new wxChoice(modBox, CommandIds::MapPropertiesDialog::DefChoiceId, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_SORT);

            wxStaticText* modText = new wxStaticText(modBox, wxID_ANY, wxT("Select a subdirectory within your Quake directory to search for entity models. ID1 is always searched in addition to the selected subdirectory."));
            modText->SetFont(*wxSMALL_FONT);
            modText->SetMinSize(wxSize(wxDefaultSize.x, 45));
            m_modChoice = new wxChoice(modBox, CommandIds::MapPropertiesDialog::ModChoiceId, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_SORT);
            
            wxSizer* modBoxSizer = new wxBoxSizer(wxVERTICAL);
            modBoxSizer->Add(defText, 0, wxEXPAND | wxLEFT | wxTOP | wxRIGHT, LayoutConstants::StaticBoxInnerMargin);
            modBoxSizer->Add(m_defChoice, 0, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::StaticBoxInnerMargin);
            modBoxSizer->AddSpacer(LayoutConstants::ControlMargin);
            modBoxSizer->AddSpacer(LayoutConstants::ControlMargin);
            modBoxSizer->Add(modText, 0, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::StaticBoxInnerMargin);
            modBoxSizer->Add(m_modChoice, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, LayoutConstants::StaticBoxInnerMargin);
            modBox->SetSizerAndFit(modBoxSizer);

            IO::FileManager fileManager;
            String resourcePath = fileManager.resourceDirectory();
            
            wxBitmap add(fileManager.appendPath(resourcePath, "Add.png"), wxBITMAP_TYPE_PNG);
            wxBitmap remove(fileManager.appendPath(resourcePath, "Remove.png"), wxBITMAP_TYPE_PNG);
            wxBitmap changePath(fileManager.appendPath(resourcePath, "Path.png"), wxBITMAP_TYPE_PNG);
            wxBitmap up(fileManager.appendPath(resourcePath, "Up.png"), wxBITMAP_TYPE_PNG);
            wxBitmap down(fileManager.appendPath(resourcePath, "Down.png"), wxBITMAP_TYPE_PNG);
            
            wxStaticBox* wadBox = new wxStaticBox(this, wxID_ANY, wxT("Texture Wads"));
            wxStaticText* wadText = new wxStaticText(wadBox, wxID_ANY, wxT("Manage the wad files for this map. Wad files are searched from bottom to top, so textures in the lower entries override textures in the upper entries if the names of the textures are the same."));
            wadText->SetFont(*wxSMALL_FONT);
            wadText->SetMinSize(wxSize(wxDefaultSize.x, 60));
            m_wadList = new wxListBox(wadBox, CommandIds::MapPropertiesDialog::WadListId, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_MULTIPLE);
            m_addWadButton = new wxBitmapButton(wadBox, CommandIds::MapPropertiesDialog::AddWadButtonId, add, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN);
            m_addWadButton->SetMinSize(wxSize(20, 20));
            m_removeWadsButton = new wxBitmapButton(wadBox, CommandIds::MapPropertiesDialog::RemoveWadsButtonId, remove, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN);
            m_removeWadsButton->SetMinSize(wxSize(20, 20));
            m_changeWadPathsButton = new wxBitmapButton(wadBox, CommandIds::MapPropertiesDialog::ChangeWadPathsButtonId, changePath, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN);
            m_changeWadPathsButton->SetMinSize(wxSize(20, 20));
            m_moveWadUpButton = new wxBitmapButton(wadBox, CommandIds::MapPropertiesDialog::MoveWadUpButtonId, up, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN);
            m_moveWadUpButton->SetMinSize(wxSize(20, 20));
            m_moveWadDownButton = new wxBitmapButton(wadBox, CommandIds::MapPropertiesDialog::MoveWadDownButtonId, down, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN);
            m_moveWadDownButton->SetMinSize(wxSize(20, 20));
            
            wxSizer* wadButtonsSizer = new wxBoxSizer(wxHORIZONTAL);
            wadButtonsSizer->Add(m_addWadButton);
            wadButtonsSizer->AddSpacer(LayoutConstants::ControlHorizontalMargin);
            wadButtonsSizer->Add(m_removeWadsButton);
            wadButtonsSizer->AddSpacer(LayoutConstants::ControlHorizontalMargin);
            wadButtonsSizer->Add(m_changeWadPathsButton);
            wadButtonsSizer->AddSpacer(LayoutConstants::ControlHorizontalMargin);
            wadButtonsSizer->Add(m_moveWadUpButton);
            wadButtonsSizer->AddSpacer(LayoutConstants::ControlHorizontalMargin);
            wadButtonsSizer->Add(m_moveWadDownButton);
            
            wxSizer* wadBoxSizer = new wxBoxSizer(wxVERTICAL);
            wadBoxSizer->Add(wadText, 0, wxEXPAND | wxLEFT | wxTOP | wxRIGHT, LayoutConstants::StaticBoxInnerMargin);
            wadBoxSizer->Add(m_wadList, 1, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::StaticBoxInnerMargin);
            wadBoxSizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
            wadBoxSizer->Add(wadButtonsSizer, 0, wxEXPAND | wxLEFT | wxBOTTOM | wxRIGHT | wxALIGN_LEFT, LayoutConstants::StaticBoxInnerMargin);
            wadBox->SetSizerAndFit(wadBoxSizer);
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(modBox, 0, wxEXPAND | wxLEFT | wxTOP | wxRIGHT, LayoutConstants::DialogOuterMargin);
            outerSizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
            outerSizer->Add(wadBox, 1, wxEXPAND | wxLEFT | wxBOTTOM | wxRIGHT, LayoutConstants::DialogOuterMargin);
            
            SetSizer(outerSizer);
            SetSize(350, 450);
            
            init();
        }

        void MapPropertiesDialog::OnDefChoiceSelected(wxCommandEvent& event) {
        }
        
        void MapPropertiesDialog::OnModChoiceSelected(wxCommandEvent& event) {
        }
        
        void MapPropertiesDialog::OnAddWadClicked(wxCommandEvent& event) {
        }
        
        void MapPropertiesDialog::OnRemoveWadsClicked(wxCommandEvent& event) {
        }
        
        void MapPropertiesDialog::OnChangeWadPathsClicked(wxCommandEvent& event) {
        }
        
        void MapPropertiesDialog::OnMoveWadUpClicked(wxCommandEvent& event) {
        }
        
        void MapPropertiesDialog::OnMoveWadDownClicked(wxCommandEvent& event) {
        }
        
        void MapPropertiesDialog::OnUpdateWadButtons(wxUpdateUIEvent& event) {
            wxArrayInt selection;
            m_wadList->GetSelections(selection);
            
            switch (event.GetId()) {
                case CommandIds::MapPropertiesDialog::AddWadButtonId:
                    event.Enable(true);
                    break;
                case CommandIds::MapPropertiesDialog::RemoveWadsButtonId:
                    event.Enable(!selection.empty());
                    break;
                case CommandIds::MapPropertiesDialog::ChangeWadPathsButtonId:
                    event.Enable(!selection.empty());
                    break;
                case CommandIds::MapPropertiesDialog::MoveWadUpButtonId:
                    event.Enable(selection.size() == 1 &&
                                 selection[0] > 0);
                    break;
                case CommandIds::MapPropertiesDialog::MoveWadDownButtonId:
                    event.Enable(selection.size() == 1 &&
                                 selection[0] < static_cast<int>(m_wadList->GetCount() - 1));
                    break;
                default:
                    event.Enable(false);
                    break;
            }
        }
    }
}
