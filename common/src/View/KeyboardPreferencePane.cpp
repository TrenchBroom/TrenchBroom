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
#include "Preferences.h"
#include "Assets/EntityDefinitionManager.h"
#include "Model/Tag.h"
#include "View/BorderLine.h"
#include "View/KeyboardShortcutItemDelegate.h"
#include "View/KeyboardShortcutModel.h"
#include "View/MapDocument.h"
#include "View/ViewConstants.h"

#include <QBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QTableView>

namespace TrenchBroom {
    namespace View {
        KeyboardPreferencePane::KeyboardPreferencePane(MapDocument* document, QWidget* parent) :
        PreferencePane(parent),
        m_table(nullptr),
        m_model(nullptr) {
            m_model = new KeyboardShortcutModel(document);

            m_table = new QTableView();
            m_table->setModel(m_model);
            m_table->setHorizontalHeader(new QHeaderView(Qt::Horizontal));
            m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeMode::ResizeToContents);
            m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeMode::ResizeToContents);
            m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeMode::Stretch);

            m_table->setItemDelegate(new KeyboardShortcutItemDelegate());

            auto* infoLabel = new QLabel("Doubleclick on a key combination to edit the shortcut.");
            makeInfo(infoLabel);

            auto* layout = new QVBoxLayout(this);
            layout->setContentsMargins(QMargins());
            layout->setSpacing(LayoutConstants::WideVMargin);
            layout->addWidget(m_table, 1);
            layout->addWidget(new BorderLine(BorderLine::Direction_Horizontal));
            layout->addWidget(infoLabel);

            setMinimumSize(900, 550);
        }

        bool KeyboardPreferencePane::doCanResetToDefaults() {
            return true;
        }

        void KeyboardPreferencePane::doResetToDefaults() {
            /* FIXME
            ActionManager& actionManager = ActionManager::instance();
            actionManager.resetShortcutsToDefaults();
             */
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
