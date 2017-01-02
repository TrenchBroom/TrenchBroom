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

#include "TrenchBroomApp.h"

#include <clocale>
#include <fstream>

#include "GLInit.h"
#include "Macros.h"
#include "TrenchBroomAppTraits.h"
#include "TrenchBroomStackWalker.h"
#include "IO/Path.h"
#include "IO/SystemPaths.h"
#include "Model/GameFactory.h"
#include "Model/MapFormat.h"
#include "View/AboutDialog.h"
#include "View/ActionManager.h"
#include "View/CommandIds.h"
#include "View/CrashDialog.h"
#include "View/ExecutableEvent.h"
#include "View/GameDialog.h"
#include "View/MapDocument.h"
#include "View/MapFrame.h"
#include "View/PreferenceDialog.h"
#include "View/WelcomeFrame.h"
#include "View/GetVersion.h"
#include "View/MapViewBase.h"

#include <wx/choicdlg.h>
#include <wx/cmdline.h>
#include <wx/filedlg.h>
#include <wx/generic/helpext.h>
#include <wx/platinfo.h>
#include <wx/utils.h>
#include <wx/stdpaths.h>
#include <wx/msgdlg.h>
#include <wx/time.h>

namespace TrenchBroom {
    namespace View {
        TrenchBroomApp& TrenchBroomApp::instance() {
            TrenchBroomApp* app = static_cast<TrenchBroomApp*>(wxTheApp);
            return *app;
        }

#if defined(_WIN32) && defined(_MSC_VER)
        LONG WINAPI TrenchBroomUnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionPtrs);
#endif

        TrenchBroomApp::TrenchBroomApp() :
        wxApp(),
        m_frameManager(NULL),
        m_recentDocuments(NULL),
        m_lastActivation(0) {

#if defined(_WIN32) && defined(_MSC_VER)
            // with MSVC, set our own handler for segfaults so we can access the context
            // pointer, to allow StackWalker to read the backtrace.
            // see also: http://crashrpt.sourceforge.net/docs/html/exception_handling.html
            SetUnhandledExceptionFilter(TrenchBroomUnhandledExceptionFilter);
#else
            // enable having TrenchBroomApp::OnFatalException called on segfaults
            if (!wxHandleFatalExceptions(true)) {
                wxLogWarning("enabling wxHandleFatalExceptions failed");
            }
#endif

            detectAndSetupUbuntu();

            // always set this locale so that we can properly parse floats from text files regardless of the platforms locale
            std::setlocale(LC_NUMERIC, "C");

            // load image handlers
            wxImage::AddHandler(new wxPNGHandler());

            SetAppName("TrenchBroom");
            SetAppDisplayName("TrenchBroom");
            SetVendorDisplayName("Kristian Duske");
            SetVendorName("Kristian Duske");

            initGLFunctions();

            // these must be initialized here and not earlier
            m_frameManager = new FrameManager(useSDI());
            m_recentDocuments = new RecentDocuments<TrenchBroomApp>(CommandIds::Menu::FileRecentDocuments, 10);
            m_recentDocuments->setHandler(this, &TrenchBroomApp::OnFileOpenRecent);

#ifdef __APPLE__
            SetExitOnFrameDelete(false);
            const ActionManager& actionManager = ActionManager::instance();
            wxMenuBar* menuBar = actionManager.createMenuBar(false);
            wxMenuBar::MacSetCommonMenuBar(menuBar);

            wxMenu* recentDocumentsMenu = actionManager.findRecentDocumentsMenu(menuBar);
            ensure(recentDocumentsMenu != NULL, "recentDocumentsMenu is null");
            addRecentDocumentMenu(recentDocumentsMenu);

            Bind(wxEVT_MENU, &TrenchBroomApp::OnFileExit, this, wxID_EXIT);

            Bind(wxEVT_UPDATE_UI, &TrenchBroomApp::OnUpdateUI, this, wxID_NEW);
            Bind(wxEVT_UPDATE_UI, &TrenchBroomApp::OnUpdateUI, this, wxID_OPEN);
            Bind(wxEVT_UPDATE_UI, &TrenchBroomApp::OnUpdateUI, this, wxID_SAVE);
            Bind(wxEVT_UPDATE_UI, &TrenchBroomApp::OnUpdateUI, this, wxID_SAVEAS);
            Bind(wxEVT_UPDATE_UI, &TrenchBroomApp::OnUpdateUI, this, wxID_CLOSE);
            Bind(wxEVT_UPDATE_UI, &TrenchBroomApp::OnUpdateUI, this, wxID_UNDO);
            Bind(wxEVT_UPDATE_UI, &TrenchBroomApp::OnUpdateUI, this, wxID_REDO);
            Bind(wxEVT_UPDATE_UI, &TrenchBroomApp::OnUpdateUI, this, wxID_CUT);
            Bind(wxEVT_UPDATE_UI, &TrenchBroomApp::OnUpdateUI, this, wxID_COPY);
            Bind(wxEVT_UPDATE_UI, &TrenchBroomApp::OnUpdateUI, this, wxID_PASTE);
            Bind(wxEVT_UPDATE_UI, &TrenchBroomApp::OnUpdateUI, this, wxID_DELETE);
            Bind(wxEVT_UPDATE_UI, &TrenchBroomApp::OnUpdateUI, this, wxID_PREFERENCES);
            Bind(wxEVT_UPDATE_UI, &TrenchBroomApp::OnUpdateUI, this, wxID_ABOUT);
            Bind(wxEVT_UPDATE_UI, &TrenchBroomApp::OnUpdateUI, this, wxID_HELP);
            Bind(wxEVT_UPDATE_UI, &TrenchBroomApp::OnUpdateUI, this, CommandIds::Menu::Lowest, CommandIds::Menu::Highest);
#endif
            Bind(wxEVT_MENU, &TrenchBroomApp::OnFileNew, this, wxID_NEW);
            Bind(wxEVT_MENU, &TrenchBroomApp::OnFileOpen, this, wxID_OPEN);
            Bind(wxEVT_MENU, &TrenchBroomApp::OnHelpShowHelp, this, wxID_HELP);
            Bind(wxEVT_MENU, &TrenchBroomApp::OnOpenPreferences, this, wxID_PREFERENCES);
            Bind(wxEVT_MENU, &TrenchBroomApp::OnOpenAbout, this, wxID_ABOUT);

            Bind(EXECUTABLE_EVENT, &TrenchBroomApp::OnExecutableEvent, this);

            m_recentDocuments->didChangeNotifier.addObserver(recentDocumentsDidChangeNotifier);
        }

