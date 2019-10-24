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

#include "Macros.h"
#include "RecoverableExceptions.h"
#include "TrenchBroomStackWalker.h"
#include "IO/Path.h"
#include "IO/PathQt.h"
#include "IO/DiskIO.h"
#include "IO/SystemPaths.h"
#include "Model/GameFactory.h"
#include "Model/MapFormat.h"
#include "View/AboutDialog.h"
#include "View/Actions.h"
#include "View/CrashDialog.h"
#include "View/GameDialog.h"
#include "View/GLContextManager.h"
#include "View/MapDocument.h"
#include "View/MapFrame.h"
#include "View/PreferenceDialog.h"
#include "View/WelcomeWindow.h"
#include "View/GetVersion.h"
#include "View/MapViewBase.h"
#include "View/wxUtils.h"
#include "StringUtils.h"
#ifdef __APPLE__
#include "View/MainMenuBuilder.h"
#endif

#include <QCommandLineParser>
#include <QtDebug>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QFile>
#include <QStandardPaths>
#include <QSysInfo>
#include <QUrl>

#include <clocale>
#include <csignal>
#include <cstdlib>
#include <fstream>

namespace TrenchBroom {
    namespace View {
        TrenchBroomApp& TrenchBroomApp::instance() {
            auto* app = dynamic_cast<TrenchBroomApp*>(qApp);
            return *app;
        }

#if defined(_WIN32) && defined(_MSC_VER)
        LONG WINAPI TrenchBroomUnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionPtrs);
#else
        static void CrashHandler(int signum);
#endif

        TrenchBroomApp::TrenchBroomApp(int& argc, char** argv) :
        QApplication(argc, argv),
        m_frameManager(nullptr),
        m_recentDocuments(nullptr),
        m_welcomeWindow(nullptr) {
            // When this flag is enabled, font and palette changes propagate as though the user had manually called the corresponding QWidget methods.
            setAttribute(Qt::AA_UseStyleSheetPropagationInWidgetStyles);

#if defined __APPLE__
            // fix default palette higlight text color
            QPalette palette;
            palette.setColor(QPalette::HighlightedText, Qt::white);
            setPalette(palette);
#endif

#if defined(_WIN32) && defined(_MSC_VER)
            // with MSVC, set our own handler for segfaults so we can access the context
            // pointer, to allow StackWalker to read the backtrace.
            // see also: http://crashrpt.sourceforge.net/docs/html/exception_handling.html
            SetUnhandledExceptionFilter(TrenchBroomUnhandledExceptionFilter);
#else
            signal(SIGSEGV, CrashHandler);
#endif

            // always set this locale so that we can properly parse floats from text files regardless of the platforms locale
            std::setlocale(LC_NUMERIC, "C");

            setApplicationName("TrenchBroom");
            // Needs to be "" otherwise Qt adds this to the paths returned by QStandardPaths
            // which would cause preferences to move from where they were with wx
            setOrganizationName("");
            setOrganizationDomain("com.kristianduske");

            if (!initializeGameFactory()) {
                QCoreApplication::exit(1);
                return;
            }

            // these must be initialized here and not earlier
            m_frameManager = std::make_unique<FrameManager>(useSDI());

            m_recentDocuments = std::make_unique<RecentDocuments>(10);
            connect(m_recentDocuments.get(), &RecentDocuments::loadDocument, this, [this](const IO::Path& path) { openDocument(path); });

#ifdef __APPLE__
            setQuitOnLastWindowClosed(false);

            auto* menuBar = new QMenuBar();
            using ActionMap = std::map<const Action*, QAction*>;
            ActionMap actionMap;

            MainMenuBuilder menuBuilder(*menuBar, actionMap, [](const Action& action) {
                ActionExecutionContext context(nullptr, nullptr);
                action.execute(context);
            });

            const auto& actionManager = ActionManager::instance();
            actionManager.visitMainMenu(menuBuilder);

            addRecentDocumentMenu(menuBuilder.recentDocumentsMenu);

            ActionExecutionContext context(nullptr, nullptr);
            for (auto [tAction, qAction] : actionMap) {
                qAction->setEnabled(tAction->enabled(context));
                if (qAction->isCheckable()) {
                    qAction->setChecked(tAction->checked(context));
                }
            }

#endif

            m_recentDocuments->didChangeNotifier.addObserver(recentDocumentsDidChangeNotifier);
        }

