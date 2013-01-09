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

#ifndef __TrenchBroom__PreferencesDialog__
#define __TrenchBroom__PreferencesDialog__

#include <wx/dialog.h>

class wxCheckBox;
class wxChoice;
class wxSlider;
class wxStaticText;

namespace TrenchBroom {
    namespace View {
        class PreferencesDialog : public wxDialog {
        protected:
            wxStaticText* m_quakePathValueLabel;
            wxSlider* m_brightnessSlider;
            wxSlider* m_gridAlphaSlider;
            wxChoice* m_gridModeChoice;
            wxSlider* m_lookSpeedSlider;
            wxCheckBox* m_invertLookXAxisCheckBox;
            wxCheckBox* m_invertLookYAxisCheckBox;
            wxSlider* m_panSpeedSlider;
            wxCheckBox* m_invertPanXAxisCheckBox;
            wxCheckBox* m_invertPanYAxisCheckBox;
            wxSlider* m_moveSpeedSlider;
            
            void updateControls();

            wxWindow* createQuakePreferences();
            wxWindow* createViewPreferences();
            wxWindow* createMousePreferences();
        public:
            PreferencesDialog();

            void OnChooseQuakePathClicked(wxCommandEvent& event);
            void OnViewSliderChanged(wxScrollEvent& event);
            void OnGridModeChoice(wxCommandEvent& event);
            void OnMouseSliderChanged(wxScrollEvent& event);
            void OnInvertAxisChanged(wxCommandEvent& event);
            void OnOkClicked(wxCommandEvent& event);
			void OnCancelClicked(wxCommandEvent& event);
			void OnCloseDialog(wxCloseEvent& event);
            void OnFileExit(wxCommandEvent& event);

            DECLARE_EVENT_TABLE();
        };
    }
}

#endif /* defined(__TrenchBroom__PreferencesDialog__) */