        TrenchBroomApp::~TrenchBroomApp() {
            wxImage::CleanUpHandlers();
            
            delete m_frameManager;
            m_frameManager = NULL;

            m_recentDocuments->didChangeNotifier.removeObserver(recentDocumentsDidChangeNotifier);
            delete m_recentDocuments;
            m_recentDocuments = NULL;
        }

        void TrenchBroomApp::detectAndSetupUbuntu() {
            // detect Ubuntu Linux and set the UBUNTU_MENUPROXY environment variable if necessary
#ifdef __WXGTK20__
            static const wxString varName("UBUNTU_MENUPROXY");
            if (!wxGetEnv(varName, NULL)) {
                const wxLinuxDistributionInfo distr = wxGetLinuxDistributionInfo();
                if (distr.Id.Lower().Find("ubuntu") != wxNOT_FOUND)
                    wxSetEnv(varName, "1");
            }
#endif
        }

        wxAppTraits* TrenchBroomApp::CreateTraits() {
            return new TrenchBroomAppTraits();
        }

        FrameManager* TrenchBroomApp::frameManager() {
            return m_frameManager;
        }

         const IO::Path::List& TrenchBroomApp::recentDocuments() const {
            return m_recentDocuments->recentDocuments();
        }

        void TrenchBroomApp::addRecentDocumentMenu(wxMenu* menu) {
            m_recentDocuments->addMenu(menu);
        }

        void TrenchBroomApp::removeRecentDocumentMenu(wxMenu* menu) {
            m_recentDocuments->removeMenu(menu);
        }

        void TrenchBroomApp::updateRecentDocument(const IO::Path& path) {
            m_recentDocuments->updatePath(path);
        }

        bool TrenchBroomApp::newDocument() {
            String gameName;
            Model::MapFormat::Type mapFormat = Model::MapFormat::Unknown;
            if (!GameDialog::showNewDocumentDialog(NULL, gameName, mapFormat))
                return false;

            Model::GameFactory& gameFactory = Model::GameFactory::instance();
            Model::GamePtr game = gameFactory.createGame(gameName);
            ensure(game.get() != NULL, "game is null");

            MapFrame* frame = m_frameManager->newFrame();
            frame->newDocument(game, mapFormat);
            return true;
        }