        void TrenchBroomApp::parseCommandLineAndShowFrame() {
            // FIXME: Do this here, or after the exec() call?
            QCommandLineParser parser;
            parser.process(*this);
            openFilesOrWelcomeFrame(parser.positionalArguments());
        }

        QSettings& TrenchBroom::View::TrenchBroomApp::settings() {
            return getSettings();
        }

        FrameManager* TrenchBroomApp::frameManager() {
            return m_frameManager.get();
        }

         const IO::Path::List& TrenchBroomApp::recentDocuments() const {
            return m_recentDocuments->recentDocuments();
        }

        void TrenchBroomApp::addRecentDocumentMenu(QMenu* menu) {
            m_recentDocuments->addMenu(menu);
        }

        void TrenchBroomApp::removeRecentDocumentMenu(QMenu* menu) {
            m_recentDocuments->removeMenu(menu);
        }

        void TrenchBroomApp::updateRecentDocument(const IO::Path& path) {
            m_recentDocuments->updatePath(path);
        }

        bool TrenchBroomApp::openDocument(const IO::Path& path) {
            MapFrame* frame = nullptr;
            try {
                String gameName;
                Model::MapFormat mapFormat = Model::MapFormat::Unknown;

                Model::GameFactory& gameFactory = Model::GameFactory::instance();
                std::tie(gameName, mapFormat) = gameFactory.detectGame(path);

                if (gameName.empty() || mapFormat == Model::MapFormat::Unknown) {
                    if (!GameDialog::showOpenDocumentDialog(nullptr, gameName, mapFormat)) {
                        return false;
                    }
                }

                frame = m_frameManager->newFrame();

                Model::GameSPtr game = gameFactory.createGame(gameName, frame->logger());
                ensure(game.get() != nullptr, "game is null");

                hideWelcomeWindow();
                frame->openDocument(game, mapFormat, path);
                return true;
            } catch (const FileNotFoundException& e) {
                m_recentDocuments->removePath(IO::Path(path));
                if (frame != nullptr) {
                    frame->close();
                }
                QMessageBox::critical(nullptr, "TrenchBroom", e.what(), QMessageBox::Ok);
                return false;
            } catch (const RecoverableException& e) {
                if (frame != nullptr) {
                    frame->close();
                }
                return recoverFromException(e, [this, &path](){ return this->openDocument(path); });
            } catch (const Exception& e) {
                if (frame != nullptr) {
                    frame->close();
                }
                QMessageBox::critical(nullptr, "TrenchBroom", e.what(), QMessageBox::Ok);
                return false;
            } catch (...) {
                if (frame != nullptr) {
                    frame->close();
                }
                QMessageBox::critical(nullptr, "TrenchBroom", IO::pathAsQString(path) + " could not be opened.", QMessageBox::Ok);
                return false;
            }
        }

