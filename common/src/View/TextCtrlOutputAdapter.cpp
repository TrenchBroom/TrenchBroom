/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "TextCtrlOutputAdapter.h"

#include <wx/textctrl.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        TextCtrlOutputAdapter::TextCtrlOutputAdapter(wxTextCtrl* textCtrl) :
        m_textCtrl(textCtrl) {
            assert(m_textCtrl != NULL);
            m_textCtrl->Bind(wxEVT_THREAD, &TextCtrlOutputAdapter::OnAsyncAppend, this, m_textCtrl->GetId());
        }

        TextCtrlOutputAdapter::~TextCtrlOutputAdapter() {
            m_textCtrl->Unbind(wxEVT_THREAD, &TextCtrlOutputAdapter::OnAsyncAppend, this, m_textCtrl->GetId());
        }

        void TextCtrlOutputAdapter::sendAppendEvent(const wxString& str) {
            wxThreadEvent* event = new wxThreadEvent(wxEVT_THREAD, m_textCtrl->GetId());
            event->SetString(str);
            m_textCtrl->GetEventHandler()->QueueEvent(event);
        }

        void TextCtrlOutputAdapter::OnAsyncAppend(wxThreadEvent& event) {
            const wxString& str = event.GetString();
            m_textCtrl->AppendText(str);
        }
    }
}
