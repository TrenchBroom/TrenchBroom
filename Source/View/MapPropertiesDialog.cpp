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

#include "Controller/EntityPropertyCommand.h"
#include "IO/FileManager.h"
#include "Model/Entity.h"
#include "Model/EntityDefinitionManager.h"
#include "Model/MapDocument.h"
#include "Model/Map.h"
#include "Model/TextureManager.h"
#include "Utility/CommandProcessor.h"
#include "Utility/List.h"
#include "Utility/Preferences.h"
#include "View/CommandIds.h"
#include "View/LayoutConstants.h"
#include "View/PathDialog.h"

#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/filedlg.h>
#include <wx/image.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/stdpaths.h>

namespace TrenchBroom {
    namespace View {
        WadListBox::WadListBox(wxWindow* parent, wxWindowID windowId) :
        wxVListBox(parent, windowId, wxDefaultPosition, wxDefaultSize, wxLB_MULTIPLE | wxBORDER_SUNKEN) {
            SetItemCount(m_wadFiles.size());
        }

        void WadListBox::OnDrawItem (wxDC &dc, const wxRect &rect, size_t n) const {
            assert(n < m_wadFiles.size());

            int width = std::min(rect.width, GetClientSize().x);

            const String name = m_wadFiles[n];
            const wxString shortString = wxControl::Ellipsize(name, dc, wxELLIPSIZE_MIDDLE, width);

            if (IsSelected(n))
                dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXHIGHLIGHTTEXT));
            else
                dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXTEXT));
            dc.DrawText(shortString, rect.x, rect.y);
        }

        void WadListBox::OnDrawBackground (wxDC &dc, const wxRect &rect, size_t n) const {
            if (IsSelected(n)) {
                dc.SetPen(*wxTRANSPARENT_PEN);
                dc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT)));
                dc.DrawRectangle(rect);
            }
        }

        wxCoord WadListBox::OnMeasureItem (size_t n) const {
            assert(n < m_wadFiles.size());
            return GetFont().GetPixelSize().y;
        }

        void WadListBox::GetSelections(wxArrayInt& selection) const {
            selection.clear();

            unsigned long cookie;
            int index = GetFirstSelected(cookie);
            while (index != wxNOT_FOUND) {
                selection.push_back(index);
                index = GetNextSelected(cookie);
            }
        }

        void WadListBox::addWad(const String path) {
            m_wadFiles.push_back(path);
            SetItemCount(m_wadFiles.size());
            Refresh();
        }

        void WadListBox::moveWadUp(size_t index) {
            assert(index > 0 && index < m_wadFiles.size());
            std::swap(m_wadFiles[index - 1], m_wadFiles[index]);
            SetItemCount(m_wadFiles.size());
            Refresh();
        }

        void WadListBox::moveWadDown(size_t index) {
            assert(index < m_wadFiles.size() - 1);
            std::swap(m_wadFiles[index + 1], m_wadFiles[index]);
            SetItemCount(m_wadFiles.size());
            Refresh();
        }

        void WadListBox::removeWad(size_t index) {
            assert(index < m_wadFiles.size());
            Utility::erase(m_wadFiles, index);
            SetItemCount(m_wadFiles.size());
            Refresh();
        }

        void WadListBox::removeWads(const wxArrayInt& indices) {
            StringList newList;
            for (size_t i = 0; i < m_wadFiles.size(); i++) {
                if (std::find(indices.begin(), indices.end(), i) == indices.end())
                    newList.push_back(m_wadFiles[i]);
            }
            m_wadFiles = newList;
            SetItemCount(m_wadFiles.size());
            Refresh();
        }

        void WadListBox::setWadString(const String& wadString) {
            m_wadFiles.clear();
            const StringList wadPaths = Utility::split(wadString, ';');
            for (size_t i = 0; i < wadPaths.size(); i++) {
                const String wadPath = Utility::trim(wadPaths[i]);
                if (!wadPath.empty())
                    m_wadFiles.push_back(wadPath);
            }
            SetItemCount(m_wadFiles.size());
            Refresh();
        }

        String WadListBox::wadString() const {
            return Utility::join(m_wadFiles, ";");
        }

        IMPLEMENT_DYNAMIC_CLASS(MapPropertiesDialog, wxDialog)

        BEGIN_EVENT_TABLE(MapPropertiesDialog, wxDialog)
        EVT_CHOICE(CommandIds::MapPropertiesDialog::DefChoiceId, MapPropertiesDialog::OnDefChoiceSelected)
        EVT_CHOICE(CommandIds::MapPropertiesDialog::ModChoiceId, MapPropertiesDialog::OnModChoiceSelected)
        EVT_CHECKBOX(CommandIds::MapPropertiesDialog::ForceIntCoordsId, MapPropertiesDialog::OnIntFacePointsCheckBoxClicked)
        EVT_BUTTON(CommandIds::MapPropertiesDialog::AddWadButtonId, MapPropertiesDialog::OnAddWadClicked)
        EVT_BUTTON(CommandIds::MapPropertiesDialog::RemoveWadsButtonId, MapPropertiesDialog::OnRemoveWadsClicked)
        EVT_BUTTON(CommandIds::MapPropertiesDialog::MoveWadUpButtonId, MapPropertiesDialog::OnMoveWadUpClicked)
        EVT_BUTTON(CommandIds::MapPropertiesDialog::MoveWadDownButtonId, MapPropertiesDialog::OnMoveWadDownClicked)
        EVT_UPDATE_UI_RANGE(CommandIds::MapPropertiesDialog::AddWadButtonId, CommandIds::MapPropertiesDialog::MoveWadDownButtonId, MapPropertiesDialog::OnUpdateWadButtons)
        EVT_BUTTON(wxID_CLOSE, MapPropertiesDialog::OnCloseClicked)
        EVT_MENU(wxID_CLOSE, MapPropertiesDialog::OnFileExit)
        END_EVENT_TABLE()

        void MapPropertiesDialog::populateDefChoice(const String& def) {
            m_defChoice->Clear();

            int quakeDefIndex = -1;
            int selectionIndex = -1;
            const StringList builtinDefs = Model::EntityDefinitionManager::builtinDefinitionFiles();

            for (size_t i = 0; i < builtinDefs.size(); i++) {
                const String& item = builtinDefs[i];
                m_defChoice->Append(item);
                if (Utility::equalsString(item, Model::Entity::DefaultDefinition, false))
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

        void MapPropertiesDialog::populateModChoice(const String& mod) {
            m_modChoice->Clear();

            IO::FileManager fileManager;
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            const String& quakePath = prefs.getString(Preferences::QuakePath);
            if (fileManager.exists(quakePath) && fileManager.isDirectory(quakePath)) {
                const StringList modDirs = fileManager.directoryContents(quakePath, "", true, false);

                int id1Index = -1;
                int selectionIndex = -1;
                for (size_t i = 0; i < modDirs.size(); i++) {
                    const String& item = modDirs[i];
                    m_modChoice->Append(item);
                    if (Utility::equalsString(item, "id1", false))
                        id1Index = static_cast<int>(i);
                    if (Utility::equalsString(item, mod, false))
                        selectionIndex = static_cast<int>(i);
                }

                if (selectionIndex == -1)
                    selectionIndex = id1Index;
                m_modChoice->SetSelection(selectionIndex);
            }
        }

        void MapPropertiesDialog::init() {
            String def = "";
            String mod = "id1";

            Model::Entity& worldspawn = m_document->worldspawn();
            const Model::PropertyValue* defValue = worldspawn.propertyForKey(Model::Entity::DefKey);
            if (defValue != NULL)
                def = *defValue;
            const Model::PropertyValue* modValue = worldspawn.propertyForKey(Model::Entity::ModKey);
            if (modValue != NULL)
                mod = *modValue;

            populateDefChoice(def);
            populateModChoice(mod);

            bool forceIntegerCoordinates = false;
            const Model::PropertyValue* value = worldspawn.propertyForKey(Model::Entity::FacePointFormatKey);
            if (value != NULL && *value == "1")
                forceIntegerCoordinates = true;
            assert(forceIntegerCoordinates == m_document->map().forceIntegerFacePoints());
            m_intFacePointsCheckBox->SetValue(forceIntegerCoordinates);

            String wad = "";
            const Model::PropertyValue* wadValue = m_document->worldspawn().propertyForKey(Model::Entity::WadKey);
            if (wadValue != NULL)
                wad = *wadValue;

            m_wadList->setWadString(wad);
        }

        void MapPropertiesDialog::updateWadProperty() {
            Controller::EntityPropertyCommand* command = Controller::EntityPropertyCommand::setEntityPropertyValue(*m_document, m_document->worldspawn(), Model::Entity::WadKey, m_wadList->wadString(), true);
            m_document->GetCommandProcessor()->Submit(command);
        }

        MapPropertiesDialog::MapPropertiesDialog() :
        wxDialog(),
        m_document(NULL),
        m_modChoice(NULL),
        m_defChoice(NULL),
        m_intFacePointsCheckBox(NULL),
        m_wadList(NULL),
        m_addWadButton(NULL),
        m_removeWadsButton(NULL),
        m_moveWadUpButton(NULL),
        m_moveWadDownButton(NULL) {}

        MapPropertiesDialog::MapPropertiesDialog(wxWindow* parent, Model::MapDocument& document) :
        wxDialog(),
        m_document(NULL),
        m_modChoice(NULL),
        m_defChoice(NULL),
        m_intFacePointsCheckBox(NULL),
        m_wadList(NULL),
        m_addWadButton(NULL),
        m_removeWadsButton(NULL),
        m_moveWadUpButton(NULL),
        m_moveWadDownButton(NULL) {
            Create(parent, document);
        }

        void MapPropertiesDialog::Create(wxWindow* parent, Model::MapDocument& document) {
            wxDialog::Create(parent, wxID_ANY, wxT("Map Properties"));
            m_document = &document;

            const int width = 330;

            wxStaticBox* modBox = new wxStaticBox(this, wxID_ANY, wxT("Entity Definitions"));
            wxStaticText* defText = new wxStaticText(modBox, wxID_ANY, wxT("Select an entity definition file for this map."));
#if defined __APPLE__
            defText->SetFont(*wxSMALL_FONT);
#endif
            defText->Wrap(width);
            m_defChoice = new wxChoice(modBox, CommandIds::MapPropertiesDialog::DefChoiceId);

            wxStaticText* modText = new wxStaticText(modBox, wxID_ANY, wxT("Select a subdirectory within your Quake directory to search for entity models. ID1 is always searched in addition to the selected subdirectory."));
#if defined __APPLE__
            modText->SetFont(*wxSMALL_FONT);
#endif
            modText->Wrap(width);
            m_modChoice = new wxChoice(modBox, CommandIds::MapPropertiesDialog::ModChoiceId);

            wxSizer* modBoxSizer = new wxBoxSizer(wxVERTICAL);
            modBoxSizer->AddSpacer(LayoutConstants::StaticBoxTopMargin);
            modBoxSizer->Add(defText, 0, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::StaticBoxSideMargin);
            modBoxSizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
            modBoxSizer->Add(m_defChoice, 0, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::StaticBoxSideMargin);
            modBoxSizer->AddSpacer(2 * LayoutConstants::ControlVerticalMargin);
            modBoxSizer->Add(modText, 0, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::StaticBoxSideMargin);
            modBoxSizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
            modBoxSizer->Add(m_modChoice, 0, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::StaticBoxSideMargin);
            modBoxSizer->AddSpacer(LayoutConstants::StaticBoxBottomMargin);
            modBox->SetSizerAndFit(modBoxSizer);

            IO::FileManager fileManager;
            String resourcePath = fileManager.resourceDirectory();

            wxBitmap add(fileManager.appendPath(resourcePath, "Add.png"), wxBITMAP_TYPE_PNG);
            wxBitmap remove(fileManager.appendPath(resourcePath, "Remove.png"), wxBITMAP_TYPE_PNG);
            wxBitmap up(fileManager.appendPath(resourcePath, "Up.png"), wxBITMAP_TYPE_PNG);
            wxBitmap down(fileManager.appendPath(resourcePath, "Down.png"), wxBITMAP_TYPE_PNG);

            wxStaticBox* wadBox = new wxStaticBox(this, wxID_ANY, wxT("Texture Wads"));
            wxStaticText* wadText = new wxStaticText(wadBox, wxID_ANY, wxT("Manage the wad files for this map. Wad files are searched from bottom to top, so textures in the lower entries override textures in the upper entries if the names of the textures are the same."));
#if defined __APPLE__
            wadText->SetFont(*wxSMALL_FONT);
#endif
            wadText->Wrap(width);

            m_wadList = new WadListBox(wadBox, CommandIds::MapPropertiesDialog::WadListId);
            m_wadList->SetMinSize(wxSize(wxDefaultSize.x, 120));
            m_addWadButton = new wxBitmapButton(wadBox, CommandIds::MapPropertiesDialog::AddWadButtonId, add, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN);
            m_addWadButton->SetMinSize(wxSize(20, 20));
            m_removeWadsButton = new wxBitmapButton(wadBox, CommandIds::MapPropertiesDialog::RemoveWadsButtonId, remove, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN);
            m_removeWadsButton->SetMinSize(wxSize(20, 20));
            m_moveWadUpButton = new wxBitmapButton(wadBox, CommandIds::MapPropertiesDialog::MoveWadUpButtonId, up, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN);
            m_moveWadUpButton->SetMinSize(wxSize(20, 20));
            m_moveWadDownButton = new wxBitmapButton(wadBox, CommandIds::MapPropertiesDialog::MoveWadDownButtonId, down, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN);
            m_moveWadDownButton->SetMinSize(wxSize(20, 20));

            wxSizer* wadButtonsSizer = new wxBoxSizer(wxHORIZONTAL);
            wadButtonsSizer->Add(m_addWadButton);
            wadButtonsSizer->AddSpacer(LayoutConstants::ControlHorizontalMargin);
            wadButtonsSizer->Add(m_removeWadsButton);
            wadButtonsSizer->AddSpacer(LayoutConstants::ControlHorizontalMargin);
            wadButtonsSizer->Add(m_moveWadUpButton);
            wadButtonsSizer->AddSpacer(LayoutConstants::ControlHorizontalMargin);
            wadButtonsSizer->Add(m_moveWadDownButton);

            wxSizer* wadBoxSizer = new wxBoxSizer(wxVERTICAL);
            wadBoxSizer->AddSpacer(LayoutConstants::StaticBoxTopMargin);
            wadBoxSizer->Add(wadText, 0, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::StaticBoxSideMargin);
            wadBoxSizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
            wadBoxSizer->Add(m_wadList, 0, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::StaticBoxSideMargin);
            wadBoxSizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
            wadBoxSizer->Add(wadButtonsSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxALIGN_LEFT, LayoutConstants::StaticBoxSideMargin);
            wadBoxSizer->AddSpacer(LayoutConstants::StaticBoxBottomMargin);
            wadBox->SetSizerAndFit(wadBoxSizer);

            wxStaticBox* coordBox = new wxStaticBox(this, wxID_ANY, wxT("Plane Point Coordinates"));
            wxStaticText* coordText = new wxStaticText(coordBox, wxID_ANY, wxT("By default, TrenchBroom stores plane point coordinates as floating point values internally and in the map file. Checking this option will force it to use integer coordinates. This improves compatibility with older compilers, but it will lead to less precision when editing vertices."));
#if defined __APPLE__
            coordText->SetFont(*wxSMALL_FONT);
#endif
            coordText->Wrap(width);
            m_intFacePointsCheckBox = new wxCheckBox(coordBox, CommandIds::MapPropertiesDialog::ForceIntCoordsId, wxT("Force integer plane points"));

            wxSizer* coordBoxSizer = new wxBoxSizer(wxVERTICAL);
            coordBoxSizer->AddSpacer(LayoutConstants::StaticBoxTopMargin);
            coordBoxSizer->Add(coordText, 0, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::StaticBoxSideMargin);
            coordBoxSizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
            coordBoxSizer->Add(m_intFacePointsCheckBox, 0, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::StaticBoxSideMargin);
            coordBoxSizer->AddSpacer(LayoutConstants::StaticBoxBottomMargin);
            coordBox->SetSizerAndFit(coordBoxSizer);

            wxSizer* buttonSizer = CreateButtonSizer(wxCLOSE);
            SetAffirmativeId(wxCLOSE);
            SetEscapeId(wxCLOSE);

            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(modBox, 0, wxEXPAND | wxLEFT | wxTOP | wxRIGHT, LayoutConstants::DialogOuterMargin);
            outerSizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
            outerSizer->Add(wadBox, 1, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::DialogOuterMargin);
            outerSizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
            outerSizer->Add(coordBox, 0, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::DialogOuterMargin);
            outerSizer->Add(buttonSizer, 0, wxEXPAND | wxALL, LayoutConstants::DialogButtonMargin);

            SetSizerAndFit(outerSizer);

#ifdef __APPLE__
            // allow the dialog to be closed using CMD+W
            wxAcceleratorEntry acceleratorEntries[1];
            acceleratorEntries[0].Set(wxACCEL_CMD, static_cast<int>('W'), wxID_CLOSE);
            wxAcceleratorTable accceleratorTable(1, acceleratorEntries);
            SetAcceleratorTable(accceleratorTable);
#endif

            init();

            CommandProcessor::BeginGroup(m_document->GetCommandProcessor(), "Edit map properties");
        }

        void MapPropertiesDialog::EndModal(int retCode) {
            CommandProcessor::EndGroup(m_document->GetCommandProcessor());
            wxDialog::EndModal(retCode);
        }

        void MapPropertiesDialog::OnDefChoiceSelected(wxCommandEvent& event) {
            int index = event.GetSelection();
            if (index < 0 || index >= static_cast<int>(m_defChoice->GetCount()))
                return;

            StringList builtinDefs = Model::EntityDefinitionManager::builtinDefinitionFiles();
            Utility::sort(builtinDefs);

            if (index < static_cast<int>(builtinDefs.size())) {
                const String defPath = "builtin:" + builtinDefs[static_cast<size_t>(index)];

                Controller::EntityPropertyCommand* command = Controller::EntityPropertyCommand::setEntityPropertyValue(*m_document, m_document->worldspawn(), Model::Entity::DefKey, defPath, true);
                m_document->GetCommandProcessor()->Submit(command);
            } else if (index == static_cast<int>(builtinDefs.size())) {
                wxFileDialog openDefinitionDialog(NULL, wxT("Choose entity definition file"), wxT(""), wxT(""), wxT("DEF files (*.def)|*.def|FGD files (*.fgd)|*.fgd"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
                if (openDefinitionDialog.ShowModal() == wxID_OK) {
                    PathDialog pathDialog(this, openDefinitionDialog.GetPath().ToStdString(), m_document->GetFilename().ToStdString());
                    if (pathDialog.ShowModal() == wxID_OK) {
                        const String defPath = "external:" + pathDialog.path();
                        Controller::EntityPropertyCommand* command = Controller::EntityPropertyCommand::setEntityPropertyValue(*m_document, m_document->worldspawn(), Model::Entity::DefKey, defPath, true);
                        m_document->GetCommandProcessor()->Submit(command);
                        init();
                    }
                }
            }

            init();
        }

        void MapPropertiesDialog::OnModChoiceSelected(wxCommandEvent& event) {
            int index = event.GetSelection();
            if (index < 0 || index >= static_cast<int>(m_modChoice->GetCount()))
                return;

            const String mod = m_modChoice->GetString(static_cast<unsigned int>(index)).ToStdString();
            Controller::EntityPropertyCommand* command = Controller::EntityPropertyCommand::setEntityPropertyValue(*m_document, m_document->worldspawn(), Model::Entity::ModKey, mod, true);
            m_document->GetCommandProcessor()->Submit(command);
            init();
        }

        void MapPropertiesDialog::OnIntFacePointsCheckBoxClicked(wxCommandEvent& event) {
            if (wxMessageBox(wxT("Changing this setting may change all brushes in your map and lead to leaks and other problems. You should only change this if your compiler cannot handle floating point coordinates.\n\n Are you sure you want to change this setting? This cannot be undone."), wxT("Force integer plane point coordinates"), wxYES_NO | wxICON_EXCLAMATION, this) == wxYES) {
                m_document->setForceIntegerCoordinates(event.IsChecked());
            } else {
                m_intFacePointsCheckBox->SetValue(!event.IsChecked());
            }
        }

        void MapPropertiesDialog::OnAddWadClicked(wxCommandEvent& event) {
            wxFileDialog openFileDialog(NULL, wxT("Choose texture wad"), wxT(""), wxT(""), wxT("*.wad"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
            if (openFileDialog.ShowModal() == wxID_OK) {
                PathDialog pathDialog(this, openFileDialog.GetPath().ToStdString(), m_document->GetFilename().ToStdString());
                if (pathDialog.ShowModal() == wxID_OK) {
                    m_wadList->addWad(pathDialog.path());
                    updateWadProperty();
                }
            }
        }

        void MapPropertiesDialog::OnRemoveWadsClicked(wxCommandEvent& event) {
            wxArrayInt selection;
            m_wadList->GetSelections(selection);
            m_wadList->removeWads(selection);
            updateWadProperty();
        }

        void MapPropertiesDialog::OnMoveWadUpClicked(wxCommandEvent& event) {
            wxArrayInt selection;
            m_wadList->GetSelections(selection);
            const size_t index = static_cast<size_t>(selection.front());
            m_wadList->moveWadUp(index);
            updateWadProperty();
            m_wadList->DeselectAll();
            m_wadList->SetSelection(static_cast<int>(index - 1));
        }

        void MapPropertiesDialog::OnMoveWadDownClicked(wxCommandEvent& event) {
            wxArrayInt selection;
            m_wadList->GetSelections(selection);
            const size_t index = static_cast<size_t>(selection.front());
            m_wadList->moveWadDown(index);
            updateWadProperty();
            m_wadList->DeselectAll();
            m_wadList->SetSelection(static_cast<int>(index + 1));
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
                case CommandIds::MapPropertiesDialog::MoveWadUpButtonId:
                    event.Enable(selection.size() == 1 &&
                                 selection[0] > 0);
                    break;
                case CommandIds::MapPropertiesDialog::MoveWadDownButtonId:
                    event.Enable(selection.size() == 1 &&
                                 selection[0] < static_cast<int>(m_document->textureManager().collections().size() - 1));
                    break;
                default:
                    event.Enable(false);
                    break;
            }
        }

        void MapPropertiesDialog::OnCloseClicked(wxCommandEvent& event) {
            EndModal(wxID_CLOSE);
        }


        void MapPropertiesDialog::OnFileExit(wxCommandEvent& event) {
            EndModal(wxID_CLOSE);
        }
    }
}
