/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#ifndef TrenchBroom_ViewPreferencePane
#define TrenchBroom_ViewPreferencePane

#include "View/PreferencePane.h"

class wxColourPickerCtrl;
class wxColourPickerEvent;
class wxCheckBox;
class wxChoice;
class wxSlider;

namespace TrenchBroom {
    namespace View {
        class ViewPreferencePane : public PreferencePane {
        private:
            wxChoice* m_layoutChoice;
            wxSlider* m_brightnessSlider;
            wxSlider* m_gridAlphaSlider;
            wxColourPickerCtrl* m_backgroundColorPicker;
            wxCheckBox* m_showAxes;
            wxChoice* m_textureModeChoice;
            wxChoice* m_textureBrowserIconSizeChoice;
        public:
            ViewPreferencePane(wxWindow* parent);

            void OnLayoutChanged(wxCommandEvent& event);
            void OnBrightnessChanged(wxScrollEvent& event);
            void OnGridAlphaChanged(wxScrollEvent& event);
            void OnBackgroundColorChanged(wxColourPickerEvent& event);
            void OnShowAxesChanged(wxCommandEvent& event);
            void OnTextureModeChanged(wxCommandEvent& event);
            void OnTextureBrowserIconSizeChanged(wxCommandEvent& event);
        private:
            void createGui();
            wxWindow* createViewPreferences();

            void bindEvents();
            
            bool doCanResetToDefaults();
            void doResetToDefaults();
            void doUpdateControls();
            bool doValidate();
            
            size_t findTextureMode(int minFilter, int magFilter) const;
        };
    }
}

#endif /* defined(TrenchBroom_ViewPreferencePane) */
