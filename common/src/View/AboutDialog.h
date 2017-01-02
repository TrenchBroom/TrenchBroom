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

#ifndef TrenchBroom_AboutDialog
#define TrenchBroom_AboutDialog

#include "StringUtils.h"

#include <wx/dialog.h>

class wxStaticText;
class wxWindow;

namespace TrenchBroom {
    namespace View {
        class AboutDialog : public wxDialog {
        private:
            static AboutDialog* instance;
        public:
            static void showAboutDialog();
            static void closeAboutDialog();
            
            ~AboutDialog();
            
            void OnClickUrl(wxMouseEvent& event);
        private:
            AboutDialog();
            void createGui();
            wxStaticText* createURLText(wxWindow* parent, const String& text, const String& tooltip, const String& url);
            
            void OnCancel(wxCommandEvent& event);
            void OnClose(wxCloseEvent& event);
        };
    }
}

#endif /* defined(TrenchBroom_AboutDialog) */
