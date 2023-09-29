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

#include "MapDocumentCommandFacade.h"

#include "Assets/EntityDefinitionFileSpec.h"
#include "Assets/TextureManager.h"
#include "Error.h"
#include "Exceptions.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushNode.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "Model/EditorContext.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/Game.h"
#include "Model/GroupNode.h"
#include "Model/Issue.h"
#include "Model/LayerNode.h"
#include "Model/ModelUtils.h"
#include "Model/PatchNode.h"
#include "Model/WorldNode.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "View/CommandProcessor.h"
#include "View/Selection.h"
#include "View/UndoableCommand.h"

#include <kdl/map_utils.h>
#include <kdl/overload.h>
#include <kdl/result.h>
#include <kdl/string_format.h>
#include <kdl/string_utils.h>
#include <kdl/vector_set.h>
#include <kdl/vector_utils.h>

#include <vecmath/polygon.h>
#include <vecmath/segment.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom
{
namespace View
{
std::shared_ptr<MapDocument> MapDocumentCommandFacade::newMapDocument()
{
  // can't use std::make_shared here because the constructor is private
  return std::shared_ptr<MapDocument>(new MapDocumentCommandFacade());
}

MapDocumentCommandFacade::MapDocumentCommandFacade()
  : m_commandProcessor(std::make_unique<CommandProcessor>(this))
{
  connectObservers();
}

MapDocumentCommandFacade::~MapDocumentCommandFacade() = default;

void MapDocumentCommandFacade::performSelect(const std::vector<Model::Node*>& nodes)
{
  selectionWillChangeNotifier();
  updateLastSelectionBounds();

  std::vector<Model::Node*> selected;
  selected.reserve(nodes.size());

  for (Model::Node* initialNode : nodes)
  {
    ensure(
      initialNode->isDescendantOf(m_world.get()) || initialNode == m_world.get(),
      "to select a node, it must be world or a descendant");
    const auto nodesToSelect = initialNode->nodesRequiredForViewSelection();
    for (Model::Node* node : nodesToSelect)
    {
      if (!node->selected() /* && m_editorContext->selectable(node) remove check to allow issue objects to be selected */)
      {
        node->select();
        selected.push_back(node);
      }
    }
  }

  m_selectedNodes.addNodes(selected);

  Selection selection;
  selection.addSelectedNodes(selected);

  selectionDidChangeNotifier(selection);
  invalidateSelectionBounds();
}

void MapDocumentCommandFacade::performSelect(
  const std::vector<Model::BrushFaceHandle>& faces)
{
  selectionWillChangeNotifier();

  const auto constrained =
    Model::faceSelectionWithLinkedGroupConstraints(*m_world.get(), faces);

  for (Model::GroupNode* node : constrained.groupsToLock)
  {
    node->setLockedByOtherSelection(true);
  }
  nodeLockingDidChangeNotifier(
    kdl::vec_element_cast<Model::Node*>(constrained.groupsToLock));

  std::vector<Model::BrushFaceHandle> selected;
  selected.reserve(constrained.facesToSelect.size());

  for (const auto& handle : constrained.facesToSelect)
  {
    Model::BrushNode* node = handle.node();
    const Model::BrushFace& face = handle.face();
    if (!face.selected() && m_editorContext->selectable(node, face))
    {
      node->selectFace(handle.faceIndex());
      selected.push_back(handle);
    }
  }

  m_selectedBrushFaces = kdl::vec_concat(std::move(m_selectedBrushFaces), selected);

  Selection selection;
  selection.addSelectedBrushFaces(selected);

  selectionDidChangeNotifier(selection);
}

void MapDocumentCommandFacade::performSelectAllNodes()
{
  performDeselectAll();

  auto* target = currentGroupOrWorld();
  const auto nodesToSelect =
    Model::collectSelectableNodes(target->children(), *m_editorContext);
  performSelect(nodesToSelect);
}

void MapDocumentCommandFacade::performSelectAllBrushFaces()
{
  performDeselectAll();
  performSelect(Model::collectSelectableBrushFaces(
    std::vector<Model::Node*>{m_world.get()}, *m_editorContext));
}

void MapDocumentCommandFacade::performConvertToBrushFaceSelection()
{
  performDeselectAll();
  performSelect(
    Model::collectSelectableBrushFaces(m_selectedNodes.nodes(), *m_editorContext));
}

void MapDocumentCommandFacade::performDeselect(const std::vector<Model::Node*>& nodes)
{
  selectionWillChangeNotifier();
  updateLastSelectionBounds();

  std::vector<Model::Node*> deselected;
  deselected.reserve(nodes.size());

  for (Model::Node* node : nodes)
  {
    if (node->selected())
    {
      node->deselect();
      deselected.push_back(node);
    }
  }

  m_selectedNodes.removeNodes(deselected);

  Selection selection;
  selection.addDeselectedNodes(deselected);

  selectionDidChangeNotifier(selection);
  invalidateSelectionBounds();
}

void MapDocumentCommandFacade::performDeselect(
  const std::vector<Model::BrushFaceHandle>& faces)
{
  const auto implicitlyLockedGroups = kdl::vector_set<Model::GroupNode*>{kdl::vec_filter(
    Model::findAllLinkedGroups({m_world.get()}),
    [](const auto* group) { return group->lockedByOtherSelection(); })};

  selectionWillChangeNotifier();

  std::vector<Model::BrushFaceHandle> deselected;
  deselected.reserve(faces.size());

  for (const auto& handle : faces)
  {
    const Model::BrushFace& face = handle.face();
    if (face.selected())
    {
      Model::BrushNode* node = handle.node();
      node->deselectFace(handle.faceIndex());
      deselected.push_back(handle);
    }
  }

  m_selectedBrushFaces = kdl::vec_erase_all(std::move(m_selectedBrushFaces), deselected);

  Selection selection;
  selection.addDeselectedBrushFaces(deselected);

  selectionDidChangeNotifier(selection);

  // Selection change is done. Next, update implicit locking of linked groups.
  // The strategy is to figure out what needs to be locked given m_selectedBrushFaces, and
  // then un-implicitly-lock all other linked groups.
  const auto groupsToLock = kdl::vector_set<Model::GroupNode*>{
    Model::faceSelectionWithLinkedGroupConstraints(*m_world.get(), m_selectedBrushFaces)
      .groupsToLock};
  for (Model::GroupNode* node : groupsToLock)
  {
    node->setLockedByOtherSelection(true);
  }
  nodeLockingDidChangeNotifier(
    kdl::vec_element_cast<Model::Node*>(groupsToLock.get_data()));

  const auto groupsToUnlock = kdl::set_difference(implicitlyLockedGroups, groupsToLock);
  for (Model::GroupNode* node : groupsToUnlock)
  {
    node->setLockedByOtherSelection(false);
  }
  nodeLockingDidChangeNotifier(kdl::vec_element_cast<Model::Node*>(groupsToUnlock));
}

void MapDocumentCommandFacade::performDeselectAll()
{
  if (hasSelectedNodes())
  {
    const auto previousSelection = m_selectedNodes.nodes();
    performDeselect(previousSelection);
  }
  if (hasSelectedBrushFaces())
  {
    const auto previousSelection = m_selectedBrushFaces;
    performDeselect(previousSelection);
  }
}

void MapDocumentCommandFacade::performAddNodes(
  const std::map<Model::Node*, std::vector<Model::Node*>>& nodes)
{
  const std::vector<Model::Node*> parents = collectParents(nodes);
  NotifyBeforeAndAfter notifyParents(
    nodesWillChangeNotifier, nodesDidChangeNotifier, parents);

  std::vector<Model::Node*> addedNodes;
  for (const auto& [parent, children] : nodes)
  {
    parent->addChildren(children);
    addedNodes = kdl::vec_concat(std::move(addedNodes), children);
  }

  setEntityDefinitions(addedNodes);
  setEntityModels(addedNodes);
  setTextures(addedNodes);
  invalidateSelectionBounds();

  nodesWereAddedNotifier(addedNodes);
}

void MapDocumentCommandFacade::performRemoveNodes(
  const std::map<Model::Node*, std::vector<Model::Node*>>& nodes)
{
  const std::vector<Model::Node*> parents = collectParents(nodes);
  NotifyBeforeAndAfter notifyParents(
    nodesWillChangeNotifier, nodesDidChangeNotifier, parents);

  const std::vector<Model::Node*> allChildren = collectChildren(nodes);
  NotifyBeforeAndAfter notifyChildren(
    nodesWillBeRemovedNotifier, nodesWereRemovedNotifier, allChildren);

  for (const auto& [parent, children] : nodes)
  {
    unsetEntityModels(children);
    unsetEntityDefinitions(children);
    unsetTextures(children);
    parent->removeChildren(std::begin(children), std::end(children));
  }

  invalidateSelectionBounds();
}

static std::vector<Model::Node*> collectOldChildren(
  const std::vector<std::pair<Model::Node*, std::vector<std::unique_ptr<Model::Node>>>>&
    nodes)
{
  std::vector<Model::Node*> result;
  for (auto& [parent, newChildren] : nodes)
  {
    result = kdl::vec_concat(std::move(result), parent->children());
  }
  return result;
}

std::vector<std::pair<Model::Node*, std::vector<std::unique_ptr<Model::Node>>>>
MapDocumentCommandFacade::performReplaceChildren(
  std::vector<std::pair<Model::Node*, std::vector<std::unique_ptr<Model::Node>>>> nodes)
{
  if (nodes.empty())
  {
    return {};
  }

  const std::vector<Model::Node*> parents = collectParents(nodes);
  NotifyBeforeAndAfter notifyParents(
    nodesWillChangeNotifier, nodesDidChangeNotifier, parents);

  const std::vector<Model::Node*> allOldChildren = collectOldChildren(nodes);
  NotifyBeforeAndAfter notifyChildren(
    nodesWillBeRemovedNotifier, nodesWereRemovedNotifier, allOldChildren);

  auto result =
    std::vector<std::pair<Model::Node*, std::vector<std::unique_ptr<Model::Node>>>>{};
  auto allNewChildren = std::vector<Model::Node*>{};

  for (auto& [parent, newChildren] : nodes)
  {
    allNewChildren = kdl::vec_concat(
      std::move(allNewChildren),
      kdl::vec_transform(newChildren, [](auto& child) { return child.get(); }));

    auto oldChildren = parent->replaceChildren(std::move(newChildren));

    result.emplace_back(parent, std::move(oldChildren));
  }

  unsetEntityModels(allOldChildren);
  unsetEntityDefinitions(allOldChildren);
  unsetTextures(allOldChildren);

  setEntityDefinitions(allNewChildren);
  setEntityModels(allNewChildren);
  setTextures(allNewChildren);

  invalidateSelectionBounds();

  nodesWereAddedNotifier(allNewChildren);

  return result;
}

static auto notifySpecialWorldProperties(
  const Model::Game& game,
  const std::vector<std::pair<Model::Node*, Model::NodeContents>>& nodesToSwap)
{
  for (const auto& [node, contents] : nodesToSwap)
  {
    if (const auto* worldNode = dynamic_cast<const Model::WorldNode*>(node))
    {
      const auto& oldEntity = worldNode->entity();
      const auto& newEntity = std::get<Model::Entity>(contents.get());

      const auto* oldWads = oldEntity.property(Model::EntityPropertyKeys::Wad);
      const auto* newWads = newEntity.property(Model::EntityPropertyKeys::Wad);

      const bool notifyWadsChange =
        (oldWads == nullptr) != (newWads == nullptr)
        || (oldWads != nullptr && newWads != nullptr && *oldWads != *newWads);

      const auto oldEntityDefinitionSpec = game.extractEntityDefinitionFile(oldEntity);
      const auto newEntityDefinitionSpec = game.extractEntityDefinitionFile(newEntity);
      const bool notifyEntityDefinitionsChange =
        oldEntityDefinitionSpec != newEntityDefinitionSpec;

      const auto oldMods = game.extractEnabledMods(oldEntity);
      const auto newMods = game.extractEnabledMods(newEntity);
      const bool notifyModsChange = oldMods != newMods;

      return std::tuple{
        notifyWadsChange, notifyEntityDefinitionsChange, notifyModsChange};
    }
  }

  return std::tuple{false, false, false};
}

void MapDocumentCommandFacade::performSwapNodeContents(
  std::vector<std::pair<Model::Node*, Model::NodeContents>>& nodesToSwap)
{
  const auto nodes =
    kdl::vec_transform(nodesToSwap, [](const auto& pair) { return pair.first; });
  const auto parents = collectParents(nodes);
  const auto descendants = collectDescendants(nodes);

  NotifyBeforeAndAfter notifyNodes(
    nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
  NotifyBeforeAndAfter notifyParents(
    nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
  NotifyBeforeAndAfter notifyDescendants(
    nodesWillChangeNotifier, nodesDidChangeNotifier, descendants);

  const auto [notifyWadsChange, notifyEntityDefinitionsChange, notifyModsChange] =
    notifySpecialWorldProperties(*game(), nodesToSwap);
  NotifyBeforeAndAfter notifyWads(
    notifyWadsChange,
    textureCollectionsWillChangeNotifier,
    textureCollectionsDidChangeNotifier);
  NotifyBeforeAndAfter notifyEntityDefinitions(
    notifyEntityDefinitionsChange,
    entityDefinitionsWillChangeNotifier,
    entityDefinitionsDidChangeNotifier);
  NotifyBeforeAndAfter notifyMods(
    notifyModsChange, modsWillChangeNotifier, modsDidChangeNotifier);

  for (auto& pair : nodesToSwap)
  {
    auto* node = pair.first;
    auto& contents = pair.second.get();

    pair.second = node->accept(kdl::overload(
      [&](Model::WorldNode* worldNode) -> Model::NodeContents {
        return Model::NodeContents(
          worldNode->setEntity(std::get<Model::Entity>(std::move(contents))));
      },
      [&](Model::LayerNode* layerNode) -> Model::NodeContents {
        return Model::NodeContents(
          layerNode->setLayer(std::get<Model::Layer>(std::move(contents))));
      },
      [&](Model::GroupNode* groupNode) -> Model::NodeContents {
        return Model::NodeContents(
          groupNode->setGroup(std::get<Model::Group>(std::move(contents))));
      },
      [&](Model::EntityNode* entityNode) -> Model::NodeContents {
        return Model::NodeContents(
          entityNode->setEntity(std::get<Model::Entity>(std::move(contents))));
      },
      [&](Model::BrushNode* brushNode) -> Model::NodeContents {
        return Model::NodeContents(
          brushNode->setBrush(std::get<Model::Brush>(std::move(contents))));
      },
      [&](Model::PatchNode* patchNode) -> Model::NodeContents {
        return Model::NodeContents(
          patchNode->setPatch(std::get<Model::BezierPatch>(std::move(contents))));
      }));
  }

  if (!notifyEntityDefinitionsChange && !notifyModsChange)
  {
    setEntityDefinitions(nodes);
    setEntityModels(nodes);
  }
  if (!notifyWadsChange)
  {
    setTextures(nodes);
  }

  invalidateSelectionBounds();
}

std::map<Model::Node*, Model::VisibilityState> MapDocumentCommandFacade::
  setVisibilityState(
    const std::vector<Model::Node*>& nodes, const Model::VisibilityState visibilityState)
{
  std::map<Model::Node*, Model::VisibilityState> result;

  std::vector<Model::Node*> changedNodes;
  changedNodes.reserve(nodes.size());

  for (Model::Node* node : nodes)
  {
    const Model::VisibilityState oldState = node->visibilityState();
    if (node->setVisibilityState(visibilityState))
    {
      changedNodes.push_back(node);
      result[node] = oldState;
    }
  }

  nodeVisibilityDidChangeNotifier(changedNodes);
  return result;
}

std::map<Model::Node*, Model::VisibilityState> MapDocumentCommandFacade::
  setVisibilityEnsured(const std::vector<Model::Node*>& nodes)
{
  std::map<Model::Node*, Model::VisibilityState> result;

  std::vector<Model::Node*> changedNodes;
  changedNodes.reserve(nodes.size());

  for (Model::Node* node : nodes)
  {
    const Model::VisibilityState oldState = node->visibilityState();
    if (node->ensureVisible())
    {
      changedNodes.push_back(node);
      result[node] = oldState;
    }
  }

  nodeVisibilityDidChangeNotifier(changedNodes);
  return result;
}

void MapDocumentCommandFacade::restoreVisibilityState(
  const std::map<Model::Node*, Model::VisibilityState>& nodes)
{
  std::vector<Model::Node*> changedNodes;
  changedNodes.reserve(nodes.size());

  for (const auto& [node, state] : nodes)
  {
    if (node->setVisibilityState(state))
      changedNodes.push_back(node);
  }

  nodeVisibilityDidChangeNotifier(changedNodes);
}

std::map<Model::Node*, Model::LockState> MapDocumentCommandFacade::setLockState(
  const std::vector<Model::Node*>& nodes, const Model::LockState lockState)
{
  std::map<Model::Node*, Model::LockState> result;

  std::vector<Model::Node*> changedNodes;
  changedNodes.reserve(nodes.size());

  for (Model::Node* node : nodes)
  {
    const Model::LockState oldState = node->lockState();
    if (node->setLockState(lockState))
    {
      changedNodes.push_back(node);
      result[node] = oldState;
    }
  }

  nodeLockingDidChangeNotifier(changedNodes);
  return result;
}

void MapDocumentCommandFacade::restoreLockState(
  const std::map<Model::Node*, Model::LockState>& nodes)
{
  std::vector<Model::Node*> changedNodes;
  changedNodes.reserve(nodes.size());

  for (const auto& [node, state] : nodes)
  {
    if (node->setLockState(state))
      changedNodes.push_back(node);
  }

  nodeLockingDidChangeNotifier(changedNodes);
}

void MapDocumentCommandFacade::performPushGroup(Model::GroupNode* group)
{
  m_editorContext->pushGroup(group);
  groupWasOpenedNotifier(group);
}

void MapDocumentCommandFacade::performPopGroup()
{
  Model::GroupNode* previousGroup = m_editorContext->currentGroup();
  m_editorContext->popGroup();
  groupWasClosedNotifier(previousGroup);
}

void MapDocumentCommandFacade::doSetIssueHidden(
  const Model::Issue& issue, const bool hidden)
{
  if (issue.hidden() != hidden)
  {
    issue.node().setIssueHidden(issue.type(), hidden);
    incModificationCount();
  }
}

void MapDocumentCommandFacade::incModificationCount(const size_t delta)
{
  m_modificationCount += delta;
  documentModificationStateDidChangeNotifier();
}

void MapDocumentCommandFacade::decModificationCount(const size_t delta)
{
  assert(m_modificationCount >= delta);
  m_modificationCount -= delta;
  documentModificationStateDidChangeNotifier();
}

void MapDocumentCommandFacade::connectObservers()
{
  m_notifierConnection +=
    m_commandProcessor->commandDoNotifier.connect(commandDoNotifier);
  m_notifierConnection +=
    m_commandProcessor->commandDoneNotifier.connect(commandDoneNotifier);
  m_notifierConnection +=
    m_commandProcessor->commandDoFailedNotifier.connect(commandDoFailedNotifier);
  m_notifierConnection +=
    m_commandProcessor->commandUndoNotifier.connect(commandUndoNotifier);
  m_notifierConnection +=
    m_commandProcessor->commandUndoneNotifier.connect(commandUndoneNotifier);
  m_notifierConnection +=
    m_commandProcessor->commandUndoFailedNotifier.connect(commandUndoFailedNotifier);
  m_notifierConnection +=
    m_commandProcessor->transactionDoneNotifier.connect(transactionDoneNotifier);
  m_notifierConnection +=
    m_commandProcessor->transactionUndoneNotifier.connect(transactionUndoneNotifier);
}

bool MapDocumentCommandFacade::isCurrentDocumentStateObservable() const
{
  return m_commandProcessor->isCurrentDocumentStateObservable();
}

bool MapDocumentCommandFacade::doCanUndoCommand() const
{
  return m_commandProcessor->canUndo();
}

bool MapDocumentCommandFacade::doCanRedoCommand() const
{
  return m_commandProcessor->canRedo();
}

const std::string& MapDocumentCommandFacade::doGetUndoCommandName() const
{
  return m_commandProcessor->undoCommandName();
}

const std::string& MapDocumentCommandFacade::doGetRedoCommandName() const
{
  return m_commandProcessor->redoCommandName();
}

void MapDocumentCommandFacade::doUndoCommand()
{
  m_commandProcessor->undo();
}

void MapDocumentCommandFacade::doRedoCommand()
{
  m_commandProcessor->redo();
}

void MapDocumentCommandFacade::doClearCommandProcessor()
{
  m_commandProcessor->clear();
}

void MapDocumentCommandFacade::doStartTransaction(
  std::string name, const TransactionScope scope)
{
  m_commandProcessor->startTransaction(std::move(name), scope);
}

void MapDocumentCommandFacade::doCommitTransaction()
{
  m_commandProcessor->commitTransaction();
}

void MapDocumentCommandFacade::doRollbackTransaction()
{
  m_commandProcessor->rollbackTransaction();
}

std::unique_ptr<CommandResult> MapDocumentCommandFacade::doExecute(
  std::unique_ptr<Command> command)
{
  return m_commandProcessor->execute(std::move(command));
}

std::unique_ptr<CommandResult> MapDocumentCommandFacade::doExecuteAndStore(
  std::unique_ptr<UndoableCommand> command)
{
  return m_commandProcessor->executeAndStore(std::move(command));
}
} // namespace View
} // namespace TrenchBroom
