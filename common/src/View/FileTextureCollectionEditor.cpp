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
#include "IO/PathQt.h"
#include "View/BorderLine.h"
#include "View/MapDocument.h"
#include "View/ViewConstants.h"
#include "View/ViewUtils.h"
#include "View/QtUtils.h"

#include <kdl/memory_utils.h>
#include <kdl/vector_utils.h>

#include <QListWidget>
#include <QVBoxLayout>
#include <QAbstractButton>
#include <QFileDialog>
#include <QSignalBlocker>

namespace TrenchBroom {
    namespace View {
        FileTextureCollectionEditor::FileTextureCollectionEditor(std::weak_ptr<MapDocument> document, QWidget* parent) :
        QWidget(parent),
        m_document(std::move(document)),
        m_collections(nullptr),
        m_addTextureCollectionsButton(nullptr),
        m_removeTextureCollectionsButton(nullptr),
        m_moveTextureCollectionUpButton(nullptr),
        m_moveTextureCollectionDownButton(nullptr),
        m_reloadTextureCollectionsButton(nullptr) {
            createGui();
            bindObservers();
            updateControls();
        }

        FileTextureCollectionEditor::~FileTextureCollectionEditor() {
            unbindObservers();
        }

        bool FileTextureCollectionEditor::debugUIConsistency() const {
            auto document = kdl::mem_lock(m_document);
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

            auto document = kdl::mem_lock(m_document);
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

            auto document = kdl::mem_lock(m_document);
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

            auto document = kdl::mem_lock(m_document);
            auto collections = document->enabledTextureCollections();

            const auto index = static_cast<size_t>(m_collections->currentRow());
            return (index + 1) < collections.size();
        }

        bool FileTextureCollectionEditor::canReloadTextureCollections() const {
            return m_collections->count() != 0;
        }

        void FileTextureCollectionEditor::addTextureCollections() {
            const QString pathQStr = QFileDialog::getOpenFileName(nullptr, tr("Load Texture Collection"), fileDialogDefaultDirectory(FileDialogDir::TextureCollection), "");
            if (pathQStr.isEmpty()) {
                return;
            }

            updateFileDialogDefaultDirectoryWithFilename(FileDialogDir::TextureCollection, pathQStr);
            loadTextureCollection(m_document, this, pathQStr);
        }

        void FileTextureCollectionEditor::removeSelectedTextureCollections() {
            if (!canRemoveTextureCollections()) {
                return;
            }

            auto document = kdl::mem_lock(m_document);

            auto collections = document->enabledTextureCollections();
            decltype(collections) toRemove;

            for (QListWidgetItem* selectedItem : m_collections->selectedItems()) {
                const auto index = static_cast<size_t>(m_collections->row(selectedItem));
                ensure(index < collections.size(), "index out of range");
                toRemove.push_back(collections[index]);
            }

            kdl::vec_erase_all(collections, toRemove);
            document->setEnabledTextureCollections(collections);
        }

        void FileTextureCollectionEditor::moveSelectedTextureCollectionsUp() {
            if (!canMoveTextureCollectionsUp()) {
                return;
            }

            const QList<QListWidgetItem*> selections = m_collections->selectedItems();
            assert(selections.size() == 1);

            auto document = kdl::mem_lock(m_document);
            auto collections = document->enabledTextureCollections();

            const auto index = static_cast<size_t>(m_collections->currentRow());
            if (index > 0) {
                using std::swap; swap(collections[index], collections[index-1]);

                document->setEnabledTextureCollections(collections);
                m_collections->setCurrentRow(static_cast<int>(index - 1));
            }
        }

        void FileTextureCollectionEditor::moveSelectedTextureCollectionsDown() {
            if (!canMoveTextureCollectionsDown()) {
                return;
            }

            const QList<QListWidgetItem*> selections = m_collections->selectedItems();
            assert(selections.size() == 1);

            auto document = kdl::mem_lock(m_document);
            auto collections = document->enabledTextureCollections();

            const auto index = static_cast<size_t>(m_collections->currentRow());
            if (index < collections.size() - 1u) {
                using std::swap; swap(collections[index], collections[index+1]);

                document->setEnabledTextureCollections(collections);
                m_collections->setCurrentRow(static_cast<int>(index + 1));
            }
        }

