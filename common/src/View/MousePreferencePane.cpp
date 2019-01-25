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
#include "View/KeyboardShortcutEditor.h"
#include "View/KeyboardShortcutEvent.h"
#include "View/ViewConstants.h"

#include <wx/checkbox.h>
#include <wx/gbsizer.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>

#include <algorithm>

namespace TrenchBroom {
    namespace View {
        MousePreferencePane::MousePreferencePane(QWidget* parent) :
        PreferencePane(parent) {
            createGui();
            bindEvents();
        }
        
        void MousePreferencePane::OnLookSpeedChanged(wxScrollEvent& event) {
            if (IsBeingDeleted()) return;

            const float value = getSliderValue(m_lookSpeedSlider);
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(Preferences::CameraLookSpeed, value);
        }
        
        void MousePreferencePane::OnInvertLookHAxisChanged(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            const bool value = event.GetInt() != 0;
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(Preferences::CameraLookInvertH, value);
        }
        
        void MousePreferencePane::OnInvertLookVAxisChanged(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            const bool value = event.GetInt() != 0;
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(Preferences::CameraLookInvertV, value);
        }
        
        void MousePreferencePane::OnPanSpeedChanged(wxScrollEvent& event) {
            if (IsBeingDeleted()) return;

            const float value = getSliderValue(m_panSpeedSlider);
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(Preferences::CameraPanSpeed, value);
        }
        
        void MousePreferencePane::OnInvertPanHAxisChanged(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            const bool value = event.GetInt() != 0;
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(Preferences::CameraPanInvertH, value);
        }
        
        void MousePreferencePane::OnInvertPanVAxisChanged(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            const bool value = event.GetInt() != 0;
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(Preferences::CameraPanInvertV, value);
        }
        
        void MousePreferencePane::OnMoveSpeedChanged(wxScrollEvent& event) {
            if (IsBeingDeleted()) return;

            const float value = getSliderValue(m_moveSpeedSlider);
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(Preferences::CameraMoveSpeed, value);
        }
        
        void MousePreferencePane::OnInvertMouseWheelChanged(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            
            const bool value = event.GetInt() != 0;
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(Preferences::CameraMouseWheelInvert, value);
        }

        void MousePreferencePane::OnEnableAltMoveChanged(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            const bool value = event.GetInt() != 0;
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(Preferences::CameraEnableAltMove, value);
        }
        
        void MousePreferencePane::OnInvertAltMoveAxisChanged(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            const bool value = event.GetInt() != 0;
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(Preferences::CameraAltMoveInvert, value);
        }
        
        void MousePreferencePane::OnMoveCameraInCursorDirChanged(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            const bool value = event.GetInt() != 0;
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(Preferences::CameraMoveInCursorDir, value);
        }
        
        void MousePreferencePane::OnForwardKeyChanged(KeyboardShortcutEvent& event) {
            if (IsBeingDeleted()) return;

            const KeyboardShortcut shortcut(event.key(), event.modifier1(), event.modifier2(), event.modifier3());
            if (!setShortcut(shortcut, Preferences::CameraFlyForward))
                event.Veto();
        }
        
        void MousePreferencePane::OnBackwardKeyChanged(KeyboardShortcutEvent& event) {
            if (IsBeingDeleted()) return;

            const KeyboardShortcut shortcut(event.key(), event.modifier1(), event.modifier2(), event.modifier3());
            if (!setShortcut(shortcut, Preferences::CameraFlyBackward))
                event.Veto();
        }
        
        void MousePreferencePane::OnLeftKeyChanged(KeyboardShortcutEvent& event) {
            if (IsBeingDeleted()) return;

            const KeyboardShortcut shortcut(event.key(), event.modifier1(), event.modifier2(), event.modifier3());
            if (!setShortcut(shortcut, Preferences::CameraFlyLeft))
                event.Veto();
        }
        
        void MousePreferencePane::OnRightKeyChanged(KeyboardShortcutEvent& event) {
            if (IsBeingDeleted()) return;

            const KeyboardShortcut shortcut(event.key(), event.modifier1(), event.modifier2(), event.modifier3());
            if (!setShortcut(shortcut, Preferences::CameraFlyRight))
                event.Veto();
        }

        void MousePreferencePane::OnUpKeyChanged(KeyboardShortcutEvent& event) {
            if (IsBeingDeleted()) return;

            const KeyboardShortcut shortcut(event.key(), event.modifier1(), event.modifier2(), event.modifier3());
            if (!setShortcut(shortcut, Preferences::CameraFlyUp))
                event.Veto();
        }

