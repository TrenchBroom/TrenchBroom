/*
 Copyright (C) 2025 Kristian Duske

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

#include "Exceptions.h"
#include "Logger.h"
#include "mdl/CommandProcessor.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Groups.h"
#include "mdl/Node.h"
#include "mdl/RepeatStack.h"
#include "mdl/UndoableCommand.h"
#include "mdl/UpdateLinkedGroupsCommand.h"
#include "mdl/WorldNode.h"

#include <memory>

namespace tb::mdl
{
namespace
{

std::vector<GroupNode*> collectGroupsWithPendingChanges(Node& node)
{
  auto result = std::vector<GroupNode*>{};

  node.accept(kdl::overload(
    [](auto&& thisLambda, const WorldNode* worldNode) {
      worldNode->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, const LayerNode* layerNode) {
      layerNode->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, GroupNode* groupNode) {
      if (groupNode->hasPendingChanges())
      {
        result.push_back(groupNode);
      }
      groupNode->visitChildren(thisLambda);
    },
    [](const EntityNode*) {},
    [](const BrushNode*) {},
    [](const PatchNode*) {}));

  return result;
}

bool updateLinkedGroups(Map& map)
{
  if (map.isCurrentDocumentStateObservable())
  {
    if (const auto allChangedLinkedGroups = collectGroupsWithPendingChanges(*map.world());
        !allChangedLinkedGroups.empty())
    {
      setHasPendingChanges(allChangedLinkedGroups, false);

      auto command = std::make_unique<UpdateLinkedGroupsCommand>(allChangedLinkedGroups);
      const auto result = map.executeAndStore(std::move(command));
      return result->success();
    }
  }

  return true;
}

class ThrowExceptionCommand : public UndoableCommand
{
public:
  ThrowExceptionCommand()
    : UndoableCommand{"Throw Exception", false}
  {
  }

private:
  std::unique_ptr<CommandResult> doPerformDo(Map&) override
  {
    throw CommandProcessorException{};
  }

  std::unique_ptr<CommandResult> doPerformUndo(Map&) override
  {
    return std::make_unique<CommandResult>(true);
  }
};

} // namespace

bool Map::canUndoCommand() const
{
  return m_commandProcessor->canUndo();
}

bool Map::canRedoCommand() const
{
  return m_commandProcessor->canRedo();
}

const std::string& Map::undoCommandName() const
{
  return m_commandProcessor->undoCommandName();
}

const std::string& Map::redoCommandName() const
{
  return m_commandProcessor->redoCommandName();
}

void Map::undoCommand()
{
  m_commandProcessor->undo();
  updateLinkedGroups(*this);

  // Undo/redo in the repeat system is not supported for now, so just clear the repeat
  // stack
  m_repeatStack->clear();
}

void Map::redoCommand()
{
  m_commandProcessor->redo();
  updateLinkedGroups(*this);

  // Undo/redo in the repeat system is not supported for now, so just clear the repeat
  // stack
  m_repeatStack->clear();
}

bool Map::isCommandCollationEnabled() const
{
  return m_commandProcessor->isCollationEnabled();
}

void Map::setIsCommandCollationEnabled(const bool isCommandCollationEnabled)
{
  m_commandProcessor->setIsCollationEnabled(isCommandCollationEnabled);
}

void Map::pushRepeatableCommand(RepeatableCommand command)
{
  m_repeatStack->push(std::move(command));
}

bool Map::canRepeatCommands() const
{
  return m_repeatStack->size() > 0u;
}

void Map::repeatCommands()
{
  m_repeatStack->repeat();
}

void Map::clearRepeatableCommands()
{
  m_repeatStack->clear();
}

void Map::startTransaction(std::string name, const TransactionScope scope)
{
  logger().debug() << "Starting transaction '" + name + "'";
  m_commandProcessor->startTransaction(std::move(name), scope);
  m_repeatStack->startTransaction();
}

void Map::rollbackTransaction()
{
  logger().debug() << "Rolling back transaction";
  m_commandProcessor->rollbackTransaction();
  m_repeatStack->rollbackTransaction();
}

bool Map::commitTransaction()
{
  logger().debug() << "Committing transaction";

  if (!updateLinkedGroups(*this))
  {
    cancelTransaction();
    return false;
  }

  m_commandProcessor->commitTransaction();
  m_repeatStack->commitTransaction();
  return true;
}

void Map::cancelTransaction()
{
  m_logger.debug() << "Cancelling transaction";
  m_commandProcessor->rollbackTransaction();
  m_repeatStack->rollbackTransaction();
  m_commandProcessor->commitTransaction();
  m_repeatStack->commitTransaction();
}

bool Map::isCurrentDocumentStateObservable() const
{
  return m_commandProcessor->isCurrentDocumentStateObservable();
}

bool Map::throwExceptionDuringCommand()
{
  const auto result = executeAndStore(std::make_unique<ThrowExceptionCommand>());
  return result->success();
}

std::unique_ptr<CommandResult> Map::execute(std::unique_ptr<Command>&& command)
{
  return m_commandProcessor->execute(std::move(command));
}

std::unique_ptr<CommandResult> Map::executeAndStore(
  std::unique_ptr<UndoableCommand>&& command)
{
  return m_commandProcessor->executeAndStore(std::move(command));
}

} // namespace tb::mdl
