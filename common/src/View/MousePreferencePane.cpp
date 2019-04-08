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

#include "StringUtils.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "View/BorderLine.h"
#include "View/FormWithSectionsLayout.h"
// #include "View/KeyboardShortcutEditor.h"
#include "View/ViewConstants.h"
#include "View/wxUtils.h"

#include <QCheckBox>
#include <QSlider>

#include <algorithm>

namespace TrenchBroom {
    namespace View {
        MousePreferencePane::MousePreferencePane(QWidget* parent) :
        PreferencePane(parent) {
            createGui();
            bindEvents();
        }

        void MousePreferencePane::createGui() {
            m_lookSpeedSlider = createSlider(1, 100);
            m_invertLookHAxisCheckBox = new QCheckBox("Invert X axis");
            m_invertLookVAxisCheckBox = new QCheckBox("Invert Y axis");

            m_panSpeedSlider = createSlider(1, 100);
            m_invertPanHAxisCheckBox = new QCheckBox("Invert X axis");
            m_invertPanVAxisCheckBox = new QCheckBox("Invert Y axis");

            m_moveSpeedSlider = createSlider(1, 100);
            m_invertMouseWheelCheckBox = new QCheckBox("Invert mouse wheel");
            m_enableAltMoveCheckBox = new QCheckBox("Alt + middle mouse drag to move camera");
            m_invertAltMoveAxisCheckBox = new QCheckBox("Invert Z axis in Alt + middle mouse drag");
            m_moveInCursorDirCheckBox = new QCheckBox("Move camera towards cursor");

            // FIXME: keyboard shortcuts for WASD

            m_flyMoveSpeedSlider = createSlider(256, 512);

            auto* layout = new FormWithSectionsLayout();
            layout->setContentsMargins(0, LayoutConstants::WideVMargin, 0, 0);
            layout->setVerticalSpacing(2);
            // override the default to make the sliders take up maximum width
            layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
            setLayout(layout);

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
            // FIXME: add keyboard shortcuts
            layout->addRow("Speed", m_flyMoveSpeedSlider);

            setMinimumWidth(500);
        }

        void MousePreferencePane::bindEvents() {
            connect(m_lookSpeedSlider, &QSlider::valueChanged, this, &MousePreferencePane::lookSpeedChanged);
            connect(m_invertLookHAxisCheckBox, &QCheckBox::stateChanged, this, &MousePreferencePane::invertLookHAxisChanged);
            connect(m_invertLookVAxisCheckBox, &QCheckBox::stateChanged, this, &MousePreferencePane::invertLookVAxisChanged);

            connect(m_panSpeedSlider, &QSlider::valueChanged, this, &MousePreferencePane::panSpeedChanged);
            connect(m_invertPanHAxisCheckBox, &QCheckBox::stateChanged, this, &MousePreferencePane::invertPanHAxisChanged);
            connect(m_invertPanVAxisCheckBox, &QCheckBox::stateChanged, this, &MousePreferencePane::invertPanVAxisChanged);

            connect(m_moveSpeedSlider, &QSlider::valueChanged, this, &MousePreferencePane::moveSpeedChanged);
            connect(m_invertMouseWheelCheckBox, &QCheckBox::stateChanged, this, &MousePreferencePane::invertMouseWheelChanged);
            connect(m_enableAltMoveCheckBox, &QCheckBox::stateChanged, this, &MousePreferencePane::enableAltMoveChanged);
            connect(m_invertAltMoveAxisCheckBox, &QCheckBox::stateChanged, this, &MousePreferencePane::invertAltMoveAxisChanged);
            connect(m_moveInCursorDirCheckBox, &QCheckBox::stateChanged, this, &MousePreferencePane::moveInCursorDirChanged);

            // FIXME: keyboard shortcuts
            /*
            m_forwardKeyEditor->Bind(KEYBOARD_SHORTCUT_EVENT, &MousePreferencePane::OnForwardKeyChanged, this);
            m_backwardKeyEditor->Bind(KEYBOARD_SHORTCUT_EVENT, &MousePreferencePane::OnBackwardKeyChanged, this);
            m_leftKeyEditor->Bind(KEYBOARD_SHORTCUT_EVENT, &MousePreferencePane::OnLeftKeyChanged, this);
            m_rightKeyEditor->Bind(KEYBOARD_SHORTCUT_EVENT, &MousePreferencePane::OnRightKeyChanged, this);
            m_upKeyEditor->Bind(KEYBOARD_SHORTCUT_EVENT, &MousePreferencePane::OnUpKeyChanged, this);
            m_downKeyEditor->Bind(KEYBOARD_SHORTCUT_EVENT, &MousePreferencePane::OnDownKeyChanged, this);
            */

            connect(m_flyMoveSpeedSlider, &QSlider::valueChanged, this, &MousePreferencePane::flyMoveSpeedChanged);
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

            /* FIXME: keyboard shortcuts
            prefs.resetToDefault(Preferences::CameraFlyForward);
            prefs.resetToDefault(Preferences::CameraFlyBackward);
            prefs.resetToDefault(Preferences::CameraFlyLeft);
            prefs.resetToDefault(Preferences::CameraFlyRight);
            prefs.resetToDefault(Preferences::CameraFlyUp);
            prefs.resetToDefault(Preferences::CameraFlyDown);
             */

            prefs.resetToDefault(Preferences::CameraFlyMoveSpeed);
        }

