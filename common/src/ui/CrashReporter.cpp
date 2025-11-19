/*
 Copyright (C) 2010 Kristian Duske

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

#include "CrashReporter.h"

#include "TrenchBroomApp.h"
#include "TrenchBroomStackWalker.h"
#include "io/DiskIO.h"
#include "io/PathInfo.h"
#include "io/PathQt.h"
#include "io/SystemPaths.h"
#include "mdl/Map.h"
#include "ui/Actions.h"
#include "ui/CrashDialog.h"
#include "ui/FrameManager.h"
#include "ui/GLContextManager.h"
#include "ui/GetVersion.h"
#include "ui/MapDocument.h"
#include "ui/MapFrame.h"
#include "ui/MapViewBase.h"
#include "ui/PreferenceDialog.h"
#include "ui/QtUtils.h"

#include "kd/path_utils.h"

#include <fmt/format.h>
#include <fmt/std.h>

#include <csignal>
#include <cstdlib>
#include <iostream>
#include <sstream>

#if defined(_WIN32) && defined(_MSC_VER)
#include <windows.h>
#endif

namespace tb::ui
{
namespace
{
std::string makeCrashReport(const std::string& stacktrace, const std::string& reason)
{
  auto ss = std::stringstream{};
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
std::filesystem::path savedMapPath()
{
  const auto document = TrenchBroomApp::instance().topDocument();
  const auto& map = document->map();
  return document && map.path().is_absolute() ? map.path() : std::filesystem::path{};
}

std::filesystem::path crashReportBasePath()
{
  const auto mapPath = savedMapPath();
  const auto crashLogPath = !mapPath.empty()
                              ? mapPath.parent_path() / mapPath.stem() += "-crash.txt"
                              : io::pathFromQString(QStandardPaths::writableLocation(
                                  QStandardPaths::DocumentsLocation))
                                  / "trenchbroom-crash.txt";

  // ensure it doesn't exist
  auto index = 0;
  auto testCrashLogPath = crashLogPath;
  while (io::Disk::pathInfo(testCrashLogPath) == io::PathInfo::File)
  {
    ++index;

    const auto testCrashLogName = fmt::format("{}-{}.txt", crashLogPath.stem(), index);
    testCrashLogPath = crashLogPath.parent_path() / testCrashLogName;
  }

  return kdl::path_remove_extension(testCrashLogPath);
}

#if defined(_WIN32) && defined(_MSC_VER)
LONG WINAPI TrenchBroomUnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionPtrs)
{
  reportCrashAndExit(
    TrenchBroomStackWalker::getStackTraceFromContext(pExceptionPtrs->ContextRecord),
    std::to_string(pExceptionPtrs->ExceptionRecord->ExceptionCode));
  // return EXCEPTION_EXECUTE_HANDLER; unreachable
}
#else
void CrashHandler(const int /* signum */)
{
  reportCrashAndExit(TrenchBroomStackWalker::getStackTrace(), "SIGSEGV");
}
#endif

bool inReportCrashAndExit = false;
bool crashReportGuiEnabled = true;

} // namespace

void setCrashReportGUIEnabled(const bool guiEnabled)
{
  crashReportGuiEnabled = guiEnabled;
}

void reportCrashAndExit(const std::string& stacktrace, const std::string& reason)
{
  // just abort if we reenter reportCrashAndExit (i.e. if it crashes)
  if (inReportCrashAndExit)
  {
    std::abort();
  }

  inReportCrashAndExit = true;

  // get the crash report as a string
  const auto report = makeCrashReport(stacktrace, reason);

  // write it to the crash log file
  const auto basePath = crashReportBasePath();

  // ensure the containing directory exists
  io::Disk::createDirectory(basePath.parent_path()) | kdl::transform([&](auto) {
    const auto reportPath = kdl::path_add_extension(basePath, ".txt");
    auto logPath = kdl::path_add_extension(basePath, ".log");
    auto mapPath = kdl::path_add_extension(basePath, ".map");

    io::Disk::withOutputStream(reportPath, [&](auto& stream) {
      stream << report;
      std::cerr << "wrote crash log to " << reportPath.string() << std::endl;
    }) | kdl::transform_error([](const auto& e) {
      std::cerr << "could not write crash log: " << e.msg << std::endl;
    });

    // save the map
    if (const auto document = TrenchBroomApp::instance().topDocument();
        document && document->map().game())
    {
      document->map().saveTo(mapPath) | kdl::transform([&]() {
        std::cerr << "wrote map to " << mapPath.string() << std::endl;
      }) | kdl::transform_error([](const auto& e) {
        std::cerr << "could not write map: " << e.msg << std::endl;
      });
    }
    else
    {
      mapPath = std::filesystem::path{};
    }

    // Copy the log file
    auto ec = std::error_code{};
    if (!std::filesystem::copy_file(io::SystemPaths::logFilePath(), logPath, ec) || ec)
    {
      logPath = std::filesystem::path{};
    }

    if (crashReportGuiEnabled)
    {
      auto dialog = CrashDialog{reason, reportPath, mapPath, logPath};
      dialog.exec();
    }
  }) | kdl::transform_error([](const auto& e) {
    std::cerr << "could not create crash folder: " << e.msg << std::endl;
  });

  // write the crash log to stderr
  std::cerr << "crash log:" << std::endl;
  std::cerr << report << std::endl;

  std::abort();
}

bool isReportingCrash()
{
  return inReportCrashAndExit;
}

void setupCrashReporter()
{
#if defined(_WIN32) && defined(_MSC_VER)
  // with MSVC, set our own handler for segfaults so we can access the context
  // pointer, to allow StackWalker to read the backtrace.
  // see also: http://crashrpt.sourceforge.net/docs/html/exception_handling.html
  SetUnhandledExceptionFilter(TrenchBroomUnhandledExceptionFilter);
#else
  signal(SIGSEGV, CrashHandler);
#endif
}

} // namespace tb::ui
