/*
 Copyright (C) 2024 Kristian Duske

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

class SetLinkIdsCommand : public UndoableCommand
{
protected:
  std::vector<std::tuple<Node*, std::string>> m_linkIds;

public:
  SetLinkIdsCommand(
    const std::string& name, std::vector<std::tuple<Node*, std::string>> linkIds);
  ~SetLinkIdsCommand() override;

  std::unique_ptr<CommandResult> doPerformDo(Map& map) override;
  std::unique_ptr<CommandResult> doPerformUndo(Map& map) override;

  bool doCollateWith(UndoableCommand& command) override;

  deleteCopyAndMove(SetLinkIdsCommand);
};

} // namespace tb::mdl
