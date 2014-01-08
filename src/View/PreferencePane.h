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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__PreferencePane__
#define __TrenchBroom__PreferencePane__

#include <wx/panel.h>
#include <wx/slider.h>

namespace TrenchBroom {
    namespace View {
        class PreferencePane : public wxPanel {
        public:
            PreferencePane(wxWindow* parent);
            virtual ~PreferencePane();
            bool validate();
        protected:
            template <typename T>
            void bindSliderEvents(wxSlider* slider, void (T::*function)(wxScrollEvent&), T* handler) {
                slider->Bind(wxEVT_SCROLL_TOP, function, handler);
                slider->Bind(wxEVT_SCROLL_BOTTOM, function, handler);
                slider->Bind(wxEVT_SCROLL_LINEUP, function, handler);
                slider->Bind(wxEVT_SCROLL_LINEDOWN, function, handler);
                slider->Bind(wxEVT_SCROLL_PAGEUP, function, handler);
                slider->Bind(wxEVT_SCROLL_PAGEDOWN, function, handler);
                slider->Bind(wxEVT_SCROLL_THUMBTRACK, function, handler);
            }
        private:
            virtual bool doValidate() = 0;
        };
    }
}

#endif /* defined(__TrenchBroom__PreferencePane__) */
