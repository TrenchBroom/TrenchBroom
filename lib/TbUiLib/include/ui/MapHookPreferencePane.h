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

#pragma once

#include "ui/PreferencePane.h"

class QCheckBox;
class QLineEdit;
class QSpinBox;

namespace tb::ui
{

/**
 * Preferences UI for MapHookRunner: whether to run a shell command on save and/or on
 * change, the debounce interval used for the "on change" trigger, and the command itself.
 */
class MapHookPreferencePane : public PreferencePane
{
  Q_OBJECT
private:
  QCheckBox* m_runOnSave = nullptr;
  QCheckBox* m_runOnChange = nullptr;
  QSpinBox* m_debounceMs = nullptr;
  QLineEdit* m_command = nullptr;

public:
  explicit MapHookPreferencePane(QWidget* parent = nullptr);

private:
  void createGui();

  bool canResetToDefaults() override;
  void doResetToDefaults() override;
  void updateControls() override;
  bool validate() override;
};

} // namespace tb::ui
