/*
 Copyright (C) 2025 Kristian Duske

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

#include "UpdatePreferencePane.h"

#include <QBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QtGlobal>

#include "PreferenceManager.h"
#include "Preferences.h"
#include "TrenchBroomApp.h"
#include "ui/FormWithSectionsLayout.h"
#include "ui/QtUtils.h"
#include "ui/SliderWithLabel.h"
#include "ui/ViewConstants.h"
#include "update/Updater.h"

namespace tb::ui
{

UpdatePreferencePane::UpdatePreferencePane(QWidget* parent)
  : PreferencePane{parent}
{
  createGui();
}

void UpdatePreferencePane::createGui()
{
  auto* viewPreferences = createUpdatePreferences();

  auto* layout = new QVBoxLayout{};
  layout->setContentsMargins(QMargins{});
  layout->setSpacing(0);

  layout->addSpacing(LayoutConstants::NarrowVMargin);
  layout->addWidget(viewPreferences, 1);
  layout->addSpacing(LayoutConstants::MediumVMargin);
  setLayout(layout);
}

QWidget* UpdatePreferencePane::createUpdatePreferences()
{
  auto* updateInfo = new QLabel{tr(
    R"(TrenchBroom can check for updates. If an update is available, you will be notified in the status bar and other places.
To download and install an available update, click on the link labeled "Update available".)")};

  m_autoCheckForUpdates = new QCheckBox{};
  connect(m_autoCheckForUpdates, &QCheckBox::checkStateChanged, [&](const auto state) {
    if (!m_disableNotifiers)
    {
      const auto value = state == Qt::Checked;
      auto& prefs = PreferenceManager::instance();
      prefs.set(Preferences::AutoCheckForUpdates, value);
    }
  });

  m_includePreReleaseUpdates = new QCheckBox{};
  connect(
    m_includePreReleaseUpdates, &QCheckBox::checkStateChanged, [&](const auto state) {
      if (!m_disableNotifiers)
      {
        const auto value = state == Qt::Checked;
        auto& prefs = PreferenceManager::instance();
        prefs.set(Preferences::IncludePreReleaseUpdates, value);
        TrenchBroomApp::instance().updater().reset();
      }
    });

  m_includeDraftReleaseUpdates = new QCheckBox{};
  connect(
    m_includeDraftReleaseUpdates, &QCheckBox::checkStateChanged, [&](const auto state) {
      if (!m_disableNotifiers)
      {
        const auto value = state == Qt::Checked;
        auto& prefs = PreferenceManager::instance();
        prefs.set(Preferences::IncludeDraftReleaseUpdates, value);
        TrenchBroomApp::instance().updater().reset();
      }
    });

  auto* preReleaseInfo = new QLabel{tr(
    R"(Pre-releases are versions of TrenchBroom that are not yet considered stable. 
They may contain new features or bug fixes that are not yet part of a stable release.)")};

  auto& app = TrenchBroomApp::instance();
  auto* updateIndicator = app.updater().createUpdateIndicator();

  m_layout = new FormWithSectionsLayout{};
  m_layout->setContentsMargins(
    LayoutConstants::DialogOuterMargin,
    LayoutConstants::DialogOuterMargin,
    LayoutConstants::DialogOuterMargin,
    LayoutConstants::DialogOuterMargin);
  m_layout->setVerticalSpacing(LayoutConstants::WideVMargin);

  m_layout->addSection("Automatic Updates");
  m_layout->addRow(updateInfo);
  m_layout->addRow(updateIndicator);
  m_layout->addSection("Update Preferences");
  m_layout->addRow("Check for updates on startup", m_autoCheckForUpdates);
  m_layout->addRow("Include pre-releases", m_includePreReleaseUpdates);

  m_includeDraftReleaseUpdatesRow = m_layout->rowCount();
  m_layout->addRow("Include draft releases", m_includeDraftReleaseUpdates);
  m_layout->addRow(preReleaseInfo);

  auto* widget = new QWidget{};
  widget->setMinimumWidth(400);
  widget->setLayout(m_layout);
  return widget;
}

bool UpdatePreferencePane::canResetToDefaults()
{
  return true;
}

void UpdatePreferencePane::doResetToDefaults()
{
  auto& prefs = PreferenceManager::instance();
  prefs.resetToDefault(Preferences::AutoCheckForUpdates);
  prefs.resetToDefault(Preferences::IncludePreReleaseUpdates);
  prefs.resetToDefault(Preferences::IncludeDraftReleaseUpdates);
}

void UpdatePreferencePane::updateControls()
{
  const auto disableNotifiers = kdl::set_temp{m_disableNotifiers, true};
  m_autoCheckForUpdates->setChecked(pref(Preferences::AutoCheckForUpdates));
  m_includePreReleaseUpdates->setChecked(pref(Preferences::IncludePreReleaseUpdates));
  m_includeDraftReleaseUpdates->setChecked(pref(Preferences::IncludeDraftReleaseUpdates));
  m_layout->setRowVisible(
    m_includeDraftReleaseUpdatesRow, pref(Preferences::EnableDraftReleaseUpdates));
}

bool UpdatePreferencePane::validate()
{
  return true;
}

} // namespace tb::ui
