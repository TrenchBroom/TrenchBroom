/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#ifndef __TrenchBroom__TrenchBroomApp__
#define __TrenchBroom__TrenchBroomApp__

#include "View/FrameManager.h"

#include <wx/wx.h>

namespace TrenchBroom {
    namespace View {
        class TrenchBroomApp : public wxApp {
        private:
            TrenchBroom::View::FrameManager* m_frameManager;
        public:
            TrenchBroomApp();
            
            FrameManager* frameManager();
            
            bool OnInit();
            int OnExit();
            void OnUnhandledException();
            
            void OnFileNew(wxCommandEvent& event);
            void OnFileOpen(wxCommandEvent& event);

#ifdef __APPLE__
            void OnOpenPreferences(wxCommandEvent& event);
            void OnFileExit(wxCommandEvent& event);
            void OnUpdateUI(wxUpdateUIEvent& event);

            void MacNewFile();
            void MacOpenFiles(const wxArrayString& filenames);
#endif
            
        private:
            static bool useSDI();
            bool newDocument();
            bool openDocument(const String& pathStr);
        };
    }
}

#ifndef TESTING
DECLARE_APP(TrenchBroom::View::TrenchBroomApp)
#endif

#endif /* defined(__TrenchBroom__TrenchBroomApp__) */
