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

#include "Exceptions.h"
#include "Preferences.h"
#include "PreferenceManager.h"
#include "Assets/EntityDefinitionFileSpec.h"
#include "Assets/TextureManager.h"
#include "Model/Brush.h"
#include "Model/BrushError.h"
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
#include "Model/WorldNode.h"
#include "View/CommandProcessor.h"
#include "View/UndoableCommand.h"
#include "View/Selection.h"

#include <kdl/map_utils.h>
#include <kdl/overload.h>
#include <kdl/result.h>
#include <kdl/string_format.h>
#include <kdl/string_utils.h>
#include <kdl/vector_set.h>
#include <kdl/vector_utils.h>

#include <vecmath/segment.h>
#include <vecmath/polygon.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace View {
        std::shared_ptr<MapDocument> MapDocumentCommandFacade::newMapDocument() {
            // can't use std::make_shared here because the constructor is private
            return std::shared_ptr<MapDocument>(new MapDocumentCommandFacade());
        }

        MapDocumentCommandFacade::MapDocumentCommandFacade() :
        m_commandProcessor(std::make_unique<CommandProcessor>(this)) {
            bindObservers();
        }

        MapDocumentCommandFacade::~MapDocumentCommandFacade() = default;

        void MapDocumentCommandFacade::performSelect(const std::vector<Model::Node*>& nodes) {
            selectionWillChangeNotifier();
            updateLastSelectionBounds();

            std::vector<Model::Node*> selected;
            selected.reserve(nodes.size());

            for (Model::Node* initialNode : nodes) {
                ensure(initialNode->isDescendantOf(m_world.get()) || initialNode == m_world.get(), "to select a node, it must be world or a descendant");
                const auto nodesToSelect = initialNode->nodesRequiredForViewSelection();
                for (Model::Node* node : nodesToSelect) {
                    if (!node->selected() /* && m_editorContext->selectable(node) remove check to allow issue objects to be selected */) {
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

        void MapDocumentCommandFacade::performSelect(const std::vector<Model::BrushFaceHandle>& faces) {
            selectionWillChangeNotifier();

            std::vector<Model::BrushFaceHandle> selected;
            selected.reserve(faces.size());

            for (const auto& handle : faces) {
                Model::BrushNode* node = handle.node();
                const Model::BrushFace& face = handle.face();
                if (!face.selected() && m_editorContext->selectable(node, face)) {
                    node->selectFace(handle.faceIndex());
                    selected.push_back(handle);
                }
            }

            m_selectedBrushFaces = kdl::vec_concat(std::move(m_selectedBrushFaces), selected);

            Selection selection;
            selection.addSelectedBrushFaces(selected);

            selectionDidChangeNotifier(selection);
        }

        void MapDocumentCommandFacade::performSelectAllNodes() {
            performDeselectAll();

            auto* target = currentGroupOrWorld();
            const auto nodesToSelect = Model::collectSelectableNodes(target->children(), *m_editorContext);
            performSelect(nodesToSelect);
        }

        void MapDocumentCommandFacade::performSelectAllBrushFaces() {
            performDeselectAll();
            performSelect(Model::collectSelectableBrushFaces(std::vector<Model::Node*>{m_world.get()}, *m_editorContext));
        }

        void MapDocumentCommandFacade::performConvertToBrushFaceSelection() {
            performDeselectAll();
            performSelect(Model::collectSelectableBrushFaces(m_selectedNodes.nodes(), *m_editorContext));
        }

        void MapDocumentCommandFacade::performDeselect(const std::vector<Model::Node*>& nodes) {
            selectionWillChangeNotifier();
            updateLastSelectionBounds();

            std::vector<Model::Node*> deselected;
            deselected.reserve(nodes.size());

            for (Model::Node* node : nodes) {
                if (node->selected()) {
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

        void MapDocumentCommandFacade::performDeselect(const std::vector<Model::BrushFaceHandle>& faces) {
            selectionWillChangeNotifier();

            std::vector<Model::BrushFaceHandle> deselected;
            deselected.reserve(faces.size());

            for (const auto& handle : faces) {
                const Model::BrushFace& face = handle.face();
                if (face.selected()) {
                    Model::BrushNode* node = handle.node();
                    node->deselectFace(handle.faceIndex());
                    deselected.push_back(handle);
                }
            }

            m_selectedBrushFaces = kdl::vec_erase_all(std::move(m_selectedBrushFaces), deselected);

            Selection selection;
            selection.addDeselectedBrushFaces(deselected);

            selectionDidChangeNotifier(selection);
        }

        void MapDocumentCommandFacade::performDeselectAll() {
            if (hasSelectedNodes())
                deselectAllNodes();
            if (hasSelectedBrushFaces())
                deselectAllBrushFaces();
        }

        void MapDocumentCommandFacade::deselectAllNodes() {
            selectionWillChangeNotifier();
            updateLastSelectionBounds();

            for (Model::Node* node : m_selectedNodes) {
                node->deselect();
            }

            Selection selection;
            selection.addDeselectedNodes(m_selectedNodes.nodes());

            m_selectedNodes.clear();

            selectionDidChangeNotifier(selection);
            invalidateSelectionBounds();
        }

        void MapDocumentCommandFacade::deselectAllBrushFaces() {
            selectionWillChangeNotifier();

            for (const auto& handle : m_selectedBrushFaces) {
                Model::BrushNode* node = handle.node();
                node->deselectFace(handle.faceIndex());
            }

            Selection selection;
            selection.addDeselectedBrushFaces(m_selectedBrushFaces);

            m_selectedBrushFaces.clear();

            selectionDidChangeNotifier(selection);
        }

        void MapDocumentCommandFacade::performAddNodes(const std::map<Model::Node*, std::vector<Model::Node*>>& nodes) {
            const std::vector<Model::Node*> parents = collectParents(nodes);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);

            std::vector<Model::Node*> addedNodes;
            for (const auto& entry : nodes) {
                Model::Node* parent = entry.first;
                const std::vector<Model::Node*>& children = entry.second;
                parent->addChildren(children);
                addedNodes = kdl::vec_concat(std::move(addedNodes), children);
            }

            setEntityDefinitions(addedNodes);
            setEntityModels(addedNodes);
            setTextures(addedNodes);
            invalidateSelectionBounds();

            nodesWereAddedNotifier(addedNodes);
        }

        void MapDocumentCommandFacade::performRemoveNodes(const std::map<Model::Node*, std::vector<Model::Node*>>& nodes) {
            const std::vector<Model::Node*> parents = collectParents(nodes);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);

            const std::vector<Model::Node*> allChildren = collectChildren(nodes);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyChildren(nodesWillBeRemovedNotifier, nodesWereRemovedNotifier, allChildren);

            for (const auto& entry : nodes) {
                Model::Node* parent = entry.first;
                const std::vector<Model::Node*>& children = entry.second;
                unsetEntityModels(children);
                unsetEntityDefinitions(children);
                unsetTextures(children);
                parent->removeChildren(std::begin(children), std::end(children));
            }

            invalidateSelectionBounds();
        }

        static auto notifySpecialWorldAttributes(const Model::Game& game, const std::vector<std::pair<Model::Node*, Model::NodeContents>>& nodesToSwap) {
            for (const auto& [node, contents] : nodesToSwap) {
                if (const auto* worldNode = dynamic_cast<const Model::WorldNode*>(node)) {
                    const auto& oldEntity = worldNode->entity();
                    const auto& newEntity = std::get<Model::Entity>(contents.get());

                    const auto oldTextureCollections = game.extractTextureCollections(oldEntity);
                    const auto newTextureCollections = game.extractTextureCollections(newEntity);
                    const bool notifyTextureCollectionChange = oldTextureCollections != newTextureCollections;

                    const auto oldEntityDefinitionSpec = game.extractEntityDefinitionFile(oldEntity);
                    const auto newEntityDefinitionSpec = game.extractEntityDefinitionFile(newEntity);
                    const bool notifyEntityDefinitionsChange = oldEntityDefinitionSpec != newEntityDefinitionSpec;

                    const auto oldMods = game.extractEnabledMods(oldEntity);
                    const auto newMods = game.extractEnabledMods(newEntity);
                    const bool notifyModsChange = oldMods != newMods;
                    
                    return std::make_tuple(notifyTextureCollectionChange, notifyEntityDefinitionsChange, notifyModsChange);
                }
            }

            return std::make_tuple(false, false, false);
        }

        void MapDocumentCommandFacade::performSwapNodeContents(std::vector<std::pair<Model::Node*, Model::NodeContents>>& nodesToSwap) {
            const auto nodes = kdl::vec_transform(nodesToSwap, [](const auto& pair) { return pair.first; });
            const auto parents = collectParents(nodes);
            const auto descendants = collectDescendants(nodes);

            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyDescendants(nodesWillChangeNotifier, nodesDidChangeNotifier, descendants);

            const auto [notifyTextureCollectionChange, notifyEntityDefinitionsChange, notifyModsChange] = notifySpecialWorldAttributes(*game(), nodesToSwap);
            Notifier<>::NotifyBeforeAndAfter notifyTextureCollections(notifyTextureCollectionChange, textureCollectionsWillChangeNotifier, textureCollectionsDidChangeNotifier);
            Notifier<>::NotifyBeforeAndAfter notifyEntityDefinitions(notifyEntityDefinitionsChange, entityDefinitionsWillChangeNotifier, entityDefinitionsDidChangeNotifier);
            Notifier<>::NotifyBeforeAndAfter notifyMods(notifyModsChange, modsWillChangeNotifier, modsDidChangeNotifier);

            for (auto& pair : nodesToSwap) {
                auto* node = pair.first;
                auto& contents = pair.second.get();

                pair.second = node->accept(kdl::overload(
                    [&](Model::WorldNode* worldNode)   -> Model::NodeContents { return Model::NodeContents(worldNode->setEntity(std::get<Model::Entity>(std::move(contents)))); },
                    [&](Model::LayerNode* layerNode)   -> Model::NodeContents { return Model::NodeContents(layerNode->setEntity(std::get<Model::Entity>(std::move(contents)))); },
                    [&](Model::GroupNode* groupNode)   -> Model::NodeContents { return Model::NodeContents(groupNode->setEntity(std::get<Model::Entity>(std::move(contents)))); },
                    [&](Model::EntityNode* entityNode) -> Model::NodeContents { return Model::NodeContents(entityNode->setEntity(std::get<Model::Entity>(std::move(contents)))); },
                    [&](Model::BrushNode* brushNode)   -> Model::NodeContents { return Model::NodeContents(brushNode->setBrush(std::get<Model::Brush>(std::move(contents)))); }
                ));
            }

            if (!notifyEntityDefinitionsChange && !notifyModsChange) {
                setEntityDefinitions(nodes);
                setEntityModels(nodes);
            }
            if (!notifyTextureCollectionChange) {
                setTextures(nodes);
            }

            invalidateSelectionBounds();
        }

        std::map<Model::Node*, Model::VisibilityState> MapDocumentCommandFacade::setVisibilityState(const std::vector<Model::Node*>& nodes, const Model::VisibilityState visibilityState) {
            std::map<Model::Node*, Model::VisibilityState> result;

            std::vector<Model::Node*> changedNodes;
            changedNodes.reserve(nodes.size());

            for (Model::Node* node : nodes) {
                const Model::VisibilityState oldState = node->visibilityState();
                if (node->setVisibilityState(visibilityState)) {
                    changedNodes.push_back(node);
                    result[node] = oldState;
                }
            }

            nodeVisibilityDidChangeNotifier(changedNodes);
            return result;
        }

        std::map<Model::Node*, Model::VisibilityState> MapDocumentCommandFacade::setVisibilityEnsured(const std::vector<Model::Node*>& nodes) {
            std::map<Model::Node*, Model::VisibilityState> result;

            std::vector<Model::Node*> changedNodes;
            changedNodes.reserve(nodes.size());

            for (Model::Node* node : nodes) {
                const Model::VisibilityState oldState = node->visibilityState();
                if (node->ensureVisible()) {
                    changedNodes.push_back(node);
                    result[node] = oldState;
                }
            }

            nodeVisibilityDidChangeNotifier(changedNodes);
            return result;
        }

        void MapDocumentCommandFacade::restoreVisibilityState(const std::map<Model::Node*, Model::VisibilityState>& nodes) {
            std::vector<Model::Node*> changedNodes;
            changedNodes.reserve(nodes.size());

            for (const auto& entry : nodes) {
                Model::Node* node = entry.first;
                const Model::VisibilityState state = entry.second;
                if (node->setVisibilityState(state))
                    changedNodes.push_back(node);
            }

            nodeVisibilityDidChangeNotifier(changedNodes);
        }

        std::map<Model::Node*, Model::LockState> MapDocumentCommandFacade::setLockState(const std::vector<Model::Node*>& nodes, const Model::LockState lockState) {
            std::map<Model::Node*, Model::LockState> result;

            std::vector<Model::Node*> changedNodes;
            changedNodes.reserve(nodes.size());

            for (Model::Node* node : nodes) {
                const Model::LockState oldState = node->lockState();
                if (node->setLockState(lockState)) {
                    changedNodes.push_back(node);
                    result[node] = oldState;
                }
            }

            nodeLockingDidChangeNotifier(changedNodes);
            return result;
        }

        void MapDocumentCommandFacade::restoreLockState(const std::map<Model::Node*, Model::LockState>& nodes) {
            std::vector<Model::Node*> changedNodes;
            changedNodes.reserve(nodes.size());

            for (const auto& entry : nodes) {
                Model::Node* node = entry.first;
                const Model::LockState state = entry.second;
                if (node->setLockState(state))
                    changedNodes.push_back(node);
            }

            nodeLockingDidChangeNotifier(changedNodes);
        }

        void MapDocumentCommandFacade::performPushGroup(Model::GroupNode* group) {
            m_editorContext->pushGroup(group);
            groupWasOpenedNotifier(group);
        }

        void MapDocumentCommandFacade::performPopGroup() {
            Model::GroupNode* previousGroup = m_editorContext->currentGroup();
            m_editorContext->popGroup();
            groupWasClosedNotifier(previousGroup);
        }

        void MapDocumentCommandFacade::doSetIssueHidden(Model::Issue* issue, const bool hidden) {
            if (issue->hidden() != hidden) {
                issue->setHidden(hidden);
                incModificationCount();
            }
        }

        void MapDocumentCommandFacade::incModificationCount(const size_t delta) {
            m_modificationCount += delta;
            documentModificationStateDidChangeNotifier();
        }

        void MapDocumentCommandFacade::decModificationCount(const size_t delta) {
            assert(m_modificationCount >= delta);
            m_modificationCount -= delta;
            documentModificationStateDidChangeNotifier();
        }

        void MapDocumentCommandFacade::bindObservers() {
            m_commandProcessor->commandDoNotifier.addObserver(commandDoNotifier);
            m_commandProcessor->commandDoneNotifier.addObserver(commandDoneNotifier);
            m_commandProcessor->commandDoFailedNotifier.addObserver(commandDoFailedNotifier);
            m_commandProcessor->commandUndoNotifier.addObserver(commandUndoNotifier);
            m_commandProcessor->commandUndoneNotifier.addObserver(commandUndoneNotifier);
            m_commandProcessor->commandUndoFailedNotifier.addObserver(commandUndoFailedNotifier);
            m_commandProcessor->transactionDoneNotifier.addObserver(transactionDoneNotifier);
            m_commandProcessor->transactionUndoneNotifier.addObserver(transactionUndoneNotifier);
            documentWasNewedNotifier.addObserver(this, &MapDocumentCommandFacade::documentWasNewed);
            documentWasLoadedNotifier.addObserver(this, &MapDocumentCommandFacade::documentWasLoaded);
        }

        void MapDocumentCommandFacade::documentWasNewed(MapDocument*) {
            m_commandProcessor->clear();
        }

        void MapDocumentCommandFacade::documentWasLoaded(MapDocument*) {
            m_commandProcessor->clear();
        }

        bool MapDocumentCommandFacade::doCanUndoCommand() const {
            return m_commandProcessor->canUndo();
        }

        bool MapDocumentCommandFacade::doCanRedoCommand() const {
            return m_commandProcessor->canRedo();
        }

        const std::string& MapDocumentCommandFacade::doGetUndoCommandName() const {
            return m_commandProcessor->undoCommandName();
        }

        const std::string& MapDocumentCommandFacade::doGetRedoCommandName() const {
            return m_commandProcessor->redoCommandName();
        }

        void MapDocumentCommandFacade::doUndoCommand() {
            m_commandProcessor->undo();
        }

        void MapDocumentCommandFacade::doRedoCommand() {
            m_commandProcessor->redo();
        }

        void MapDocumentCommandFacade::doStartTransaction(const std::string& name) {
            m_commandProcessor->startTransaction(name);
        }

        void MapDocumentCommandFacade::doCommitTransaction() {
            m_commandProcessor->commitTransaction();
        }

        void MapDocumentCommandFacade::doRollbackTransaction() {
            m_commandProcessor->rollbackTransaction();
        }

        std::unique_ptr<CommandResult> MapDocumentCommandFacade::doExecute(std::unique_ptr<Command>&& command) {
            return m_commandProcessor->execute(std::move(command));
        }

        std::unique_ptr<CommandResult> MapDocumentCommandFacade::doExecuteAndStore(std::unique_ptr<UndoableCommand>&& command) {
            return m_commandProcessor->executeAndStore(std::move(command));
        }
    }
}
