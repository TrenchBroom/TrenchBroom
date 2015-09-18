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

#ifndef TrenchBroom_AboutFrame
#define TrenchBroom_AboutFrame

#include <wx/frame.h>

#include "StringUtils.h"

class wxStaticText;
class wxWindow;

namespace TrenchBroom {
    namespace View {
        class AboutFrame : public wxFrame {
        private:
            static AboutFrame* instance;
        public:
            static void showAboutFrame();
            
            ~AboutFrame();
            
            void OnClickUrl(wxMouseEvent& event);
        private:
            AboutFrame();
            void createGui();
            wxStaticText* createURLText(wxWindow* parent, const String& text, const String& tooltip, const String& url);
        };
    }
}

#endif /* defined(TrenchBroom_AboutFrame) */
