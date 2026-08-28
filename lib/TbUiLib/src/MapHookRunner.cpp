/*
 Copyright (C) 2026

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

#include "ui/MapHookRunner.h"

#include <QProcess>
#include <QString>
#include <QTimer>
#include <QtSystemDetection>

#include "base/Error.h"
#include "base/Logger.h"
#include "base/PreferenceManager.h"
#include "el/Interpolate.h"
#include "mdl/Map.h"
#include "prefs/Preferences.h"
#include "ui/CompilationVariables.h"
#include "ui/MapDocument.h"

#include "kd/overload.h"

#include <algorithm>

namespace tb::ui
{
namespace
{

/**
 * Reuses the same variable set as compilation profiles (MAP_FULL_NAME, MAP_DIR_PATH,
 * MAP_BASE_NAME, GAME_DIR_PATH, MODS, APP_DIR_PATH and any per-game compilation tool
 * paths), see CompilationVariables.h.
 */
class MapHookVariables : public CommonCompilationVariables
{
public:
  explicit MapHookVariables(const mdl::Map& map)
    : CommonCompilationVariables{map}
  {
  }
};

} // namespace

MapHookRunner::MapHookRunner(MapDocument& document, QObject* parent)
  : QObject{parent}
  , m_document{document}
  , m_debounceTimer{new QTimer{this}}
{
  m_debounceTimer->setSingleShot(true);
  connect(m_debounceTimer, &QTimer::timeout, this, &MapHookRunner::debounceTimeout);

  connectObservers();
}

bool MapHookRunner::debouncePending() const
{
  return m_debounceTimer->isActive();
}

void MapHookRunner::connectObservers()
{
  m_notifierConnection +=
    m_document.documentWasSavedNotifier.connect(this, &MapHookRunner::documentWasSaved);
  m_notifierConnection +=
    m_document.documentDidChangeNotifier.connect(this, &MapHookRunner::documentDidChange);
}

void MapHookRunner::documentWasSaved()
{
  if (pref(Preferences::MapHookRunOnSave))
  {
    execute();
  }
}

void MapHookRunner::documentDidChange()
{
  if (!pref(Preferences::MapHookRunOnChange))
  {
    return;
  }

  // Restarting an already-running single-shot QTimer resets its countdown, which is
  // exactly the debounce behavior we need: rapid-fire changes (e.g. every step of a
  // vertex drag) keep pushing the deadline out instead of each queuing their own timeout.
  const auto intervalMs = std::max(0, pref(Preferences::MapHookDebounceMs));
  m_debounceTimer->setInterval(intervalMs);
  m_debounceTimer->start();
}

void MapHookRunner::debounceTimeout()
{
  // Re-check in case the preference was disabled while the timer was pending.
  if (pref(Preferences::MapHookRunOnChange))
  {
    execute();
  }
}

void MapHookRunner::execute()
{
  const auto& map = m_document.map();
  const auto commandTemplate = pref(Preferences::MapHookCommand);
  if (commandTemplate.empty())
  {
    return;
  }

  const auto variables = MapHookVariables{map};
  el::interpolate(variables, commandTemplate)
    .visit(kdl::overload(
      [&](const std::string& command) {
        if (!command.empty())
        {
          emit commandExecuted(QString::fromStdString(command));
          runCommand(command);
        }
      },
      [&](const Error& error) {
        m_document.logger().error()
          << "Failed to interpolate map hook command: " << error.msg;
      }));
}

void MapHookRunner::runCommand(const std::string& command)
{
  auto* process = new QProcess{this};
  connect(
    process,
    qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
    process,
    &QObject::deleteLater);
  connect(
    process,
    &QProcess::errorOccurred,
    this,
    [this, process](QProcess::ProcessError) {
      if (process->state() == QProcess::NotRunning)
      {
        m_document.logger().error() << "Map hook command failed to start: "
                                     << process->errorString().toStdString();
        process->deleteLater();
      }
    });

#if defined(Q_OS_WIN)
  process->setProgram("cmd.exe");
  process->setArguments({"/c", QString::fromStdString(command)});
#else
  process->setProgram("/bin/sh");
  process->setArguments({"-c", QString::fromStdString(command)});
#endif

  process->start();
}

} // namespace tb::ui
