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
#include "ui/RotateTool.h"

#include "vm/vec.h"

class QCheckBox;
class QComboBox;
class QPushButton;

namespace tb::mdl
{
class Map;

struct SelectionChange;
} // namespace tb::mdl

namespace tb::ui
{
class RotateTool;
class SpinControl;

class RotateToolPage : public QWidget
{
  Q_OBJECT
private:
  mdl::Map& m_map;
  RotateTool& m_tool;

  QComboBox* m_recentlyUsedCentersList = nullptr;
  QPushButton* m_resetCenterButton = nullptr;

  SpinControl* m_angle = nullptr;
  QComboBox* m_axis = nullptr;
  QPushButton* m_rotateButton = nullptr;
  QCheckBox* m_updateAnglePropertyAfterTransformCheckBox = nullptr;

  NotifierConnection m_notifierConnection;

  std::vector<vm::vec3d> m_recentlyUsedCenters;

public:
  RotateToolPage(mdl::Map& map, RotateTool& tool, QWidget* parent = nullptr);

private:
  void connectObservers();

  void createGui();
  void updateGui();

  void mapWasCreated(mdl::Map& map);
  void mapWasLoaded(mdl::Map& map);
  void selectionDidChange(const mdl::SelectionChange& selectionChange);

  void rotationCenterDidChange(const vm::vec3d& center);
  void rotationCenterWasUsed(const vm::vec3d& center);
  void handleHitAreaDidChange(RotateHandle::HitArea area);

  void centerChanged();
  void resetCenterClicked();
  void angleChanged(double value);
  void rotateClicked();
  void updateAnglePropertyAfterTransformClicked();
  vm::vec3d getAxis() const;
};

} // namespace tb::ui