        void MousePreferencePane::OnDownKeyChanged(KeyboardShortcutEvent& event) {
            if (IsBeingDeleted()) return;

            const KeyboardShortcut shortcut(event.key(), event.modifier1(), event.modifier2(), event.modifier3());
            if (!setShortcut(shortcut, Preferences::CameraFlyDown))
                event.Veto();
        }

        bool MousePreferencePane::setShortcut(const KeyboardShortcut& shortcut, Preference<KeyboardShortcut>& preference) {
            if (!hasConflict(shortcut, preference)) {
                PreferenceManager& prefs = PreferenceManager::instance();
                prefs.set(preference, shortcut);
                return true;
            } else {
                return false;
            }
        }

        bool MousePreferencePane::hasConflict(const KeyboardShortcut& shortcut, const Preference<KeyboardShortcut>& preference) const {
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
        }

        void MousePreferencePane::OnFlyMoveSpeedChanged(wxScrollEvent& event) {
            if (IsBeingDeleted()) return;
            
            const float value = getSliderValue(m_flyMoveSpeedSlider);
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(Preferences::CameraFlyMoveSpeed, value);
        }

        void MousePreferencePane::createGui() {
            QWidget* mousePreferences = createCameraPreferences();

            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->AddSpacer(LayoutConstants::NarrowVMargin);
            sizer->Add(mousePreferences, 1, wxEXPAND);
            sizer->AddSpacer(LayoutConstants::WideVMargin);
            
            SetMinSize(sizer->GetMinSize());
            SetSizer(sizer);
        }
        
