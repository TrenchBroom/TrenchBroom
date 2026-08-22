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

#pragma once

#include "base/NotifierConnection.h"

#include <QObject>
#include <QString>

#include <string>

class QTimer;

namespace tb::ui
{
class MapDocument;

/**
 * Runs a user-configured shell command whenever the map is saved and/or whenever it
 * changes, so that edits made in TrenchBroom can be pushed into an external tool (e.g. a
 * running game engine) without a manual step.
 *
 * Both triggers are controlled by preferences and default to off:
 *  - Preferences::MapHookRunOnSave: run the command after MapDocument::documentWasSavedNotifier
 *    fires.
 *  - Preferences::MapHookRunOnChange: run the command some time after
 *    MapDocument::documentDidChangeNotifier fires. This is debounced using
 *    Preferences::MapHookDebounceMs (a QTimer that is restarted on every subsequent
 *    change), so that a drag interaction that produces many change notifications only
 *    results in a single command invocation once things settle down.
 *
 * The command string (Preferences::MapHookCommand) supports the same ${VARIABLE}
 * substitutions as the compilation profiles, see CompilationVariables.h.
 *
 * This class must only be used from the main thread: MapDocument's notifiers fire
 * synchronously on the thread that triggered them, and MapDocument/mdl::Map/mdl::Node are
 * not safe to touch from any other thread. All of the work this class does in response to
 * a notifier - reading preferences, building variables from mdl::Map, formatting the
 * command - happens synchronously on the calling (main) thread; only the spawned process
 * itself runs independently.
 */
class MapHookRunner : public QObject
{
  Q_OBJECT
private:
  MapDocument& m_document;
  QTimer* m_debounceTimer;

  NotifierConnection m_notifierConnection;

public:
  explicit MapHookRunner(MapDocument& document, QObject* parent = nullptr);

  /**
   * Exposed for testing: returns true while a debounced "on change" run is pending.
   */
  bool debouncePending() const;

signals:
  /**
   * Emitted right before a (fully variable-substituted) command is handed off to
   * runCommand(). Mainly useful for tests, which can QSignalSpy::wait() on this instead
   * of sleeping past the debounce interval.
   */
  void commandExecuted(const QString& command);

private:
  void connectObservers();

  void documentWasSaved();
  void documentDidChange();
  void debounceTimeout();

  void execute();

protected:
  /**
   * Runs the given (already variable-substituted) shell command. Virtual so that tests
   * can override it instead of spawning a real process.
   */
  virtual void runCommand(const std::string& command);
};

} // namespace tb::ui
