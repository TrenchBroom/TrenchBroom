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
#include "IO/PathQt.h"
#include "View/BorderLine.h"
#include "View/MapDocument.h"
#include "View/TitledPanel.h"
#include "View/ViewConstants.h"
#include "View/QtUtils.h"

#include <kdl/memory_utils.h>

#include <kdl/vector_utils.h>

#include <QListWidget>
#include <QVBoxLayout>
#include <QAbstractButton>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        DirectoryTextureCollectionEditor::DirectoryTextureCollectionEditor(std::weak_ptr<MapDocument> document, QWidget* parent) :
        QWidget(parent),
        m_document(std::move(document)),
        m_availableCollectionsList(nullptr),
        m_enabledCollectionsList(nullptr),
        m_addCollectionsButton(nullptr),
        m_removeCollectionsButton(nullptr),
        m_reloadCollectionsButton(nullptr) {
            createGui();
            bindObservers();
            updateAllTextureCollections();
            updateButtons();
        }

        DirectoryTextureCollectionEditor::~DirectoryTextureCollectionEditor() {
            unbindObservers();
        }

        void DirectoryTextureCollectionEditor::addSelectedTextureCollections() {
            const auto availableCollections = availableTextureCollections();
            auto enabledCollections = enabledTextureCollections();

            for (QListWidgetItem* selectedItem : m_availableCollectionsList->selectedItems()) {
                const auto index = static_cast<size_t>(m_availableCollectionsList->row(selectedItem));
                enabledCollections.push_back(availableCollections[index]);
            }

            enabledCollections = kdl::vec_sort_and_remove_duplicates(std::move(enabledCollections));

            auto document = kdl::mem_lock(m_document);
            document->setEnabledTextureCollections(enabledCollections);
        }

        void DirectoryTextureCollectionEditor::removeSelectedTextureCollections() {
            auto enabledCollections = enabledTextureCollections();

            std::vector<int> selections;
            for (QListWidgetItem* selectedItem : m_enabledCollectionsList->selectedItems()) {
                selections.push_back(m_enabledCollectionsList->row(selectedItem));
            }

            // erase back to front
            for (auto sIt = std::rbegin(selections), sEnd = std::rend(selections); sIt != sEnd; ++sIt) {
                const auto index = static_cast<size_t>(*sIt);
                enabledCollections = kdl::vec_erase_at(std::move(enabledCollections), index);
            }

            auto document = kdl::mem_lock(m_document);
            document->setEnabledTextureCollections(enabledCollections);
        }

        void DirectoryTextureCollectionEditor::reloadTextureCollections() {
            auto document = kdl::mem_lock(m_document);
            document->reloadTextureCollections();
        }

        void DirectoryTextureCollectionEditor::availableTextureCollectionSelectionChanged() {
            updateButtons();
        }

        void DirectoryTextureCollectionEditor::enabledTextureCollectionSelectionChanged() {
            updateButtons();
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
            auto* availableCollectionsContainer = new TitledPanel("Available", false, true);
            availableCollectionsContainer->setBackgroundRole(QPalette::Base);
            availableCollectionsContainer->setAutoFillBackground(true);

            m_availableCollectionsList = new QListWidget();
            m_availableCollectionsList->setSelectionMode(QAbstractItemView::ExtendedSelection);

            auto* availableCollectionsContainerLayout = new QVBoxLayout();
            availableCollectionsContainerLayout->setContentsMargins(0, 0, 0, 0);
            availableCollectionsContainerLayout->setSpacing(0);
            availableCollectionsContainerLayout->addWidget(m_availableCollectionsList);
            availableCollectionsContainer->getPanel()->setLayout(availableCollectionsContainerLayout);

            auto* enabledCollectionsContainer = new TitledPanel("Enabled", false, true);
            enabledCollectionsContainer->setBackgroundRole(QPalette::Base);
            enabledCollectionsContainer->setAutoFillBackground(true);

            m_enabledCollectionsList = new QListWidget();
            m_enabledCollectionsList->setSelectionMode(QAbstractItemView::ExtendedSelection);

            auto* enabledCollectionsContainerLayout = new QVBoxLayout();
            enabledCollectionsContainerLayout->setContentsMargins(0, 0, 0, 0);
            enabledCollectionsContainerLayout->setSpacing(0);
            enabledCollectionsContainerLayout->addWidget(m_enabledCollectionsList);
            enabledCollectionsContainer->getPanel()->setLayout(enabledCollectionsContainerLayout);

            m_addCollectionsButton = createBitmapButton("Add.svg", tr("Enable the selected texture collections"), this);
            m_removeCollectionsButton = createBitmapButton("Remove.svg", tr("Disable the selected texture collections"), this);
            m_reloadCollectionsButton = createBitmapButton("Refresh.svg", tr("Reload all enabled texture collections"), this);

            auto* toolBar = createMiniToolBarLayout(
                m_addCollectionsButton,
                m_removeCollectionsButton,
                LayoutConstants::WideHMargin,
                m_reloadCollectionsButton);

            auto* layout = new QGridLayout();
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);

            layout->addWidget(availableCollectionsContainer,                    0, 0);
            layout->addWidget(new BorderLine(BorderLine::Direction::Vertical),   0, 1, 3, 1);
            layout->addWidget(enabledCollectionsContainer,                      0, 2);
            layout->addWidget(new BorderLine(BorderLine::Direction::Horizontal), 1, 0, 1, 3);
            layout->addLayout(toolBar,                                     2, 2);

            setLayout(layout);

            connect(m_availableCollectionsList, &QListWidget::itemSelectionChanged, this,
                &DirectoryTextureCollectionEditor::availableTextureCollectionSelectionChanged);
            connect(m_enabledCollectionsList, &QListWidget::itemSelectionChanged, this,
                &DirectoryTextureCollectionEditor::enabledTextureCollectionSelectionChanged);
            connect(m_availableCollectionsList, &QListWidget::itemDoubleClicked, this,
                &DirectoryTextureCollectionEditor::addSelectedTextureCollections);
            connect(m_enabledCollectionsList, &QListWidget::itemDoubleClicked, this,
                &DirectoryTextureCollectionEditor::removeSelectedTextureCollections);
            connect(m_addCollectionsButton, &QAbstractButton::clicked, this,
                &DirectoryTextureCollectionEditor::addSelectedTextureCollections);
            connect(m_removeCollectionsButton, &QAbstractButton::clicked, this,
                &DirectoryTextureCollectionEditor::removeSelectedTextureCollections);
            connect(m_reloadCollectionsButton, &QAbstractButton::clicked, this,
                &DirectoryTextureCollectionEditor::reloadTextureCollections);
        }

        void DirectoryTextureCollectionEditor::updateButtons() {
            m_addCollectionsButton->setEnabled(canAddTextureCollections());
            m_removeCollectionsButton->setEnabled(canRemoveTextureCollections());
            m_reloadCollectionsButton->setEnabled(canReloadTextureCollections());
        }

        void DirectoryTextureCollectionEditor::bindObservers() {
            auto document = kdl::mem_lock(m_document);
            document->textureCollectionsDidChangeNotifier.addObserver(this, &DirectoryTextureCollectionEditor::textureCollectionsDidChange);
            document->modsDidChangeNotifier.addObserver(this, &DirectoryTextureCollectionEditor::modsDidChange);

            auto& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.addObserver(this, &DirectoryTextureCollectionEditor::preferenceDidChange);
        }

        void DirectoryTextureCollectionEditor::unbindObservers() {
            if (!kdl::mem_expired(m_document)) {
                auto document = kdl::mem_lock(m_document);
                document->textureCollectionsDidChangeNotifier.removeObserver(this, &DirectoryTextureCollectionEditor::textureCollectionsDidChange);
                document->modsDidChangeNotifier.removeObserver(this, &DirectoryTextureCollectionEditor::modsDidChange);
            }

            auto& prefs = PreferenceManager::instance();
            assertResult(prefs.preferenceDidChangeNotifier.removeObserver(this, &DirectoryTextureCollectionEditor::preferenceDidChange));
        }

        void DirectoryTextureCollectionEditor::textureCollectionsDidChange() {
            updateAllTextureCollections();
            updateButtons();
        }

        void DirectoryTextureCollectionEditor::modsDidChange() {
            updateAllTextureCollections();
            updateButtons();
        }

        void DirectoryTextureCollectionEditor::preferenceDidChange(const IO::Path& path) {
            auto document = kdl::mem_lock(m_document);
            if (document->isGamePathPreference(path)) {
                updateAllTextureCollections();
                updateButtons();
            }
        }

        void DirectoryTextureCollectionEditor::updateAllTextureCollections() {
            updateAvailableTextureCollections();
            updateEnabledTextureCollections();
        }

        void DirectoryTextureCollectionEditor::updateAvailableTextureCollections() {
            updateListBox(m_availableCollectionsList, availableTextureCollections());
        }

        void DirectoryTextureCollectionEditor::updateEnabledTextureCollections() {
            updateListBox(m_enabledCollectionsList, enabledTextureCollections());
        }

        void DirectoryTextureCollectionEditor::updateListBox(QListWidget* box, const std::vector<IO::Path>& paths) {
            // We need to block QListWidget::itemSelectionChanged from firing while clearing and rebuilding the list
            // because it will cause debugUIConsistency() to fail, as the number of list items in the UI won't match
            // the document's texture collections lists.
            QSignalBlocker blocker(box);

            box->clear();
            for (const auto& path : paths) {
                box->addItem(IO::pathAsQString(path));
            }
        }

        std::vector<IO::Path> DirectoryTextureCollectionEditor::availableTextureCollections() const {
            auto document = kdl::mem_lock(m_document);
            auto availableCollections = document->availableTextureCollections();
            availableCollections = kdl::vec_erase_all(std::move(availableCollections), document->enabledTextureCollections());
            return availableCollections;
        }

        std::vector<IO::Path> DirectoryTextureCollectionEditor::enabledTextureCollections() const {
            auto document = kdl::mem_lock(m_document);
            return document->enabledTextureCollections();
        }
    }
}