        QWidget* MousePreferencePane::createCameraPreferences() {
            QWidget* box = new QWidget(this);
            
            QLabel* lookPrefsHeader = new QLabel(box, wxID_ANY, "Mouse Look");
            lookPrefsHeader->SetFont(lookPrefsHeader->GetFont().Bold());
            QLabel* lookSpeedLabel = new QLabel(box, wxID_ANY, "Sensitivity");
            m_lookSpeedSlider = new wxSlider(box, wxID_ANY, 50, 1, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_BOTTOM);
            m_invertLookHAxisCheckBox = new wxCheckBox(box, wxID_ANY, "Invert X Axis");
            m_invertLookVAxisCheckBox = new wxCheckBox(box, wxID_ANY, "Invert Y Axis");
            
            QLabel* panPrefsHeader = new QLabel(box, wxID_ANY, "Mouse Pan");
            panPrefsHeader->SetFont(panPrefsHeader->GetFont().Bold());
            QLabel* panSpeedLabel = new QLabel(box, wxID_ANY, "Sensitivity");
            m_panSpeedSlider = new wxSlider(box, wxID_ANY, 50, 1, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_BOTTOM);
            
            m_invertPanHAxisCheckBox = new wxCheckBox(box, wxID_ANY, "Invert X Axis");
            m_invertPanVAxisCheckBox = new wxCheckBox(box, wxID_ANY, "Invert Y Axis");
            QLabel* movePrefsHeader = new QLabel(box, wxID_ANY, "Mouse Move");
            movePrefsHeader->SetFont(movePrefsHeader->GetFont().Bold());
            
            QLabel* moveSpeedLabel = new QLabel(box, wxID_ANY, "Sensitivity");
            m_moveSpeedSlider = new wxSlider(box, wxID_ANY, 50, 1, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_BOTTOM);
            m_invertMouseWheelCheckBox = new wxCheckBox(box, wxID_ANY, "Invert mouse wheel");
            m_enableAltMoveCheckBox = new wxCheckBox(box, wxID_ANY, "Alt+MMB drag to move camera");
            m_invertAltMoveAxisCheckBox = new wxCheckBox(box, wxID_ANY, "Invert Z axis in Alt+MMB drag");
            m_moveInCursorDirCheckBox = new wxCheckBox(box, wxID_ANY, "Move camera towards cursor");
            
            QLabel* keyPrefsHeader = new QLabel(box, wxID_ANY, "Move Keys");
            keyPrefsHeader->SetFont(lookPrefsHeader->GetFont().Bold());

            QLabel* forwardKeyLabel = new QLabel(box, wxID_ANY, "Forward");
            m_forwardKeyEditor = new KeyboardShortcutEditor(box, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME);
            m_forwardKeyEditor->SetMinSize(wxSize(80, wxDefaultCoord));
            QLabel* backwardKeyLabel = new QLabel(box, wxID_ANY, "Backward");
            m_backwardKeyEditor = new KeyboardShortcutEditor(box, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME);
            m_backwardKeyEditor->SetMinSize(wxSize(80, wxDefaultCoord));
            QLabel* leftKeyLabel = new QLabel(box, wxID_ANY, "Left");
            m_leftKeyEditor = new KeyboardShortcutEditor(box, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME);
            m_leftKeyEditor->SetMinSize(wxSize(80, wxDefaultCoord));
            QLabel* rightKeyLabel = new QLabel(box, wxID_ANY, "Right");
            m_rightKeyEditor = new KeyboardShortcutEditor(box, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME);
            m_rightKeyEditor->SetMinSize(wxSize(80, wxDefaultCoord));
            QLabel* upKeyLabel = new QLabel(box, wxID_ANY, "Up");
            m_upKeyEditor = new KeyboardShortcutEditor(box, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME);
            m_upKeyEditor->SetMinSize(wxSize(80, wxDefaultCoord));
            QLabel* downKeyLabel = new QLabel(box, wxID_ANY, "Down");
            m_downKeyEditor = new KeyboardShortcutEditor(box, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME);
            m_downKeyEditor->SetMinSize(wxSize(80, wxDefaultCoord));

            QLabel* flyMoveSpeedLabel = new QLabel(box, wxID_ANY, "Speed");
            m_flyMoveSpeedSlider = new wxSlider(box, wxID_ANY, 256, 64, 512, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_BOTTOM);

            const int HMargin           = LayoutConstants::WideHMargin;
            const int LMargin           = LayoutConstants::WideVMargin;
            const int HeaderFlags       = wxLEFT;
            const int LabelFlags        = wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxLEFT;
            const int SliderFlags       = wxEXPAND | wxRIGHT;
            const int CheckBoxFlags     = wxRIGHT;
            const int KeyEditorFlags    = wxRIGHT;
            const int LineFlags         = wxEXPAND | wxTOP;
            
            int r = 0;
            
            wxGridBagSizer* sizer = new wxGridBagSizer(LayoutConstants::NarrowVMargin, LayoutConstants::WideHMargin);
            sizer->Add(lookPrefsHeader,             wxGBPosition(r, 0), wxGBSpan(1,2), HeaderFlags, HMargin);
            ++r;
            
            sizer->Add(lookSpeedLabel,              wxGBPosition(r, 0), wxDefaultSpan, LabelFlags, HMargin);
            sizer->Add(m_lookSpeedSlider,           wxGBPosition(r, 1), wxDefaultSpan, SliderFlags, HMargin);
            ++r;
            
            sizer->Add(m_invertLookHAxisCheckBox,   wxGBPosition(r, 1), wxDefaultSpan, CheckBoxFlags, HMargin);
            ++r;
            
            sizer->Add(m_invertLookVAxisCheckBox,   wxGBPosition(r, 1), wxDefaultSpan, CheckBoxFlags, HMargin);
            ++r;
            
            sizer->Add(new BorderLine(box),         wxGBPosition(r, 0), wxGBSpan(1,2), LineFlags, LMargin);
            ++r;
            
            sizer->Add(panPrefsHeader,              wxGBPosition(r, 0), wxGBSpan(1,2), HeaderFlags, HMargin);
            ++r;
            
            sizer->Add(panSpeedLabel,               wxGBPosition(r, 0), wxDefaultSpan, LabelFlags, HMargin);
            sizer->Add(m_panSpeedSlider,            wxGBPosition(r, 1), wxDefaultSpan, SliderFlags, HMargin);
            ++r;
            
            sizer->Add(m_invertPanHAxisCheckBox,    wxGBPosition(r, 1), wxDefaultSpan, CheckBoxFlags, HMargin);
            ++r;
            
            sizer->Add(m_invertPanVAxisCheckBox,    wxGBPosition(r, 1), wxDefaultSpan, CheckBoxFlags, HMargin);
            ++r;
            
            sizer->Add(new BorderLine(box),         wxGBPosition(r, 0), wxGBSpan(1,2), LineFlags, LMargin);
            ++r;
            
            sizer->Add(movePrefsHeader,             wxGBPosition(r, 0), wxGBSpan(1,2), HeaderFlags, HMargin);
            ++r;
            
            sizer->Add(moveSpeedLabel,              wxGBPosition(r, 0), wxDefaultSpan, LabelFlags, HMargin);
            sizer->Add(m_moveSpeedSlider,           wxGBPosition(r, 1), wxDefaultSpan, SliderFlags, HMargin);
            ++r;

            sizer->Add(m_invertMouseWheelCheckBox,  wxGBPosition(r, 1), wxDefaultSpan, CheckBoxFlags, HMargin);
            ++r;
            
            sizer->Add(m_enableAltMoveCheckBox,     wxGBPosition(r, 1), wxDefaultSpan, CheckBoxFlags, HMargin);
            ++r;
            
            sizer->Add(m_invertAltMoveAxisCheckBox, wxGBPosition(r, 1), wxDefaultSpan, CheckBoxFlags, HMargin);
            ++r;
            
            sizer->Add(m_moveInCursorDirCheckBox,   wxGBPosition(r, 1), wxDefaultSpan, CheckBoxFlags, HMargin);
            ++r;
            
            sizer->Add(new BorderLine(box),         wxGBPosition(r, 0), wxGBSpan(1,2), LineFlags, LMargin);
            ++r;
            
            
            sizer->Add(keyPrefsHeader,              wxGBPosition(r, 0), wxGBSpan(1,2), HeaderFlags, HMargin);
            ++r;
            
            sizer->Add(forwardKeyLabel,             wxGBPosition(r, 0), wxDefaultSpan, LabelFlags, HMargin);
            sizer->Add(m_forwardKeyEditor,          wxGBPosition(r, 1), wxDefaultSpan, KeyEditorFlags, HMargin);
            ++r;
            
            sizer->Add(backwardKeyLabel,            wxGBPosition(r, 0), wxDefaultSpan, LabelFlags, HMargin);
            sizer->Add(m_backwardKeyEditor,         wxGBPosition(r, 1), wxDefaultSpan, KeyEditorFlags, HMargin);
            ++r;
            
            sizer->Add(leftKeyLabel,                wxGBPosition(r, 0), wxDefaultSpan, LabelFlags, HMargin);
            sizer->Add(m_leftKeyEditor,             wxGBPosition(r, 1), wxDefaultSpan, KeyEditorFlags, HMargin);
            ++r;
            
            sizer->Add(rightKeyLabel,               wxGBPosition(r, 0), wxDefaultSpan, LabelFlags, HMargin);
            sizer->Add(m_rightKeyEditor,            wxGBPosition(r, 1), wxDefaultSpan, KeyEditorFlags, HMargin);
            ++r;

            sizer->Add(upKeyLabel,                  wxGBPosition(r, 0), wxDefaultSpan, LabelFlags, HMargin);
            sizer->Add(m_upKeyEditor,               wxGBPosition(r, 1), wxDefaultSpan, KeyEditorFlags, HMargin);
            ++r;

            sizer->Add(downKeyLabel,                wxGBPosition(r, 0), wxDefaultSpan, LabelFlags, HMargin);
            sizer->Add(m_downKeyEditor,             wxGBPosition(r, 1), wxDefaultSpan, KeyEditorFlags, HMargin);
            ++r;

            sizer->Add(flyMoveSpeedLabel,           wxGBPosition(r, 0), wxDefaultSpan, LabelFlags, HMargin);
            sizer->Add(m_flyMoveSpeedSlider,        wxGBPosition(r, 1), wxDefaultSpan, SliderFlags, HMargin);
            ++r;
            

            sizer->AddGrowableCol(1);
            sizer->SetMinSize(500, wxDefaultCoord);
            box->SetSizer(sizer);
            return box;
        }
        
