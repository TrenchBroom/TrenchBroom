/*
 Copyright (C) 2010 Kristian Duske

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

#include <QBoxLayout>
#include <QCloseEvent>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QToolBar>
#include <QToolButton>

#include "PreferenceManager.h"
#include "io/ResourceUtils.h"
#include "ui/UpdatePreferencePane.h"
#if !defined __APPLE__
#include "ui/BorderLine.h"
#endif
#include "ui/ColorsPreferencePane.h"
#include "ui/GamesPreferencePane.h"
#include "ui/KeyboardPreferencePane.h"
#include "ui/MousePreferencePane.h"
#include "ui/PreferencePane.h"
#include "ui/QtUtils.h"
#include "ui/ViewPreferencePane.h"

#include <filesystem>

namespace tb::ui
{
enum class PreferenceDialog::PrefPane
{
  First = 0,
  Games = 0,
  View = 1,
  Colors = 2,
  Mouse = 3,
  Keyboard = 4,
  Update = 5,
  Last = 5
} PrefPane;


PreferenceDialog::PreferenceDialog(MapDocument* document, QWidget* parent)
  : QDialog{parent}
  , m_document{document}
{
  setWindowTitle("Preferences");
  setWindowIconTB(this);
  createGui();
  switchToPane(PrefPane::First);
  currentPane()->updateControls();
}

void PreferenceDialog::closeEvent(QCloseEvent* event)
{
  if (currentPane()->validate())
  {
    auto& prefs = PreferenceManager::instance();
    if (!prefs.saveInstantly())
    {
      prefs.discardChanges();
    }

    event->accept();
  }
  else
  {
    event->ignore();
  }
}

void PreferenceDialog::createGui()
{
  const auto gamesImage = io::loadSVGIcon("GeneralPreferences.svg");
  const auto viewImage = io::loadSVGIcon("ViewPreferences.svg");
  const auto colorsImage = io::loadSVGIcon("ColorPreferences.svg");
  const auto mouseImage = io::loadSVGIcon("MousePreferences.svg");
  const auto keyboardImage = io::loadSVGIcon("KeyboardPreferences.svg");
  const auto updateImage = io::loadSVGIcon("UpdatePreferences.svg");

  m_toolBar = new QToolBar{};
  m_toolBar->setFloatable(false);
  m_toolBar->setMovable(false);
  m_toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  m_toolBar->addAction(gamesImage, "Games", [&]() { switchToPane(PrefPane::Games); });
  m_toolBar->addAction(viewImage, "View", [&]() { switchToPane(PrefPane::View); });
  m_toolBar->addAction(colorsImage, "Colors", [&]() { switchToPane(PrefPane::Colors); });
  m_toolBar->addAction(mouseImage, "Mouse", [&]() { switchToPane(PrefPane::Mouse); });
  m_toolBar->addAction(
    keyboardImage, "Keyboard", [&]() { switchToPane(PrefPane::Keyboard); });
  m_toolBar->addAction(updateImage, "Update", [&]() { switchToPane(PrefPane::Update); });

  // Don't display tooltips for pane switcher buttons...
  for (auto* button : m_toolBar->findChildren<QToolButton*>())
  {
    button->installEventFilter(this);
  }

  m_stackedWidget = new QStackedWidget{};
  m_stackedWidget->addWidget(new GamesPreferencePane{m_document});
  m_stackedWidget->addWidget(new ViewPreferencePane{});
  m_stackedWidget->addWidget(new ColorsPreferencePane{});
  m_stackedWidget->addWidget(new MousePreferencePane{});
  m_stackedWidget->addWidget(new KeyboardPreferencePane{m_document});
  m_stackedWidget->addWidget(new UpdatePreferencePane{});

  m_buttonBox = new QDialogButtonBox{
    QDialogButtonBox::RestoreDefaults
#if !defined __APPLE__
      | QDialogButtonBox::Ok | QDialogButtonBox::Apply | QDialogButtonBox::Cancel
#endif
    ,
    this};

  auto* resetButton = m_buttonBox->button(QDialogButtonBox::RestoreDefaults);
  connect(resetButton, &QPushButton::clicked, this, &PreferenceDialog::resetToDefaults);

#if !defined __APPLE__
  connect(m_buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, [&]() {
    auto& prefs = PreferenceManager::instance();
    prefs.saveChanges();
    this->close();
  });
  connect(
    m_buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, [&]() {
      auto& prefs = PreferenceManager::instance();
      prefs.saveChanges();
    });
  connect(
    m_buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, [&]() {
      this->close();
    });
#endif

  auto* layout = new QVBoxLayout{};
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  setLayout(layout);

  layout->setMenuBar(m_toolBar);
#if !defined __APPLE__
  layout->addWidget(new BorderLine{});
#endif
  layout->addWidget(m_stackedWidget, 1);
  layout->addLayout(wrapDialogButtonBox(m_buttonBox));
}

void PreferenceDialog::switchToPane(const PrefPane pane)
{
  if (currentPane()->validate())
  {
    m_stackedWidget->setCurrentIndex(int(pane));
    currentPane()->updateControls();

    auto* resetButton = m_buttonBox->button(QDialogButtonBox::RestoreDefaults);
    resetButton->setEnabled(currentPane()->canResetToDefaults());
  }
}

PreferencePane* PreferenceDialog::currentPane() const
{
  return static_cast<PreferencePane*>(m_stackedWidget->currentWidget());
}

void PreferenceDialog::resetToDefaults()
{
  currentPane()->resetToDefaults();
}

// Don't display tooltips for pane switcher buttons...
bool PreferenceDialog::eventFilter(QObject* o, QEvent* e)
{
  return e->type() != QEvent::ToolTip ? QDialog::eventFilter(o, e) : true;
}

} // namespace tb::ui
