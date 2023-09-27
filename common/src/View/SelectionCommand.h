/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "FloatType.h"
#include "Macros.h"
#include "View/UndoableCommand.h"

#include <memory>
#include <string>
#include <tuple>
#include <vector>

namespace TrenchBroom::Model
{
class BrushFace;
class BrushFaceHandle;
class BrushFaceReference;
class BrushNode;
class Node;
} // namespace TrenchBroom::Model

namespace TrenchBroom::View
{

class SelectionCommand : public UndoableCommand
{
private:
  enum class Action
  {
    SelectNodes,
    SelectFaces,
    SelectAllNodes,
    SelectAllFaces,
    ConvertToFaces,
    DeselectNodes,
    DeselectFaces,
    DeselectAll
  };

  Action m_action;

  std::vector<Model::Node*> m_nodes;
  std::vector<Model::BrushFaceReference> m_faceRefs;

  std::vector<Model::Node*> m_previouslySelectedNodes;
  std::vector<Model::BrushFaceReference> m_previouslySelectedFaceRefs;

public:
  static std::unique_ptr<SelectionCommand> select(std::vector<Model::Node*> nodes);
  static std::unique_ptr<SelectionCommand> select(
    std::vector<Model::BrushFaceHandle> faces);

  static std::unique_ptr<SelectionCommand> convertToFaces();
  static std::unique_ptr<SelectionCommand> selectAllNodes();
  static std::unique_ptr<SelectionCommand> selectAllFaces();

  static std::unique_ptr<SelectionCommand> deselect(std::vector<Model::Node*> nodes);
  static std::unique_ptr<SelectionCommand> deselect(
    std::vector<Model::BrushFaceHandle> faces);
  static std::unique_ptr<SelectionCommand> deselectAll();

  SelectionCommand(
    Action action,
    std::vector<Model::Node*> nodes,
    std::vector<Model::BrushFaceHandle> faces);
  ~SelectionCommand() override;

private:
  static std::string makeName(Action action, size_t nodeCount, size_t faceCount);

  std::unique_ptr<CommandResult> doPerformDo(MapDocumentCommandFacade* document) override;
  std::unique_ptr<CommandResult> doPerformUndo(
    MapDocumentCommandFacade* document) override;

  deleteCopyAndMove(SelectionCommand);
};

} // namespace TrenchBroom::View