        void MousePreferencePane::bindEvents() {
            m_invertLookHAxisCheckBox->Bind(wxEVT_CHECKBOX, &MousePreferencePane::OnInvertLookHAxisChanged, this);
            m_invertLookVAxisCheckBox->Bind(wxEVT_CHECKBOX, &MousePreferencePane::OnInvertLookVAxisChanged, this);
            m_invertPanHAxisCheckBox->Bind(wxEVT_CHECKBOX, &MousePreferencePane::OnInvertPanHAxisChanged, this);
            m_invertPanVAxisCheckBox->Bind(wxEVT_CHECKBOX, &MousePreferencePane::OnInvertPanVAxisChanged, this);
            m_invertMouseWheelCheckBox->Bind(wxEVT_CHECKBOX, &MousePreferencePane::OnInvertMouseWheelChanged, this);
            m_enableAltMoveCheckBox->Bind(wxEVT_CHECKBOX, &MousePreferencePane::OnEnableAltMoveChanged, this);
            m_moveInCursorDirCheckBox->Bind(wxEVT_CHECKBOX, &MousePreferencePane::OnMoveCameraInCursorDirChanged, this);

            bindSliderEvents(m_lookSpeedSlider, &MousePreferencePane::OnLookSpeedChanged, this);
            bindSliderEvents(m_panSpeedSlider, &MousePreferencePane::OnPanSpeedChanged, this);
            bindSliderEvents(m_moveSpeedSlider, &MousePreferencePane::OnMoveSpeedChanged, this);

            m_forwardKeyEditor->Bind(KEYBOARD_SHORTCUT_EVENT, &MousePreferencePane::OnForwardKeyChanged, this);
            m_backwardKeyEditor->Bind(KEYBOARD_SHORTCUT_EVENT, &MousePreferencePane::OnBackwardKeyChanged, this);
            m_leftKeyEditor->Bind(KEYBOARD_SHORTCUT_EVENT, &MousePreferencePane::OnLeftKeyChanged, this);
            m_rightKeyEditor->Bind(KEYBOARD_SHORTCUT_EVENT, &MousePreferencePane::OnRightKeyChanged, this);
            m_upKeyEditor->Bind(KEYBOARD_SHORTCUT_EVENT, &MousePreferencePane::OnUpKeyChanged, this);
            m_downKeyEditor->Bind(KEYBOARD_SHORTCUT_EVENT, &MousePreferencePane::OnDownKeyChanged, this);

            bindSliderEvents(m_flyMoveSpeedSlider, &MousePreferencePane::OnFlyMoveSpeedChanged, this);
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
            
            prefs.resetToDefault(Preferences::CameraFlyForward);
            prefs.resetToDefault(Preferences::CameraFlyBackward);
            prefs.resetToDefault(Preferences::CameraFlyLeft);
            prefs.resetToDefault(Preferences::CameraFlyRight);
            prefs.resetToDefault(Preferences::CameraFlyUp);
            prefs.resetToDefault(Preferences::CameraFlyDown);

            prefs.resetToDefault(Preferences::CameraFlyMoveSpeed);
        }

