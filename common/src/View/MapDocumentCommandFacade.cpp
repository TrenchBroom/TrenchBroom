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
#include "Model/EntityAttributeSnapshot.h"
#include "Model/Game.h"
#include "Model/GroupNode.h"
#include "Model/Issue.h"
#include "Model/LayerNode.h"
#include "Model/ModelUtils.h"
#include "Model/Snapshot.h"
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

        void MapDocumentCommandFacade::performSwapNodeContents(std::vector<std::pair<Model::Node*, Model::NodeContents>>& nodesToSwap) {
            const auto nodes = kdl::vec_transform(nodesToSwap, [](const auto& pair) { return pair.first; });
            const auto parents = collectParents(nodes);
            const auto descendants = collectDescendants(nodes);

            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyDescendants(nodesWillChangeNotifier, nodesDidChangeNotifier, descendants);

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

            setEntityDefinitions(nodes);
            setEntityModels(nodes);
            setTextures(nodes);

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

        std::map<Model::GroupNode*, std::string> MapDocumentCommandFacade::performRenameGroups(const std::string& newName) {
            const std::vector<Model::Node*>& nodes = m_selectedNodes.nodes();
            const std::vector<Model::Node*> parents = collectParents(nodes);

            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);

            std::map<Model::GroupNode*, std::string> oldNames;
            for (auto* node : nodes) {
                node->accept(kdl::overload(
                    [] (Model::WorldNode*) {},
                    [] (Model::LayerNode*) {},
                    [&](Model::GroupNode* group) {
                        oldNames[group] = group->name();
                        group->setName(newName);
                    },
                    [] (Model::EntityNode*) {},
                    [] (Model::BrushNode*) {}
                ));
            }

            return oldNames;
        }

        void MapDocumentCommandFacade::performUndoRenameGroups(const std::map<Model::GroupNode*, std::string>& newNames) {
            const std::vector<Model::Node*>& nodes = m_selectedNodes.nodes();
            const std::vector<Model::Node*> parents = collectParents(nodes);

            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);

            for (auto* node : nodes) {
                node->accept(kdl::overload(
                    [] (Model::WorldNode*) {},
                    [] (Model::LayerNode*) {},
                    [&](Model::GroupNode* group) {
                        assert(newNames.count(group) == 1);
                        const std::string& newName = kdl::map_find_or_default(newNames, group, group->name());
                        group->setName(newName);
                    },
                    [] (Model::EntityNode*) {},
                    [] (Model::BrushNode*) {}
                ));
            }
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

        MapDocumentCommandFacade::EntityAttributeSnapshotMap MapDocumentCommandFacade::performSetAttribute(const std::string& name, const std::string& value) {
            const std::vector<Model::AttributableNode*> attributableNodes = allSelectedAttributableNodes();
            return performSetAttributeForNodes(attributableNodes, name, value);
        }

        MapDocumentCommandFacade::EntityAttributeSnapshotMap MapDocumentCommandFacade::performSetAttributeForNodes(const std::vector<Model::AttributableNode*>& attributableNodes, const std::string& name, const std::string& value) {            
            const std::vector<Model::Node*> nodes(std::begin(attributableNodes), std::end(attributableNodes));
            const std::vector<Model::Node*> parents = collectParents(nodes);
            const std::vector<Model::Node*> descendants = collectDescendants(nodes);

            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyDescendants(nodesWillChangeNotifier, nodesDidChangeNotifier, descendants);

            MapDocumentCommandFacade::EntityAttributeSnapshotMap snapshot;

            for (Model::AttributableNode* node : attributableNodes) {
                snapshot[node].push_back(node->attributeSnapshot(name));
                auto entity = node->entity();
                entity.addOrUpdateAttribute(name, value);
                node->setEntity(std::move(entity));
            }

            setEntityDefinitions(nodes);
            setEntityModels(nodes);
            invalidateSelectionBounds();

            return snapshot;
        }

        MapDocumentCommandFacade::EntityAttributeSnapshotMap MapDocumentCommandFacade::performRemoveAttribute(const std::string& name) {
            const std::vector<Model::AttributableNode*> attributableNodes = allSelectedAttributableNodes();
            return performRemoveAttributeForNodes(attributableNodes, name);
        }

        MapDocumentCommandFacade::EntityAttributeSnapshotMap MapDocumentCommandFacade::performRemoveAttributeForNodes(const std::vector<Model::AttributableNode*>& attributableNodes, const std::string& name) {            
            const std::vector<Model::Node*> nodes(std::begin(attributableNodes), std::end(attributableNodes));
            const std::vector<Model::Node*> parents = collectParents(nodes);
            const std::vector<Model::Node*> descendants = collectDescendants(nodes);

            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyDescendants(nodesWillChangeNotifier, nodesDidChangeNotifier, descendants);

            MapDocumentCommandFacade::EntityAttributeSnapshotMap snapshot;

            for (Model::AttributableNode* node : attributableNodes) {
                snapshot[node].push_back(node->attributeSnapshot(name));

                auto entity = node->entity();
                entity.removeAttribute(name);
                node->setEntity(std::move(entity));
            }

            setEntityDefinitions(nodes);
            setEntityModels(nodes);
            invalidateSelectionBounds();

            return snapshot;
        }

        MapDocumentCommandFacade::EntityAttributeSnapshotMap MapDocumentCommandFacade::performUpdateSpawnflag(const std::string& name, const size_t flagIndex, const bool setFlag) {
            const std::vector<Model::AttributableNode*> attributableNodes = allSelectedAttributableNodes();
            const std::vector<Model::Node*> nodes(attributableNodes.begin(), attributableNodes.end());
            const std::vector<Model::Node*> parents = collectParents(nodes);
            const std::vector<Model::Node*> descendants = collectDescendants(nodes);

            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyDescendants(nodesWillChangeNotifier, nodesDidChangeNotifier, descendants);

            MapDocumentCommandFacade::EntityAttributeSnapshotMap snapshot;

            std::vector<Model::AttributableNode*>::const_iterator it, end;
            for (it = attributableNodes.begin(), end = attributableNodes.end(); it != end; ++it) {
                Model::AttributableNode* node = *it;
                snapshot[node].push_back(node->attributeSnapshot(name));

                const auto* strValue = node->entity().attribute(name);
                int intValue = strValue ? kdl::str_to_int(*strValue).value_or(0) : 0;
                const int flagValue = (1 << flagIndex);

                if (setFlag)
                    intValue |= flagValue;
                else
                    intValue &= ~flagValue;

                auto entity = node->entity();
                entity.addOrUpdateAttribute(name, kdl::str_to_string(intValue));
                node->setEntity(std::move(entity));
            }

            setEntityDefinitions(nodes);
            setEntityModels(nodes);

            return snapshot;
        }

        MapDocumentCommandFacade::EntityAttributeSnapshotMap MapDocumentCommandFacade::performConvertColorRange(const std::string& name, Assets::ColorRange::Type colorRange) {
            const std::vector<Model::AttributableNode*> attributableNodes = allSelectedAttributableNodes();
            const std::vector<Model::Node*> nodes(std::begin(attributableNodes), std::end(attributableNodes));
            const std::vector<Model::Node*> parents = collectParents(nodes);
            const std::vector<Model::Node*> descendants = collectDescendants(nodes);

            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyDescendants(nodesWillChangeNotifier, nodesDidChangeNotifier, descendants);

            MapDocumentCommandFacade::EntityAttributeSnapshotMap snapshot;

            for (Model::AttributableNode* node : attributableNodes) {
                if (const auto* oldValue = node->entity().attribute(name)) {
                    snapshot[node].push_back(node->attributeSnapshot(name));

                    auto entity = node->entity();
                    entity.addOrUpdateAttribute(name, Model::convertEntityColor(*oldValue, colorRange));
                    node->setEntity(std::move(entity));
                }
            }

            return snapshot;
        }
               
        MapDocumentCommandFacade::EntityAttributeSnapshotMap MapDocumentCommandFacade::performRenameAttribute(const std::string& oldName, const std::string& newName) {
            const std::vector<Model::AttributableNode*> attributableNodes = allSelectedAttributableNodes();
            return performRenameAttributeForNodes(attributableNodes, oldName, newName);
        }

        MapDocumentCommandFacade::EntityAttributeSnapshotMap MapDocumentCommandFacade::performRenameAttributeForNodes(const std::vector<Model::AttributableNode*>& attributableNodes, const std::string& oldName, const std::string& newName) {
            const std::vector<Model::Node*> nodes(std::begin(attributableNodes), std::end(attributableNodes));
            const std::vector<Model::Node*> parents = collectParents(nodes);
            const std::vector<Model::Node*> descendants = collectDescendants(nodes);

            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyDescendants(nodesWillChangeNotifier, nodesDidChangeNotifier, descendants);

            MapDocumentCommandFacade::EntityAttributeSnapshotMap snapshot;
            for (Model::AttributableNode* node : attributableNodes) {
                snapshot[node].push_back(node->attributeSnapshot(oldName));
                snapshot[node].push_back(node->attributeSnapshot(newName));

                auto entity = node->entity();
                entity.renameAttribute(oldName, newName);
                node->setEntity(std::move(entity));
            }

            setEntityDefinitions(nodes);
            setEntityModels(nodes);
            invalidateSelectionBounds();

            return snapshot;
        }

        void MapDocumentCommandFacade::restoreAttributes(const MapDocumentCommandFacade::EntityAttributeSnapshotMap& attributes) {
            const std::vector<Model::AttributableNode*> attributableNodes = kdl::map_keys(attributes);
            const std::vector<Model::Node*> nodes(std::begin(attributableNodes), std::end(attributableNodes));
            const std::vector<Model::Node*> parents = collectParents(nodes);
            const std::vector<Model::Node*> descendants = collectDescendants(nodes);

            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyDescendants(nodesWillChangeNotifier, nodesDidChangeNotifier, descendants);

            for (const auto& entry : attributes) {
                auto* node = entry.first;

                const auto& snapshots = entry.second;
                for (const auto& snapshot : snapshots) {
                    snapshot.restore(node);
                }
            }

            setEntityDefinitions(nodes);
            setEntityModels(nodes);
            invalidateSelectionBounds();
        }

        std::optional<std::vector<vm::polygon3>> MapDocumentCommandFacade::performResizeBrushes(const std::vector<vm::polygon3>& polygons, const vm::vec3& delta) {
            const std::vector<Model::BrushNode*>& selectedBrushes = m_selectedNodes.brushes();

            std::vector<vm::polygon3> result;
            std::vector<Model::Node*> changedNodes;
            std::map<Model::BrushNode*, Model::Brush> changes;

            for (Model::BrushNode* brushNode : selectedBrushes) {
                const Model::Brush& original = brushNode->brush();
                const auto faceIndex = original.findFace(polygons);
                if (!faceIndex) {
                    // We allow resizing only some of the brushes
                    continue;
                }

                const bool success = original.moveBoundary(m_worldBounds, *faceIndex, delta, pref(Preferences::TextureLock))
                    .visit(kdl::overload(
                        [&](Model::Brush&& copy) -> bool {
                            if (m_worldBounds.contains(copy.bounds())) {
                                result.push_back(copy.face(*faceIndex).polygon());
                                changedNodes.push_back(brushNode);
                                changes.emplace(brushNode, std::move(copy));
                                return true;
                            } else {
                                return false;
                            }
                        },
                        [&](const Model::BrushError e) -> bool {
                            error() << "Could not resize brush: " << e;
                            return false;
                        }
                    ));

                if (!success) {
                    return std::nullopt;
                }
            }

            const auto parents = collectParents(changedNodes);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, changedNodes);

            for (auto& [brushNode, brush] : changes) {
                brushNode->setBrush(std::move(brush));
            }
            invalidateSelectionBounds();

            return { result };
        }

        void MapDocumentCommandFacade::performMoveTextures(const vm::vec3f& cameraUp, const vm::vec3f& cameraRight, const vm::vec2f& delta) {
            for (const auto& faceHandle : m_selectedBrushFaces) {
                Model::BrushNode* node = faceHandle.node();
                Model::Brush brush = node->brush();
                Model::BrushFace& face = brush.face(faceHandle.faceIndex());
                face.moveTexture(vm::vec3(cameraUp), vm::vec3(cameraRight), delta);
                node->setBrush(brush);
            }
            brushFacesDidChangeNotifier(m_selectedBrushFaces);
        }

        void MapDocumentCommandFacade::performRotateTextures(const float angle) {
            for (const auto& faceHandle : m_selectedBrushFaces) {
                Model::BrushNode* node = faceHandle.node();
                Model::Brush brush = node->brush();
                Model::BrushFace& face = brush.face(faceHandle.faceIndex());
                face.rotateTexture(angle);
                node->setBrush(brush);
            }
            brushFacesDidChangeNotifier(m_selectedBrushFaces);
        }

        void MapDocumentCommandFacade::performShearTextures(const vm::vec2f& factors) {
            for (const auto& faceHandle : m_selectedBrushFaces) {
                Model::BrushNode* node = faceHandle.node();
                Model::Brush brush = node->brush();
                Model::BrushFace& face = brush.face(faceHandle.faceIndex());
                face.shearTexture(factors);
                node->setBrush(brush);
            }
            brushFacesDidChangeNotifier(m_selectedBrushFaces);
        }

        void MapDocumentCommandFacade::performCopyTexCoordSystemFromFace(const Model::TexCoordSystemSnapshot& coordSystemSnapshot, const Model::BrushFaceAttributes& attribs, const vm::plane3& sourceFacePlane, const Model::WrapStyle wrapStyle) {
            for (const auto& faceHandle : m_selectedBrushFaces) {
                Model::BrushNode* node = faceHandle.node();
                Model::Brush brush = node->brush();
                Model::BrushFace& face = brush.face(faceHandle.faceIndex());
                face.copyTexCoordSystemFromFace(coordSystemSnapshot, attribs, sourceFacePlane, wrapStyle);
                node->setBrush(brush);
            }
            brushFacesDidChangeNotifier(m_selectedBrushFaces);
        }

        void MapDocumentCommandFacade::performChangeBrushFaceAttributes(const Model::ChangeBrushFaceAttributesRequest& request) {
            if (request.evaluate(allSelectedBrushFaces())) {
                setTextures(allSelectedBrushFaces());
                brushFacesDidChangeNotifier(allSelectedBrushFaces());
            }
        }

        bool MapDocumentCommandFacade::performSnapVertices(const FloatType snapTo) {
            const std::vector<Model::BrushNode*> brushNodes = m_selectedNodes.brushesRecursively();

            const std::vector<Model::Node*> nodes(std::begin(brushNodes), std::end(brushNodes));
            const std::vector<Model::Node*> parents = collectParents(nodes);

            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);

            size_t succeededBrushCount = 0;
            size_t failedBrushCount = 0;

            for (Model::BrushNode* brushNode : brushNodes) {
                if (brushNode->brush().canSnapVertices(m_worldBounds, snapTo)) {
                    brushNode->brush().snapVertices(m_worldBounds, snapTo, pref(Preferences::UVLock))
                        .visit(kdl::overload(
                            [&](Model::Brush&& brush) {
                                brushNode->setBrush(std::move(brush));
                                succeededBrushCount += 1;
                            },
                            [&](const Model::BrushError e) {
                                error() << "Could not snap vertices: " << e;
                                failedBrushCount += 1;
                            }
                        ));
                } else {
                    failedBrushCount += 1;
                }
            }

            invalidateSelectionBounds();

            if (succeededBrushCount > 0) {
                info(kdl::str_to_string("Snapped vertices of ", succeededBrushCount, " ", kdl::str_plural(succeededBrushCount, "brush", "brushes")));
            }
            if (failedBrushCount > 0) {
                info(kdl::str_to_string("Failed to snap vertices of ", failedBrushCount, " ", kdl::str_plural(failedBrushCount, "brush", "brushes")));
            }

            return true;
        }

        std::vector<vm::vec3> MapDocumentCommandFacade::performMoveVertices(const std::map<Model::BrushNode*, std::vector<vm::vec3>>& vertices, const vm::vec3& delta) {
            const std::vector<Model::Node*>& nodes = m_selectedNodes.nodes();
            const std::vector<Model::Node*> parents = collectParents(nodes);

            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);

            std::vector<vm::vec3> newVertexPositions;
            for (const auto& entry : vertices) {
                Model::BrushNode* brushNode = entry.first;
                const std::vector<vm::vec3>& oldPositions = entry.second;
                
                brushNode->brush().moveVertices(m_worldBounds, oldPositions, delta, pref(Preferences::UVLock))
                    .visit(kdl::overload(
                        [&](Model::Brush&& brush) {
                            auto newPositions = brush.findClosestVertexPositions(oldPositions + delta);
                            newVertexPositions = kdl::vec_concat(std::move(newVertexPositions), std::move(newPositions));
                            brushNode->setBrush(std::move(brush));
                        },
                        [&](const Model::BrushError e) {
                            error() << "Could not move vertices: " << e;
                        }
                    ));
            }

            invalidateSelectionBounds();

            return kdl::vec_sort_and_remove_duplicates(std::move(newVertexPositions));
        }

        std::vector<vm::segment3> MapDocumentCommandFacade::performMoveEdges(const std::map<Model::BrushNode*, std::vector<vm::segment3>>& edges, const vm::vec3& delta) {
            const std::vector<Model::Node*>& nodes = m_selectedNodes.nodes();
            const std::vector<Model::Node*> parents = collectParents(nodes);

            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);

            std::vector<vm::segment3> newEdgePositions;
            for (const auto& entry : edges) {
                Model::BrushNode* brushNode = entry.first;
                const std::vector<vm::segment3>& oldPositions = entry.second;
                brushNode->brush().moveEdges(m_worldBounds, oldPositions, delta, pref(Preferences::UVLock))
                    .visit(kdl::overload(
                        [&](Model::Brush&& brush) {
                            auto newPositions = brush.findClosestEdgePositions(kdl::vec_transform(oldPositions, [&](const auto& s) {
                                return s.translate(delta);
                            }));
                            newEdgePositions = kdl::vec_concat(std::move(newEdgePositions), std::move(newPositions));
                            brushNode->setBrush(std::move(brush));
                        },
                        [&](const Model::BrushError e) {
                            error() << "Couild not move edges: " << e;
                        }
                    ));
            }

            invalidateSelectionBounds();

            return kdl::vec_sort_and_remove_duplicates(std::move(newEdgePositions));
        }

        std::vector<vm::polygon3> MapDocumentCommandFacade::performMoveFaces(const std::map<Model::BrushNode*, std::vector<vm::polygon3>>& faces, const vm::vec3& delta) {
            const std::vector<Model::Node*>& nodes = m_selectedNodes.nodes();
            const std::vector<Model::Node*> parents = collectParents(nodes);

            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);

            std::vector<vm::polygon3> newFacePositions;
            for (const auto& entry : faces) {
                Model::BrushNode* brushNode = entry.first;
                const std::vector<vm::polygon3>& oldPositions = entry.second;
                
                brushNode->brush().moveFaces(m_worldBounds, oldPositions, delta, pref(Preferences::UVLock))
                    .visit(kdl::overload(
                        [&](Model::Brush&& brush) {
                            auto newPositions = brush.findClosestFacePositions(kdl::vec_transform(oldPositions, [&](const auto& f) {
                                return f.translate(delta);
                            }));
                            newFacePositions = kdl::vec_concat(std::move(newFacePositions), std::move(newPositions));
                            brushNode->setBrush(std::move(brush));
                        },
                        [&](const Model::BrushError e) {
                            error() << "Could not move faces: " << e;
                        }
                    ));
            }

            invalidateSelectionBounds();

            return kdl::vec_sort_and_remove_duplicates(std::move(newFacePositions));
        }

        void MapDocumentCommandFacade::performAddVertices(const std::map<vm::vec3, std::vector<Model::BrushNode*>>& vertices) {
            const std::vector<Model::Node*>& nodes = m_selectedNodes.nodes();
            const std::vector<Model::Node*> parents = collectParents(nodes);

            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);

            for (const auto& entry : vertices) {
                const vm::vec3& position = entry.first;
                const std::vector<Model::BrushNode*>& brushNodes = entry.second;
                for (Model::BrushNode* brushNode : brushNodes) {
                    brushNode->brush().addVertex(m_worldBounds, position)
                        .visit(kdl::overload(
                            [&](Model::Brush&& brush) {
                                brushNode->setBrush(std::move(brush));
                            },
                            [&](const Model::BrushError e) {
                                error() << "Could not add vertex: " << e;
                            }
                        ));
                }
            }

            invalidateSelectionBounds();
        }

        void MapDocumentCommandFacade::performRemoveVertices(const std::map<Model::BrushNode*, std::vector<vm::vec3>>& vertices) {
            const std::vector<Model::Node*>& nodes = m_selectedNodes.nodes();
            const std::vector<Model::Node*> parents = collectParents(nodes);

            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);

            for (const auto& entry : vertices) {
                Model::BrushNode* brushNode = entry.first;
                const std::vector<vm::vec3>& positions = entry.second;
                
                brushNode->brush().removeVertices(m_worldBounds, positions)
                    .visit(kdl::overload(
                        [&](Model::Brush&& brush) {
                            brushNode->setBrush(std::move(brush));
                        },
                        [&](const Model::BrushError e) {
                            error() << "Could not remove vertex: " << e;
                        }
                    ));
            }

            invalidateSelectionBounds();
        }

        void MapDocumentCommandFacade::restoreSnapshot(Model::Snapshot* snapshot) {
            const auto restoreNodesAndLogErrors = [&]() {
                snapshot->restoreNodes(m_worldBounds).
                    visit(kdl::overload(
                        []() {},
                        [&](const Model::SnapshotErrors& errors) {
                            for (const auto& e : errors) {
                                error() << kdl::str_to_string(e);
                            }
                        }
                    ));
            };
        
            if (!m_selectedNodes.empty()) {
                const std::vector<Model::Node*>& nodes = m_selectedNodes.nodes();
                const std::vector<Model::Node*> parents = collectParents(nodes);

                Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
                Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);

                restoreNodesAndLogErrors();

                setTextures(m_selectedNodes.nodes());
                setEntityDefinitions(m_selectedNodes.nodes());
                setEntityModels(m_selectedNodes.nodes());
                invalidateSelectionBounds();
            }

            const auto& faceHandles = selectedBrushFaces();
            if (!faceHandles.empty()) {
                restoreNodesAndLogErrors();

                // Restoring the snapshots will invalidate all texture pointers on the BrushNode,
                // since the snapshot has a whole brush granularity, so we need to call
                // setTextures on the whole node.
                kdl::vector_set<Model::Node*> nodes;
                nodes.reserve(faceHandles.size());
                for (const auto& faceHandle : faceHandles) {
                    nodes.insert(faceHandle.node());
                }

                setTextures(nodes.release_data());
                brushFacesDidChangeNotifier(faceHandles);
            }
        }

        void MapDocumentCommandFacade::performSetEntityDefinitionFile(const Assets::EntityDefinitionFileSpec& spec) {
            const std::vector<Model::Node*> nodes(1, m_world.get());
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            Notifier<>::NotifyAfter notifyEntityDefinitions(entityDefinitionsDidChangeNotifier);

            // to avoid backslashes being misinterpreted as escape sequences
            const std::string formatted = kdl::str_replace_every(spec.asString(), "\\", "/");

            auto entity = m_world->entity();
            entity.addOrUpdateAttribute(Model::AttributeNames::EntityDefinitions, formatted);
            m_world->setEntity(std::move(entity));

            reloadEntityDefinitionsInternal();
        }

        void MapDocumentCommandFacade::performSetTextureCollections(const std::vector<IO::Path>& paths) {
            const std::vector<Model::Node*> nodes(1, m_world.get());
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            Notifier<>::NotifyBeforeAndAfter notifyTextureCollections(textureCollectionsWillChangeNotifier, textureCollectionsDidChangeNotifier);

            m_game->updateTextureCollections(*m_world, paths);
            unsetTextures();
            loadTextures();
            setTextures();
        }

        void MapDocumentCommandFacade::performSetMods(const std::vector<std::string>& mods) {
            const std::vector<Model::Node*> nodes(1, m_world.get());
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            Notifier<>::NotifyAfter notifyMods(modsDidChangeNotifier);

            unsetEntityModels();
            unsetEntityDefinitions();
            clearEntityModels();

            auto entity = m_world->entity();
            if (mods.empty()) {
                entity.removeAttribute(Model::AttributeNames::Mods);
            } else {
                const std::string newValue = kdl::str_join(mods, ";");
                entity.addOrUpdateAttribute(Model::AttributeNames::Mods, newValue);
            }
            m_world->setEntity(std::move(entity));

            updateGameSearchPaths();
            setEntityDefinitions();
            setEntityModels();
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
