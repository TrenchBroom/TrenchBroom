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
        END_EVENT_TABLE()

        MapPropertiesDialog::MapPropertiesDialog() :
        wxDialog(NULL, wxID_ANY, wxT("Map Properties")) {
            wxStaticBox* modBox = new wxStaticBox(this, wxID_ANY, wxT("Entity Definitions"));
            wxStaticText* defText = new wxStaticText(modBox, wxID_ANY, wxT("Select an entity definition file for this map."));
            defText->SetFont(*wxSMALL_FONT);
            m_defChoice = new wxChoice(modBox, CommandIds::MapPropertiesDialog::DefChoiceId);

            wxStaticText* modText = new wxStaticText(modBox, wxID_ANY, wxT("Select a subdirectory within your Quake directory to search for entity models. ID1 is always searched."));
            modText->SetFont(*wxSMALL_FONT);
            m_modChoice = new wxChoice(modBox, CommandIds::MapPropertiesDialog::ModListBoxId);
            
            wxSizer* modBoxSizer = new wxBoxSizer(wxVERTICAL);
            modBoxSizer->Add(defText, 0, wxEXPAND | wxLEFT | wxTOP | wxRIGHT, LayoutConstants::StaticBoxInnerMargin);
            modBoxSizer->Add(m_defChoice, 0, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::StaticBoxInnerMargin);
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
            outerSizer->Add(wadBox, 0, wxEXPAND | wxLEFT | wxBOTTOM | wxRIGHT, LayoutConstants::DialogOuterMargin);
            
            SetSizeHints(350, wxDefaultCoord, 350, 1000);
            SetSizer(outerSizer);
        }
    }
}
