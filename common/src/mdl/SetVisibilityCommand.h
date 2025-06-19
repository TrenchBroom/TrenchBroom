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
class Node;
enum class VisibilityState;

class SetVisibilityCommand : public UndoableCommand
{
private:
  enum class Action;

  std::vector<mdl::Node*> m_nodes;
  Action m_action;
  std::vector<std::tuple<mdl::Node*, mdl::VisibilityState>> m_oldState;

public:
  static std::unique_ptr<SetVisibilityCommand> show(std::vector<mdl::Node*> nodes);
  static std::unique_ptr<SetVisibilityCommand> hide(std::vector<mdl::Node*> nodes);
  static std::unique_ptr<SetVisibilityCommand> ensureVisible(
    std::vector<mdl::Node*> nodes);
  static std::unique_ptr<SetVisibilityCommand> reset(std::vector<mdl::Node*> nodes);

  SetVisibilityCommand(std::vector<mdl::Node*> nodes, Action action);

private:
  static std::string makeName(Action action);

  std::unique_ptr<CommandResult> doPerformDo(ui::MapDocument& document) override;
  std::unique_ptr<CommandResult> doPerformUndo(ui::MapDocument& document) override;

  deleteCopyAndMove(SetVisibilityCommand);
};

} // namespace tb::mdl
