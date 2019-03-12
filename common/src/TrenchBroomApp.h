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
//#include "View/RecentDocuments.h"

#include <QApplication>

namespace TrenchBroom {
    class Logger;
    class RecoverableException;

    namespace View {
        class ExecutableEvent;
        
        class TrenchBroomApp : public QApplication {
        private:
            FrameManager* m_frameManager;
            // FIXME:
            //RecentDocuments<TrenchBroomApp>* m_recentDocuments;
        public:
            Notifier0 recentDocumentsDidChangeNotifier;
        public:
            static TrenchBroomApp& instance();

            TrenchBroomApp(int& argc, char** argv);
            ~TrenchBroomApp() override;

        public:
            FrameManager* frameManager();

            // FIXME: recent
#if 0
            const IO::Path::List& recentDocuments() const;
            void addRecentDocumentMenu(wxMenu* menu);
            void removeRecentDocumentMenu(wxMenu* menu);
            void updateRecentDocument(const IO::Path& path);
#endif

            bool newDocument();
            bool openDocument(const String& pathStr);
            bool recoverFromException(const RecoverableException& e, const std::function<bool()>& op);
            void openPreferences();
            void openAbout();
            bool initializeGameFactory();

#if 0
            bool OnExceptionInMainLoop() override;
            void OnUnhandledException() override;
            void OnFatalException() override;
#endif
        private:
            void handleException();
        public:
            
            void OnFileNew();
            void OnFileOpen();
            void OnFileOpenRecent();
            void OnHelpShowManual();
            void OnOpenPreferences();
            void OnOpenAbout();
            void OnDebugShowCrashReportDialog();

//            void OnExecutableEvent(ExecutableEvent& event);

// FIXME: add apple only for Qt
#if 0
            void OnFileExit(wxCommandEvent& event);
            void OnUpdateUI(wxUpdateUIEvent& event);

            void MacNewFile() override;
            void MacOpenFiles(const QStringList& filenames) override;
#else
            bool openFilesOrWelcomeFrame(const QStringList& fileNames);
#endif
        private:
            static bool useSDI();
            void showWelcomeFrame();
        };

        void setCrashReportGUIEnbled(const bool guiEnabled);
        void reportCrashAndExit(const String &stacktrace, const String &reason);
        bool isReportingCrash();
    }
}

#endif /* defined(TrenchBroom_TrenchBroomApp) */
