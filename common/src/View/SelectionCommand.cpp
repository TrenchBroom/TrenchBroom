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

#include "SelectionCommand.h"

#include "Ensure.h"
#include "Macros.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceHandle.h"
#include "Model/BrushFaceReference.h"
#include "Model/BrushNode.h"
#include "Model/EntityNode.h"
#include "Model/WorldNode.h"
#include "View/MapDocumentCommandFacade.h"

#include <kdl/string_format.h>
#include <kdl/vector_utils.h>

#include <sstream>
#include <string>

namespace TrenchBroom {
namespace View {
const Command::CommandType SelectionCommand::Type = Command::freeType();

std::unique_ptr<SelectionCommand> SelectionCommand::select(const std::vector<Model::Node*>& nodes) {
  return std::make_unique<SelectionCommand>(
    Action::SelectNodes, nodes, std::vector<Model::BrushFaceHandle>{});
}

std::unique_ptr<SelectionCommand> SelectionCommand::select(
  const std::vector<Model::BrushFaceHandle>& faces) {
  return std::make_unique<SelectionCommand>(
    Action::SelectFaces, std::vector<Model::Node*>{}, faces);
}

std::unique_ptr<SelectionCommand> SelectionCommand::convertToFaces() {
  return std::make_unique<SelectionCommand>(
    Action::ConvertToFaces, std::vector<Model::Node*>{}, std::vector<Model::BrushFaceHandle>{});
}

std::unique_ptr<SelectionCommand> SelectionCommand::selectAllNodes() {
  return std::make_unique<SelectionCommand>(
    Action::SelectAllNodes, std::vector<Model::Node*>{}, std::vector<Model::BrushFaceHandle>{});
}

std::unique_ptr<SelectionCommand> SelectionCommand::selectAllFaces() {
  return std::make_unique<SelectionCommand>(
    Action::SelectAllFaces, std::vector<Model::Node*>{}, std::vector<Model::BrushFaceHandle>{});
}

std::unique_ptr<SelectionCommand> SelectionCommand::deselect(
  const std::vector<Model::Node*>& nodes) {
  return std::make_unique<SelectionCommand>(
    Action::DeselectNodes, nodes, std::vector<Model::BrushFaceHandle>{});
}

std::unique_ptr<SelectionCommand> SelectionCommand::deselect(
  const std::vector<Model::BrushFaceHandle>& faces) {
  return std::make_unique<SelectionCommand>(
    Action::DeselectFaces, std::vector<Model::Node*>{}, faces);
}

std::unique_ptr<SelectionCommand> SelectionCommand::deselectAll() {
  return std::make_unique<SelectionCommand>(
    Action::DeselectAll, std::vector<Model::Node*>{}, std::vector<Model::BrushFaceHandle>{});
}

SelectionCommand::SelectionCommand(
  const Action action, const std::vector<Model::Node*>& nodes,
  const std::vector<Model::BrushFaceHandle>& faces)
  : UndoableCommand(Type, makeName(action, nodes.size(), faces.size()), false)
  , m_action(action)
  , m_nodes(nodes)
  , m_faceRefs(Model::createRefs(faces)) {}

SelectionCommand::~SelectionCommand() = default;

std::string SelectionCommand::makeName(
  const Action action, const size_t nodeCount, const size_t faceCount) {
  std::stringstream result;
  switch (action) {
    case Action::SelectNodes:
      result << "Select " << nodeCount << " " << kdl::str_plural(nodeCount, "Object", "Objects");
      break;
    case Action::SelectFaces:
      result << "Select " << faceCount << " "
             << kdl::str_plural(faceCount, "Brush Face", "Brush Faces");
      break;
    case Action::SelectAllNodes:
      result << "Select All Objects";
      break;
    case Action::SelectAllFaces:
      result << "Select All Brush Faces";
      break;
    case Action::ConvertToFaces:
      result << "Convert to Brush Face Selection";
      break;
    case Action::DeselectNodes:
      result << "Deselect " << nodeCount << " " << kdl::str_plural(nodeCount, "Object", "Objects");
      break;
    case Action::DeselectFaces:
      result << "Deselect " << faceCount << " "
             << kdl::str_plural(faceCount, "Brush Face", "Brush Faces");
      break;
    case Action::DeselectAll:
      return "Select None";
      switchDefault();
  }
  return result.str();
}

std::unique_ptr<CommandResult> SelectionCommand::doPerformDo(MapDocumentCommandFacade* document) {
  m_previouslySelectedNodes = document->selectedNodes().nodes();
  m_previouslySelectedFaceRefs = Model::createRefs(document->selectedBrushFaces());

  switch (m_action) {
    case Action::SelectNodes:
      document->performSelect(m_nodes);
      break;
    case Action::SelectFaces:
      document->performSelect(Model::resolveAllRefs(m_faceRefs));
      break;
    case Action::SelectAllNodes:
      document->performSelectAllNodes();
      break;
    case Action::SelectAllFaces:
      document->performSelectAllBrushFaces();
      break;
    case Action::ConvertToFaces:
      document->performConvertToBrushFaceSelection();
      break;
    case Action::DeselectNodes:
      document->performDeselect(m_nodes);
      break;
    case Action::DeselectFaces:
      document->performDeselect(Model::resolveAllRefs(m_faceRefs));
      break;
    case Action::DeselectAll:
      document->performDeselectAll();
      break;
  }
  return std::make_unique<CommandResult>(true);
}

std::unique_ptr<CommandResult> SelectionCommand::doPerformUndo(MapDocumentCommandFacade* document) {
  document->performDeselectAll();
  if (!m_previouslySelectedNodes.empty()) {
    document->performSelect(m_previouslySelectedNodes);
  }
  if (!m_previouslySelectedFaceRefs.empty()) {
    document->performSelect(Model::resolveAllRefs(m_previouslySelectedFaceRefs));
  }
  return std::make_unique<CommandResult>(true);
}

bool SelectionCommand::doCollateWith(UndoableCommand&) {
  return false;
}
} // namespace View
} // namespace TrenchBroom
