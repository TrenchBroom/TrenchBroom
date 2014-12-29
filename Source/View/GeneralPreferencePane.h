/*
 Copyright (C) 2010-2012 Kristian Duske
 
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

#ifndef __TrenchBroom__GeneralPreferencePane__
#define __TrenchBroom__GeneralPreferencePane__

#include "View/PreferencePane.h"

class wxCheckBox;
class wxChoice;
class wxSlider;
class wxStaticText;

namespace TrenchBroom {
    namespace View {
        namespace PreferencesFrameLayout {
            static const int MinimumLabelWidth = 100;
        }

        class GeneralPreferencePane : public PreferencePane {
        private:
            wxStaticText* m_quakePathValueLabel;
            wxSlider* m_brightnessSlider;
            wxSlider* m_gridAlphaSlider;
            wxChoice* m_gridModeChoice;
            wxChoice* m_textureBrowserIconSizeChoice;
            wxChoice* m_instancingModeChoice;
            wxSlider* m_lookSpeedSlider;
            wxCheckBox* m_invertLookXAxisCheckBox;
            wxCheckBox* m_invertLookYAxisCheckBox;
            wxSlider* m_panSpeedSlider;
            wxCheckBox* m_invertPanXAxisCheckBox;
            wxCheckBox* m_invertPanYAxisCheckBox;
            wxSlider* m_moveSpeedSlider;
            wxCheckBox* m_enableAltMoveCheckBox;
            wxCheckBox* m_invertAltMoveAxisCheckBox;
            wxCheckBox* m_moveInCursorDirCheckBox;
            
            void updateControls();
            
            wxWindow* createQuakePreferences();
            wxWindow* createViewPreferences();
            wxWindow* createMousePreferences();
        public:
            GeneralPreferencePane(wxWindow* parent);

            bool validate();

            void OnChooseQuakePathClicked(wxCommandEvent& event);
            void OnViewSliderChanged(wxScrollEvent& event);
            void OnGridModeChoice(wxCommandEvent& event);
            void OnInstancingModeChoice(wxCommandEvent& event);
            void OnTextureBrowserIconSizeChoice(wxCommandEvent& event);
            void OnMouseSliderChanged(wxScrollEvent& event);
            void OnInvertAxisChanged(wxCommandEvent& event);
            void OnEnableAltMoveChanged(wxCommandEvent& event);
            void OnMoveCameraInCursorDirChanged(wxCommandEvent& event);
            
            DECLARE_EVENT_TABLE();
        };
    }
}

#endif /* defined(__TrenchBroom__GeneralPreferencePane__) */
