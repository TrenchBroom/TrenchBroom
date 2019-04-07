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

#include "FileTextureCollectionEditor.h"

#include "PreferenceManager.h"
#include "Assets/TextureManager.h"
#include "Assets/TextureCollection.h"
#include "View/BorderLine.h"
#include "View/ChoosePathTypeDialog.h"
#include "View/MapDocument.h"
#include "View/ViewConstants.h"
#include "View/ViewUtils.h"
#include "View/wxUtils.h"

#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QAbstractButton>
#include <QFileDialog>
#include <QSignalBlocker>

namespace TrenchBroom {
    namespace View {
        FileTextureCollectionEditor::FileTextureCollectionEditor(QWidget* parent, MapDocumentWPtr document) :
        QWidget(parent),
        m_document(document) {
            createGui();
            bindObservers();
            updateControls();
            updateButtons();
        }

        FileTextureCollectionEditor::~FileTextureCollectionEditor() {
            unbindObservers();
        }

        bool FileTextureCollectionEditor::debugUIConsistency() const {
            auto document = lock(m_document);
            auto collections = document->enabledTextureCollections();

            assert(m_collections->count() == static_cast<int>(collections.size()));

            std::vector<int> selectedIndices;
            for (QListWidgetItem* item : m_collections->selectedItems()) {
                selectedIndices.push_back(m_collections->row(item));
            }

            for (size_t i = 0; i < selectedIndices.size(); ++i) {
                assert(selectedIndices[i] >= 0);
                assert(static_cast<size_t>(selectedIndices[i]) < collections.size());
            }
            return true;
        }

        bool FileTextureCollectionEditor::canRemoveTextureCollections() const {
            assert(debugUIConsistency());

            std::vector<int> selections;
            for (QListWidgetItem* item : m_collections->selectedItems()) {
                selections.push_back(m_collections->row(item));
            }

            if (selections.empty()) {
                return false;
            }

            auto document = lock(m_document);
            auto collections = document->enabledTextureCollections();
            for (size_t i = 0; i < selections.size(); ++i) {
                const auto index = static_cast<size_t>(selections[i]);
                if (index >= collections.size()) {
                    return false;
                }
            }

            return true;
        }

        bool FileTextureCollectionEditor::canMoveTextureCollectionsUp() const {
            assert(debugUIConsistency());

            const QList<QListWidgetItem*> selections = m_collections->selectedItems();
            if (selections.size() != 1) {
                return false;
            }

            auto document = lock(m_document);
            auto collections = document->enabledTextureCollections();

            const auto index = static_cast<size_t>(m_collections->currentRow());
            return index >= 1 && index < collections.size();
        }

        bool FileTextureCollectionEditor::canMoveTextureCollectionsDown() const {
            assert(debugUIConsistency());

            const QList<QListWidgetItem*> selections = m_collections->selectedItems();
            if (selections.size() != 1) {
                return false;
            }

            auto document = lock(m_document);
            auto collections = document->enabledTextureCollections();

            const auto index = static_cast<size_t>(m_collections->currentRow());
            return (index + 1) < collections.size();
        }

        bool FileTextureCollectionEditor::canReloadTextureCollections() const {
            return m_collections->count() != 0;
        }

        void FileTextureCollectionEditor::OnAddTextureCollectionsClicked() {
            const QString pathQStr = QFileDialog::getOpenFileName(nullptr, "Load Texture Collection", "", "");
            if (pathQStr.isEmpty()) {
                return;
            }

            loadTextureCollection(m_document, this, pathQStr);
        }

        void FileTextureCollectionEditor::OnRemoveTextureCollectionsClicked() {
            if (!canRemoveTextureCollections()) {
                return;
            }

            auto document = lock(m_document);

            auto collections = document->enabledTextureCollections();
            decltype(collections) toRemove;

            for (QListWidgetItem* selectedItem : m_collections->selectedItems()) {
                const auto index = static_cast<size_t>(m_collections->row(selectedItem));
                ensure(index < collections.size(), "index out of range");
                toRemove.push_back(collections[index]);
            }

            VectorUtils::eraseAll(collections, toRemove);
            document->setEnabledTextureCollections(collections);
        }

        void FileTextureCollectionEditor::OnMoveTextureCollectionUpClicked() {
            if (!canMoveTextureCollectionsUp()) {
                return;
            }

            const QList<QListWidgetItem*> selections = m_collections->selectedItems();
            assert(selections.size() == 1);

            auto document = lock(m_document);
            auto collections = document->enabledTextureCollections();

            const auto index = static_cast<size_t>(m_collections->currentRow());
            VectorUtils::swapPred(collections, index);

            document->setEnabledTextureCollections(collections);
            m_collections->setCurrentRow(static_cast<int>(index - 1));
        }

        void FileTextureCollectionEditor::OnMoveTextureCollectionDownClicked() {
            if (!canMoveTextureCollectionsDown()) {
                return;
            }

            const QList<QListWidgetItem*> selections = m_collections->selectedItems();
            assert(selections.size() == 1);

            auto document = lock(m_document);
            auto collections = document->enabledTextureCollections();

            const auto index = static_cast<size_t>(m_collections->currentRow());
            VectorUtils::swapSucc(collections, index);

            document->setEnabledTextureCollections(collections);
            m_collections->setCurrentRow(static_cast<int>(index + 1));
        }

