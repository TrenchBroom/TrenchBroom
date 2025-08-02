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

#include "Macros.h"
#include "mdl/UndoableCommand.h"

#include <memory>
#include <string>
#include <tuple>
#include <vector>

namespace tb::mdl
{
enum class LockState;
class Node;

class SetLockStateCommand : public UndoableCommand
{
private:
  std::vector<Node*> m_nodes;
  LockState m_lockState;
  std::vector<std::tuple<Node*, LockState>> m_oldLockState;

public:
  static std::unique_ptr<SetLockStateCommand> lock(std::vector<Node*> nodes);
  static std::unique_ptr<SetLockStateCommand> unlock(std::vector<Node*> nodes);
  static std::unique_ptr<SetLockStateCommand> reset(std::vector<Node*> nodes);

  SetLockStateCommand(std::vector<Node*> nodes, LockState lockState);

private:
  static std::string makeName(LockState lockState);

  std::unique_ptr<CommandResult> doPerformDo(Map& map) override;
  std::unique_ptr<CommandResult> doPerformUndo(Map& map) override;

  deleteCopyAndMove(SetLockStateCommand);
};

} // namespace tb::mdl