        bool TrenchBroomApp::openDocument(const String& pathStr) {
            MapFrame* frame = NULL;
            const IO::Path path(pathStr);
            try {
                String gameName = "";
                Model::MapFormat::Type mapFormat = Model::MapFormat::Unknown;
                
                Model::GameFactory& gameFactory = Model::GameFactory::instance();
                const std::pair<String, Model::MapFormat::Type> detected = gameFactory.detectGame(path);
                gameName = detected.first;
                mapFormat = detected.second;
                
                if (gameName.empty() || mapFormat == Model::MapFormat::Unknown) {
                    if (!GameDialog::showOpenDocumentDialog(NULL, gameName, mapFormat))
                        return false;
                }

                Model::GamePtr game = gameFactory.createGame(gameName);
                ensure(game.get() != NULL, "game is null");

                frame = m_frameManager->newFrame();
                frame->openDocument(game, mapFormat, path);
                return true;
            } catch (const FileNotFoundException& e) {
                m_recentDocuments->removePath(IO::Path(path));
                if (frame != NULL)
                    frame->Close();
                ::wxMessageBox(e.what(), "TrenchBroom", wxOK, NULL);
                return false;
            } catch (const Exception& e) {
                if (frame != NULL)
                    frame->Close();
                ::wxMessageBox(e.what(), "TrenchBroom", wxOK, NULL);
                return false;
            } catch (...) {
                if (frame != NULL)
                    frame->Close();
                ::wxMessageBox(pathStr + " could not be opened.", "TrenchBroom", wxOK, NULL);
                return false;
            }
        }

        void TrenchBroomApp::openPreferences() {
            PreferenceDialog dialog;
            dialog.ShowModal();
        }

        void TrenchBroomApp::openAbout() {
            AboutDialog::showAboutDialog();
        }

        bool TrenchBroomApp::OnInit() {
            if (!wxApp::OnInit())
                return false;

            SetAppName("TrenchBroom");
            SetAppDisplayName("TrenchBroom");
            SetVendorDisplayName("Kristian Duske");
            SetVendorName("Kristian Duske");

            return true;
        }
        
        static String makeCrashReport(const String &stacktrace, const String &reason) {
            StringStream ss;
            ss << "OS:\t" << wxGetOsDescription() << std::endl;
            ss << "wxWidgets:\n" << wxGetLibraryVersionInfo().ToString() << std::endl;
            ss << "GL_VENDOR:\t" << MapViewBase::glVendorString() << std::endl;
            ss << "GL_RENDERER:\t" << MapViewBase::glRendererString() << std::endl;
            ss << "GL_VERSION:\t" << MapViewBase::glVersionString() << std::endl;
            ss << "TrenchBroom Version:\t" << getBuildVersion() << " " << getBuildChannel() << std::endl;
            ss << "TrenchBroom Build:\t" << getBuildId() << " " << getBuildType() << std::endl;
            ss << "Reason:\t" << reason << std::endl;
            ss << "Stack trace:" << std::endl;
            ss << stacktrace << std::endl;
            return ss.str();
        }
        
        // returns the topmost MapDocument as a shared pointer, or the empty shared pointer
        static MapDocumentSPtr topDocument() {
            FrameManager *fm = TrenchBroomApp::instance().frameManager();
            if (fm == NULL)
                return MapDocumentSPtr();
            
            MapFrame *frame = fm->topFrame();
            if (frame == NULL)
                return MapDocumentSPtr();
            
            return frame->document();
        }
        
        // returns the empty path for unsaved maps, or if we can't determine the current map
        static IO::Path savedMapPath() {
            MapDocumentSPtr doc = topDocument();
            if (doc.get() == NULL)
                return IO::Path();
            
            IO::Path mapPath = doc->path();
            if (!mapPath.isAbsolute())
                return IO::Path();
            
            return mapPath;
        }

