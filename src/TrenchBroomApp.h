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

#include "View/DocumentManager.h"

#include <wx/wx.h>

class TrenchBroomApp : public wxApp {
private:
    TrenchBroom::View::DocumentManager m_documentManager;
public:
    TrenchBroomApp();
    
    inline TrenchBroom::View::DocumentManager& documentManager() {
        return m_documentManager;
    }
    
    bool OnInit();
    int OnExit();

#ifdef __APPLE__
    void MacNewFile();
    void MacOpenFiles(const wxArrayString& filenames);
#endif
    
private:
    static bool useSDI();
    bool newDocument();
    bool openDocument(const String& pathStr);
};

namespace {
    inline static TrenchBroom::View::DocumentManager& documentManager() {
        TrenchBroomApp* app = static_cast<TrenchBroomApp*>(wxTheApp);
        return app->documentManager();
    }
}

#ifndef TESTING
DECLARE_APP(TrenchBroomApp)
#endif

#endif /* defined(__TrenchBroom__TrenchBroomApp__) */
