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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "KeyboardPreferencePane.h"

#include "Macros.h"
#include "View/Actions.h"
#include "View/KeyboardShortcutItemDelegate.h"
#include "View/KeyboardShortcutModel.h"
#include "View/MapDocument.h"
#include "View/ViewConstants.h"
#include "View/QtUtils.h"

#include <QBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QTableView>
#include <QSortFilterProxyModel>
#include <QLineEdit>

namespace TrenchBroom {
    namespace View {
        KeyboardPreferencePane::KeyboardPreferencePane(MapDocument* document, QWidget* parent) :
        PreferencePane(parent),
        m_table(nullptr),
        m_model(nullptr),
        m_proxy(nullptr) {
            m_model = new KeyboardShortcutModel(document, this);
            m_proxy = new QSortFilterProxyModel(this);
            m_proxy->setSourceModel(m_model);
            m_proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
            m_proxy->setFilterKeyColumn(2); // Filter based on the text in the Description column

            m_table = new QTableView();
            m_table->setModel(m_proxy);

            autoResizeRows(m_table);

            m_table->setHorizontalHeader(new QHeaderView(Qt::Horizontal));
            m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeMode::ResizeToContents);
            m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeMode::ResizeToContents);
            m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeMode::Stretch);

            m_table->setItemDelegate(new KeyboardShortcutItemDelegate());

            QLineEdit* searchBox = createSearchBox();

            auto* searchLayout = new QHBoxLayout();
            searchLayout->setContentsMargins(LayoutConstants::WideHMargin,
                                             LayoutConstants::WideVMargin,
                                             LayoutConstants::WideHMargin,
                                             LayoutConstants::WideVMargin);
            searchLayout->addStretch();
            searchLayout->addWidget(searchBox);

            auto* infoLabel = new QLabel(tr("Double-click on a key combination, then click into the shortcut editor to edit the shortcut."));
            makeInfo(infoLabel);

            auto* infoLabelLayout = new QHBoxLayout();
            infoLabelLayout->setContentsMargins(
                LayoutConstants::WideHMargin,
                LayoutConstants::WideVMargin,
                LayoutConstants::WideHMargin,
                LayoutConstants::WideVMargin);
            infoLabelLayout->addWidget(infoLabel);

            auto* layout = new QVBoxLayout();
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);
            layout->addLayout(searchLayout);
            layout->addWidget(m_table, 1);
            layout->addLayout(infoLabelLayout);
            setLayout(layout);

            setMinimumSize(900, 550);

            connect(searchBox, &QLineEdit::textChanged, this, [=](const QString& newText){
                m_proxy->setFilterFixedString(newText);
            });
        }

        bool KeyboardPreferencePane::doCanResetToDefaults() {
            return true;
        }

        void KeyboardPreferencePane::doResetToDefaults() {
            auto& actionManager = ActionManager::instance();
            actionManager.resetAllKeySequences();
            m_model->reset();
        }

        void KeyboardPreferencePane::doUpdateControls() {
            m_table->update();
        }

        bool KeyboardPreferencePane::doValidate() {
            if (m_model->hasConflicts()) {
                QMessageBox::warning(this, "Conflicts", "Please fix all conflicting shortcuts (highlighted in red).");
                return false;
            }
            return true;
        }
    }
}
