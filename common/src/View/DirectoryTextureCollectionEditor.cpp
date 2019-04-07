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

#include "DirectoryTextureCollectionEditor.h"

#include "PreferenceManager.h"
#include "View/BorderLine.h"
#include "View/MapDocument.h"
#include "View/TitledPanel.h"
#include "View/ViewConstants.h"
#include "View/ViewUtils.h"
#include "View/wxUtils.h"

#include <QListWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QAbstractButton>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        DirectoryTextureCollectionEditor::DirectoryTextureCollectionEditor(QWidget* parent, MapDocumentWPtr document) :
        QWidget(parent),
        m_document(document),
        m_availableCollectionsList(nullptr),
        m_enabledCollectionsList(nullptr) {
            createGui();
            bindObservers();
            update();
        }

		DirectoryTextureCollectionEditor::~DirectoryTextureCollectionEditor() {
			unbindObservers();
		}

		void DirectoryTextureCollectionEditor::OnAddTextureCollections() {
            const auto availableCollections = availableTextureCollections();
            auto enabledCollections = enabledTextureCollections();

            for (QListWidgetItem* selectedItem : m_availableCollectionsList->selectedItems()) {
                const auto index = static_cast<size_t>(m_availableCollectionsList->row(selectedItem));
                enabledCollections.push_back(availableCollections[index]);
            }

            VectorUtils::sortAndRemoveDuplicates(enabledCollections);

            MapDocumentSPtr document = lock(m_document);
            document->setEnabledTextureCollections(enabledCollections);
        }

        void DirectoryTextureCollectionEditor::OnRemoveTextureCollections() {
            auto enabledCollections = enabledTextureCollections();

            std::vector<int> selections;
            for (QListWidgetItem* selectedItem : m_enabledCollectionsList->selectedItems()) {
                selections.push_back(m_enabledCollectionsList->row(selectedItem));
            }

            // erase back to front
            for (auto sIt = std::rbegin(selections), sEnd = std::rend(selections); sIt != sEnd; ++sIt) {
                const auto index = static_cast<size_t>(*sIt);
                VectorUtils::erase(enabledCollections, index);
            }

            auto document = lock(m_document);
            document->setEnabledTextureCollections(enabledCollections);
        }

        void DirectoryTextureCollectionEditor::OnReloadTextureCollections() {
            auto document = lock(m_document);
            document->reloadTextureCollections();
        }

        bool DirectoryTextureCollectionEditor::canAddTextureCollections() const {
            return !m_availableCollectionsList->selectedItems().empty();
        }

        bool DirectoryTextureCollectionEditor::canRemoveTextureCollections() const {
            return !m_enabledCollectionsList->selectedItems().empty();
        }

        bool DirectoryTextureCollectionEditor::canReloadTextureCollections() const {
            return m_enabledCollectionsList->count() != 0;
        }

        /**
         * See ModEditor::createGui
         */
        void DirectoryTextureCollectionEditor::createGui() {
            auto* availableCollectionsContainer = new TitledPanel(this, "Available", false);

            m_availableCollectionsList = new QListWidget();
            m_availableCollectionsList->setSelectionMode(QAbstractItemView::ExtendedSelection);

            auto* availableModContainerSizer = new QVBoxLayout();
            availableModContainerSizer->addWidget(m_availableCollectionsList);//, wxSizerFlags().Expand().Proportion(1));
            availableCollectionsContainer->getPanel()->setLayout(availableModContainerSizer);

            auto* enabledCollectionsContainer = new TitledPanel(this, "Enabled", false);
//            enabledCollectionsContainer->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
            m_enabledCollectionsList = new QListWidget();
            m_enabledCollectionsList->setSelectionMode(QAbstractItemView::ExtendedSelection);

            auto* enabledCollectionsContainerSizer = new QVBoxLayout();
            enabledCollectionsContainerSizer->addWidget(m_enabledCollectionsList, 1);// wxSizerFlags().Expand().Proportion(1));
            enabledCollectionsContainer->getPanel()->setLayout(enabledCollectionsContainerSizer);

            m_addCollectionsButton = createBitmapButton("Add.png", tr("Enable the selected texture collections"), this);
            m_removeCollectionsButton = createBitmapButton("Remove.png", tr("Disable the selected texture collections"),
                                                           this);
            m_reloadCollectionsButton = createBitmapButton("Refresh.png", tr("Reload all enabled texture collections"),
                                                           this);

            auto* buttonSizer = new QHBoxLayout();
            buttonSizer->addWidget(m_addCollectionsButton); //, wxSizerFlags().CenterVertical().Border(wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin));
            buttonSizer->addWidget(m_removeCollectionsButton); //, wxSizerFlags().CenterVertical().Border(wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin));
            buttonSizer->addSpacing(LayoutConstants::WideHMargin);
            buttonSizer->addWidget(m_reloadCollectionsButton); //, wxSizerFlags().CenterVertical().Border(wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin));
            buttonSizer->addStretch(1);

            auto* sizer = new QGridLayout();
            sizer->addWidget(availableCollectionsContainer,                    0, 0);
            sizer->addWidget(new BorderLine(BorderLine::Direction_Vertical),   0, 1, 3, 1);
            sizer->addWidget(enabledCollectionsContainer,                      0, 2);
            sizer->addWidget(new BorderLine(BorderLine::Direction_Horizontal), 1, 0, 1, 3);
            sizer->addLayout(buttonSizer,                                      2, 2);
//            sizer->SetItemMinSize(availableCollectionsContainer, 100, 100);
//            sizer->SetItemMinSize(enabledCollectionsContainer, 100, 100);
//            sizer->AddGrowableCol(0);
//            sizer->AddGrowableCol(2);
//            sizer->AddGrowableRow(1);

            setLayout(sizer);

            // Unnecessary, Qt can automatically drop unused args
//            connect(m_availableCollectionsList, &QListWidget::itemDoubleClicked, this, [=](QListWidgetItem *item){
//                OnAddTextureCollections();
//            });
            connect(m_availableCollectionsList, &QListWidget::itemDoubleClicked, this, &DirectoryTextureCollectionEditor::OnAddTextureCollections);
            connect(m_enabledCollectionsList, &QListWidget::itemDoubleClicked, this, &DirectoryTextureCollectionEditor::OnRemoveTextureCollections);
            connect(m_addCollectionsButton, &QAbstractButton::clicked, this, &DirectoryTextureCollectionEditor::OnAddTextureCollections);
            connect(m_removeCollectionsButton, &QAbstractButton::clicked, this, &DirectoryTextureCollectionEditor::OnRemoveTextureCollections);
            connect(m_reloadCollectionsButton, &QAbstractButton::clicked, this, &DirectoryTextureCollectionEditor::OnReloadTextureCollections);
        }

        void DirectoryTextureCollectionEditor::updateButtons() {
            m_addCollectionsButton->setEnabled(canAddTextureCollections());
            m_removeCollectionsButton->setEnabled(canRemoveTextureCollections());
            m_reloadCollectionsButton->setEnabled(canReloadTextureCollections());
        }

        void DirectoryTextureCollectionEditor::bindObservers() {
            auto document = lock(m_document);
            document->textureCollectionsDidChangeNotifier.addObserver(this, &DirectoryTextureCollectionEditor::textureCollectionsDidChange);
            document->modsDidChangeNotifier.addObserver(this, &DirectoryTextureCollectionEditor::modsDidChange);

            auto& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.addObserver(this, &DirectoryTextureCollectionEditor::preferenceDidChange);
        }

        void DirectoryTextureCollectionEditor::unbindObservers() {
            if (!expired(m_document)) {
                auto document = lock(m_document);
                document->textureCollectionsDidChangeNotifier.removeObserver(this, &DirectoryTextureCollectionEditor::textureCollectionsDidChange);
                document->modsDidChangeNotifier.removeObserver(this, &DirectoryTextureCollectionEditor::modsDidChange);
            }

            auto& prefs = PreferenceManager::instance();
            assertResult(prefs.preferenceDidChangeNotifier.removeObserver(this, &DirectoryTextureCollectionEditor::preferenceDidChange));
        }

        void DirectoryTextureCollectionEditor::textureCollectionsDidChange() {
            update();
        }

        void DirectoryTextureCollectionEditor::modsDidChange() {
            update();
        }

        void DirectoryTextureCollectionEditor::preferenceDidChange(const IO::Path& path) {
            auto document = lock(m_document);
            if (document->isGamePathPreference(path)) {
                update();
            }
        }

        void DirectoryTextureCollectionEditor::update() {
            updateAvailableTextureCollections();
            updateEnabledTextureCollections();
        }

        void DirectoryTextureCollectionEditor::updateAvailableTextureCollections() {
            updateListBox(m_availableCollectionsList, availableTextureCollections());
        }

        void DirectoryTextureCollectionEditor::updateEnabledTextureCollections() {
            updateListBox(m_enabledCollectionsList, enabledTextureCollections());
        }

        void DirectoryTextureCollectionEditor::updateListBox(QListWidget* box, const IO::Path::List& paths) {
            box->clear();
            for (const auto& path : paths) {
                box->addItem(path.asQString());
            }
        }

        IO::Path::List DirectoryTextureCollectionEditor::availableTextureCollections() const {
            auto document = lock(m_document);
            auto availableCollections = document->availableTextureCollections();
            VectorUtils::eraseAll(availableCollections, document->enabledTextureCollections());
            return availableCollections;
        }

        IO::Path::List DirectoryTextureCollectionEditor::enabledTextureCollections() const {
            auto document = lock(m_document);
            return document->enabledTextureCollections();
        }
    }
}
