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

#include "Preferences.h"
#include "PreferenceManager.h"
#include "Assets/EntityDefinitionFileSpec.h"
#include "Assets/TextureManager.h"
#include "Model/Brush.h"
#include "Model/BrushNode.h"
#include "Model/BrushFace.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "Model/CollectSelectableBrushFacesVisitor.h"
#include "Model/CollectSelectableNodesVisitor.h"
#include "Model/EditorContext.h"
#include "Model/EntityNode.h"
#include "Model/EntityAttributeSnapshot.h"
#include "Model/Game.h"
#include "Model/GroupNode.h"
#include "Model/Issue.h"
#include "Model/ModelUtils.h"
#include "Model/Snapshot.h"
#include "Model/TransformObjectVisitor.h"
#include "Model/WorldNode.h"
#include "Model/NodeVisitor.h"
#include "View/CommandProcessor.h"
#include "View/UndoableCommand.h"
#include "View/Selection.h"

#include <kdl/map_utils.h>
#include <kdl/string_format.h>
#include <kdl/string_utils.h>
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
                Model::BrushFace* face = handle.face();
                if (!face->selected() && m_editorContext->selectable(node, face)) {
                    face->select();
                    selected.push_back(handle);
                }
            }

            kdl::vec_append(m_selectedBrushFaces, selected);

            Selection selection;
            selection.addSelectedBrushFaces(selected);

            selectionDidChangeNotifier(selection);
        }

        void MapDocumentCommandFacade::performSelectAllNodes() {
            performDeselectAll();

            Model::CollectSelectableNodesVisitor visitor(*m_editorContext);

            Model::Node* target = currentGroupOrWorld();
            target->recurse(visitor);
            performSelect(visitor.nodes());
        }

        void MapDocumentCommandFacade::performSelectAllBrushFaces() {
            performDeselectAll();

            Model::CollectSelectableBrushFacesVisitor visitor(*m_editorContext);
            m_world->acceptAndRecurse(visitor);
            performSelect(visitor.faces());
        }

        void MapDocumentCommandFacade::performConvertToBrushFaceSelection() {
            Model::CollectSelectableBrushFacesVisitor visitor(*m_editorContext);
            Model::Node::acceptAndRecurse(std::begin(m_selectedNodes), std::end(m_selectedNodes), visitor);

            performDeselectAll();
            performSelect(visitor.faces());
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
                Model::BrushFace* face = handle.face();
                if (face->selected()) {
                    face->deselect();
                    deselected.push_back(handle);
                }
            }

            kdl::vec_erase_all(m_selectedBrushFaces, deselected);

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
                handle.face()->deselect();
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
                kdl::vec_append(addedNodes, children);
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

        class MapDocumentCommandFacade::RenameGroupsVisitor : public Model::NodeVisitor {
        private:
            const std::string& m_newName;
            std::map<Model::GroupNode*, std::string> m_oldNames;
        public:
            explicit RenameGroupsVisitor(const std::string& newName) : m_newName(newName) {}
            const std::map<Model::GroupNode*, std::string>& oldNames() const { return m_oldNames; }
        private:
            void doVisit(Model::WorldNode*) override  {}
            void doVisit(Model::LayerNode*) override  {}
            void doVisit(Model::GroupNode* group) override {
                m_oldNames[group] = group->name();
                group->setName(m_newName);
            }
            void doVisit(Model::EntityNode*) override {}
            void doVisit(Model::BrushNode*) override  {}
        };

        class MapDocumentCommandFacade::UndoRenameGroupsVisitor : public Model::NodeVisitor {
        private:
            const std::map<Model::GroupNode*, std::string>& m_newNames;
        public:
            explicit UndoRenameGroupsVisitor(const std::map<Model::GroupNode*, std::string>& newNames) : m_newNames(newNames) {}
        private:
            void doVisit(Model::WorldNode*) override  {}
            void doVisit(Model::LayerNode*) override  {}
            void doVisit(Model::GroupNode* group) override {
                assert(m_newNames.count(group) == 1);
                const std::string& newName = kdl::map_find_or_default(m_newNames, group, group->name());
                group->setName(newName);
            }
            void doVisit(Model::EntityNode*) override {}
            void doVisit(Model::BrushNode*) override  {}
        };

        std::map<Model::GroupNode*, std::string> MapDocumentCommandFacade::performRenameGroups(const std::string& newName) {
            const std::vector<Model::Node*>& nodes = m_selectedNodes.nodes();
            const std::vector<Model::Node*> parents = collectParents(nodes);

            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);

            RenameGroupsVisitor visitor(newName);
            Model::Node::accept(std::begin(nodes), std::end(nodes), visitor);
            return visitor.oldNames();
        }

        void MapDocumentCommandFacade::performUndoRenameGroups(const std::map<Model::GroupNode*, std::string>& newNames) {
            const std::vector<Model::Node*>& nodes = m_selectedNodes.nodes();
            const std::vector<Model::Node*> parents = collectParents(nodes);

            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);

            UndoRenameGroupsVisitor visitor(newNames);
            Model::Node::accept(std::begin(nodes), std::end(nodes), visitor);
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

        class CanTransformVisitor : public Model::ConstNodeVisitor, public Model::NodeQuery<bool> {
        private:
            vm::mat4x4 m_transform;
            vm::bbox3 m_worldBounds;
        public:
            CanTransformVisitor(const vm::mat4x4& transform, const vm::bbox3& worldBounds) :
                m_transform(transform),
                m_worldBounds(worldBounds) {}
        private:
            void doVisit(const Model::WorldNode*) override  { setResult(true); }
            void doVisit(const Model::LayerNode*) override  { setResult(true); }
            void doVisit(const Model::GroupNode*) override  { setResult(true); }
            void doVisit(const Model::EntityNode*) override { setResult(true); }
            void doVisit(const Model::BrushNode* brush) override { setResult(brush->canTransform(m_transform, m_worldBounds)); }
            bool doCombineResults(const bool oldResult, const bool newResult) const override {
                return newResult && oldResult;
            }
        };

        bool MapDocumentCommandFacade::performTransform(const vm::mat4x4 &transform, const bool lockTextures) {
          // Test whether all brushes can be transformed; abort if any fail.
          CanTransformVisitor canTransform(transform, m_worldBounds);
          for (const auto* node : m_selectedNodes.nodes()) {
              node->acceptAndRecurse(canTransform);
          }
          if (canTransform.hasResult() && !canTransform.result()) {
              return false;
          }

          const std::vector<Model::Node*> &nodes = m_selectedNodes.nodes();
          const std::vector<Model::Node*> parents = collectParents(nodes);

          Notifier<const std::vector<Model::Node*> &>::NotifyBeforeAndAfter
              notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier,
                            parents);
          Notifier<const std::vector<Model::Node*> &>::NotifyBeforeAndAfter notifyNodes(
              nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);

          Model::TransformObjectVisitor visitor(transform, lockTextures,
                                                m_worldBounds);
          Model::Node::accept(std::begin(nodes), std::end(nodes), visitor);

          invalidateSelectionBounds();
          return true;
        }

        MapDocumentCommandFacade::EntityAttributeSnapshotMap MapDocumentCommandFacade::performSetAttribute(const std::string& name, const std::string& value) {
            const std::vector<Model::AttributableNode*> attributableNodes = allSelectedAttributableNodes();
            const std::vector<Model::Node*> nodes(std::begin(attributableNodes), std::end(attributableNodes));
            const std::vector<Model::Node*> parents = collectParents(std::begin(nodes), std::end(nodes));
            const std::vector<Model::Node*> descendants = collectDescendants(nodes);

            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyDescendants(nodesWillChangeNotifier, nodesDidChangeNotifier, descendants);

            MapDocumentCommandFacade::EntityAttributeSnapshotMap snapshot;

            for (Model::AttributableNode* node : attributableNodes) {
                snapshot[node].push_back(node->attributeSnapshot(name));
                node->addOrUpdateAttribute(name, value);
            }

            setEntityDefinitions(nodes);
            setEntityModels(nodes);

            return snapshot;
        }

        MapDocumentCommandFacade::EntityAttributeSnapshotMap MapDocumentCommandFacade::performRemoveAttribute(const std::string& name) {
            const std::vector<Model::AttributableNode*> attributableNodes = allSelectedAttributableNodes();
            const std::vector<Model::Node*> nodes(std::begin(attributableNodes), std::end(attributableNodes));
            const std::vector<Model::Node*> parents = collectParents(std::begin(nodes), std::end(nodes));
            const std::vector<Model::Node*> descendants = collectDescendants(nodes);

            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyDescendants(nodesWillChangeNotifier, nodesDidChangeNotifier, descendants);

            MapDocumentCommandFacade::EntityAttributeSnapshotMap snapshot;

            for (Model::AttributableNode* node : attributableNodes) {
                snapshot[node].push_back(node->attributeSnapshot(name));
                node->removeAttribute(name);
            }

            setEntityDefinitions(nodes);
            setEntityModels(nodes);

            return snapshot;
        }

        MapDocumentCommandFacade::EntityAttributeSnapshotMap MapDocumentCommandFacade::performUpdateSpawnflag(const std::string& name, const size_t flagIndex, const bool setFlag) {
            const std::vector<Model::AttributableNode*> attributableNodes = allSelectedAttributableNodes();
            const std::vector<Model::Node*> nodes(attributableNodes.begin(), attributableNodes.end());
            const std::vector<Model::Node*> parents = collectParents(nodes.begin(), nodes.end());
            const std::vector<Model::Node*> descendants = collectDescendants(nodes);

            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyDescendants(nodesWillChangeNotifier, nodesDidChangeNotifier, descendants);

            MapDocumentCommandFacade::EntityAttributeSnapshotMap snapshot;

            std::vector<Model::AttributableNode*>::const_iterator it, end;
            for (it = attributableNodes.begin(), end = attributableNodes.end(); it != end; ++it) {
                Model::AttributableNode* node = *it;
                snapshot[node].push_back(node->attributeSnapshot(name));

                int intValue = node->hasAttribute(name) ? std::atoi(node->attribute(name).c_str()) : 0;
                const int flagValue = (1 << flagIndex);

                if (setFlag)
                    intValue |= flagValue;
                else
                    intValue &= ~flagValue;

                node->addOrUpdateAttribute(name, kdl::str_to_string(intValue));
            }

            setEntityDefinitions(nodes);
            setEntityModels(nodes);

            return snapshot;
        }

        MapDocumentCommandFacade::EntityAttributeSnapshotMap MapDocumentCommandFacade::performConvertColorRange(const std::string& name, Assets::ColorRange::Type colorRange) {
            const std::vector<Model::AttributableNode*> attributableNodes = allSelectedAttributableNodes();
            const std::vector<Model::Node*> nodes(std::begin(attributableNodes), std::end(attributableNodes));
            const std::vector<Model::Node*> parents = collectParents(std::begin(nodes), std::end(nodes));
            const std::vector<Model::Node*> descendants = collectDescendants(nodes);

            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyDescendants(nodesWillChangeNotifier, nodesDidChangeNotifier, descendants);

            static const std::string DefaultValue = "";
            MapDocumentCommandFacade::EntityAttributeSnapshotMap snapshot;

            for (Model::AttributableNode* node : attributableNodes) {
                const std::string& oldValue = node->attribute(name, DefaultValue);
                if (oldValue != DefaultValue) {
                    snapshot[node].push_back(node->attributeSnapshot(name));
                    node->addOrUpdateAttribute(name, Model::convertEntityColor(oldValue, colorRange));
                }
            }

            return snapshot;
        }

        MapDocumentCommandFacade::EntityAttributeSnapshotMap MapDocumentCommandFacade::performRenameAttribute(const std::string& oldName, const std::string& newName) {
            const std::vector<Model::AttributableNode*> attributableNodes = allSelectedAttributableNodes();
            const std::vector<Model::Node*> nodes(std::begin(attributableNodes), std::end(attributableNodes));
            const std::vector<Model::Node*> parents = collectParents(std::begin(nodes), std::end(nodes));
            const std::vector<Model::Node*> descendants = collectDescendants(nodes);

            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyDescendants(nodesWillChangeNotifier, nodesDidChangeNotifier, descendants);

            MapDocumentCommandFacade::EntityAttributeSnapshotMap snapshot;
            for (Model::AttributableNode* node : attributableNodes) {
                snapshot[node].push_back(node->attributeSnapshot(oldName));
                snapshot[node].push_back(node->attributeSnapshot(newName));
                node->renameAttribute(oldName, newName);
            }

            setEntityDefinitions(nodes);
            setEntityModels(nodes);

            return snapshot;
        }

        void MapDocumentCommandFacade::restoreAttributes(const MapDocumentCommandFacade::EntityAttributeSnapshotMap& attributes) {
            const std::vector<Model::AttributableNode*> attributableNodes = kdl::map_keys(attributes);
            const std::vector<Model::Node*> nodes(std::begin(attributableNodes), std::end(attributableNodes));
            const std::vector<Model::Node*> parents = collectParents(std::begin(nodes), std::end(nodes));
            const std::vector<Model::Node*> descendants = collectDescendants(nodes);

            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyDescendants(nodesWillChangeNotifier, nodesDidChangeNotifier, descendants);

            for (const auto& entry : attributes) {
                auto* node = entry.first;
                assert(node->parent() == nullptr || node->selected() || node->descendantSelected());

                const auto& snapshots = entry.second;
                for (const auto& snapshot : snapshots) {
                    snapshot.restore(node);
                }
            }

            setEntityDefinitions(nodes);
            setEntityModels(nodes);
        }

        std::vector<vm::polygon3> MapDocumentCommandFacade::performResizeBrushes(const std::vector<vm::polygon3>& polygons, const vm::vec3& delta) {
            std::vector<vm::polygon3> result;

            const std::vector<Model::BrushNode*>& selectedBrushes = m_selectedNodes.brushes();
            std::vector<Model::Node*> changedNodes;

            for (Model::BrushNode* brushNode : selectedBrushes) {
                const Model::Brush& brush = brushNode->brush();
                Model::BrushFace* face = brush.findFace(polygons);
                if (face != nullptr) {
                    if (!brush.canMoveBoundary(m_worldBounds, face, delta)) {
                        return result;
                    }

                    changedNodes.push_back(brushNode);
                }
            }

            const auto parents = collectParents(std::begin(changedNodes), std::end(changedNodes));
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, changedNodes);

            for (Model::BrushNode* brushNode : selectedBrushes) {
                Model::Brush brush = brushNode->brush();
                Model::BrushFace* face = brush.findFace(polygons);
                if (face != nullptr) {
                    brush.moveBoundary(m_worldBounds, face, delta, pref(Preferences::TextureLock));
                    result.push_back(face->polygon());
                    brushNode->setBrush(std::move(brush));
                }
            }
            
            invalidateSelectionBounds();

            return result;
        }

        void MapDocumentCommandFacade::performMoveTextures(const vm::vec3f& cameraUp, const vm::vec3f& cameraRight, const vm::vec2f& delta) {
            const auto faces = Model::toFaces(m_selectedBrushFaces);
            for (auto* face : faces) {
                face->moveTexture(vm::vec3(cameraUp), vm::vec3(cameraRight), delta);
            }
            brushFacesDidChangeNotifier(faces);
        }

        void MapDocumentCommandFacade::performRotateTextures(const float angle) {
            const auto faces = Model::toFaces(m_selectedBrushFaces);
            for (auto* face : faces) {
                face->rotateTexture(angle);
            }
            brushFacesDidChangeNotifier(faces);
        }

        void MapDocumentCommandFacade::performShearTextures(const vm::vec2f& factors) {
            const auto faces = Model::toFaces(m_selectedBrushFaces);
            for (auto* face : faces) {
                face->shearTexture(factors);
            }
            brushFacesDidChangeNotifier(faces);
        }

        void MapDocumentCommandFacade::performCopyTexCoordSystemFromFace(const Model::TexCoordSystemSnapshot& coordSystemSnapshot, const Model::BrushFaceAttributes& attribs, const vm::plane3& sourceFacePlane, const Model::WrapStyle wrapStyle) {
            const auto faces = Model::toFaces(m_selectedBrushFaces);
            for (auto* face : faces) {
                face->copyTexCoordSystemFromFace(coordSystemSnapshot, attribs, sourceFacePlane, wrapStyle);
            }
            brushFacesDidChangeNotifier(faces);
        }

        void MapDocumentCommandFacade::performChangeBrushFaceAttributes(const Model::ChangeBrushFaceAttributesRequest& request) {
            const auto faces = Model::toFaces(allSelectedBrushFaces());
            if (request.evaluate(faces)) {
                setTextures(faces);
                brushFacesDidChangeNotifier(faces);
            }
        }

        bool MapDocumentCommandFacade::performFindPlanePoints() {
            const std::vector<Model::BrushNode*>& brushes = m_selectedNodes.brushes();

            const std::vector<Model::Node*> nodes(std::begin(brushes), std::end(brushes));
            const std::vector<Model::Node*> parents = collectParents(nodes);

            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);

            for (Model::BrushNode* brush : brushes) {
                brush->findIntegerPlanePoints(m_worldBounds);
            }

            return true;
        }

        bool MapDocumentCommandFacade::performSnapVertices(const FloatType snapTo) {
            const std::vector<Model::BrushNode*>& brushes = m_selectedNodes.brushes();

            const std::vector<Model::Node*> nodes(std::begin(brushes), std::end(brushes));
            const std::vector<Model::Node*> parents = collectParents(nodes);

            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);

            size_t succeededBrushCount = 0;
            size_t failedBrushCount = 0;

            for (Model::BrushNode* brush : brushes) {
                if (brush->canSnapVertices(m_worldBounds, snapTo)) {
                    brush->snapVertices(m_worldBounds, snapTo, pref(Preferences::UVLock));
                    succeededBrushCount += 1;
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
                Model::BrushNode* brush = entry.first;
                const std::vector<vm::vec3>& oldPositions = entry.second;
                const std::vector<vm::vec3> newPositions = brush->moveVertices(m_worldBounds, oldPositions, delta, pref(Preferences::UVLock));
                kdl::vec_append(newVertexPositions, newPositions);
            }

            invalidateSelectionBounds();

            kdl::vec_sort_and_remove_duplicates(newVertexPositions);
            return newVertexPositions;
        }

        std::vector<vm::segment3> MapDocumentCommandFacade::performMoveEdges(const std::map<Model::BrushNode*, std::vector<vm::segment3>>& edges, const vm::vec3& delta) {
            const std::vector<Model::Node*>& nodes = m_selectedNodes.nodes();
            const std::vector<Model::Node*> parents = collectParents(nodes);

            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);

            std::vector<vm::segment3> newEdgePositions;
            for (const auto& entry : edges) {
                Model::BrushNode* brush = entry.first;
                const std::vector<vm::segment3>& oldPositions = entry.second;
                const std::vector<vm::segment3> newPositions = brush->moveEdges(m_worldBounds, oldPositions, delta, pref(Preferences::UVLock));
                kdl::vec_append(newEdgePositions, newPositions);
            }

            invalidateSelectionBounds();

            kdl::vec_sort_and_remove_duplicates(newEdgePositions);
            return newEdgePositions;
        }

        std::vector<vm::polygon3> MapDocumentCommandFacade::performMoveFaces(const std::map<Model::BrushNode*, std::vector<vm::polygon3>>& faces, const vm::vec3& delta) {
            const std::vector<Model::Node*>& nodes = m_selectedNodes.nodes();
            const std::vector<Model::Node*> parents = collectParents(nodes);

            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);

            std::vector<vm::polygon3> newFacePositions;
            for (const auto& entry : faces) {
                Model::BrushNode* brush = entry.first;
                const std::vector<vm::polygon3>& oldPositions = entry.second;
                const std::vector<vm::polygon3> newPositions = brush->moveFaces(m_worldBounds, oldPositions, delta, pref(Preferences::UVLock));
                kdl::vec_append(newFacePositions, newPositions);
            }

            invalidateSelectionBounds();

            kdl::vec_sort_and_remove_duplicates(newFacePositions);
            return newFacePositions;
        }

        void MapDocumentCommandFacade::performAddVertices(const std::map<vm::vec3, std::vector<Model::BrushNode*>>& vertices) {
            const std::vector<Model::Node*>& nodes = m_selectedNodes.nodes();
            const std::vector<Model::Node*> parents = collectParents(nodes);

            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);

            for (const auto& entry : vertices) {
                const vm::vec3& position = entry.first;
                const std::vector<Model::BrushNode*>& brushes = entry.second;
                for (Model::BrushNode* brush : brushes)
                    brush->addVertex(m_worldBounds, position);
            }

            invalidateSelectionBounds();
        }

        void MapDocumentCommandFacade::performRemoveVertices(const std::map<Model::BrushNode*, std::vector<vm::vec3>>& vertices) {
            const std::vector<Model::Node*>& nodes = m_selectedNodes.nodes();
            const std::vector<Model::Node*> parents = collectParents(nodes);

            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);

            for (const auto& entry : vertices) {
                Model::BrushNode* brush = entry.first;
                const std::vector<vm::vec3>& positions = entry.second;
                brush->removeVertices(m_worldBounds, positions);
            }

            invalidateSelectionBounds();
        }

        void MapDocumentCommandFacade::restoreSnapshot(Model::Snapshot* snapshot) {
            if (!m_selectedNodes.empty()) {
                const std::vector<Model::Node*>& nodes = m_selectedNodes.nodes();
                const std::vector<Model::Node*> parents = collectParents(nodes);

                Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
                Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);

                snapshot->restoreNodes(m_worldBounds);

                invalidateSelectionBounds();
            }

            const auto faces = Model::toFaces(allSelectedBrushFaces());
            if (!faces.empty()) {
                snapshot->restoreBrushFaces();
                setTextures(faces);
                brushFacesDidChangeNotifier(faces);
            }
        }

        void MapDocumentCommandFacade::performSetEntityDefinitionFile(const Assets::EntityDefinitionFileSpec& spec) {
            const std::vector<Model::Node*> nodes(1, m_world.get());
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            Notifier<>::NotifyAfter notifyEntityDefinitions(entityDefinitionsDidChangeNotifier);

            // to avoid backslashes being misinterpreted as escape sequences
            const std::string formatted = kdl::str_replace_every(spec.asString(), "\\", "/");
            m_world->addOrUpdateAttribute(Model::AttributeNames::EntityDefinitions, formatted);
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

            if (mods.empty()) {
                m_world->removeAttribute(Model::AttributeNames::Mods);
            } else {
                const std::string newValue = kdl::str_join(mods, ";");
                m_world->addOrUpdateAttribute(Model::AttributeNames::Mods, newValue);
            }

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

        bool MapDocumentCommandFacade::doCanRepeatCommands() const {
            return m_commandProcessor->canRepeat();
        }

        std::unique_ptr<CommandResult> MapDocumentCommandFacade::doRepeatCommands() {
            return m_commandProcessor->repeat();
        }

        void MapDocumentCommandFacade::doClearRepeatableCommands() {
            m_commandProcessor->clearRepeatStack();
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