        void MousePreferencePane::doUpdateControls() {
            setSliderValue(m_lookSpeedSlider, pref(Preferences::CameraLookSpeed));
            m_invertLookHAxisCheckBox->SetValue(pref(Preferences::CameraLookInvertH));
            m_invertLookVAxisCheckBox->SetValue(pref(Preferences::CameraLookInvertV));

            setSliderValue(m_panSpeedSlider, pref(Preferences::CameraPanSpeed));
            m_invertPanHAxisCheckBox->SetValue(pref(Preferences::CameraPanInvertH));
            m_invertPanVAxisCheckBox->SetValue(pref(Preferences::CameraPanInvertV));

            setSliderValue(m_moveSpeedSlider, pref(Preferences::CameraMoveSpeed));
            m_invertMouseWheelCheckBox->SetValue(pref(Preferences::CameraMouseWheelInvert));
            m_enableAltMoveCheckBox->SetValue(pref(Preferences::CameraEnableAltMove));
            m_invertAltMoveAxisCheckBox->SetValue(pref(Preferences::CameraAltMoveInvert));
            m_moveInCursorDirCheckBox->SetValue(pref(Preferences::CameraMoveInCursorDir));

            m_forwardKeyEditor->SetShortcut(pref(Preferences::CameraFlyForward));
            m_backwardKeyEditor->SetShortcut(pref(Preferences::CameraFlyBackward));
            m_leftKeyEditor->SetShortcut(pref(Preferences::CameraFlyLeft));
            m_rightKeyEditor->SetShortcut(pref(Preferences::CameraFlyRight));
            m_upKeyEditor->SetShortcut(pref(Preferences::CameraFlyUp));
            m_downKeyEditor->SetShortcut(pref(Preferences::CameraFlyDown));

            setSliderValue(m_flyMoveSpeedSlider, pref(Preferences::CameraFlyMoveSpeed));
        }
        
        bool MousePreferencePane::doValidate() {
            return true;
        }
	}
}
