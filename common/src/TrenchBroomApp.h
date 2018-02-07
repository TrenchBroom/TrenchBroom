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

#ifndef TrenchBroom_TrenchBroomApp
#define TrenchBroom_TrenchBroomApp

#include "Notifier.h"
#include "IO/Path.h"
#include "View/FrameManager.h"
#include "View/RecentDocuments.h"
#include <wx/app.h>

#include <functional>

class wxExtHelpController;

namespace TrenchBroom {
    class Logger;
    class RecoverableException;

    namespace View {
        class ExecutableEvent;
        
        class TrenchBroomApp : public wxApp {
        private:
            static const size_t MaxRecentDocuments = 15;

            FrameManager* m_frameManager;
            RecentDocuments<TrenchBroomApp>* m_recentDocuments;
            wxLongLong m_lastActivation;
        public:
            Notifier0 recentDocumentsDidChangeNotifier;
        public:
            static TrenchBroomApp& instance();

            TrenchBroomApp();
            ~TrenchBroomApp();
            
            void detectAndSetupUbuntu();
        protected:
            wxAppTraits* CreateTraits();
        public:
            FrameManager* frameManager();
            
            const IO::Path::List& recentDocuments() const;
            void addRecentDocumentMenu(wxMenu* menu);
            void removeRecentDocumentMenu(wxMenu* menu);
            void updateRecentDocument(const IO::Path& path);
            
            bool newDocument();
            bool openDocument(const String& pathStr);
            bool recoverFromException(const RecoverableException& e, const std::function<bool()>& op);
            void openPreferences();
            void openAbout();

            bool OnInit();
            
            bool OnExceptionInMainLoop();
            void OnUnhandledException();
            void OnFatalException();
        private:
            void handleException();
        public:
            
            int OnRun();
            
            void OnFileNew(wxCommandEvent& event);
            void OnFileOpen(wxCommandEvent& event);
            void OnFileOpenRecent(wxCommandEvent& event);
            void OnHelpShowManual(wxCommandEvent& event);
            void OnOpenPreferences(wxCommandEvent& event);
            void OnOpenAbout(wxCommandEvent& event);
            void OnDebugShowCrashReportDialog(wxCommandEvent& event);

            void OnExecutableEvent(ExecutableEvent& event);
            
            int FilterEvent(wxEvent& event);
#ifdef __APPLE__
            void OnFileExit(wxCommandEvent& event);
            void OnUpdateUI(wxUpdateUIEvent& event);

            void MacNewFile();
            void MacOpenFiles(const wxArrayString& filenames);
#else
            void OnInitCmdLine(wxCmdLineParser& parser);
            bool OnCmdLineParsed(wxCmdLineParser& parser);
#endif
        private:
            static bool useSDI();
        public:
            void showWelcomeFrame();
        };

        void setCrashReportGUIEnbled(const bool guiEnabled);
        void reportCrashAndExit(const String &stacktrace, const String &reason);
        bool isReportingCrash();
    }
}

#endif /* defined(TrenchBroom_TrenchBroomApp) */
