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

#include "PreferenceDialog.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "IO/Path.h"
#include "IO/ResourceUtils.h"
#include "View/BorderLine.h"
#include "View/ColorsPreferencePane.h"
#include "View/GamesPreferencePane.h"
#include "View/KeyboardPreferencePane.h"
#include "View/MousePreferencePane.h"
#include "View/ViewPreferencePane.h"
#include "View/PreferencePane.h"
#include "View/QtUtils.h"

#include <QBoxLayout>
#include <QCloseEvent>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QToolBar>
#include <QToolButton>

namespace TrenchBroom {
    namespace View {
        PreferenceDialog::PreferenceDialog(std::shared_ptr<MapDocument> document, QWidget* parent) :
        QDialog(parent),
        m_document(std::move(document)),
        m_toolBar(nullptr),
        m_stackedWidget(nullptr),
        m_buttonBox(nullptr) {
            setWindowTitle("Preferences");
            setWindowIconTB(this);
            createGui();
            switchToPane(PrefPane_First);
            currentPane()->updateControls();
        }

        void PreferenceDialog::closeEvent(QCloseEvent* event) {
            if (!currentPane()->validate()) {
                event->ignore();
                return;
            }

            auto& prefs = PreferenceManager::instance();
            if (!prefs.saveInstantly()) {
                prefs.discardChanges();
            }

            event->accept();
        }

        void PreferenceDialog::createGui() {
            const auto gamesImage    = IO::loadSVGIcon(IO::Path("GeneralPreferences.svg"));
            const auto viewImage     = IO::loadSVGIcon(IO::Path("ViewPreferences.svg"));
            const auto colorsImage   = IO::loadSVGIcon(IO::Path("ColorPreferences.svg"));
            const auto mouseImage    = IO::loadSVGIcon(IO::Path("MousePreferences.svg"));
            const auto keyboardImage = IO::loadSVGIcon(IO::Path("KeyboardPreferences.svg"));

            m_toolBar = new QToolBar();
            m_toolBar->setFloatable(false);
            m_toolBar->setMovable(false);
            m_toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
            m_toolBar->addAction(gamesImage, "Games", [this](){ switchToPane(PrefPane_Games); });
            m_toolBar->addAction(viewImage, "View", [this](){ switchToPane(PrefPane_View); });
            m_toolBar->addAction(colorsImage, "Colors", [this](){ switchToPane(PrefPane_Colors); });
            m_toolBar->addAction(mouseImage, "Mouse", [this](){ switchToPane(PrefPane_Mouse); });
            m_toolBar->addAction(keyboardImage, "Keyboard", [this](){ switchToPane(PrefPane_Keyboard); });

            foreach (auto *button, m_toolBar->findChildren<QToolButton*>()) {
                button->installEventFilter(this);
            }

            m_stackedWidget = new QStackedWidget();
            m_stackedWidget->addWidget(new GamesPreferencePane());
            m_stackedWidget->addWidget(new ViewPreferencePane());
            m_stackedWidget->addWidget(new ColorsPreferencePane());
            m_stackedWidget->addWidget(new MousePreferencePane());
            m_stackedWidget->addWidget(new KeyboardPreferencePane(m_document.get()));

            m_buttonBox = new QDialogButtonBox(
                QDialogButtonBox::RestoreDefaults
#if !defined __APPLE__
                | QDialogButtonBox::Ok | QDialogButtonBox::Apply | QDialogButtonBox::Cancel
#endif
                , this);

            auto* resetButton = m_buttonBox->button(QDialogButtonBox::RestoreDefaults);
            connect(resetButton, &QPushButton::clicked, this, &PreferenceDialog::resetToDefaults);

#if !defined __APPLE__
            connect(m_buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, [this]() {
                auto& prefs = PreferenceManager::instance();
                prefs.saveChanges();
                this->close();
            });
            connect(m_buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, [this]() {
                auto& prefs = PreferenceManager::instance();
                prefs.saveChanges();
            });
            connect(m_buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, [this]() {
                this->close();
            });
#endif

            auto* layout = new QVBoxLayout();
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);
            setLayout(layout);

            layout->setMenuBar(m_toolBar);
#if !defined __APPLE__
            layout->addWidget(new BorderLine(BorderLine::Direction::Horizontal));
#endif
            layout->addWidget(m_stackedWidget, 1);
            layout->addLayout(wrapDialogButtonBox(m_buttonBox));
        }

        void PreferenceDialog::switchToPane(const PrefPane pane) {
            if (!currentPane()->validate()) {
                return;
            }

            m_stackedWidget->setCurrentIndex(pane);
            currentPane()->updateControls();

            auto* resetButton = m_buttonBox->button(QDialogButtonBox::RestoreDefaults);
            resetButton->setEnabled(currentPane()->canResetToDefaults());
        }

        PreferencePane* PreferenceDialog::currentPane() const {
            return static_cast<PreferencePane*>(m_stackedWidget->currentWidget());
        }

        void PreferenceDialog::resetToDefaults() {
            currentPane()->resetToDefaults();
        }

        // Don't display tooltips for pane switcher buttons...
        bool PreferenceDialog::eventFilter(QObject* o, QEvent* e) {
            if (e->type() == QEvent::ToolTip) {
                return true;
            }
            return QDialog::eventFilter(o, e);
        }
    }
}