        bool TrenchBroomApp::recoverFromException(const RecoverableException& e, const std::function<bool()>& retry) {
            // Guard against recursion. It's ok to use a static here since the functions calling this are not reentrant.
            static bool recovering = false;

            if (!recovering) {
                StringStream message;
                message << e.what() << "\n\n" << e.query();

                const auto result = QMessageBox::question(nullptr, QString("TrenchBroom"), QString::fromStdString(message.str()), QMessageBox::Yes | QMessageBox::No);
                if (result == QMessageBox::Yes) {
                    TemporarilySetBool setRecovering(recovering);
                    e.recover();
                    return retry(); // Recursive call here.
                } else {
                    return false;
                }
            } else {
                QMessageBox::critical(nullptr, "TrenchBroom", e.what(), QMessageBox::Ok);
                return false;
            }
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

        void TrenchBroomApp::openPreferences() {
            PreferenceDialog dialog(topDocument());
            dialog.exec();
        }

        void TrenchBroomApp::openAbout() {
            AboutDialog::showAboutDialog();
        }

        bool TrenchBroomApp::initializeGameFactory() {
            try {
                auto& gameFactory = Model::GameFactory::instance();
                gameFactory.initialize();
            } catch (const std::exception& e) {
                qCritical() << e.what();
                return false;
            } catch (const StringList& errors) {
                StringStream str;
                if (errors.size() == 1) {
                    str << "An error occurred while loading the game configuration files:\n\n";
                    str << StringUtils::join(errors, "\n\n");
                    str << "\n\nThis file has been ignored.";
                } else {
                    str << "Multiple errors occurred while loading the game configuration files:\n\n";
                    str << StringUtils::join(errors, "\n\n");
                    str << "\n\nThese files have been ignored.";
                }

                QMessageBox::critical(nullptr, "TrenchBroom", QString::fromStdString(str.str()), QMessageBox::Ok);
            }
            return true;
        }

        static String makeCrashReport(const String &stacktrace, const String &reason) {
            StringStream ss;
            ss << "OS:\t" << QSysInfo::prettyProductName().toStdString() << std::endl;
            ss << "Qt:\t" << qVersion() << std::endl;
            ss << "GL_VENDOR:\t" << GLContextManager::GLVendor << std::endl;
            ss << "GL_RENDERER:\t" << GLContextManager::GLRenderer << std::endl;
            ss << "GL_VERSION:\t" << GLContextManager::GLVersion << std::endl;
            ss << "TrenchBroom Version:\t" << getBuildVersion().toStdString() << std::endl;
            ss << "TrenchBroom Build:\t" << getBuildIdStr().toStdString() << std::endl;
            ss << "Reason:\t" << reason << std::endl;
            ss << "Stack trace:" << std::endl;
            ss << stacktrace << std::endl;
            return ss.str();
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
            const IO::Path mapPath = savedMapPath();
            IO::Path crashLogPath;

            if (mapPath.isEmpty()) {
                const IO::Path docsDir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation).toStdString());
                crashLogPath = docsDir + IO::Path("trenchbroom-crash.txt");
            } else {
                const String crashFileName = mapPath.lastComponent().deleteExtension().asString() + "-crash.txt";
                crashLogPath = mapPath.deleteLastComponent() + IO::Path(crashFileName);
            }

            // ensure it doesn't exist
            int index = 0;
            IO::Path testCrashLogPath = crashLogPath;
            while (IO::Disk::fileExists(testCrashLogPath)) {
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
                std::abort();

            inReportCrashAndExit = true;

            // get the crash report as a string
            const String report = makeCrashReport(stacktrace, reason);

            // write it to the crash log file
            const IO::Path basePath = crashReportBasePath();

            // ensure the containing directory exists
            const IO::Path containerPath = basePath.deleteLastComponent();
            if (!IO::Disk::directoryExists(containerPath)) {
                IO::Disk::createDirectory(containerPath);
            }

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
            if (!QFile::copy(IO::pathAsQString(IO::SystemPaths::logFilePath()), QString::fromStdString(logPath.asString()))) {
                logPath = IO::Path();
            }

            // write the crash log to stderr
            std::cerr << "crash log:" << std::endl;
            std::cerr << report << std::endl;

            if (crashReportGuiEnabled) {
                CrashDialog dialog(reportPath, mapPath, logPath);
                dialog.exec();
            }

            std::abort();
        }

        bool isReportingCrash() {
            return inReportCrashAndExit;
        }

#if defined(_WIN32) && defined(_MSC_VER)
        LONG WINAPI TrenchBroomUnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionPtrs) {
            reportCrashAndExit(TrenchBroomStackWalker::getStackTraceFromContext(pExceptionPtrs->ContextRecord), "TrenchBroomUnhandledExceptionFilter");
            return EXCEPTION_EXECUTE_HANDLER;
        }
#else
        static void CrashHandler(int signum) {
            TrenchBroom::View::reportCrashAndExit(TrenchBroom::TrenchBroomStackWalker::getStackTrace(), "SIGSEGV");
        }