        void MousePreferencePane::doUpdateControls() {
            setSliderValue(m_lookSpeedSlider, pref(Preferences::CameraLookSpeed));
            m_invertLookHAxisCheckBox->setChecked(pref(Preferences::CameraLookInvertH));
            m_invertLookVAxisCheckBox->setChecked(pref(Preferences::CameraLookInvertV));

            setSliderValue(m_panSpeedSlider, pref(Preferences::CameraPanSpeed));
            m_invertPanHAxisCheckBox->setChecked(pref(Preferences::CameraPanInvertH));
            m_invertPanVAxisCheckBox->setChecked(pref(Preferences::CameraPanInvertV));

            setSliderValue(m_moveSpeedSlider, pref(Preferences::CameraMoveSpeed));
            m_invertMouseWheelCheckBox->setChecked(pref(Preferences::CameraMouseWheelInvert));
            m_enableAltMoveCheckBox->setChecked(pref(Preferences::CameraEnableAltMove));
            m_invertAltMoveAxisCheckBox->setChecked(pref(Preferences::CameraAltMoveInvert));
            m_moveInCursorDirCheckBox->setChecked(pref(Preferences::CameraMoveInCursorDir));

            /* FIXME: keyboard shortcuts
            m_forwardKeyEditor->SetShortcut(pref(Preferences::CameraFlyForward));
            m_backwardKeyEditor->SetShortcut(pref(Preferences::CameraFlyBackward));
            m_leftKeyEditor->SetShortcut(pref(Preferences::CameraFlyLeft));
            m_rightKeyEditor->SetShortcut(pref(Preferences::CameraFlyRight));
            m_upKeyEditor->SetShortcut(pref(Preferences::CameraFlyUp));
            m_downKeyEditor->SetShortcut(pref(Preferences::CameraFlyDown));
            */

            setSliderValue(m_flyMoveSpeedSlider, pref(Preferences::CameraFlyMoveSpeed));
        }

        bool MousePreferencePane::doValidate() {
            return true;
        }

        void MousePreferencePane::lookSpeedChanged(const int value) {
            const auto ratio = getSliderRatio(m_lookSpeedSlider);
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

        void MousePreferencePane::panSpeedChanged(const int value) {
            const auto ratio = getSliderRatio(m_panSpeedSlider);
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

        void MousePreferencePane::moveSpeedChanged(const int value) {
            const auto ratio = getSliderRatio(m_moveSpeedSlider);
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

        /* FIXME: keyboard shortcuts
        void MousePreferencePane::forwardKeyChanged(KeyboardShortcutEvent& event) {
            const KeyboardShortcut shortcut(event.key(), event.modifier1(), event.modifier2(), event.modifier3());
            if (!setShortcut(shortcut, Preferences::CameraFlyForward))
                event.Veto();
        }

        void MousePreferencePane::backwardKeyChanged(KeyboardShortcutEvent& event) {


            const KeyboardShortcut shortcut(event.key(), event.modifier1(), event.modifier2(), event.modifier3());
            if (!setShortcut(shortcut, Preferences::CameraFlyBackward))
                event.Veto();
        }

        void MousePreferencePane::leftKeyChanged(KeyboardShortcutEvent& event) {


            const KeyboardShortcut shortcut(event.key(), event.modifier1(), event.modifier2(), event.modifier3());
            if (!setShortcut(shortcut, Preferences::CameraFlyLeft))
                event.Veto();
        }

        void MousePreferencePane::rightKeyChanged(KeyboardShortcutEvent& event) {


            const KeyboardShortcut shortcut(event.key(), event.modifier1(), event.modifier2(), event.modifier3());
            if (!setShortcut(shortcut, Preferences::CameraFlyRight))
                event.Veto();
        }

        void MousePreferencePane::upKeyChanged(KeyboardShortcutEvent& event) {


            const KeyboardShortcut shortcut(event.key(), event.modifier1(), event.modifier2(), event.modifier3());
            if (!setShortcut(shortcut, Preferences::CameraFlyUp))
                event.Veto();
        }

        void MousePreferencePane::downKeyChanged(KeyboardShortcutEvent& event) {


            const KeyboardShortcut shortcut(event.key(), event.modifier1(), event.modifier2(), event.modifier3());
            if (!setShortcut(shortcut, Preferences::CameraFlyDown))
                event.Veto();
        }
        */

        void MousePreferencePane::flyMoveSpeedChanged(const int value) {
            const auto ratio = getSliderRatio(m_flyMoveSpeedSlider);
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(Preferences::CameraFlyMoveSpeed, ratio);
        }

        bool MousePreferencePane::setShortcut(const KeyboardShortcut& shortcut, Preference<KeyboardShortcut>& preference) {
            /* FIXME: keybard shortcuts
            if (!hasConflict(shortcut, preference)) {
                PreferenceManager& prefs = PreferenceManager::instance();
                prefs.set(preference, shortcut);
                return true;
            } else {
                return false;
            }
             */
            return true;
        }

        bool MousePreferencePane::hasConflict(const KeyboardShortcut& shortcut, const Preference<KeyboardShortcut>& preference) const {
            /* FIXME: keyboard shortcuts
            const auto prefs = std::vector<Preference<KeyboardShortcut>*>{
                &Preferences::CameraFlyForward,
                &Preferences::CameraFlyBackward,
                &Preferences::CameraFlyLeft,
                &Preferences::CameraFlyRight,
                &Preferences::CameraFlyUp,
                &Preferences::CameraFlyDown
            };

            return std::any_of(std::begin(prefs), std::end(prefs), [&shortcut, &preference](const auto* other){
                return preference.path() != other->path() && pref(*other).hasKey() && pref(*other) == shortcut;
            });
             */
            return false;
        }
    }
}
