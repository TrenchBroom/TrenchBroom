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

#ifndef __TrenchBroom__TrenchBroomApp__
#define __TrenchBroom__TrenchBroomApp__

#include "Notifier.h"
#include "IO/Path.h"
#include "Model/ModelTypes.h"
#include "View/FrameManager.h"
#include "View/RecentDocuments.h"

#include <wx/wx.h>

namespace TrenchBroom {
    class Logger;
    
    namespace View {
        class ExecutableEvent;
        
        class TrenchBroomApp : public wxApp {
        private:
            FrameManager* m_frameManager;
            RecentDocuments<TrenchBroomApp>* m_recentDocuments;
            wxLongLong m_lastActivation;
        public:
            Notifier0 recentDocumentsDidChangeNotifier;
        public:
            static TrenchBroomApp& instance();

            TrenchBroomApp();
            ~TrenchBroomApp();
            
            FrameManager* frameManager();
            
            const IO::Path::List& recentDocuments() const;
            void addRecentDocumentMenu(wxMenu* menu);
            void removeRecentDocumentMenu(wxMenu* menu);
            void updateRecentDocument(const IO::Path& path);
            
            bool newDocument();
            bool openDocument(const String& pathStr);
            void openPreferences();
            void openAbout();

            bool OnInit();
            int OnExit();
            void OnUnhandledException();
            
            void OnFileNew(wxCommandEvent& event);
            void OnFileOpen(wxCommandEvent& event);
            void OnFileOpenRecent(wxCommandEvent& event);
            void OnOpenPreferences(wxCommandEvent& event);
            void OnOpenAbout(wxCommandEvent& event);
            void OnExecutableEvent(ExecutableEvent& event);
            
            int FilterEvent(wxEvent& event);

#ifdef __APPLE__
            void OnFileExit(wxCommandEvent& event);
            void OnUpdateUI(wxUpdateUIEvent& event);

            void MacNewFile();
            void MacOpenFiles(const wxArrayString& filenames);
#endif
            
        private:
            static bool useSDI();
            void showWelcomeFrame();
        };
    }
}

#endif /* defined(__TrenchBroom__TrenchBroomApp__) */
