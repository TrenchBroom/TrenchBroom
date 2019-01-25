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

#include "BitmapStaticButton.h"

#include <wx/bitmap.h>

namespace TrenchBroom {
    namespace View {
        BitmapStaticButton::BitmapStaticButton(QWidget* parent, const wxWindowID windowId, const wxBitmap& bitmap) :
        BitmapButton(parent, windowId),
        m_bitmap(bitmap),
        m_disabledBitmap(bitmap.ConvertToDisabled()) {
            assert(m_bitmap.IsOk());
            assert(m_disabledBitmap.IsOk());
        }

        wxBitmap BitmapStaticButton::currentBitmap() const {
            if (IsEnabled()) {
                return m_bitmap;
            } else {
                return m_disabledBitmap;
            }
        }

        void BitmapStaticButton::processClick() {
            wxCommandEvent buttonEvent(wxEVT_BUTTON, GetId());
            buttonEvent.SetEventObject(this);
            buttonEvent.SetInt(1);

            ProcessEvent(buttonEvent);
        }
    }
}