        static IO::Path crashLogPath() {
            IO::Path mapPath = savedMapPath();
            IO::Path crashLogPath;
            
            if (mapPath.isEmpty()) {
                IO::Path docsDir(wxStandardPaths::Get().GetDocumentsDir().ToStdString());
                crashLogPath = docsDir + IO::Path("trenchbroom-crash.txt");
            } else {
                String crashFileName = mapPath.lastComponent().deleteExtension().asString() + "-crash.txt";
                crashLogPath = mapPath.deleteLastComponent() + IO::Path(crashFileName);
            }
            
            // ensure it doesn't exist
            int index = 0;
            IO::Path testCrashLogPath = crashLogPath;
            while (wxFileExists(testCrashLogPath.asString())) {
                index++;
                
                StringStream testCrashLogName;
                testCrashLogName << crashLogPath.lastComponent().deleteExtension().asString() << "-" << index << ".txt";
                
                testCrashLogPath = crashLogPath.deleteLastComponent() + testCrashLogName.str();
            }
            return testCrashLogPath.asString();
        }
        
        static bool inReportCrashAndExit = false;
        static bool crashReportGuiEnabled = true;

        void setCrashReportGUIEnbled(const bool guiEnabled) {
            crashReportGuiEnabled = guiEnabled;
        }

        void reportCrashAndExit(const String &stacktrace, const String &reason) {
            // just abort if we reenter reportCrashAndExit (i.e. if it crashes)
            if (inReportCrashAndExit) {
                wxAbort();
            }
            inReportCrashAndExit = true;
            
            // get the crash report as a string
            String report = makeCrashReport(stacktrace, reason);
            
            // write it to the crash log file
            IO::Path logPath = crashLogPath();
            IO::Path mapPath = logPath.deleteExtension().addExtension("map");
            
            std::ofstream logStream(logPath.asString().c_str());
            logStream << report;
            logStream.close();
            std::cerr << "wrote crash log to " << logPath.asString() << std::endl;
            
            // save the map
            MapDocumentSPtr doc = topDocument();
            if (doc) {
                doc->saveDocumentTo(mapPath);
                std::cerr << "wrote map to " << mapPath.asString() << std::endl;
            } else {
                mapPath = IO::Path();
            }

            // write the crash log to stdout
            std::cerr << "crash log:" << std::endl;
            std::cerr << report << std::endl;

            if (crashReportGuiEnabled) {
                CrashDialog dialog;
                dialog.Create(logPath, mapPath);
                dialog.ShowModal();
            }
            
            wxAbort();
        }

        bool isReportingCrash() {
            return inReportCrashAndExit;
        }
        
        void TrenchBroomApp::OnUnhandledException() {
            handleException();
        }

        bool TrenchBroomApp::OnExceptionInMainLoop() {
            handleException();
            return false;
        }

        void TrenchBroomApp::OnFatalException() {
            reportCrashAndExit(TrenchBroomStackWalker::getStackTrace(), "OnFatalException");
        }
        
#if defined(_WIN32) && defined(_MSC_VER)
        LONG WINAPI TrenchBroomUnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionPtrs) {
            reportCrashAndExit(TrenchBroomStackWalker::getStackTraceFromContext(pExceptionPtrs->ContextRecord), "TrenchBroomUnhandledExceptionFilter");
            return EXCEPTION_EXECUTE_HANDLER;
        }
