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

#include "TrenchBroomApp.h"

#include <clocale>
#include <fstream>

#include "Macros.h"
#include "RecoverableExceptions.h"
#include "PreferenceManager.h"
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
        m_frameManager(nullptr),
        m_recentDocuments(nullptr),
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
            ensure(recentDocumentsMenu != nullptr, "recentDocumentsMenu is null");
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
            Bind(wxEVT_MENU, &TrenchBroomApp::OnHelpShowManual, this, wxID_HELP);
            Bind(wxEVT_MENU, &TrenchBroomApp::OnOpenPreferences, this, wxID_PREFERENCES);
            Bind(wxEVT_MENU, &TrenchBroomApp::OnOpenAbout, this, wxID_ABOUT);
            Bind(wxEVT_MENU, &TrenchBroomApp::OnDebugShowCrashReportDialog, this, CommandIds::Menu::DebugCrashReportDialog);

            Bind(EXECUTABLE_EVENT, &TrenchBroomApp::OnExecutableEvent, this);

            m_recentDocuments->didChangeNotifier.addObserver(recentDocumentsDidChangeNotifier);
        }

        TrenchBroomApp::~TrenchBroomApp() {
            wxImage::CleanUpHandlers();
            
            delete m_frameManager;
            m_frameManager = nullptr;

            m_recentDocuments->didChangeNotifier.removeObserver(recentDocumentsDidChangeNotifier);
            delete m_recentDocuments;
            m_recentDocuments = nullptr;
        }

        void TrenchBroomApp::detectAndSetupUbuntu() {
            // detect Ubuntu Linux and set the UBUNTU_MENUPROXY environment variable if necessary
#ifdef __WXGTK20__
            static const wxString varName("UBUNTU_MENUPROXY");
            if (!wxGetEnv(varName, nullptr)) {
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
            MapFrame* frame = nullptr;

            try {
                String gameName;
                Model::MapFormat::Type mapFormat = Model::MapFormat::Unknown;
                if (!GameDialog::showNewDocumentDialog(nullptr, gameName, mapFormat))
                    return false;

                frame = m_frameManager->newFrame();

                Model::GameFactory &gameFactory = Model::GameFactory::instance();
                Model::GameSPtr game = gameFactory.createGame(gameName, frame->logger());
                ensure(game.get() != nullptr, "game is null");

                frame->newDocument(game, mapFormat);
                return true;
            } catch (const RecoverableException& e) {
                if (frame != nullptr)
                    frame->Close();

                return recoverFromException(e, [this](){ return this->newDocument(); });
            } catch (const Exception& e) {
                if (frame != nullptr)
                    frame->Close();
                ::wxMessageBox(e.what(), "TrenchBroom", wxOK, nullptr);
                return false;
            }
        }

        bool TrenchBroomApp::openDocument(const String& pathStr) {
            MapFrame* frame = nullptr;
            const IO::Path path(pathStr);
            try {
                String gameName = "";
                Model::MapFormat::Type mapFormat = Model::MapFormat::Unknown;
                
                Model::GameFactory& gameFactory = Model::GameFactory::instance();
                std::tie(gameName, mapFormat) = gameFactory.detectGame(path);
                
                if (gameName.empty() || mapFormat == Model::MapFormat::Unknown) {
                    if (!GameDialog::showOpenDocumentDialog(nullptr, gameName, mapFormat))
                        return false;
                }

                frame = m_frameManager->newFrame();

                Model::GameSPtr game = gameFactory.createGame(gameName, frame->logger());
                ensure(game.get() != nullptr, "game is null");

                frame->openDocument(game, mapFormat, path);
                return true;
            } catch (const FileNotFoundException& e) {
                m_recentDocuments->removePath(IO::Path(path));
                if (frame != nullptr)
                    frame->Close();
                ::wxMessageBox(e.what(), "TrenchBroom", wxOK, nullptr);
                return false;
            } catch (const RecoverableException& e) {
                if (frame != nullptr)
                    frame->Close();

                return recoverFromException(e, [this, &pathStr](){ return this->openDocument(pathStr); });
            } catch (const Exception& e) {
                if (frame != nullptr)
                    frame->Close();
                ::wxMessageBox(e.what(), "TrenchBroom", wxOK, nullptr);
                return false;
            } catch (...) {
                if (frame != nullptr)
                    frame->Close();
                ::wxMessageBox(pathStr + " could not be opened.", "TrenchBroom", wxOK, nullptr);
                return false;
            }
        }

        bool TrenchBroomApp::recoverFromException(const RecoverableException &e, const std::function<bool()>& op) {
            // Guard against recursion. It's ok to use a static here since the functions calling this are not reentrant.
            static bool recovering = false;

            if (!recovering) {
                StringStream message;
                message << e.what() << "\n\n" << e.query();
                if (::wxMessageBox(message.str(), "TrenchBroom", wxYES_NO, nullptr) == wxYES) {
                    TemporarilySetBool setRecovering(recovering);
                    e.recover();
                    return op(); // Recursive call here.
                } else {
                    return false;
                }
            } else {
                ::wxMessageBox(e.what(), "TrenchBroom", wxOK, nullptr);
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
            SetAppName("TrenchBroom");
            SetAppDisplayName("TrenchBroom");
            SetVendorDisplayName("Kristian Duske");
            SetVendorName("Kristian Duske");

#if defined(_WIN32)
            // Make wxStandardPaths return the actual executable directory, without stripping off the "Debug" or "Release" directory
            // See: https://github.com/kduske/TrenchBroom/issues/1605
            wxStandardPaths::Get().DontIgnoreAppSubDir();
#endif

            return wxApp::OnInit();
        }

        int TrenchBroomApp::OnExit() {
            auto& prefs = PreferenceManager::instance();
            prefs.saveChanges();
            return wxAppBase::OnExit();
        }

        static String makeCrashReport(const String &stacktrace, const String &reason) {
            StringStream ss;
            ss << "OS:\t" << wxGetOsDescription() << std::endl;
            ss << "wxWidgets:\n" << wxGetLibraryVersionInfo().ToString() << std::endl;
            ss << "GL_VENDOR:\t" << MapViewBase::glVendorString() << std::endl;
            ss << "GL_RENDERER:\t" << MapViewBase::glRendererString() << std::endl;
            ss << "GL_VERSION:\t" << MapViewBase::glVersionString() << std::endl;
            ss << "TrenchBroom Version:\t" << getBuildVersion() << std::endl;
            ss << "TrenchBroom Build:\t" << getBuildIdStr() << std::endl;
            ss << "Reason:\t" << reason << std::endl;
            ss << "Stack trace:" << std::endl;
            ss << stacktrace << std::endl;
            return ss.str();
        }
        
        // returns the topmost MapDocument as a shared pointer, or the empty shared pointer
        static MapDocumentSPtr topDocument() {
            FrameManager *fm = TrenchBroomApp::instance().frameManager();
            if (fm == nullptr)
                return MapDocumentSPtr();
            
            MapFrame *frame = fm->topFrame();
            if (frame == nullptr)
                return MapDocumentSPtr();
            
            return frame->document();
        }
        
        // returns the empty path for unsaved maps, or if we can't determine the current map
        static IO::Path savedMapPath() {
            MapDocumentSPtr doc = topDocument();
            if (doc.get() == nullptr)
                return IO::Path();
            
            IO::Path mapPath = doc->path();
            if (!mapPath.isAbsolute())
                return IO::Path();
            
            return mapPath;
        }

        static IO::Path crashReportBasePath() {
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
                
                testCrashLogPath = crashLogPath.deleteLastComponent() + IO::Path(testCrashLogName.str());
            }
            return testCrashLogPath.deleteExtension();
        }
        
        static bool inReportCrashAndExit = false;
        static bool crashReportGuiEnabled = true;

        void setCrashReportGUIEnbled(const bool guiEnabled) {
            crashReportGuiEnabled = guiEnabled;
        }

        void reportCrashAndExit(const String &stacktrace, const String &reason) {
            // just abort if we reenter reportCrashAndExit (i.e. if it crashes)
            if (inReportCrashAndExit)
                wxAbort();
            
            inReportCrashAndExit = true;
            
            // get the crash report as a string
            const String report = makeCrashReport(stacktrace, reason);
            
            // write it to the crash log file
            const IO::Path basePath = crashReportBasePath();
            IO::Path reportPath = basePath.addExtension("txt");
            IO::Path mapPath = basePath.addExtension("map");
            IO::Path logPath = basePath.addExtension("log");
            
            std::ofstream reportStream(reportPath.asString().c_str());
            reportStream << report;
            reportStream.close();
            std::cerr << "wrote crash log to " << reportPath.asString() << std::endl;
            
            // save the map
            MapDocumentSPtr doc = topDocument();
            if (doc.get() != nullptr) {
                doc->saveDocumentTo(mapPath);
                std::cerr << "wrote map to " << mapPath.asString() << std::endl;
            } else {
                mapPath = IO::Path();
            }

            // Copy the log file
            if (!wxCopyFile(IO::SystemPaths::logFilePath().asString(), logPath.asString()))
                logPath = IO::Path();
            
            // write the crash log to stdout
            std::cerr << "crash log:" << std::endl;
            std::cerr << report << std::endl;

            if (crashReportGuiEnabled) {
                CrashDialog dialog;
                dialog.Create(reportPath, mapPath, logPath);
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
                                                          "", nullptr);
            if (!pathStr.empty())
                openDocument(pathStr.ToStdString());
        }

        void TrenchBroomApp::OnFileOpenRecent(wxCommandEvent& event) {
            const wxVariant* object = static_cast<wxVariant*>(event.m_callbackUserData); // this must be changed in 2.9.5 to event.GetEventUserData()
            ensure(object != nullptr, "object is null");
            const wxString data = object->GetString();

            openDocument(data.ToStdString());
        }

        void TrenchBroomApp::OnHelpShowManual(wxCommandEvent& event) {
            const IO::Path manualPath = IO::SystemPaths::resourceDirectory() + IO::Path("manual/index.html");
            wxLaunchDefaultApplication(manualPath.asString());
        }

        void TrenchBroomApp::OnOpenPreferences(wxCommandEvent& event) {
            openPreferences();
        }

        void TrenchBroomApp::OnOpenAbout(wxCommandEvent& event) {
            openAbout();
        }

        void TrenchBroomApp::OnDebugShowCrashReportDialog(wxCommandEvent& event) {
            const IO::Path reportPath(IO::SystemPaths::userDataDirectory() + IO::Path("crashreport.txt"));
            const IO::Path mapPath(IO::SystemPaths::userDataDirectory() + IO::Path("crashreport.map"));
            const IO::Path logPath(IO::SystemPaths::userDataDirectory() + IO::Path("crashreport.log"));
            
            CrashDialog dialog;
            dialog.Create(reportPath, mapPath, logPath);
            dialog.ShowModal();
        }

        void TrenchBroomApp::OnExecutableEvent(ExecutableEvent& event) {
            event.execute();
        }

        int TrenchBroomApp::FilterEvent(wxEvent& event) {
            if (event.GetEventObject() != nullptr) {
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
                case CommandIds::Menu::DebugCrashReportDialog:
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
                { wxCMD_LINE_PARAM,  nullptr, nullptr, "input file", wxCMD_LINE_VAL_STRING, useSDI() ? wxCMD_LINE_PARAM_OPTIONAL : (wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE) },
                { wxCMD_LINE_NONE, nullptr, nullptr, nullptr, wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL }
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