        void FileTextureCollectionEditor::reloadTextureCollections() {
            auto document = kdl::mem_lock(m_document);
            document->reloadTextureCollections();
        }

        void FileTextureCollectionEditor::createGui() {
            m_collections = new QListWidget();
            m_collections->setSelectionMode(QAbstractItemView::ExtendedSelection);

            m_addTextureCollectionsButton = createBitmapButton("Add.png", "Add texture collections from the file system");
            m_removeTextureCollectionsButton = createBitmapButton("Remove.png", "Remove the selected texture collections");
            m_moveTextureCollectionUpButton = createBitmapButton("Up.png", "Move the selected texture collection up");
            m_moveTextureCollectionDownButton = createBitmapButton("Down.png", "Move the selected texture collection down");
            m_reloadTextureCollectionsButton = createBitmapButton("Refresh.png", "Reload all texture collections");

            auto* toolBar = createMiniToolBarLayout(
                m_addTextureCollectionsButton,
                m_removeTextureCollectionsButton,
                LayoutConstants::WideHMargin,
                m_moveTextureCollectionUpButton,
                m_moveTextureCollectionDownButton,
                LayoutConstants::WideHMargin,
                m_reloadTextureCollectionsButton);

            auto* layout = new QVBoxLayout();
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);
            layout->addWidget(m_collections, 1);
            layout->addWidget(new BorderLine(BorderLine::Direction::Horizontal), 0);
            layout->addLayout(toolBar, 0);

            setLayout(layout);

            connect(m_collections, &QListWidget::itemSelectionChanged, this, &FileTextureCollectionEditor::updateButtons);

            connect(m_addTextureCollectionsButton, &QAbstractButton::clicked, this,
                &FileTextureCollectionEditor::addTextureCollections);
            connect(m_removeTextureCollectionsButton, &QAbstractButton::clicked, this,
                &FileTextureCollectionEditor::removeSelectedTextureCollections);
            connect(m_moveTextureCollectionUpButton, &QAbstractButton::clicked, this,
                &FileTextureCollectionEditor::moveSelectedTextureCollectionsUp);
            connect(m_moveTextureCollectionDownButton, &QAbstractButton::clicked, this,
                &FileTextureCollectionEditor::moveSelectedTextureCollectionsDown);
            connect(m_reloadTextureCollectionsButton, &QAbstractButton::clicked, this,
                &FileTextureCollectionEditor::reloadTextureCollections);
        }

        void FileTextureCollectionEditor::updateButtons() {
            m_removeTextureCollectionsButton->setEnabled(canRemoveTextureCollections());
            m_moveTextureCollectionUpButton->setEnabled(canMoveTextureCollectionsUp());
            m_moveTextureCollectionDownButton->setEnabled(canMoveTextureCollectionsDown());
            m_reloadTextureCollectionsButton->setEnabled(canReloadTextureCollections());
        }

        void FileTextureCollectionEditor::bindObservers() {
            auto document = kdl::mem_lock(m_document);
            document->textureCollectionsDidChangeNotifier.addObserver(this, &FileTextureCollectionEditor::textureCollectionsDidChange);

            auto& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.addObserver(this, &FileTextureCollectionEditor::preferenceDidChange);
        }

        void FileTextureCollectionEditor::unbindObservers() {
            if (!kdl::mem_expired(m_document)) {
                auto document = kdl::mem_lock(m_document);
                document->textureCollectionsDidChangeNotifier.removeObserver(this, &FileTextureCollectionEditor::textureCollectionsDidChange);
            }

            auto& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.removeObserver(this, &FileTextureCollectionEditor::preferenceDidChange);
        }

        void FileTextureCollectionEditor::textureCollectionsDidChange() {
            updateControls();
        }

        void FileTextureCollectionEditor::preferenceDidChange(const IO::Path& path) {
            auto document = kdl::mem_lock(m_document);
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

            auto document = kdl::mem_lock(m_document);
            for (const auto& path : document->enabledTextureCollections()) {
                m_collections->addItem(IO::pathAsQString(path));
            }

            // Manually update the button states, since QSignalBlocker is blocking the automatic updates
            updateButtons();
        }
    }
}
