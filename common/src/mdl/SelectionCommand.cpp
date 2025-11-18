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

#include "SelectionCommand.h"

#include "Ensure.h"
#include "Logger.h"
#include "Macros.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushFaceHandle.h"
#include "mdl/BrushFaceReference.h"
#include "mdl/EditorContext.h"
#include "mdl/LinkedGroupUtils.h"
#include "mdl/Map.h"
#include "mdl/Map_Groups.h"
#include "mdl/ModelUtils.h"
#include "mdl/WorldNode.h" // IWYU pragma: keep

#include "kd/ranges/to.h"
#include "kd/result.h"
#include "kd/string_format.h"

#include <sstream>
#include <string>

namespace tb::mdl
{
namespace
{

void doDeselectNodes(const std::vector<Node*>& nodes, Map& map)
{
  map.selectionWillChangeNotifier();

  auto deselected = std::vector<Node*>{};
  deselected.reserve(nodes.size());

  for (auto* node : nodes)
  {
    if (node->selected())
    {
      node->deselect();
      deselected.push_back(node);
    }
  }

  auto selectionChange = SelectionChange{};
  selectionChange.deselectedNodes = deselected;
  map.selectionDidChangeNotifier(selectionChange);
}

void doDeselectBrushFaces(const std::vector<BrushFaceHandle>& faces, Map& map)
{
  map.selectionWillChangeNotifier();

  const auto implicitlyLockedGroups = kdl::vec_sort(
    collectGroups({map.world()}) | std::views::filter([](const auto* groupNode) {
      return groupNode->lockedByOtherSelection();
    })
    | kdl::ranges::to<std::vector>());

  auto deselected = std::vector<BrushFaceHandle>{};
  deselected.reserve(faces.size());

  for (const auto& handle : faces)
  {
    const auto& face = handle.face();
    if (face.selected())
    {
      auto* node = handle.node();
      node->deselectFace(handle.faceIndex());
      deselected.push_back(handle);
    }
  }

  auto selectionChange = SelectionChange{};
  selectionChange.deselectedBrushFaces = deselected;
  map.selectionDidChangeNotifier(selectionChange);

  // Selection change is done. Next, update implicit locking of linked groups.
  // The strategy is to figure out what needs to be locked given selection().brushFaces,
  // and then un-implicitly-lock all other linked groups.
  const auto groupsToLock = kdl::vec_sort(
    faceSelectionWithLinkedGroupConstraints(*map.world(), map.selection().brushFaces)
      .groupsToLock);
  for (auto* node : groupsToLock)
  {
    node->setLockedByOtherSelection(true);
  }
  map.nodeLockingDidChangeNotifier(kdl::vec_static_cast<Node*>(groupsToLock));

  const auto groupsToUnlock = kdl::set_difference(implicitlyLockedGroups, groupsToLock);
  for (auto* node : groupsToUnlock)
  {
    node->setLockedByOtherSelection(false);
  }
  map.nodeLockingDidChangeNotifier(kdl::vec_static_cast<Node*>(groupsToUnlock));
}

void doDeselectAll(Map& map)
{
  if (map.selection().hasNodes())
  {
    doDeselectNodes(map.selection().nodes, map);
  }
  if (map.selection().hasBrushFaces())
  {
    doDeselectBrushFaces(map.selection().brushFaces, map);
  }
}

void doSelectNodes(const std::vector<Node*>& nodes, Map& map)
{
  map.selectionWillChangeNotifier();

  auto selected = std::vector<Node*>{};
  selected.reserve(nodes.size());

  const auto& worldNode = *map.world();
  for (auto* initialNode : nodes)
  {
    ensure(
      initialNode->isDescendantOf(&worldNode) || initialNode == &worldNode,
      "to select a node, it must be world or a descendant");
    const auto nodesToSelect = initialNode->nodesRequiredForViewSelection();
    for (auto* node : nodesToSelect)
    {
      if (!node->selected() /* && m_editorContext->selectable(node) remove check to allow issue objects to be selected */)
      {
        node->select();
        selected.push_back(node);
      }
    }
  }

  auto selectionChange = SelectionChange{};
  selectionChange.selectedNodes = selected;
  map.selectionDidChangeNotifier(selectionChange);
}

void doSelectBrushFaces(const std::vector<BrushFaceHandle>& faces, Map& map)
{
  map.selectionWillChangeNotifier();

  const auto constrained = faceSelectionWithLinkedGroupConstraints(*map.world(), faces);

  for (auto* node : constrained.groupsToLock)
  {
    node->setLockedByOtherSelection(true);
  }
  map.nodeLockingDidChangeNotifier(kdl::vec_static_cast<Node*>(constrained.groupsToLock));

  auto selected = std::vector<BrushFaceHandle>{};
  selected.reserve(constrained.facesToSelect.size());

  auto& editorContext = map.editorContext();
  for (const auto& handle : constrained.facesToSelect)
  {
    auto* node = handle.node();
    const auto& face = handle.face();
    if (!face.selected() && editorContext.selectable(*node, face))
    {
      node->selectFace(handle.faceIndex());
      selected.push_back(handle);
    }
  }

  auto selectionChange = SelectionChange{};
  selectionChange.selectedBrushFaces = selected;
  map.selectionDidChangeNotifier(selectionChange);
}

void doSelectAllNodes(Map& map)
{
  doDeselectAll(map);

  auto* target = currentGroupOrWorld(map);
  const auto nodesToSelect =
    collectSelectableNodes(target->children(), map.editorContext());

  doSelectNodes(nodesToSelect, map);
}

void doSelectAllBrushFaces(Map& map)
{
  doDeselectAll(map);

  auto* target = currentGroupOrWorld(map);
  auto facesToSelect =
    collectSelectableBrushFaces(std::vector<Node*>{target}, map.editorContext());

  doSelectBrushFaces(facesToSelect, map);
}

void doConvertToBrushFaceSelection(Map& map)
{
  const auto facesToSelect =
    collectSelectableBrushFaces(map.selection().nodes, map.editorContext());

  doDeselectAll(map);
  doSelectBrushFaces(facesToSelect, map);
}

} // namespace

enum class SelectionCommand::Action
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

std::unique_ptr<SelectionCommand> SelectionCommand::select(std::vector<Node*> nodes)
{
  return std::make_unique<SelectionCommand>(
    Action::SelectNodes, std::move(nodes), std::vector<BrushFaceHandle>{});
}

std::unique_ptr<SelectionCommand> SelectionCommand::select(
  std::vector<BrushFaceHandle> faces)
{
  return std::make_unique<SelectionCommand>(
    Action::SelectFaces, std::vector<Node*>{}, std::move(faces));
}

std::unique_ptr<SelectionCommand> SelectionCommand::convertToFaces()
{
  return std::make_unique<SelectionCommand>(
    Action::ConvertToFaces, std::vector<Node*>{}, std::vector<BrushFaceHandle>{});
}

std::unique_ptr<SelectionCommand> SelectionCommand::selectAllNodes()
{
  return std::make_unique<SelectionCommand>(
    Action::SelectAllNodes, std::vector<Node*>{}, std::vector<BrushFaceHandle>{});
}

std::unique_ptr<SelectionCommand> SelectionCommand::selectAllFaces()
{
  return std::make_unique<SelectionCommand>(
    Action::SelectAllFaces, std::vector<Node*>{}, std::vector<BrushFaceHandle>{});
}

std::unique_ptr<SelectionCommand> SelectionCommand::deselect(std::vector<Node*> nodes)
{
  return std::make_unique<SelectionCommand>(
    Action::DeselectNodes, std::move(nodes), std::vector<BrushFaceHandle>{});
}

std::unique_ptr<SelectionCommand> SelectionCommand::deselect(
  std::vector<BrushFaceHandle> faces)
{
  return std::make_unique<SelectionCommand>(
    Action::DeselectFaces, std::vector<Node*>{}, std::move(faces));
}

std::unique_ptr<SelectionCommand> SelectionCommand::deselectAll()
{
  return std::make_unique<SelectionCommand>(
    Action::DeselectAll, std::vector<Node*>{}, std::vector<BrushFaceHandle>{});
}

SelectionCommand::SelectionCommand(
  const Action action, std::vector<Node*> nodes, std::vector<BrushFaceHandle> faces)
  : UndoableCommand{makeName(action, nodes.size(), faces.size()), false}
  , m_action{action}
  , m_nodes{std::move(nodes)}
  , m_faceRefs{createRefs(faces)}
{
}

SelectionCommand::~SelectionCommand() = default;

std::string SelectionCommand::makeName(
  const Action action, const size_t nodeCount, const size_t faceCount)
{
  std::stringstream result;
  switch (action)
  {
  case Action::SelectNodes:
    result << "Select " << nodeCount << " "
           << kdl::str_plural(nodeCount, "Object", "Objects");
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
    result << "Deselect " << nodeCount << " "
           << kdl::str_plural(nodeCount, "Object", "Objects");
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

std::unique_ptr<CommandResult> SelectionCommand::doPerformDo(Map& map)
{
  m_previouslySelectedNodes = map.selection().nodes;
  m_previouslySelectedFaceRefs = createRefs(map.selection().brushFaces);

  return std::make_unique<CommandResult>(
    doSelect(map)
    | kdl::transform_error([&](const auto& e) { map.logger().error() << e.msg; })
    | kdl::is_success());
}

std::unique_ptr<CommandResult> SelectionCommand::doPerformUndo(Map& map)
{
  doDeselectAll(map);

  if (!m_previouslySelectedNodes.empty())
  {
    doSelectNodes(m_previouslySelectedNodes, map);
  }

  if (!m_previouslySelectedFaceRefs.empty())
  {
    return std::make_unique<CommandResult>(
      resolveAllRefs(m_previouslySelectedFaceRefs)
      | kdl::transform(
        [&](const auto& faceHandles) { return doSelectBrushFaces(faceHandles, map); })
      | kdl::transform_error([&](const auto& e) { map.logger().error() << e.msg; })
      | kdl::is_success());
  }

  return std::make_unique<CommandResult>(true);
}

Result<void> SelectionCommand::doSelect(Map& map) const
{
  switch (m_action)
  {
  case Action::SelectNodes:
    doSelectNodes(m_nodes, map);
    return Result<void>{};
  case Action::SelectFaces:
    return resolveAllRefs(m_faceRefs) | kdl::transform([&](const auto& faceHandles) {
             return doSelectBrushFaces(faceHandles, map);
           });
  case Action::SelectAllNodes:
    doSelectAllNodes(map);
    return Result<void>{};
  case Action::SelectAllFaces:
    doSelectAllBrushFaces(map);
    return Result<void>{};
  case Action::ConvertToFaces:
    doConvertToBrushFaceSelection(map);
    return Result<void>{};
  case Action::DeselectNodes:
    doDeselectNodes(m_nodes, map);
    return Result<void>{};
  case Action::DeselectFaces:
    return resolveAllRefs(m_faceRefs) | kdl::transform([&](const auto& faceHandles) {
             return doDeselectBrushFaces(faceHandles, map);
           });
  case Action::DeselectAll:
    doDeselectAll(map);
    return Result<void>{};
    switchDefault();
  }
}

} // namespace tb::mdl
