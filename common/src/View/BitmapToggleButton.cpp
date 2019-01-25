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

#include "BitmapToggleButton.h"

namespace TrenchBroom {
    namespace View {
        BitmapToggleButton::BitmapToggleButton(QWidget* parent, const wxWindowID windowId, const wxBitmap& upBitmap, const wxBitmap& downBitmap) :
        BitmapButton(parent, windowId),
        m_upBitmap(upBitmap),
        m_downBitmap(downBitmap),
        m_upDisabledBitmap(m_upBitmap.ConvertToDisabled()),
        m_downDisabledBitmap(m_downBitmap.ConvertToDisabled()),
        m_state(false) {
            assert(m_upBitmap.IsOk());
            assert(m_upDisabledBitmap.IsOk());
            assert(m_downBitmap.IsOk());
            assert(m_downDisabledBitmap.IsOk());
        }

        void BitmapToggleButton::DoUpdateWindowUI(wxUpdateUIEvent& event) {
            if (event.GetSetEnabled() && IsEnabled() != event.GetEnabled()) {
                Enable(event.GetEnabled());
                Refresh();
            }
            if (event.GetSetChecked()) {
                if (m_state != event.GetChecked()) {
                    m_state = event.GetChecked();
                    Refresh();
                }
            }
        }

        wxBitmap BitmapToggleButton::currentBitmap() const {
            if (IsEnabled()) {
                return m_state ? m_downBitmap : m_upBitmap;
            } else {
                return m_state ? m_downDisabledBitmap : m_upDisabledBitmap;
            }
        }

        void BitmapToggleButton::processClick() {
            m_state = !m_state;
            Refresh();

            wxCommandEvent buttonEvent(wxEVT_BUTTON, GetId());
            buttonEvent.SetEventObject(this);
            buttonEvent.SetInt(static_cast<int>(m_state));

            ProcessEvent(buttonEvent);
        }
    }
}
