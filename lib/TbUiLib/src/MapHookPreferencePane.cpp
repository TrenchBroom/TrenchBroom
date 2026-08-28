/*
 Copyright (C) 2026

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

#include "ui/MapHookPreferencePane.h"

#include <QBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>

#include "base/PreferenceManager.h"
#include "prefs/Preferences.h"
#include "ui/FormWithSectionsLayout.h"
#include "ui/QStyleUtils.h"
#include "ui/ViewConstants.h"

namespace tb::ui
{

MapHookPreferencePane::MapHookPreferencePane(QWidget* parent)
  : PreferencePane{parent}
{
  createGui();
}

void MapHookPreferencePane::createGui()
{
  m_runOnSave = new QCheckBox{};
  connect(m_runOnSave, &QCheckBox::checkStateChanged, [&](const auto state) {
    auto& prefs = PreferenceManager::instance();
    prefs.set(Preferences::MapHookRunOnSave, state == Qt::Checked);
  });

  m_runOnChange = new QCheckBox{};
  connect(m_runOnChange, &QCheckBox::checkStateChanged, [&](const auto state) {
    auto& prefs = PreferenceManager::instance();
    prefs.set(Preferences::MapHookRunOnChange, state == Qt::Checked);
  });

  m_debounceMs = new QSpinBox{};
  m_debounceMs->setRange(0, 60'000);
  m_debounceMs->setSingleStep(50);
  m_debounceMs->setSuffix(tr(" ms"));
  connect(
    m_debounceMs,
    &QSpinBox::valueChanged,
    this,
    [&](const int value) {
      auto& prefs = PreferenceManager::instance();
      prefs.set(Preferences::MapHookDebounceMs, value);
    });

  m_command = new QLineEdit{};
  m_command->setPlaceholderText(tr("e.g. rsync ${MAP_FULL_NAME} myserver:/maps/"));
  connect(m_command, &QLineEdit::editingFinished, this, [&]() {
    auto& prefs = PreferenceManager::instance();
    prefs.set(Preferences::MapHookCommand, m_command->text().toStdString());
  });

  auto* commandInfo = new QLabel{tr(
    R"(The command is run through a shell, so pipes, redirection etc. are supported.
It supports the same ${VARIABLE} substitutions as compilation profiles, e.g. ${MAP_FULL_NAME}, ${MAP_DIR_PATH}, ${MAP_BASE_NAME}, ${GAME_DIR_PATH}.
"On change" runs are debounced: rapid edits (e.g. dragging a vertex) only trigger one run, some time after things settle down.)")};
  setInfoStyle(commandInfo);

  auto* layout = new FormWithSectionsLayout{};
  layout->setContentsMargins(
    LayoutConstants::DialogOuterMargin,
    LayoutConstants::DialogOuterMargin,
    LayoutConstants::DialogOuterMargin,
    LayoutConstants::DialogOuterMargin);
  layout->setVerticalSpacing(LayoutConstants::WideVMargin);

  layout->addSection("Run Command On Save / Change");
  layout->addRow("Run on save", m_runOnSave);
  layout->addRow("Run on change", m_runOnChange);
  layout->addRow("Debounce interval", m_debounceMs);
  layout->addRow("Command", m_command);
  layout->addRow(commandInfo);

  auto* outerLayout = new QVBoxLayout{};
  outerLayout->setContentsMargins(QMargins{});
  outerLayout->setSpacing(0);
  outerLayout->addLayout(layout, 1);

  createScrollableContent(outerLayout);
}

bool MapHookPreferencePane::canResetToDefaults()
{
  return true;
}

void MapHookPreferencePane::doResetToDefaults()
{
  auto& prefs = PreferenceManager::instance();
  prefs.resetToDefault(Preferences::MapHookRunOnSave);
  prefs.resetToDefault(Preferences::MapHookRunOnChange);
  prefs.resetToDefault(Preferences::MapHookDebounceMs);
  prefs.resetToDefault(Preferences::MapHookCommand);
}

void MapHookPreferencePane::updateControls()
{
  auto& prefs = PreferenceManager::instance();

  m_runOnSave->setChecked(prefs.getPendingValue(Preferences::MapHookRunOnSave));
  m_runOnChange->setChecked(prefs.getPendingValue(Preferences::MapHookRunOnChange));
  m_debounceMs->setValue(prefs.getPendingValue(Preferences::MapHookDebounceMs));

  const auto command = prefs.getPendingValue(Preferences::MapHookCommand);
  if (m_command->text().toStdString() != command)
  {
    m_command->setText(QString::fromStdString(command));
  }
}

bool MapHookPreferencePane::validate()
{
  return true;
}

} // namespace tb::ui
