/*
 Copyright (C) 2021 Amara M. Kilic
 Copyright (C) 2021 Kristian Duske

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

#include <QDialog>

class QLineEdit;
class QPushButton;
class QRadioButton;

namespace tb::ui
{
class MapWindow;

class ObjExportDialog : public QDialog
{
  Q_OBJECT
private:
  MapWindow* m_mapWindow;

  QLineEdit* m_exportPathEdit = nullptr;
  QPushButton* m_browseExportPathButton = nullptr;
  QRadioButton* m_relativeToGamePathRadioButton = nullptr;
  QRadioButton* m_relativeToExportPathRadioButton = nullptr;
  QPushButton* m_exportButton = nullptr;
  QPushButton* m_closeButton = nullptr;

public:
  explicit ObjExportDialog(MapWindow* mapWindow);

  void updateExportPath();

private:
  void createGui();
};

} // namespace tb::ui
