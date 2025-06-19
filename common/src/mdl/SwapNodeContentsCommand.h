/*
 Copyright (C) 2020 Kristian Duske

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
#include "mdl/NodeContents.h"
#include "mdl/UpdateLinkedGroupsCommandBase.h"

#include <memory>
#include <string>
#include <vector>

namespace tb::mdl
{
class Node;
} // namespace tb::mdl

namespace tb::ui
{

class SwapNodeContentsCommand : public UpdateLinkedGroupsCommandBase
{
protected:
  std::vector<std::pair<mdl::Node*, mdl::NodeContents>> m_nodes;

public:
  SwapNodeContentsCommand(
    std::string name, std::vector<std::pair<mdl::Node*, mdl::NodeContents>> nodes);
  ~SwapNodeContentsCommand() override;

  std::unique_ptr<CommandResult> doPerformDo(MapDocument& document) override;
  std::unique_ptr<CommandResult> doPerformUndo(MapDocument& document) override;

  bool doCollateWith(UndoableCommand& command) override;

  deleteCopyAndMove(SwapNodeContentsCommand);
};

} // namespace tb::ui
