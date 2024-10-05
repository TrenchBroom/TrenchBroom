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

#include "NotifierConnection.h"

#include "vm/forward.h"
#include "vm/util.h"
#include "vm/vec.h" // IWYU pragma: keep

#include <memory>

class QCheckBox;
class QComboBox;
class QPushButton;

namespace TrenchBroom::View
{
class MapDocument;
class RotateObjectsTool;
class Selection;
class SpinControl;

class RotateObjectsToolPage : public QWidget
{
  Q_OBJECT
private:
  std::weak_ptr<MapDocument> m_document;
  RotateObjectsTool& m_tool;

  QComboBox* m_recentlyUsedCentersList = nullptr;
  QPushButton* m_resetCenterButton = nullptr;

  SpinControl* m_angle = nullptr;
  QComboBox* m_axis = nullptr;
  QPushButton* m_rotateButton = nullptr;
  QCheckBox* m_updateAnglePropertyAfterTransformCheckBox = nullptr;

  NotifierConnection m_notifierConnection;

public:
  RotateObjectsToolPage(
    std::weak_ptr<MapDocument> document,
    RotateObjectsTool& tool,
    QWidget* parent = nullptr);

  void setAxis(vm::axis::type axis);
  void setRecentlyUsedCenters(const std::vector<vm::vec3d>& centers);
  void setCurrentCenter(const vm::vec3d& center);

private:
  void connectObservers();

  void createGui();
  void updateGui();

  void selectionDidChange(const Selection& selection);
  void documentWasNewedOrLoaded(MapDocument* document);

  void centerChanged();
  void resetCenterClicked();
  void angleChanged(double value);
  void rotateClicked();
  void updateAnglePropertyAfterTransformClicked();
  vm::vec3d getAxis() const;
};

} // namespace TrenchBroom::View
