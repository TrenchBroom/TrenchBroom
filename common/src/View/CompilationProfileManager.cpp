/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "CompilationProfileManager.h"

#include "Model/CompilationConfig.h"
#include "Model/CompilationProfile.h"
#include "View/BorderLine.h"
#include "View/CompilationProfileListBox.h"
#include "View/CompilationProfileEditor.h"
#include "View/TitledPanel.h"
#include "View/ViewConstants.h"
#include "View/wxUtils.h"

#include <wx/settings.h>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        CompilationProfileManager::CompilationProfileManager(QWidget* parent, MapDocumentWPtr document, Model::CompilationConfig& config) :
        QWidget(parent),
        m_config(config),
        m_profileList(nullptr),
        m_profileEditor() {
            SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));

            TitledPanel* listPanel = new TitledPanel(this, "Profiles");
            TitledPanel* editorPanel = new TitledPanel(this, "Details");
            listPanel->getPanel()->SetBackgroundColour(GetBackgroundColour());
            editorPanel->getPanel()->SetBackgroundColour(GetBackgroundColour());

            m_profileList = new CompilationProfileListBox(listPanel->getPanel(), m_config);
            m_profileEditor = new CompilationProfileEditor(editorPanel->getPanel(), document);

            QWidget* addProfileButton = createBitmapButton(listPanel->getPanel(), "Add.png", "Add profile");
            QWidget* removeProfileButton = createBitmapButton(listPanel->getPanel(), "Remove.png", "Remove the selected profile");

            addProfileButton->Bind(&QAbstractButton::clicked, &CompilationProfileManager::OnAddProfile, this);
            removeProfileButton->Bind(&QAbstractButton::clicked, &CompilationProfileManager::OnRemoveProfile, this);
            addProfileButton->Bind(wxEVT_UPDATE_UI, &CompilationProfileManager::OnUpdateAddProfileButtonUI, this);
            removeProfileButton->Bind(wxEVT_UPDATE_UI, &CompilationProfileManager::OnUpdateRemoveProfileButtonUI, this);

            auto* buttonSizer = new QHBoxLayout();
            buttonSizer->addWidget(addProfileButton, 0, Qt::AlignVCenter | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->addWidget(removeProfileButton, 0, Qt::AlignVCenter | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->addStretch(1);

            auto* listSizer = new QVBoxLayout();
            listSizer->addWidget(m_profileList, 1, wxEXPAND);
            listSizer->addWidget(new BorderLine(listPanel->getPanel(), BorderLine::Direction_Horizontal), 0, wxEXPAND);
            listSizer->addWidget(buttonSizer);
            listPanel->getPanel()->setLayout(listSizer);

            auto* editorSizer = new QVBoxLayout();
            editorSizer->addWidget(m_profileEditor, 1, wxEXPAND);
            editorPanel->getPanel()->setLayout(editorSizer);

            auto* outerSizer = new QHBoxLayout();
            outerSizer->addWidget(listPanel, 0, wxEXPAND);
            outerSizer->addWidget(new BorderLine(nullptr, BorderLine::Direction_Vertical), 0, wxEXPAND);
            outerSizer->addWidget(editorPanel, 1, wxEXPAND);
            outerSizer->SetItemMinSize(listPanel, wxSize(200, 200));
            setLayout(outerSizer);

            m_profileList->Bind(wxEVT_LISTBOX, &CompilationProfileManager::OnProfileSelectionChanged, this);
        }

        const Model::CompilationProfile* CompilationProfileManager::selectedProfile() const {
            const int index = m_profileList->GetSelection();
            if (index == wxNOT_FOUND)
                return nullptr;
            return m_config.profile(static_cast<size_t>(index));
        }

        void CompilationProfileManager::OnAddProfile() {
            m_config.addProfile(new Model::CompilationProfile("unnamed", "${MAP_DIR_PATH}"));
            m_profileList->SetSelection(static_cast<int>(m_config.profileCount() - 1));
        }

        void CompilationProfileManager::OnRemoveProfile() {
            const int index = m_profileList->GetSelection();
            assert(index != wxNOT_FOUND);

            if (m_config.profileCount() == 1) {
                m_profileList->SetSelection(wxNOT_FOUND);
                m_config.removeProfile(static_cast<size_t>(index));
            } else if (index > 0) {
                m_profileList->SetSelection(index - 1);
                m_config.removeProfile(static_cast<size_t>(index));
            } else {
                m_profileList->SetSelection(1);
                m_config.removeProfile(static_cast<size_t>(index));
                m_profileList->SetSelection(0);
            }
        }

        void CompilationProfileManager::OnUpdateAddProfileButtonUI() {
            event.Enable(true);
        }

        void CompilationProfileManager::OnUpdateRemoveProfileButtonUI() {
            event.Enable(m_profileList->GetSelection() != wxNOT_FOUND);
        }

        void CompilationProfileManager::OnProfileSelectionChanged() {
            const int selection = m_profileList->GetSelection();
            if (selection != wxNOT_FOUND) {
                Model::CompilationProfile* profile = m_config.profile(static_cast<size_t>(selection));
                m_profileEditor->setProfile(profile);
            } else {
                m_profileEditor->setProfile(nullptr);
            }
        }
    }
}
