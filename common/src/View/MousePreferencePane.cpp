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

#include "MousePreferencePane.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "View/FormWithSectionsLayout.h"
#include "View/KeySequenceEdit.h"
#include "View/QtUtils.h"
#include "View/SliderWithLabel.h"
#include "View/ViewConstants.h"

#include <QCheckBox>
#include <QLabel>

#include <algorithm>

namespace TrenchBroom {
namespace View {
MousePreferencePane::MousePreferencePane(QWidget* parent)
  : PreferencePane(parent)
  , m_lookSpeedSlider(nullptr)
  , m_invertLookHAxisCheckBox(nullptr)
  , m_invertLookVAxisCheckBox(nullptr)
  , m_panSpeedSlider(nullptr)
  , m_invertPanHAxisCheckBox(nullptr)
  , m_invertPanVAxisCheckBox(nullptr)
  , m_moveSpeedSlider(nullptr)
  , m_invertMouseWheelCheckBox(nullptr)
  , m_enableAltMoveCheckBox(nullptr)
  , m_invertAltMoveAxisCheckBox(nullptr)
  , m_moveInCursorDirCheckBox(nullptr)
  , m_forwardKeyEditor(nullptr)
  , m_backwardKeyEditor(nullptr)
  , m_leftKeyEditor(nullptr)
  , m_rightKeyEditor(nullptr)
  , m_upKeyEditor(nullptr)
  , m_downKeyEditor(nullptr)
  , m_flyMoveSpeedSlider(nullptr) {
  createGui();
  bindEvents();
}

void MousePreferencePane::createGui() {
  m_lookSpeedSlider = new SliderWithLabel(1, 100);
  m_lookSpeedSlider->setMaximumWidth(400);
  m_invertLookHAxisCheckBox = new QCheckBox("Invert X axis");
  m_invertLookVAxisCheckBox = new QCheckBox("Invert Y axis");

  m_panSpeedSlider = new SliderWithLabel(1, 100);
  m_panSpeedSlider->setMaximumWidth(400);
  m_invertPanHAxisCheckBox = new QCheckBox("Invert X axis");
  m_invertPanVAxisCheckBox = new QCheckBox("Invert Y axis");

  m_moveSpeedSlider = new SliderWithLabel(1, 100);
  m_moveSpeedSlider->setMaximumWidth(400);
  m_invertMouseWheelCheckBox = new QCheckBox("Invert mouse wheel");
  m_enableAltMoveCheckBox = new QCheckBox("Alt + middle mouse drag to move camera");
  m_invertAltMoveAxisCheckBox = new QCheckBox("Invert Z axis in Alt + middle mouse drag");
  m_moveInCursorDirCheckBox = new QCheckBox("Move camera towards cursor");

  m_forwardKeyEditor = new KeySequenceEdit(1);
  m_forwardKeyEditor->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
  m_backwardKeyEditor = new KeySequenceEdit(1);
  m_backwardKeyEditor->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
  m_leftKeyEditor = new KeySequenceEdit(1);
  m_leftKeyEditor->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
  m_rightKeyEditor = new KeySequenceEdit(1);
  m_rightKeyEditor->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
  m_upKeyEditor = new KeySequenceEdit(1);
  m_upKeyEditor->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
  m_downKeyEditor = new KeySequenceEdit(1);
  m_downKeyEditor->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

  m_flyMoveSpeedSlider = new SliderWithLabel(0, 100);
  m_flyMoveSpeedSlider->setMaximumWidth(400);

  auto* layout = new FormWithSectionsLayout();
  layout->setContentsMargins(0, LayoutConstants::MediumVMargin, 0, 0);
  layout->setVerticalSpacing(2);
  // override the default to make the sliders take up maximum width
  layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

  layout->addSection("Mouse Look");
  layout->addRow("Sensitivity", m_lookSpeedSlider);
  layout->addRow("", m_invertLookHAxisCheckBox);
  layout->addRow("", m_invertLookVAxisCheckBox);

  layout->addSection("Mouse Pan");
  layout->addRow("Sensitivity", m_panSpeedSlider);
  layout->addRow("", m_invertPanHAxisCheckBox);
  layout->addRow("", m_invertPanVAxisCheckBox);

  layout->addSection("Mouse Move");
  layout->addRow("Sensitivity", m_moveSpeedSlider);
  layout->addRow("", m_invertMouseWheelCheckBox);
  layout->addRow("", m_enableAltMoveCheckBox);
  layout->addRow("", m_invertAltMoveAxisCheckBox);
  layout->addRow("", m_moveInCursorDirCheckBox);

  layout->addSection("Move Keys");
  layout->addRow("Forward", m_forwardKeyEditor);
  layout->addRow("Backward", m_backwardKeyEditor);
  layout->addRow("Left", m_leftKeyEditor);
  layout->addRow("Right", m_rightKeyEditor);
  layout->addRow("Up", m_upKeyEditor);
  layout->addRow("Down", m_downKeyEditor);
  layout->addRow("Speed", m_flyMoveSpeedSlider);
  layout->addRow(
    "",
    makeInfo(new QLabel(
      "Turn mouse wheel while holding right mouse button in 3D view to adjust speed on the fly.")));

  setLayout(layout);
  setMinimumWidth(400);
}

void MousePreferencePane::bindEvents() {
  connect(
    m_lookSpeedSlider, &SliderWithLabel::valueChanged, this,
    &MousePreferencePane::lookSpeedChanged);
  connect(
    m_invertLookHAxisCheckBox, &QCheckBox::stateChanged, this,
    &MousePreferencePane::invertLookHAxisChanged);
  connect(
    m_invertLookVAxisCheckBox, &QCheckBox::stateChanged, this,
    &MousePreferencePane::invertLookVAxisChanged);

  connect(
    m_panSpeedSlider, &SliderWithLabel::valueChanged, this, &MousePreferencePane::panSpeedChanged);
  connect(
    m_invertPanHAxisCheckBox, &QCheckBox::stateChanged, this,
    &MousePreferencePane::invertPanHAxisChanged);
  connect(
    m_invertPanVAxisCheckBox, &QCheckBox::stateChanged, this,
    &MousePreferencePane::invertPanVAxisChanged);

  connect(
    m_moveSpeedSlider, &SliderWithLabel::valueChanged, this,
    &MousePreferencePane::moveSpeedChanged);
  connect(
    m_invertMouseWheelCheckBox, &QCheckBox::stateChanged, this,
    &MousePreferencePane::invertMouseWheelChanged);
  connect(
    m_enableAltMoveCheckBox, &QCheckBox::stateChanged, this,
    &MousePreferencePane::enableAltMoveChanged);
  connect(
    m_invertAltMoveAxisCheckBox, &QCheckBox::stateChanged, this,
    &MousePreferencePane::invertAltMoveAxisChanged);
  connect(
    m_moveInCursorDirCheckBox, &QCheckBox::stateChanged, this,
    &MousePreferencePane::moveInCursorDirChanged);

  connect(
    m_forwardKeyEditor, &KeySequenceEdit::editingFinished, this,
    &MousePreferencePane::forwardKeyChanged);
  connect(
    m_backwardKeyEditor, &KeySequenceEdit::editingFinished, this,
    &MousePreferencePane::backwardKeyChanged);
  connect(
    m_leftKeyEditor, &KeySequenceEdit::editingFinished, this, &MousePreferencePane::leftKeyChanged);
  connect(
    m_rightKeyEditor, &KeySequenceEdit::editingFinished, this,
    &MousePreferencePane::rightKeyChanged);
  connect(
    m_upKeyEditor, &KeySequenceEdit::editingFinished, this, &MousePreferencePane::upKeyChanged);
  connect(
    m_downKeyEditor, &KeySequenceEdit::editingFinished, this, &MousePreferencePane::downKeyChanged);

  connect(
    m_flyMoveSpeedSlider, &SliderWithLabel::valueChanged, this,
    &MousePreferencePane::flyMoveSpeedChanged);
}

bool MousePreferencePane::doCanResetToDefaults() {
  return true;
}

void MousePreferencePane::doResetToDefaults() {
  PreferenceManager& prefs = PreferenceManager::instance();
  prefs.resetToDefault(Preferences::CameraLookSpeed);
  prefs.resetToDefault(Preferences::CameraLookInvertH);
  prefs.resetToDefault(Preferences::CameraLookInvertV);

  prefs.resetToDefault(Preferences::CameraPanSpeed);
  prefs.resetToDefault(Preferences::CameraPanInvertH);
  prefs.resetToDefault(Preferences::CameraPanInvertV);

  prefs.resetToDefault(Preferences::CameraMoveSpeed);
  prefs.resetToDefault(Preferences::CameraMouseWheelInvert);
  prefs.resetToDefault(Preferences::CameraEnableAltMove);
  prefs.resetToDefault(Preferences::CameraAltMoveInvert);
  prefs.resetToDefault(Preferences::CameraMoveInCursorDir);

  prefs.resetToDefault(Preferences::CameraFlyForward());
  prefs.resetToDefault(Preferences::CameraFlyBackward());
  prefs.resetToDefault(Preferences::CameraFlyLeft());
  prefs.resetToDefault(Preferences::CameraFlyRight());
  prefs.resetToDefault(Preferences::CameraFlyUp());
  prefs.resetToDefault(Preferences::CameraFlyDown());

  prefs.resetToDefault(Preferences::CameraFlyMoveSpeed);
}

void MousePreferencePane::doUpdateControls() {
  m_lookSpeedSlider->setRatio(pref(Preferences::CameraLookSpeed));
  m_invertLookHAxisCheckBox->setChecked(pref(Preferences::CameraLookInvertH));
  m_invertLookVAxisCheckBox->setChecked(pref(Preferences::CameraLookInvertV));

  m_panSpeedSlider->setRatio(pref(Preferences::CameraPanSpeed));
  m_invertPanHAxisCheckBox->setChecked(pref(Preferences::CameraPanInvertH));
  m_invertPanVAxisCheckBox->setChecked(pref(Preferences::CameraPanInvertV));

  m_moveSpeedSlider->setRatio(pref(Preferences::CameraMoveSpeed));
  m_invertMouseWheelCheckBox->setChecked(pref(Preferences::CameraMouseWheelInvert));
  m_enableAltMoveCheckBox->setChecked(pref(Preferences::CameraEnableAltMove));
  m_invertAltMoveAxisCheckBox->setChecked(pref(Preferences::CameraAltMoveInvert));
  m_moveInCursorDirCheckBox->setChecked(pref(Preferences::CameraMoveInCursorDir));

  m_forwardKeyEditor->setKeySequence(pref(Preferences::CameraFlyForward()));
  m_backwardKeyEditor->setKeySequence(pref(Preferences::CameraFlyBackward()));
  m_leftKeyEditor->setKeySequence(pref(Preferences::CameraFlyLeft()));
  m_rightKeyEditor->setKeySequence(pref(Preferences::CameraFlyRight()));
  m_upKeyEditor->setKeySequence(pref(Preferences::CameraFlyUp()));
  m_downKeyEditor->setKeySequence(pref(Preferences::CameraFlyDown()));

  m_flyMoveSpeedSlider->setRatio(
    pref(Preferences::CameraFlyMoveSpeed) / Preferences::MaxCameraFlyMoveSpeed);
}

bool MousePreferencePane::doValidate() {
  return true;
}

void MousePreferencePane::lookSpeedChanged(const int /* value */) {
  const auto ratio = m_lookSpeedSlider->ratio();
  PreferenceManager& prefs = PreferenceManager::instance();
  prefs.set(Preferences::CameraLookSpeed, ratio);
}

void MousePreferencePane::invertLookHAxisChanged(const int state) {
  const auto value = (state == Qt::Checked);
  PreferenceManager& prefs = PreferenceManager::instance();
  prefs.set(Preferences::CameraLookInvertH, value);
}

void MousePreferencePane::invertLookVAxisChanged(const int state) {
  const auto value = (state == Qt::Checked);
  PreferenceManager& prefs = PreferenceManager::instance();
  prefs.set(Preferences::CameraLookInvertV, value);
}

void MousePreferencePane::panSpeedChanged(const int /* value */) {
  const auto ratio = m_panSpeedSlider->ratio();
  PreferenceManager& prefs = PreferenceManager::instance();
  prefs.set(Preferences::CameraPanSpeed, ratio);
}

void MousePreferencePane::invertPanHAxisChanged(const int state) {
  const auto value = (state == Qt::Checked);
  PreferenceManager& prefs = PreferenceManager::instance();
  prefs.set(Preferences::CameraPanInvertH, value);
}

void MousePreferencePane::invertPanVAxisChanged(const int state) {
  const auto value = (state == Qt::Checked);
  PreferenceManager& prefs = PreferenceManager::instance();
  prefs.set(Preferences::CameraPanInvertV, value);
}

void MousePreferencePane::moveSpeedChanged(const int /* value */) {
  const auto ratio = m_moveSpeedSlider->ratio();
  PreferenceManager& prefs = PreferenceManager::instance();
  prefs.set(Preferences::CameraMoveSpeed, ratio);
}

void MousePreferencePane::invertMouseWheelChanged(const int state) {
  const auto value = (state == Qt::Checked);
  PreferenceManager& prefs = PreferenceManager::instance();
  prefs.set(Preferences::CameraMouseWheelInvert, value);
}

void MousePreferencePane::enableAltMoveChanged(const int state) {
  const auto value = (state == Qt::Checked);
  PreferenceManager& prefs = PreferenceManager::instance();
  prefs.set(Preferences::CameraEnableAltMove, value);
}

void MousePreferencePane::invertAltMoveAxisChanged(const int state) {
  const auto value = (state == Qt::Checked);
  PreferenceManager& prefs = PreferenceManager::instance();
  prefs.set(Preferences::CameraAltMoveInvert, value);
}

void MousePreferencePane::moveInCursorDirChanged(const int state) {
  const auto value = (state == Qt::Checked);
  PreferenceManager& prefs = PreferenceManager::instance();
  prefs.set(Preferences::CameraMoveInCursorDir, value);
}

void MousePreferencePane::forwardKeyChanged() {
  setKeySequence(m_forwardKeyEditor, Preferences::CameraFlyForward());
}

void MousePreferencePane::backwardKeyChanged() {
  setKeySequence(m_backwardKeyEditor, Preferences::CameraFlyBackward());
}

void MousePreferencePane::leftKeyChanged() {
  setKeySequence(m_leftKeyEditor, Preferences::CameraFlyLeft());
}

void MousePreferencePane::rightKeyChanged() {
  setKeySequence(m_rightKeyEditor, Preferences::CameraFlyRight());
}

void MousePreferencePane::upKeyChanged() {
  setKeySequence(m_upKeyEditor, Preferences::CameraFlyUp());
}

void MousePreferencePane::downKeyChanged() {
  setKeySequence(m_downKeyEditor, Preferences::CameraFlyDown());
}

void MousePreferencePane::flyMoveSpeedChanged(const int /* value */) {
  const auto ratio = Preferences::MaxCameraFlyMoveSpeed * m_flyMoveSpeedSlider->ratio();
  PreferenceManager& prefs = PreferenceManager::instance();
  prefs.set(Preferences::CameraFlyMoveSpeed, ratio);
}

void MousePreferencePane::setKeySequence(
  KeySequenceEdit* editor, Preference<QKeySequence>& preference) {
  const auto keySequence = editor->keySequence();
  PreferenceManager& prefs = PreferenceManager::instance();
  if (!hasConflict(keySequence, preference)) {
    prefs.set(preference, keySequence);
  } else {
    editor->setKeySequence(prefs.get(preference));
  }
}

bool MousePreferencePane::hasConflict(
  const QKeySequence& keySequence, const Preference<QKeySequence>& preference) const {
  const auto prefs = std::vector<Preference<QKeySequence>*>{
    &Preferences::CameraFlyForward(), &Preferences::CameraFlyBackward(),
    &Preferences::CameraFlyLeft(),    &Preferences::CameraFlyRight(),
    &Preferences::CameraFlyUp(),      &Preferences::CameraFlyDown()};

  return std::any_of(std::begin(prefs), std::end(prefs), [&keySequence, &preference](auto* other) {
    return preference.path() != other->path() && pref(*other) == keySequence;
  });
}
} // namespace View
} // namespace TrenchBroom
