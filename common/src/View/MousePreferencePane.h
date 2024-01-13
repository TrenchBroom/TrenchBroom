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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "View/PreferencePane.h"

class QCheckBox;
class QKeySequence;
class QLabel;

namespace TrenchBroom
{
template <typename T>
class Preference;
}

namespace TrenchBroom::View
{
class KeySequenceEdit;
class SliderWithLabel;

class MousePreferencePane : public PreferencePane
{
private:
  SliderWithLabel* m_lookSpeedSlider = nullptr;
  QCheckBox* m_invertLookHAxisCheckBox = nullptr;
  QCheckBox* m_invertLookVAxisCheckBox = nullptr;
  SliderWithLabel* m_panSpeedSlider = nullptr;
  QCheckBox* m_invertPanHAxisCheckBox = nullptr;
  QCheckBox* m_invertPanVAxisCheckBox = nullptr;
  SliderWithLabel* m_moveSpeedSlider = nullptr;
  QCheckBox* m_invertMouseWheelCheckBox = nullptr;
  QCheckBox* m_enableAltMoveCheckBox = nullptr;
  QCheckBox* m_invertAltMoveAxisCheckBox = nullptr;
  QCheckBox* m_moveInCursorDirCheckBox = nullptr;

  KeySequenceEdit* m_forwardKeyEditor = nullptr;
  KeySequenceEdit* m_backwardKeyEditor = nullptr;
  KeySequenceEdit* m_leftKeyEditor = nullptr;
  KeySequenceEdit* m_rightKeyEditor = nullptr;
  KeySequenceEdit* m_upKeyEditor = nullptr;
  KeySequenceEdit* m_downKeyEditor = nullptr;
  QLabel* m_forwardKeyConflictIcon = nullptr;
  QLabel* m_backwardKeyConflictIcon = nullptr;
  QLabel* m_leftKeyConflictIcon = nullptr;
  QLabel* m_rightKeyConflictIcon = nullptr;
  QLabel* m_upKeyConflictIcon = nullptr;
  QLabel* m_downKeyConflictIcon = nullptr;

  SliderWithLabel* m_flyMoveSpeedSlider = nullptr;

public:
  explicit MousePreferencePane(QWidget* parent = nullptr);

private:
  void createGui();

  void bindEvents();

  bool doCanResetToDefaults() override;
  void doResetToDefaults() override;
  void doUpdateControls() override;
  bool doValidate() override;
private slots:
  void lookSpeedChanged(int value);
  void invertLookHAxisChanged(int state);
  void invertLookVAxisChanged(int state);

  void panSpeedChanged(int value);
  void invertPanHAxisChanged(int state);
  void invertPanVAxisChanged(int state);

  void moveSpeedChanged(int value);
  void invertMouseWheelChanged(int state);
  void enableAltMoveChanged(int state);
  void invertAltMoveAxisChanged(int state);
  void moveInCursorDirChanged(int state);

  void forwardKeyChanged();
  void backwardKeyChanged();
  void leftKeyChanged();
  void rightKeyChanged();
  void upKeyChanged();
  void downKeyChanged();

  void flyMoveSpeedChanged(int value);

private:
  void updateConflicts();
};

} // namespace TrenchBroom::View