#endif
        
        void TrenchBroomApp::handleException() {
            try {
                throw;
            } catch (Exception& e) {
                const String reason = String("Exception: ") + e.what();
                reportCrashAndExit("", reason);
            } catch (std::exception& e) {
                const String reason = String("std::exception: ") + e.what();
                reportCrashAndExit("", reason);
            } catch (...) {
                reportCrashAndExit("", "Unknown exception");
            }
        }

        int TrenchBroomApp::OnRun() {
            const int result = wxApp::OnRun();
            wxConfigBase* config = wxConfig::Get(false);
            if (config != NULL)
                config->Flush();
            DeletePendingObjects();
            return result;
        }

        void TrenchBroomApp::OnFileNew(wxCommandEvent& event) {
            newDocument();
        }

        void TrenchBroomApp::OnFileOpen(wxCommandEvent& event) {
            const wxString pathStr = ::wxLoadFileSelector("",
#ifdef __WXGTK20__
                                                          "",
#else
                                                          "map",
#endif
                                                          "", NULL);
            if (!pathStr.empty())
                openDocument(pathStr.ToStdString());
        }

        void TrenchBroomApp::OnFileOpenRecent(wxCommandEvent& event) {
            const wxVariant* object = static_cast<wxVariant*>(event.m_callbackUserData); // this must be changed in 2.9.5 to event.GetEventUserData()
            ensure(object != NULL, "object is null");
            const wxString data = object->GetString();

            openDocument(data.ToStdString());
        }

        void TrenchBroomApp::OnHelpShowHelp(wxCommandEvent& event) {
            const IO::Path helpPath = IO::SystemPaths::resourceDirectory() + IO::Path("help/index.html");
            wxLaunchDefaultApplication(helpPath.asString());
        }

        void TrenchBroomApp::OnOpenPreferences(wxCommandEvent& event) {
            openPreferences();
        }

        void TrenchBroomApp::OnOpenAbout(wxCommandEvent& event) {
            openAbout();
        }

        void TrenchBroomApp::OnExecutableEvent(ExecutableEvent& event) {
            event.execute();
        }

        int TrenchBroomApp::FilterEvent(wxEvent& event) {
            if (event.GetEventObject() != NULL) {
                if (event.GetEventType() == wxEVT_ACTIVATE) {
                    m_lastActivation = wxGetLocalTimeMillis();
                } else if (event.GetEventType() == wxEVT_LEFT_DOWN ||
                           event.GetEventType() == wxEVT_MIDDLE_DOWN ||
                           event.GetEventType() == wxEVT_RIGHT_DOWN ||
                           event.GetEventType() == wxEVT_LEFT_UP ||
                           event.GetEventType() == wxEVT_MIDDLE_UP ||
                           event.GetEventType() == wxEVT_RIGHT_UP) {
                    if (wxGetLocalTimeMillis() - m_lastActivation <= 10)
                        return 1;
                }
            }
            return wxApp::FilterEvent(event);
        }

#ifdef __APPLE__
        void TrenchBroomApp::OnFileExit(wxCommandEvent& event) {
            if (m_frameManager->closeAllFrames())
                ExitMainLoop();
        }

        void TrenchBroomApp::OnUpdateUI(wxUpdateUIEvent& event) {
            switch (event.GetId()) {
                case wxID_PREFERENCES:
                case wxID_ABOUT:
                case wxID_NEW:
                case wxID_OPEN:
                case wxID_EXIT:
                case wxID_HELP:
                case CommandIds::Menu::FileOpenRecent:
                    event.Enable(true);
                    break;
                default:
                    if (event.GetId() >= CommandIds::Menu::FileRecentDocuments &&
                        event.GetId() < CommandIds::Menu::FileRecentDocuments + 10)
                        event.Enable(true);
                    else if (m_frameManager->allFramesClosed())
                        event.Enable(false);
                    break;
            }
        }

        void TrenchBroomApp::MacNewFile() {
            showWelcomeFrame();
        }

        void TrenchBroomApp::MacOpenFiles(const wxArrayString& filenames) {
            for (const wxString& filename : filenames)
                openDocument(filename.ToStdString());
        }
#else
        void TrenchBroomApp::OnInitCmdLine(wxCmdLineParser& parser) {
            static const wxCmdLineEntryDesc cmdLineDesc[] =
            {
                { wxCMD_LINE_PARAM,  NULL, NULL, "input file", wxCMD_LINE_VAL_STRING, useSDI() ? wxCMD_LINE_PARAM_OPTIONAL : (wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE) },
                { wxCMD_LINE_NONE, NULL, NULL, NULL, wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL }
            };

            parser.SetDesc(cmdLineDesc);
            parser.SetSwitchChars("-");
        }

        bool TrenchBroomApp::OnCmdLineParsed(wxCmdLineParser& parser) {
            if (parser.GetParamCount() > 0) {
                if (useSDI()) {
                    const wxString param = parser.GetParam(0);
                    openDocument(param.ToStdString());
                } else {
                    for (size_t i = 0; i < parser.GetParamCount(); ++i) {
                        const wxString param = parser.GetParam(i);
                        openDocument(param.ToStdString());
                    }
                }
            } else {
                showWelcomeFrame();
            }
            return true;
        }
#endif

        bool TrenchBroomApp::useSDI() {
#ifdef _WIN32
            return true;
#else
            return false;
#endif
        }

        void TrenchBroomApp::showWelcomeFrame() {
            WelcomeFrame* welcomeFrame = new WelcomeFrame();
            welcomeFrame->Show();
        }
    }
}
