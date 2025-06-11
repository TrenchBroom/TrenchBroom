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

#pragma once

#include "NotifierConnection.h"
#include "ui/MapDocument.h"

#include <memory>
#include <string>

namespace kdl
{
class task_manager;
}

namespace tb::mdl
{
enum class LockState;
enum class VisibilityState;
} // namespace tb::mdl

namespace tb::ui
{
class CommandProcessor;

/**
 * MapDocument API that is private to Command classes.
 *
 * These `performSomething()` methods will actually do an action, where
 * the corresponding `something()` in MapDocument would create and execute a
 * Command object which then calls `performSomething()`.
 */
class MapDocumentCommandFacade : public MapDocument
{
private:
  std::unique_ptr<CommandProcessor> m_commandProcessor;

  NotifierConnection m_notifierConnection;

public:
  static std::shared_ptr<MapDocument> newMapDocument(kdl::task_manager& taskManager);

  explicit MapDocumentCommandFacade(kdl::task_manager& taskManager);

  ~MapDocumentCommandFacade() override;

public: // layers
  using MapDocument::performSetCurrentLayer;

public: // modification count
  void incModificationCount(size_t delta = 1);
  void decModificationCount(size_t delta = 1);

private: // notification
  void connectObservers();
  void documentWasNewed(MapDocument* document);
  void documentWasLoaded(MapDocument* document);

private: // implement MapDocument interface
  bool isCurrentDocumentStateObservable() const override;

  bool doCanUndoCommand() const override;
  bool doCanRedoCommand() const override;
  const std::string& doGetUndoCommandName() const override;
  const std::string& doGetRedoCommandName() const override;
  void doUndoCommand() override;
  void doRedoCommand() override;

  void doClearCommandProcessor() override;
  void doStartTransaction(std::string name, TransactionScope scope) override;
  void doCommitTransaction() override;
  void doRollbackTransaction() override;

  std::unique_ptr<CommandResult> doExecute(std::unique_ptr<Command> command) override;
  std::unique_ptr<CommandResult> doExecuteAndStore(
    std::unique_ptr<UndoableCommand> command) override;
};

} // namespace tb::ui
