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

#pragma once

#include <QWidget>

namespace tb::ui
{

class PreferencePane : public QWidget
{
public:
  explicit PreferencePane(QWidget* parent = nullptr);
  ~PreferencePane() override;

  virtual bool canResetToDefaults() = 0;
  void resetToDefaults();
  virtual void updateControls() = 0;

  /**
   * Returns whether the settings in the preference pane are valid to save.
   * If the aren't, it also displays an error dialog box asking the user to correct the
   * issues.
   */
  virtual bool validate() = 0;

private:
  virtual void doResetToDefaults() = 0;
};

} // namespace tb::ui
