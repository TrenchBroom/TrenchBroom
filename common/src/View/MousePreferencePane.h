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

namespace TrenchBroom {
template <typename T> class Preference;

namespace View {
class KeySequenceEdit;
class SliderWithLabel;

class MousePreferencePane : public PreferencePane {
private:
  SliderWithLabel* m_lookSpeedSlider;
  QCheckBox* m_invertLookHAxisCheckBox;
  QCheckBox* m_invertLookVAxisCheckBox;
  SliderWithLabel* m_panSpeedSlider;
  QCheckBox* m_invertPanHAxisCheckBox;
  QCheckBox* m_invertPanVAxisCheckBox;
  SliderWithLabel* m_moveSpeedSlider;
  QCheckBox* m_invertMouseWheelCheckBox;
  QCheckBox* m_enableAltMoveCheckBox;
  QCheckBox* m_invertAltMoveAxisCheckBox;
  QCheckBox* m_moveInCursorDirCheckBox;

  KeySequenceEdit* m_forwardKeyEditor;
  KeySequenceEdit* m_backwardKeyEditor;
  KeySequenceEdit* m_leftKeyEditor;
  KeySequenceEdit* m_rightKeyEditor;
  KeySequenceEdit* m_upKeyEditor;
  KeySequenceEdit* m_downKeyEditor;
  SliderWithLabel* m_flyMoveSpeedSlider;

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
  void setKeySequence(KeySequenceEdit* editor, Preference<QKeySequence>& preference);
  bool hasConflict(
    const QKeySequence& keySequence, const Preference<QKeySequence>& preference) const;
};
} // namespace View
} // namespace TrenchBroom
