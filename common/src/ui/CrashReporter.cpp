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

#include <QStandardPaths>

#include "TrenchBroomApp.h"
#include "fs/DiskIO.h"
#include "fs/PathInfo.h"
#include "gl/ContextManager.h"
#include "mdl/Map.h"
#include "ui/AppController.h"
#include "ui/CrashDialog.h"
#include "ui/FrameManager.h"
#include "ui/GetVersion.h"
#include "ui/MapDocument.h"
#include "ui/MapFrame.h"
#include "ui/MapViewBase.h"
#include "ui/PreferenceDialog.h"
#include "ui/QPathUtils.h"
#include "ui/SystemPaths.h"

#include "kd/path_utils.h"

#include <cpptrace/basic.hpp>
#include <cpptrace/from_current.hpp>
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

bool inReportCrashAndExit = false;
bool crashReportGuiEnabled = true;

const MapDocument* topDocument()
{
  auto& app = TrenchBroomApp::instance();
  if (const auto* topFrame = app.appController().frameManager().topFrame())
  {
    return &topFrame->document();
  }
  return nullptr;
}

std::string makeCrashReport(const auto& stacktrace, const auto& reason)
{
  auto ss = std::stringstream{};

  ss << "OS:\t" << QSysInfo::prettyProductName().toStdString() << std::endl;
  ss << "Qt:\t" << qVersion() << std::endl;

  ss << "GL_VENDOR:\t" << gl::ContextManager::GLVendor << std::endl;
  ss << "GL_RENDERER:\t" << gl::ContextManager::GLRenderer << std::endl;
  ss << "GL_VERSION:\t" << gl::ContextManager::GLVersion << std::endl;

  ss << "TrenchBroom Version:\t" << getBuildVersion().toStdString() << std::endl;
  ss << "TrenchBroom Build:\t" << getBuildIdStr().toStdString() << std::endl;

  ss << "Reason:\t" << reason << std::endl;

  stacktrace.print(ss);

  return ss.str();
}

// returns the empty path for unsaved maps, or if we can't determine the current map
std::filesystem::path savedMapPath()
{
  const auto* document = topDocument();
  return document ? document->map().path() : std::filesystem::path{};
}

std::filesystem::path crashReportBasePath()
{
  const auto mapPath = savedMapPath();
  const auto crashLogPath = !mapPath.empty()
                              ? mapPath.parent_path() / mapPath.stem() += "-crash.txt"
                              : pathFromQString(QStandardPaths::writableLocation(
                                  QStandardPaths::DocumentsLocation))
                                  / "trenchbroom-crash.txt";

  // ensure it doesn't exist
  auto index = 0;
  auto testCrashLogPath = crashLogPath;
  while (fs::Disk::pathInfo(testCrashLogPath) == fs::PathInfo::File)
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
  reportCrashAndExit(std::to_string(pExceptionPtrs->ExceptionRecord->ExceptionCode));
  // return EXCEPTION_EXECUTE_HANDLER; unreachable
}
#else
void CrashHandler(const int /* signum */)
{
  reportCrashAndExit("SIGSEGV");
}
#endif

[[noreturn]] void reportCrashAndExit(
  const cpptrace::stacktrace& stacktrace, const std::string& reason)
{
  // just abort if we reenter reportCrashAndExit (i.e. if it crashes)
  if (std::exchange(inReportCrashAndExit, true))
  {
    std::abort();
  }

  // get the crash report as a string
  const auto report = makeCrashReport(stacktrace, reason);

  // write it to the crash log file
  const auto basePath = crashReportBasePath();

  // ensure the containing directory exists
  fs::Disk::createDirectory(basePath.parent_path()) | kdl::transform([&](auto) {
    const auto reportPath = kdl::path_add_extension(basePath, ".txt");
    auto logPath = kdl::path_add_extension(basePath, ".log");
    auto mapPath = kdl::path_add_extension(basePath, ".map");

    fs::Disk::withOutputStream(reportPath, [&](auto& stream) {
      stream << report;
      std::cerr << "wrote crash log to " << reportPath.string() << std::endl;
    }) | kdl::transform_error([](const auto& e) {
      std::cerr << "could not write crash log: " << e.msg << std::endl;
    });

    // save the map
    if (const auto* document = topDocument())
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
    if (!std::filesystem::copy_file(SystemPaths::logFilePath(), logPath, ec) || ec)
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

} // namespace

void setCrashReportGUIEnabled(const bool guiEnabled)
{
  crashReportGuiEnabled = guiEnabled;
}

void reportCrashAndExit(const std::string& reason)
{
  reportCrashAndExit(cpptrace::generate_trace(), reason);
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

void runWithCrashReporting(const ThrowingFunction& func)
{
  CPPTRACE_TRY
  {
    func();
  }
  CPPTRACE_CATCH(const std::exception& e)
  {
    // Note that this will not catch all exceptions that are thrown from Qt event handlers
    // because Qt doesn't guarantee that exceptions can propagate through its signal/slot
    // mechanism. We will have to fix that by getting rid of exceptions altogether or by
    // wrapping every slot in a try/catch block.
    reportCrashAndExit(cpptrace::from_current_exception(), e.what());
  }
}

} // namespace tb::ui
