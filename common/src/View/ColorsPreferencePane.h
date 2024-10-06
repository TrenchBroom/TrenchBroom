/*
 Copyright (C) 2020 MaxED

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

#include "View/PreferencePane.h"

class QWidget;
class QTableView;
class QSortFilterProxyModel;

namespace tb::View
{
class ColorModel;

class ColorsPreferencePane : public PreferencePane
{
  Q_OBJECT
private:
  QTableView* m_table = nullptr;
  ColorModel* m_model = nullptr;
  QSortFilterProxyModel* m_proxy = nullptr;

public:
  explicit ColorsPreferencePane(QWidget* parent = nullptr);

private:
  bool canResetToDefaults() override;
  void doResetToDefaults() override;
  void updateControls() override;
  bool validate() override;
};

} // namespace tb::View
