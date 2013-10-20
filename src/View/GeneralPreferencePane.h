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

class wxButton;
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
            wxButton* m_chooseQuakePathButton;
            wxSlider* m_brightnessSlider;
            wxSlider* m_gridAlphaSlider;
            wxChoice* m_textureBrowserIconSizeChoice;
            wxSlider* m_lookSpeedSlider;
            wxCheckBox* m_invertLookHAxisCheckBox;
            wxCheckBox* m_invertLookVAxisCheckBox;
            wxSlider* m_panSpeedSlider;
            wxCheckBox* m_invertPanHAxisCheckBox;
            wxCheckBox* m_invertPanVAxisCheckBox;
            wxSlider* m_moveSpeedSlider;
            wxCheckBox* m_enableAltMoveCheckBox;
            wxCheckBox* m_moveInCursorDirCheckBox;
            
        public:
            GeneralPreferencePane(wxWindow* parent);

            void OnChooseQuakePathClicked(wxCommandEvent& event);
            void OnBrightnessChanged(wxScrollEvent& event);
            void OnGridAlphaChanged(wxScrollEvent& event);
            void OnTextureBrowserIconSizeChanged(wxCommandEvent& event);
            
            void OnLookSpeedChanged(wxScrollEvent& event);
            void OnInvertLookHAxisChanged(wxCommandEvent& event);
            void OnInvertLookVAxisChanged(wxCommandEvent& event);
            
            void OnPanSpeedChanged(wxScrollEvent& event);
            void OnInvertPanHAxisChanged(wxCommandEvent& event);
            void OnInvertPanVAxisChanged(wxCommandEvent& event);
            
            void OnMoveSpeedChanged(wxScrollEvent& event);
            
            void OnEnableAltMoveChanged(wxCommandEvent& event);
            void OnMoveCameraInCursorDirChanged(wxCommandEvent& event);

        private:
            void createGui();
            wxWindow* createQuakePreferences();
            wxWindow* createViewPreferences();
            wxWindow* createMousePreferences();

            void bindEvents();
            void bindSliderEvents(wxSlider* slider, void (GeneralPreferencePane::*function)(wxScrollEvent&));
            
            void updateControls();
            bool doValidate();
        };
    }
}

#endif /* defined(__TrenchBroom__GeneralPreferencePane__) */