#endif

        bool TrenchBroomApp::newDocument() {
            MapFrame* frame = nullptr;
            try {
                String gameName;
                Model::MapFormat mapFormat = Model::MapFormat::Unknown;
                if (!GameDialog::showNewDocumentDialog(nullptr, gameName, mapFormat)) {
                    return false;
                }

                frame = m_frameManager->newFrame();

                Model::GameFactory &gameFactory = Model::GameFactory::instance();
                Model::GameSPtr game = gameFactory.createGame(gameName, frame->logger());
                ensure(game.get() != nullptr, "game is null");

                hideWelcomeWindow();
                frame->newDocument(game, mapFormat);
                return true;
            } catch (const RecoverableException& e) {
                if (frame != nullptr) {
                    frame->close();
                }
                return recoverFromException(e, [this](){ return this->newDocument(); });
            } catch (const Exception& e) {
                if (frame != nullptr) {
                    frame->close();
                }

                QMessageBox::critical(nullptr, "", e.what());
                return false;
            }
        }

        void TrenchBroomApp::openDocument() {
            const auto pathStr = QFileDialog::getOpenFileName(nullptr, "Open Map", "", "Map files (*.map);;Any files (*.*)");
            const auto path = IO::pathFromQString(pathStr);

            if (!path.isEmpty()) {
                openDocument(path);
            }
        }

        void TrenchBroomApp::showManual() {
            const IO::Path manualPath = IO::SystemPaths::findResourceFile(IO::Path("manual/index.html"));
            const String manualPathString = manualPath.asString();
            const QUrl manualPathUrl = QUrl::fromLocalFile(QString::fromStdString(manualPathString));
            QDesktopServices::openUrl(manualPathUrl);
        }

        void TrenchBroomApp::showPreferences() {
            openPreferences();
        }

        void TrenchBroomApp::showAboutDialog() {
            openAbout();
        }

        void TrenchBroomApp::debugShowCrashReportDialog() {
            const IO::Path reportPath(IO::SystemPaths::userDataDirectory() + IO::Path("crashreport.txt"));
            const IO::Path mapPath(IO::SystemPaths::userDataDirectory() + IO::Path("crashreport.map"));
            const IO::Path logPath(IO::SystemPaths::userDataDirectory() + IO::Path("crashreport.log"));

            CrashDialog dialog(reportPath, mapPath, logPath);
            dialog.exec();
        }

        /**
         * If we catch exceptions in main() that are otherwise uncaught, Qt prints a warning to override QCoreApplication::notify()
         * and catch exceptions there instead.
         */
        bool TrenchBroomApp::notify(QObject* receiver, QEvent* event) {
            try {
                return QApplication::notify(receiver, event);
            } catch (const std::exception& e) {
                // Unfortunately we can't portably get the stack trace of the exception itself
                TrenchBroom::View::reportCrashAndExit("<uncaught exception>", e.what());
                return false;
            }
        }

#ifdef __APPLE__
        bool TrenchBroomApp::event(QEvent* event) {
            if (event->type() == QEvent::FileOpen) {
                const auto* openEvent = static_cast<QFileOpenEvent*>(event);
                const auto pathStr = openEvent->file().toStdString();
                const auto path = IO::Path(pathStr);
                if (openDocument(path)) {
                    hideWelcomeWindow();
                    return true;
                } else {
                    return false;
                }
            } else if (event->type() == QEvent::ApplicationActivate) {
                if (m_frameManager->allFramesClosed()) {
                    showWelcomeWindow();
                }
            }
            return QApplication::event(event);
        }
#endif

        bool TrenchBroomApp::openFilesOrWelcomeFrame(const QStringList& fileNames) {
            if (!fileNames.isEmpty()) {
                if (useSDI()) {
                    const auto path = IO::pathFromQString(fileNames.at(0));
                    if (!path.isEmpty()) {
                        openDocument(path);
                    }
                } else {
                    for (const auto& fileName : fileNames) {
                        const auto path = IO::pathFromQString(fileName);
                        openDocument(path);
                    }
                }
            } else {
                showWelcomeWindow();
            }
            return true;
        }

        void TrenchBroomApp::showWelcomeWindow() {
            if (m_welcomeWindow == nullptr) {
                // must be initialized after m_recentDocuments!
                m_welcomeWindow = std::make_unique<WelcomeWindow>();
            }
            m_welcomeWindow->show();
        }

        void TrenchBroomApp::hideWelcomeWindow() {
            if (m_welcomeWindow != nullptr) {
                m_welcomeWindow->hide();
                if (quitOnLastWindowClosed() && m_frameManager->allFramesClosed()) {
                    closeWelcomeWindow();
                }
            }
        }

        void TrenchBroomApp::closeWelcomeWindow() {
            if (m_welcomeWindow != nullptr) {
                m_welcomeWindow->close();
                m_welcomeWindow = nullptr;
            }
        }

        bool TrenchBroomApp::useSDI() {
#ifdef _WIN32
            return true;
#else
            return false;
#endif
        }
    }
}
