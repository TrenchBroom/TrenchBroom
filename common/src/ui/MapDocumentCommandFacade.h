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
#include "mdl/NodeContents.h"
#include "ui/MapDocument.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

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

public: // adding and removing nodes
  void performAddNodes(const std::map<mdl::Node*, std::vector<mdl::Node*>>& nodes);
  void performRemoveNodes(const std::map<mdl::Node*, std::vector<mdl::Node*>>& nodes);

  std::vector<std::pair<mdl::Node*, std::vector<std::unique_ptr<mdl::Node>>>>
  performReplaceChildren(
    std::vector<std::pair<mdl::Node*, std::vector<std::unique_ptr<mdl::Node>>>> nodes);

public: // swapping node contents
  void performSwapNodeContents(
    std::vector<std::pair<mdl::Node*, mdl::NodeContents>>& nodesToSwap);

public: // Node Visibility
  std::map<mdl::Node*, mdl::VisibilityState> setVisibilityState(
    const std::vector<mdl::Node*>& nodes, mdl::VisibilityState visibilityState);
  std::map<mdl::Node*, mdl::VisibilityState> setVisibilityEnsured(
    const std::vector<mdl::Node*>& nodes);
  void restoreVisibilityState(const std::map<mdl::Node*, mdl::VisibilityState>& nodes);
  std::map<mdl::Node*, mdl::LockState> setLockState(
    const std::vector<mdl::Node*>& nodes, mdl::LockState lockState);
  void restoreLockState(const std::map<mdl::Node*, mdl::LockState>& nodes);

public: // layers
  using MapDocument::performSetCurrentLayer;

public:
  void performPushGroup(mdl::GroupNode* group);
  void performPopGroup();

private:
  void doSetIssueHidden(const mdl::Issue& issue, bool hidden) override;

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
