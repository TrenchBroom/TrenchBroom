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
#include "Result.h"
#include "mdl/SelectionChange.h"
#include "mdl/UndoableCommand.h"

#include <memory>
#include <string>
#include <vector>

namespace tb::mdl
{
class BrushFace;
class BrushFaceHandle;
class BrushFaceReference;
class BrushNode;
class Node;

class SelectionCommand : public UndoableCommand
{
private:
  enum class Action;

  Action m_action;

  std::vector<Node*> m_nodes;
  std::vector<BrushFaceReference> m_faceRefs;

  std::vector<Node*> m_previouslySelectedNodes;
  std::vector<BrushFaceReference> m_previouslySelectedFaceRefs;

public:
  static std::unique_ptr<SelectionCommand> select(std::vector<Node*> nodes);
  static std::unique_ptr<SelectionCommand> select(std::vector<BrushFaceHandle> faces);

  static std::unique_ptr<SelectionCommand> convertToFaces();
  static std::unique_ptr<SelectionCommand> selectAllNodes();
  static std::unique_ptr<SelectionCommand> selectAllFaces();

  static std::unique_ptr<SelectionCommand> deselect(std::vector<Node*> nodes);
  static std::unique_ptr<SelectionCommand> deselect(std::vector<BrushFaceHandle> faces);
  static std::unique_ptr<SelectionCommand> deselectAll();

  SelectionCommand(
    Action action, std::vector<Node*> nodes, std::vector<BrushFaceHandle> faces);
  ~SelectionCommand() override;

private:
  static std::string makeName(Action action, size_t nodeCount, size_t faceCount);

  bool doPerformDo(Map& map) override;
  bool doPerformUndo(Map& map) override;

  Result<void> doSelect(Map& map) const;

  deleteCopyAndMove(SelectionCommand);
};

} // namespace tb::mdl
