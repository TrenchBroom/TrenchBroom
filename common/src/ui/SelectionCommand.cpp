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
#include "Macros.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushFaceHandle.h"
#include "mdl/BrushFaceReference.h"
#include "mdl/EditorContext.h"
#include "mdl/LinkedGroupUtils.h"
#include "mdl/ModelUtils.h"
#include "mdl/WorldNode.h"
#include "ui/MapDocumentCommandFacade.h"

#include "kdl/range_to_vector.h"
#include "kdl/result.h"
#include "kdl/string_format.h"

#include <sstream>
#include <string>

namespace tb::ui
{
namespace
{
void doDeselectNodes(const std::vector<mdl::Node*>& nodes, MapDocument& document)
{
  document.selectionWillChangeNotifier();

  auto deselected = std::vector<mdl::Node*>{};
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
  document.selectionDidChangeNotifier(selectionChange);
}

void doDeselectBrushFaces(
  const std::vector<mdl::BrushFaceHandle>& faces, MapDocument& document)
{
  document.selectionWillChangeNotifier();

  const auto implicitlyLockedGroups = kdl::vec_sort(
    mdl::collectGroups({document.world()})
    | std::views::filter(
      [](const auto* groupNode) { return groupNode->lockedByOtherSelection(); })
    | kdl::to_vector);

  auto deselected = std::vector<mdl::BrushFaceHandle>{};
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
  document.selectionDidChangeNotifier(selectionChange);

  // Selection change is done. Next, update implicit locking of linked groups.
  // The strategy is to figure out what needs to be locked given selection().brushFaces,
  // and then un-implicitly-lock all other linked groups.
  const auto groupsToLock =
    kdl::vec_sort(mdl::faceSelectionWithLinkedGroupConstraints(
                    *document.world(), document.selection().brushFaces)
                    .groupsToLock);
  for (auto* node : groupsToLock)
  {
    node->setLockedByOtherSelection(true);
  }
  document.nodeLockingDidChangeNotifier(kdl::vec_static_cast<mdl::Node*>(groupsToLock));

  const auto groupsToUnlock = kdl::set_difference(implicitlyLockedGroups, groupsToLock);
  for (auto* node : groupsToUnlock)
  {
    node->setLockedByOtherSelection(false);
  }
  document.nodeLockingDidChangeNotifier(kdl::vec_static_cast<mdl::Node*>(groupsToUnlock));
}

void doDeselectAll(MapDocument& document)
{
  if (document.selection().hasNodes())
  {
    doDeselectNodes(document.selection().nodes, document);
  }
  if (document.selection().hasBrushFaces())
  {
    doDeselectBrushFaces(document.selection().brushFaces, document);
  }
}

void doSelectNodes(const std::vector<mdl::Node*>& nodes, MapDocument& document)
{
  document.selectionWillChangeNotifier();

  auto selected = std::vector<mdl::Node*>{};
  selected.reserve(nodes.size());

  const auto& worldNode = *document.world();
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
  document.selectionDidChangeNotifier(selectionChange);
}

void doSelectBrushFaces(
  const std::vector<mdl::BrushFaceHandle>& faces, MapDocument& document)
{
  document.selectionWillChangeNotifier();

  const auto constrained =
    mdl::faceSelectionWithLinkedGroupConstraints(*document.world(), faces);

  for (auto* node : constrained.groupsToLock)
  {
    node->setLockedByOtherSelection(true);
  }
  document.nodeLockingDidChangeNotifier(
    kdl::vec_static_cast<mdl::Node*>(constrained.groupsToLock));

  auto selected = std::vector<mdl::BrushFaceHandle>{};
  selected.reserve(constrained.facesToSelect.size());

  auto& editorContext = document.editorContext();
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
  document.selectionDidChangeNotifier(selectionChange);
}

void doSelectAllNodes(MapDocument& document)
{
  doDeselectAll(document);

  auto* target = document.currentGroupOrWorld();
  const auto nodesToSelect =
    mdl::collectSelectableNodes(target->children(), document.editorContext());

  doSelectNodes(nodesToSelect, document);
}

void doSelectAllBrushFaces(MapDocument& document)
{
  doDeselectAll(document);

  auto* target = document.currentGroupOrWorld();
  auto facesToSelect = mdl::collectSelectableBrushFaces(
    std::vector<mdl::Node*>{target}, document.editorContext());

  doSelectBrushFaces(facesToSelect, document);
}

void doConvertToBrushFaceSelection(MapDocument& document)
{
  const auto facesToSelect = mdl::collectSelectableBrushFaces(
    document.selection().nodes, document.editorContext());

  doDeselectAll(document);
  doSelectBrushFaces(facesToSelect, document);
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

std::unique_ptr<SelectionCommand> SelectionCommand::select(std::vector<mdl::Node*> nodes)
{
  return std::make_unique<SelectionCommand>(
    Action::SelectNodes, std::move(nodes), std::vector<mdl::BrushFaceHandle>{});
}

std::unique_ptr<SelectionCommand> SelectionCommand::select(
  std::vector<mdl::BrushFaceHandle> faces)
{
  return std::make_unique<SelectionCommand>(
    Action::SelectFaces, std::vector<mdl::Node*>{}, std::move(faces));
}

std::unique_ptr<SelectionCommand> SelectionCommand::convertToFaces()
{
  return std::make_unique<SelectionCommand>(
    Action::ConvertToFaces,
    std::vector<mdl::Node*>{},
    std::vector<mdl::BrushFaceHandle>{});
}

std::unique_ptr<SelectionCommand> SelectionCommand::selectAllNodes()
{
  return std::make_unique<SelectionCommand>(
    Action::SelectAllNodes,
    std::vector<mdl::Node*>{},
    std::vector<mdl::BrushFaceHandle>{});
}

std::unique_ptr<SelectionCommand> SelectionCommand::selectAllFaces()
{
  return std::make_unique<SelectionCommand>(
    Action::SelectAllFaces,
    std::vector<mdl::Node*>{},
    std::vector<mdl::BrushFaceHandle>{});
}

std::unique_ptr<SelectionCommand> SelectionCommand::deselect(
  std::vector<mdl::Node*> nodes)
{
  return std::make_unique<SelectionCommand>(
    Action::DeselectNodes, std::move(nodes), std::vector<mdl::BrushFaceHandle>{});
}

std::unique_ptr<SelectionCommand> SelectionCommand::deselect(
  std::vector<mdl::BrushFaceHandle> faces)
{
  return std::make_unique<SelectionCommand>(
    Action::DeselectFaces, std::vector<mdl::Node*>{}, std::move(faces));
}

std::unique_ptr<SelectionCommand> SelectionCommand::deselectAll()
{
  return std::make_unique<SelectionCommand>(
    Action::DeselectAll, std::vector<mdl::Node*>{}, std::vector<mdl::BrushFaceHandle>{});
}

SelectionCommand::SelectionCommand(
  const Action action,
  std::vector<mdl::Node*> nodes,
  std::vector<mdl::BrushFaceHandle> faces)
  : UndoableCommand{makeName(action, nodes.size(), faces.size()), false}
  , m_action{action}
  , m_nodes{std::move(nodes)}
  , m_faceRefs{mdl::createRefs(faces)}
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

std::unique_ptr<CommandResult> SelectionCommand::doPerformDo(
  MapDocumentCommandFacade& document)
{
  m_previouslySelectedNodes = document.selection().nodes;
  m_previouslySelectedFaceRefs = mdl::createRefs(document.selection().brushFaces);

  return std::make_unique<CommandResult>(
    doSelect(document)
    | kdl::transform_error([&](const auto& e) { document.error() << e.msg; })
    | kdl::is_success());
}

std::unique_ptr<CommandResult> SelectionCommand::doPerformUndo(
  MapDocumentCommandFacade& document)
{
  doDeselectAll(document);

  if (!m_previouslySelectedNodes.empty())
  {
    doSelectNodes(m_previouslySelectedNodes, document);
  }

  if (!m_previouslySelectedFaceRefs.empty())
  {
    return std::make_unique<CommandResult>(
      mdl::resolveAllRefs(m_previouslySelectedFaceRefs)
      | kdl::transform([&](const auto& faceHandles) {
          return doSelectBrushFaces(faceHandles, document);
        })
      | kdl::transform_error([&](const auto& e) { document.error() << e.msg; })
      | kdl::is_success());
  }

  return std::make_unique<CommandResult>(true);
}

Result<void> SelectionCommand::doSelect(MapDocument& document) const
{
  switch (m_action)
  {
  case Action::SelectNodes:
    doSelectNodes(m_nodes, document);
    return Result<void>{};
  case Action::SelectFaces:
    return mdl::resolveAllRefs(m_faceRefs) | kdl::transform([&](const auto& faceHandles) {
             return doSelectBrushFaces(faceHandles, document);
           });
  case Action::SelectAllNodes:
    doSelectAllNodes(document);
    return Result<void>{};
  case Action::SelectAllFaces:
    doSelectAllBrushFaces(document);
    return Result<void>{};
  case Action::ConvertToFaces:
    doConvertToBrushFaceSelection(document);
    return Result<void>{};
  case Action::DeselectNodes:
    doDeselectNodes(m_nodes, document);
    return Result<void>{};
  case Action::DeselectFaces:
    return mdl::resolveAllRefs(m_faceRefs) | kdl::transform([&](const auto& faceHandles) {
             return doDeselectBrushFaces(faceHandles, document);
           });
  case Action::DeselectAll:
    doDeselectAll(document);
    return Result<void>{};
    switchDefault();
  }
}

} // namespace tb::ui