        void FileTextureCollectionEditor::OnReloadTextureCollectionsClicked() {
            auto document = lock(m_document);
            document->reloadTextureCollections();
        }

        void FileTextureCollectionEditor::createGui() {
            m_collections = new QListWidget();
            m_collections->setSelectionMode(QAbstractItemView::ExtendedSelection);

            m_addTextureCollectionsButton = createBitmapButton("Add.png",
                                                               "Add texture collections from the file system", this);
            m_removeTextureCollectionsButton = createBitmapButton("Remove.png",
                                                                  "Remove the selected texture collections", this);
            m_moveTextureCollectionUpButton = createBitmapButton("Up.png", "Move the selected texture collection up",
                                                                 this);
            m_moveTextureCollectionDownButton = createBitmapButton("Down.png",
                                                                   "Move the selected texture collection down", this);
            m_reloadTextureCollectionsButton = createBitmapButton("Refresh.png", "Reload all texture collections",
                                                                  this);

            connect(m_addTextureCollectionsButton, &QAbstractButton::clicked, this, &FileTextureCollectionEditor::OnAddTextureCollectionsClicked);
            connect(m_removeTextureCollectionsButton, &QAbstractButton::clicked, this, &FileTextureCollectionEditor::OnRemoveTextureCollectionsClicked);
            connect(m_moveTextureCollectionUpButton, &QAbstractButton::clicked, this, &FileTextureCollectionEditor::OnMoveTextureCollectionUpClicked);
            connect(m_moveTextureCollectionDownButton, &QAbstractButton::clicked, this, &FileTextureCollectionEditor::OnMoveTextureCollectionDownClicked);
            connect(m_reloadTextureCollectionsButton, &QAbstractButton::clicked, this, &FileTextureCollectionEditor::OnReloadTextureCollectionsClicked);

            connect(m_collections, &QListWidget::itemSelectionChanged, this, &FileTextureCollectionEditor::updateButtons);

            auto* buttonSizer = new QHBoxLayout();
            buttonSizer->setSpacing(0);
            buttonSizer->addWidget(m_addTextureCollectionsButton, 0, Qt::AlignVCenter);
            buttonSizer->addWidget(m_removeTextureCollectionsButton, 0, Qt::AlignVCenter);
            buttonSizer->addSpacing(LayoutConstants::WideHMargin);
            buttonSizer->addWidget(m_moveTextureCollectionUpButton, 0, Qt::AlignVCenter);
            buttonSizer->addWidget(m_moveTextureCollectionDownButton, 0, Qt::AlignVCenter);
            buttonSizer->addSpacing(LayoutConstants::WideHMargin);
            buttonSizer->addWidget(m_reloadTextureCollectionsButton, 0, Qt::AlignVCenter);
            buttonSizer->addStretch(1);

            auto* sizer = new QVBoxLayout();
            sizer->setContentsMargins(0, 0, 0, 0);
            sizer->setSpacing(0);
            sizer->addWidget(m_collections, 1);
            sizer->addWidget(new BorderLine(BorderLine::Direction_Horizontal), 0); //, wxEXPAND);
            sizer->addLayout(buttonSizer, 0); //, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::NarrowHMargin);
            //sizer->SetItemMinSize(m_collections, 100, 70);

            setLayout(sizer);
        }

        void FileTextureCollectionEditor::updateButtons() {
            m_removeTextureCollectionsButton->setEnabled(canRemoveTextureCollections());
            m_moveTextureCollectionUpButton->setEnabled(canMoveTextureCollectionsUp());
            m_moveTextureCollectionDownButton->setEnabled(canMoveTextureCollectionsDown());
            m_reloadTextureCollectionsButton->setEnabled(canReloadTextureCollections());
        }

        void FileTextureCollectionEditor::bindObservers() {
            auto document = lock(m_document);
            document->textureCollectionsDidChangeNotifier.addObserver(this, &FileTextureCollectionEditor::textureCollectionsDidChange);

            auto& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.addObserver(this, &FileTextureCollectionEditor::preferenceDidChange);
        }

        void FileTextureCollectionEditor::unbindObservers() {
            if (!expired(m_document)) {
                auto document = lock(m_document);
                document->textureCollectionsDidChangeNotifier.removeObserver(this, &FileTextureCollectionEditor::textureCollectionsDidChange);
            }

            auto& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.removeObserver(this, &FileTextureCollectionEditor::preferenceDidChange);
        }

        void FileTextureCollectionEditor::textureCollectionsDidChange() {
            updateControls();
        }

        void FileTextureCollectionEditor::preferenceDidChange(const IO::Path& path) {
            auto document = lock(m_document);
            if (document->isGamePathPreference(path))
                updateControls();
        }

        /**
         * Rebuilds the list widget
         */
        void FileTextureCollectionEditor::updateControls() {
            // We need to block QListWidget::itemSelectionChanged from firing while clearing and rebuilding the list
            // because it will cause debugUIConsistency() to fail, as the number of list items in the UI won't match
            // the document's texture collections lists.
            QSignalBlocker blocker(m_collections);

            m_collections->clear();

            auto document = lock(m_document);
            for (const auto& path : document->enabledTextureCollections()) {
                m_collections->addItem(path.asQString());
            }

            // Manually update the button states, since QSignalBlocker is blocking the automatic updates
            updateButtons();
        }
    }
}
