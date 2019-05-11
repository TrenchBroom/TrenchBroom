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

#ifndef PopupWindow_h
#define PopupWindow_h

#include <wx/frame.h>

namespace TrenchBroom {
    namespace View {
        class PopupWindow : public wxFrame {
        public:
            PopupWindow();
            PopupWindow(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& caption = "", const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style =  wxFRAME_FLOAT_ON_PARENT | wxFRAME_NO_TASKBAR | wxBORDER_NONE, const wxString& name = wxFrameNameStr);

            bool Create(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& caption = "", const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style =  wxFRAME_FLOAT_ON_PARENT | wxFRAME_NO_TASKBAR | wxBORDER_NONE, const wxString& name = wxFrameNameStr);

            void Position(const wxPoint& point, const wxSize& size);
            void Popup();
            void Dismiss();
        private:
            void OnActivate(wxActivateEvent& event);
        };
    }
}

#endif /* PopupWindow_h */
