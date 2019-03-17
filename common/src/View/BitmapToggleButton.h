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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_BitmapToggleButton
#define TrenchBroom_BitmapToggleButton

#include "View/BitmapButton.h"

#include <wx/bitmap.h>

namespace TrenchBroom {
    namespace View {
        class BitmapToggleButton : public BitmapButton {
        private:
            wxBitmap m_upBitmap;
            wxBitmap m_downBitmap;
            wxBitmap m_upDisabledBitmap;
            wxBitmap m_downDisabledBitmap;
            bool m_state;
        public:
            BitmapToggleButton(wxWindow* parent, wxWindowID windowId, const wxBitmap& upBitmap, const wxBitmap& downBitmap);

            void DoUpdateWindowUI(wxUpdateUIEvent& event) override;
        private:
            wxBitmap currentBitmap() const override;
            void processClick() override;
        };
    }
}

#endif /* defined(TrenchBroom_BitmapToggleButton) */
