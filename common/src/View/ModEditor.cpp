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

#include "ModEditor.h"

#include "Notifier.h"
#include "Preferences.h"
#include "PreferenceManager.h"
#include "Model/Entity.h"
#include "Model/Game.h"
#include "Model/Object.h"
#include "View/BorderLine.h"
#include "View/MapDocument.h"
#include "View/TitledPanel.h"
#include "View/ViewConstants.h"
#include "View/ViewUtils.h"
#include "View/wxUtils.h"

#include <QLineEdit>
#include <QListWidget>
#include <QWidget>
#include <QLabel>
#include <QAbstractButton>
#include <QVBoxLayout>
#include <QGridLayout>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        ModEditor::ModEditor(QWidget* parent, MapDocumentWPtr document) :
        QWidget(parent),
        m_document(document),
        m_availableModList(nullptr),
        m_enabledModList(nullptr),
        m_filterBox(nullptr) {
            createGui();
            bindObservers();
        }

        ModEditor::~ModEditor() {
            unbindObservers();
        }

        void ModEditor::OnAddModClicked() {
            const QList<QListWidgetItem*> selections = m_availableModList->selectedItems();
            if (selections.empty()) {
                return;
            }

            auto document = lock(m_document);

            StringList mods = document->mods();
            for (QListWidgetItem* item : selections) {
                mods.push_back(item->text().toStdString());
            }
            document->setMods(mods);
        }

        void ModEditor::OnRemoveModClicked() {
            const QList<QListWidgetItem*> selections = m_enabledModList->selectedItems();
            if (selections.empty()) {
                return;
            }

            auto document = lock(m_document);

            StringList mods = document->mods();
            for (QListWidgetItem* item : selections) {
                const std::string mod = item->text().toStdString();
                VectorUtils::erase(mods, mod);
            }
            document->setMods(mods);
        }

        void ModEditor::OnMoveModUpClicked() {
            const QList<QListWidgetItem*> selections = m_enabledModList->selectedItems();
            assert(selections.size() == 1);

            auto document = lock(m_document);
            auto mods = document->mods();

            const int index = m_enabledModList->row(selections.first());
            ensure(index > 0 && index < mods.size(), "index out of range");

            using std::swap;
            swap(mods[index - 1], mods[index]);
            document->setMods(mods);

            m_enabledModList->clearSelection();
            m_enabledModList->setItemSelected(m_enabledModList->item(index - 1), true);
        }

        void ModEditor::OnMoveModDownClicked() {
            const QList<QListWidgetItem*> selections = m_enabledModList->selectedItems();
            assert(selections.size() == 1);

            auto document = lock(m_document);
            auto mods = document->mods();

            const int index = m_enabledModList->row(selections.first());
            ensure(index < mods.size() - 1, "index out of range");

            using std::swap;
            swap(mods[index + 1], mods[index]);
            document->setMods(mods);

            m_enabledModList->clearSelection();
            m_enabledModList->setItemSelected(m_enabledModList->item(index + 1), true);
        }

        bool ModEditor::canEnableAddButton() const {
            return m_availableModList->selectedItems().size() > 0;
        }

        bool ModEditor::canEnableRemoveButton() const {
            return m_enabledModList->selectedItems().size() > 0;
        }

        bool ModEditor::canEnableMoveUpButton() const {
            return m_enabledModList->selectedItems().size() == 1 && m_enabledModList->row(m_enabledModList->selectedItems().front()) > 0;
        }

        bool ModEditor::canEnableMoveDownButton() const {
            const auto enabledModCount = m_enabledModList->count();

            return m_enabledModList->selectedItems().size() == 1 && m_enabledModList->row(m_enabledModList->selectedItems().front()) < enabledModCount - 1;
        }

        void ModEditor::OnFilterBoxChanged() {
            updateMods();
        }

        void ModEditor::createGui() {
            auto* availableModContainer = new TitledPanel(nullptr, "Available", false);
//            availableModContainer->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
            m_availableModList = new QListWidget(); //(availableModContainer->getPanel(), wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxLB_MULTIPLE | wxBORDER_NONE);
            m_availableModList->setSelectionMode(QAbstractItemView::ExtendedSelection);

            auto* availableModContainerSizer = new QVBoxLayout();
            availableModContainerSizer->addWidget(m_availableModList, 1);// wxSizerFlags().Expand().Proportion(1));
            availableModContainer->getPanel()->setLayout(availableModContainerSizer);

            m_filterBox = createSearchBox();
            m_filterBox->setToolTip(tr("Filter the list of available mods"));
//            m_filterBox->setFont(m_availableModList->font());

            auto* filterBoxSizer = new QVBoxLayout();
//            filterBoxSizer->addSpacing(LayoutConstants::NarrowVMargin);
            filterBoxSizer->addWidget(m_filterBox, 1);
//            filterBoxSizer->addSpacing(LayoutConstants::NarrowVMargin);

            auto* enabledModContainer = new TitledPanel(nullptr, "Enabled", false);
//            enabledModContainer->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
            m_enabledModList = new QListWidget(); //enabledModContainer->getPanel(), wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxLB_MULTIPLE | wxBORDER_NONE);
            m_enabledModList->setSelectionMode(QAbstractItemView::ExtendedSelection);

            auto* enabledModContainerSizer = new QVBoxLayout();
            enabledModContainerSizer->addWidget(m_enabledModList, 1);//wxSizerFlags().Expand().Proportion(1));
            enabledModContainer->getPanel()->setLayout(enabledModContainerSizer);

            m_addModsButton = createBitmapButton(this, "Add.png", "Enable the selected mods");
            m_removeModsButton = createBitmapButton(this, "Remove.png", "Disable the selected mods");
            m_moveModUpButton = createBitmapButton(this, "Up.png", "Move the selected mod up");
            m_moveModDownButton = createBitmapButton(this, "Down.png", "Move the selected mod down");

            auto* buttonSizer = new QHBoxLayout();
            buttonSizer->addWidget(m_addModsButton);//, wxSizerFlags().CenterVertical().Border(wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin));
            buttonSizer->addWidget(m_removeModsButton);//, wxSizerFlags().CenterVertical().Border(wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin));
            buttonSizer->addSpacing(LayoutConstants::WideHMargin);
            buttonSizer->addWidget(m_moveModUpButton);//, wxSizerFlags().CenterVertical().Border(wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin));
            buttonSizer->addWidget(m_moveModDownButton);//, wxSizerFlags().CenterVertical().Border(wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin));
            buttonSizer->addStretch(1);

            auto* sizer = new QGridLayout();
            sizer->addWidget(availableModContainer,                                   0, 0);
            sizer->addWidget(new BorderLine(nullptr, BorderLine::Direction_Vertical),    0, 1, 3, 1);
            sizer->addWidget(enabledModContainer,                                     0, 2);
            sizer->addWidget(new BorderLine(nullptr, BorderLine::Direction_Horizontal),  1, 0, 1, 3);
            sizer->addLayout(filterBoxSizer,                                          2, 0);
            sizer->addLayout(buttonSizer,                                             2, 2);
//            sizer->SetItemMinSize(availableModContainer, 100, 100);
//            sizer->SetItemMinSize(enabledModContainer, 100, 100);
//            sizer->AddGrowableCol(0);
//            sizer->AddGrowableCol(2);
//            sizer->AddGrowableRow(1);

            setLayout(sizer);

            connect(m_availableModList, &QListWidget::itemDoubleClicked, this, &ModEditor::OnAddModClicked);
            connect(m_enabledModList, &QListWidget::itemDoubleClicked, this, &ModEditor::OnRemoveModClicked);
            connect(m_filterBox, &QLineEdit::textEdited, this, &ModEditor::OnFilterBoxChanged);
            connect(m_addModsButton, &QAbstractButton::clicked, this, &ModEditor::OnAddModClicked);
            connect(m_removeModsButton, &QAbstractButton::clicked, this, &ModEditor::OnRemoveModClicked);
            connect(m_moveModUpButton, &QAbstractButton::clicked, this, &ModEditor::OnMoveModUpClicked);
            connect(m_moveModDownButton, &QAbstractButton::clicked, this, &ModEditor::OnMoveModDownClicked);

            //connect(m_availableModList, )

            updateButtons();
        }

        void ModEditor::updateButtons() {
            m_addModsButton->setEnabled(canEnableAddButton());
            m_removeModsButton->setEnabled(canEnableRemoveButton());
            m_moveModUpButton->setEnabled(canEnableMoveUpButton());
            m_moveModDownButton->setEnabled(canEnableMoveDownButton());
        }

        void ModEditor::bindObservers() {
            auto document = lock(m_document);
            document->documentWasNewedNotifier.addObserver(this, &ModEditor::documentWasNewed);
            document->documentWasLoadedNotifier.addObserver(this, &ModEditor::documentWasLoaded);
            document->modsDidChangeNotifier.addObserver(this, &ModEditor::modsDidChange);

            auto& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.addObserver(this, &ModEditor::preferenceDidChange);
        }

        void ModEditor::unbindObservers() {
            if (!expired(m_document)) {
                auto document = lock(m_document);
                document->documentWasNewedNotifier.removeObserver(this, &ModEditor::documentWasNewed);
                document->documentWasLoadedNotifier.removeObserver(this, &ModEditor::documentWasLoaded);
                document->modsDidChangeNotifier.removeObserver(this, &ModEditor::modsDidChange);
            }

            auto& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.removeObserver(this, &ModEditor::preferenceDidChange);
        }

        void ModEditor::documentWasNewed(MapDocument* document) {
            updateAvailableMods();
            updateMods();
        }

        void ModEditor::documentWasLoaded(MapDocument* document) {
            updateAvailableMods();
            updateMods();
        }

        void ModEditor::modsDidChange() {
            updateMods();
        }

        void ModEditor::preferenceDidChange(const IO::Path& path) {
            auto document = lock(m_document);
            if (document->isGamePathPreference(path)) {
                updateAvailableMods();
                updateMods();
            }
        }

        void ModEditor::updateAvailableMods() {
            auto document = lock(m_document);
            StringList availableMods = document->game()->availableMods();
            StringUtils::sortCaseInsensitive(availableMods);

            m_availableMods.clear();
            m_availableMods.reserve(availableMods.size());
            for (size_t i = 0; i < availableMods.size(); ++i) {
                m_availableMods.push_back(availableMods[i]);
            }
        }

        void ModEditor::updateMods() {
            m_availableModList->clear();
            m_enabledModList->clear();

            const auto pattern = m_filterBox->text().toStdString();

            auto document = lock(m_document);
            auto enabledMods = document->mods();

            QStringList availableModItems;
            for (size_t i = 0; i < m_availableMods.size(); ++i) {
                const auto& mod = m_availableMods[i];
                if (StringUtils::containsCaseInsensitive(mod, pattern) &&
                    !VectorUtils::contains(enabledMods, mod)) {
                    m_availableModList->addItem(QString::fromStdString(mod));
                }
            }

            QStringList enabledModItems;
            for (size_t i = 0; i < enabledMods.size(); ++i) {
                if (StringUtils::containsCaseInsensitive(enabledMods[i], pattern)) {
                    m_enabledModList->addItem(QString::fromStdString(enabledMods[i]));
                }
            }
        }
    }
}
