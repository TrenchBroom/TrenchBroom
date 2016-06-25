/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "CollectionUtils.h"
#include "Assets/EntityDefinitionFileSpec.h"
#include "Assets/TextureManager.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "Model/CollectNodesWithDescendantSelectionCountVisitor.h"
#include "Model/CollectRecursivelySelectedNodesVisitor.h"
#include "Model/CollectSelectableBrushFacesVisitor.h"
#include "Model/CollectSelectableNodesVisitor.h"
#include "Model/EditorContext.h"
#include "Model/Entity.h"
#include "Model/Game.h"
#include "Model/Group.h"
#include "Model/Issue.h"
#include "Model/ModelUtils.h"
#include "Model/Snapshot.h"
#include "Model/TransformObjectVisitor.h"
#include "Model/World.h"
#include "View/Selection.h"

namespace TrenchBroom {
    namespace View {
        MapDocumentSPtr MapDocumentCommandFacade::newMapDocument() {
            return MapDocumentSPtr(new MapDocumentCommandFacade());
        }

        MapDocumentCommandFacade::MapDocumentCommandFacade() :
        m_commandProcessor(this) {
            bindObservers();
        }

        void MapDocumentCommandFacade::performSelect(const Model::NodeList& nodes) {
            selectionWillChangeNotifier();
            updateLastSelectionBounds();

            Model::NodeList selected;
            selected.reserve(nodes.size());
            
            Model::CollectNodesWithDescendantSelectionCountVisitor ancestors(0);
            Model::CollectRecursivelySelectedNodesVisitor descendants(false);

            Model::NodeList::const_iterator it, end;
            for (it = nodes.begin(), end = nodes.end(); it != end; ++it) {
                Model::Node* node = *it;
                if (!node->selected() && m_editorContext->selectable(node)) {
                    node->escalate(ancestors);
                    node->recurse(descendants);
                    node->select();
                    selected.push_back(node);
                }
            }

            const Model::NodeList& partiallySelected = ancestors.nodes();
            const Model::NodeList& recursivelySelected = descendants.nodes();
            
            m_selectedNodes.addNodes(selected);
            m_partiallySelectedNodes.addNodes(partiallySelected);
            
            Selection selection;
            selection.addSelectedNodes(selected);
            selection.addPartiallySelectedNodes(partiallySelected);
            selection.addRecursivelySelectedNodes(recursivelySelected);
            
            selectionDidChangeNotifier(selection);
            invalidateSelectionBounds();
        }
        
        void MapDocumentCommandFacade::performSelect(const Model::BrushFaceList& faces) {
            selectionWillChangeNotifier();
            
            Model::BrushFaceList selected;
            selected.reserve(faces.size());
            
            Model::CollectNodesWithDescendantSelectionCountVisitor visitor(0);
            
            Model::BrushFaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                Model::BrushFace* face = *it;
                if (!face->selected() && m_editorContext->selectable(face)) {
                    face->brush()->acceptAndEscalate(visitor);
                    face->select();
                    selected.push_back(face);
                }
            }
            
            const Model::NodeList& partiallySelected = visitor.nodes();
            
            VectorUtils::append(m_selectedBrushFaces, selected);
            m_partiallySelectedNodes.addNodes(partiallySelected);
            
            Selection selection;
            selection.addSelectedBrushFaces(selected);
            selection.addPartiallySelectedNodes(partiallySelected);
            
            selectionDidChangeNotifier(selection);
        }
        
        void MapDocumentCommandFacade::performSelectAllNodes() {
            performDeselectAll();
            
            Model::CollectSelectableNodesVisitor visitor(*m_editorContext);
            m_world->acceptAndRecurse(visitor);
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
            Model::Node::acceptAndRecurse(m_selectedNodes.begin(), m_selectedNodes.end(), visitor);
            
            performDeselectAll();
            performSelect(visitor.faces());
        }

        void MapDocumentCommandFacade::performDeselect(const Model::NodeList& nodes) {
            selectionWillChangeNotifier();
            updateLastSelectionBounds();
            
            Model::NodeList deselected;
            deselected.reserve(nodes.size());
            
            Model::CollectNodesWithDescendantSelectionCountVisitor ancestors(0);
            Model::CollectRecursivelySelectedNodesVisitor descendants(false);
            
            Model::NodeList::const_iterator it, end;
            for (it = nodes.begin(), end = nodes.end(); it != end; ++it) {
                Model::Node* node = *it;
                if (node->selected()) {
                    node->deselect();
                    deselected.push_back(node);
                    node->escalate(ancestors);
                    node->recurse(descendants);
                }
            }
            
            const Model::NodeList& partiallyDeselected = ancestors.nodes();
            const Model::NodeList& recursivelyDeselected = descendants.nodes();
            
            m_selectedNodes.removeNodes(deselected);
            m_partiallySelectedNodes.removeNodes(partiallyDeselected);
            
            Selection selection;
            selection.addDeselectedNodes(deselected);
            selection.addPartiallyDeselectedNodes(partiallyDeselected);
            selection.addRecursivelyDeselectedNodes(recursivelyDeselected);
            
            selectionDidChangeNotifier(selection);
            invalidateSelectionBounds();
        }
        
        void MapDocumentCommandFacade::performDeselect(const Model::BrushFaceList& faces) {
            selectionWillChangeNotifier();
            
            Model::BrushFaceList deselected;
            deselected.reserve(faces.size());
            
            Model::CollectNodesWithDescendantSelectionCountVisitor visitor(0);
            
            Model::BrushFaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                Model::BrushFace* face = *it;
                if (face->selected()) {
                    face->deselect();
                    deselected.push_back(face);
                    face->brush()->acceptAndEscalate(visitor);
                }
            }
            
            const Model::NodeList& partiallyDeselected = visitor.nodes();

            VectorUtils::eraseAll(m_selectedBrushFaces, deselected);
            m_selectedNodes.removeNodes(partiallyDeselected);
            
            Selection selection;
            selection.addDeselectedBrushFaces(deselected);
            selection.addPartiallyDeselectedNodes(partiallyDeselected);
            
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

            Model::CollectRecursivelySelectedNodesVisitor descendants(false);

            Model::NodeList::const_iterator it, end;
            for (it = m_selectedNodes.begin(), end = m_selectedNodes.end(); it != end; ++it) {
                Model::Node* node = *it;
                node->deselect();
                node->recurse(descendants);
            }
            
            Selection selection;
            selection.addDeselectedNodes(m_selectedNodes.nodes());
            selection.addPartiallyDeselectedNodes(m_partiallySelectedNodes.nodes());
            selection.addRecursivelyDeselectedNodes(descendants.nodes());

            m_selectedNodes.clear();
            m_partiallySelectedNodes.clear();
            
            selectionDidChangeNotifier(selection);
            invalidateSelectionBounds();
        }
        
        void MapDocumentCommandFacade::deselectAllBrushFaces() {
            selectionWillChangeNotifier();
            
            Model::BrushFaceList::const_iterator it, end;
            for (it = m_selectedBrushFaces.begin(), end = m_selectedBrushFaces.end(); it != end; ++it) {
                Model::BrushFace* face = *it;
                face->deselect();
            }
            
            Selection selection;
            selection.addDeselectedBrushFaces(m_selectedBrushFaces);
            selection.addPartiallyDeselectedNodes(m_partiallySelectedNodes.nodes());
            
            m_selectedBrushFaces.clear();
            m_partiallySelectedNodes.clear();
            
            selectionDidChangeNotifier(selection);
        }

        Model::NodeList MapDocumentCommandFacade::performAddNodes(const Model::ParentChildrenMap& nodes) {
            const Model::NodeList parents = collectParents(nodes);
            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            
            Model::NodeList addedNodes;
            Model::ParentChildrenMap::const_iterator pIt, pEnd;
            for (pIt = nodes.begin(), pEnd = nodes.end(); pIt != pEnd; ++pIt) {
                Model::Node* parent = pIt->first;
                const Model::NodeList& children = pIt->second;
                parent->addChildren(children);
                VectorUtils::append(addedNodes, children);
            }
            
            setEntityDefinitions(addedNodes);
            setEntityModels(addedNodes);
            setTextures(addedNodes);
            invalidateSelectionBounds();

            nodesWereAddedNotifier(addedNodes);
            return addedNodes;
        }
        
        Model::ParentChildrenMap MapDocumentCommandFacade::performRemoveNodes(const Model::NodeList& nodes) {
            Model::ParentChildrenMap removedNodes = parentChildrenMap(nodes);
            addEmptyNodes(removedNodes);
            
            const Model::NodeList parents = collectParents(removedNodes);
            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            
            const Model::NodeList allChildren = collectChildren(removedNodes);
            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyChildren(nodesWillBeRemovedNotifier, nodesWereRemovedNotifier, allChildren);
            
            Model::ParentChildrenMap::const_iterator it, end;
            for (it = removedNodes.begin(), end = removedNodes.end(); it != end; ++it) {
                Model::Node* parent = it->first;
                const Model::NodeList& children = it->second;
                unsetEntityDefinitions(children);
                unsetTextures(children);
                parent->removeChildren(children.begin(), children.end());
            }
            
            invalidateSelectionBounds();

            return removedNodes;
        }
        
        void MapDocumentCommandFacade::addEmptyNodes(Model::ParentChildrenMap& nodes) const {
            Model::NodeList emptyNodes = collectEmptyNodes(nodes);
            while (!emptyNodes.empty()) {
                removeEmptyNodes(nodes, emptyNodes);
                emptyNodes = collectEmptyNodes(nodes);
            }
        }
        
        Model::NodeList MapDocumentCommandFacade::collectEmptyNodes(const Model::ParentChildrenMap& nodes) const {
            Model::NodeList result;
            
            Model::ParentChildrenMap::const_iterator it, end;
            for (it = nodes.begin(), end = nodes.end(); it != end; ++it) {
                Model::Node* node = it->first;
                const Model::NodeList& children = it->second;
                if (node->removeIfEmpty() && node->childCount() == children.size())
                    result.push_back(node);
            }
            
            return result;
        }
        
        void MapDocumentCommandFacade::removeEmptyNodes(Model::ParentChildrenMap& nodes, const Model::NodeList& emptyNodes) const {
            Model::NodeList::const_iterator it, end;
            for (it = emptyNodes.begin(), end = emptyNodes.end(); it != end; ++it) {
                Model::Node* node = *it;
                Model::Node* parent = node->parent();
                nodes.erase(node);
                assert(!VectorUtils::contains(nodes[parent], node));
                nodes[parent].push_back(node);
            }
        }

        MapDocumentCommandFacade::ReparentResult::ReparentResult(const Model::ParentChildrenMap& i_movedNodes, const Model::ParentChildrenMap& i_removedNodes) :
        movedNodes(i_movedNodes),
        removedNodes(i_removedNodes) {}

        MapDocumentCommandFacade::ReparentResult MapDocumentCommandFacade::performReparentNodes(const Model::ParentChildrenMap& nodes, const EmptyNodePolicy emptyNodePolicy) {
            const Model::NodeList emptyParents = emptyNodePolicy == RemoveEmptyNodes ? findRemovableEmptyParentNodes(nodes) : Model::EmptyNodeList;
            
            const Model::NodeList nodesToNotify = Model::collectChildren(nodes);
            const Model::NodeList parentsToNotify = VectorUtils::eraseAll(Model::collectParents(nodes), emptyParents);
            
            Model::NodeList nodesWithChangedLockState;
            Model::NodeList nodesWithChangedVisibilityState;
            
            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parentsToNotify);
            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodesToNotify);

            Model::ParentChildrenMap movedNodes;
            
            Model::ParentChildrenMap::const_iterator pcIt, pcEnd;
            Model::NodeList::const_iterator nIt, nEnd;
            
            for (pcIt = nodes.begin(), pcEnd = nodes.end(); pcIt != pcEnd; ++pcIt) {
                Model::Node* newParent = pcIt->first;
                const Model::NodeList& children = pcIt->second;
                
                for (nIt = children.begin(), nEnd = children.end(); nIt != nEnd; ++nIt) {
                    Model::Node* child = *nIt;
                    Model::Node* oldParent = child->parent();
                    assert(oldParent != NULL);
                    
                    const bool wasLocked = child->locked();
                    const bool wasHidden = child->hidden();
                    
                    movedNodes[oldParent].push_back(child);
                    oldParent->removeChild(child);
                    newParent->addChild(child);
                    
                    if (wasLocked != child->locked())
                        nodesWithChangedLockState.push_back(child);
                    if (wasHidden != child->hidden())
                        nodesWithChangedVisibilityState.push_back(child);
                }
            }
            
            nodeLockingDidChangeNotifier(nodesWithChangedLockState);
            nodeVisibilityDidChangeNotifier(nodesWithChangedVisibilityState);
            
            const Model::ParentChildrenMap removedNodes = performRemoveNodes(emptyParents);
            return ReparentResult(movedNodes, removedNodes);
        }

        Model::NodeList MapDocumentCommandFacade::findRemovableEmptyParentNodes(const Model::ParentChildrenMap& nodes) const {
            Model::NodeList emptyParents;
            
            typedef std::map<Model::Node*, size_t> RemoveCounts;
            RemoveCounts counts;

            Model::ParentChildrenMap::const_iterator pcIt, pcEnd;
            Model::NodeList::const_iterator nIt, nEnd;
            
            for (pcIt = nodes.begin(), pcEnd = nodes.end(); pcIt != pcEnd; ++pcIt) {
                const Model::NodeList& children = pcIt->second;
                
                for (nIt = children.begin(), nEnd = children.end(); nIt != nEnd; ++nIt) {
                    Model::Node* child = *nIt;
                    Model::Node* oldParent = child->parent();
                    assert(oldParent != NULL);

                    const size_t count = MapUtils::find(counts, oldParent, size_t(0)) + 1;
                    MapUtils::insertOrReplace(counts, oldParent, count);

                    if (oldParent->removeIfEmpty() && oldParent->childCount() == count)
                        emptyParents.push_back(oldParent);
                }
            }
            
            return emptyParents;
        }
        
        Model::VisibilityMap MapDocumentCommandFacade::setVisibilityState(const Model::NodeList& nodes, const Model::VisibilityState visibilityState) {
            Model::VisibilityMap result;
            
            Model::NodeList changedNodes;
            changedNodes.reserve(nodes.size());
            
            Model::NodeList::const_iterator it, end;
            for (it = nodes.begin(), end = nodes.end(); it != end; ++it) {
                Model::Node* node = *it;
                const Model::VisibilityState oldState = node->visibilityState();
                if (node->setVisiblityState(visibilityState)) {
                    changedNodes.push_back(node);
                    result[node] = oldState;
                }
            }
            
            nodeVisibilityDidChangeNotifier(changedNodes);
            return result;
        }
        
        Model::VisibilityMap MapDocumentCommandFacade::setVisibilityEnsured(const Model::NodeList& nodes) {
            Model::VisibilityMap result;
            
            Model::NodeList changedNodes;
            changedNodes.reserve(nodes.size());
            
            Model::NodeList::const_iterator it, end;
            for (it = nodes.begin(), end = nodes.end(); it != end; ++it) {
                Model::Node* node = *it;
                const Model::VisibilityState oldState = node->visibilityState();
                if (node->ensureVisible()) {
                    changedNodes.push_back(node);
                    result[node] = oldState;
                }
            }
            
            nodeVisibilityDidChangeNotifier(changedNodes);
            return result;
        }

        void MapDocumentCommandFacade::restoreVisibilityState(const Model::VisibilityMap& nodes) {
            Model::NodeList changedNodes;
            changedNodes.reserve(nodes.size());
            
            Model::VisibilityMap::const_iterator it, end;
            for (it = nodes.begin(), end = nodes.end(); it != end; ++it) {
                Model::Node* node = it->first;
                const Model::VisibilityState state = it->second;
                if (node->setVisiblityState(state))
                    changedNodes.push_back(node);
            }

            nodeVisibilityDidChangeNotifier(changedNodes);
        }

        Model::LockStateMap MapDocumentCommandFacade::setLockState(const Model::NodeList& nodes, const Model::LockState lockState) {
            Model::LockStateMap result;
            
            Model::NodeList changedNodes;
            changedNodes.reserve(nodes.size());
            
            Model::NodeList::const_iterator it, end;
            for (it = nodes.begin(), end = nodes.end(); it != end; ++it) {
                Model::Node* node = *it;
                const Model::LockState oldState = node->lockState();
                if (node->setLockState(lockState)) {
                    changedNodes.push_back(node);
                    result[node] = oldState;
                }
            }
            
            nodeLockingDidChangeNotifier(changedNodes);
            return result;
        }
        
        void MapDocumentCommandFacade::restoreLockState(const Model::LockStateMap& nodes) {
            Model::NodeList changedNodes;
            changedNodes.reserve(nodes.size());
            
            Model::LockStateMap::const_iterator it, end;
            for (it = nodes.begin(), end = nodes.end(); it != end; ++it) {
                Model::Node* node = it->first;
                const Model::LockState state = it->second;
                if (node->setLockState(state))
                    changedNodes.push_back(node);
            }
            
            nodeLockingDidChangeNotifier(changedNodes);
        }
        
        class MapDocumentCommandFacade::RenameGroupsVisitor : public Model::NodeVisitor {
        private:
            const String& m_newName;
            Model::GroupNameMap m_oldNames;
        public:
            RenameGroupsVisitor(const String& newName) : m_newName(newName) {}
            const Model::GroupNameMap& oldNames() const { return m_oldNames; }
        private:
            void doVisit(Model::World* world)   {}
            void doVisit(Model::Layer* layer)   {}
            void doVisit(Model::Group* group)   {
                m_oldNames[group] = group->name();
                group->setName(m_newName);
            }
            void doVisit(Model::Entity* entity) {}
            void doVisit(Model::Brush* brush)   {}
        };

        class MapDocumentCommandFacade::UndoRenameGroupsVisitor : public Model::NodeVisitor {
        private:
            const Model::GroupNameMap& m_newNames;
        public:
            UndoRenameGroupsVisitor(const Model::GroupNameMap& newNames) : m_newNames(newNames) {}
        private:
            void doVisit(Model::World* world)   {}
            void doVisit(Model::Layer* layer)   {}
            void doVisit(Model::Group* group)   {
                assert(m_newNames.count(group) == 1);
                const String& newName = MapUtils::find(m_newNames, group, group->name());
                group->setName(newName);
            }
            void doVisit(Model::Entity* entity) {}
            void doVisit(Model::Brush* brush)   {}
        };
        
        Model::GroupNameMap MapDocumentCommandFacade::performRenameGroups(const String& newName) {
            const Model::NodeList& nodes = m_selectedNodes.nodes();
            const Model::NodeList parents = collectParents(nodes);
            
            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            
            RenameGroupsVisitor visitor(newName);
            Model::Node::accept(nodes.begin(), nodes.end(), visitor);
            return visitor.oldNames();
        }

        void MapDocumentCommandFacade::performUndoRenameGroups(const Model::GroupNameMap& newNames) {
            const Model::NodeList& nodes = m_selectedNodes.nodes();
            const Model::NodeList parents = collectParents(nodes);
            
            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);

            UndoRenameGroupsVisitor visitor(newNames);
            Model::Node::accept(nodes.begin(), nodes.end(), visitor);
        }

        void MapDocumentCommandFacade::performPushGroup(Model::Group* group) {
            m_editorContext->pushGroup(group);
            groupWasOpenedNotifier(group);
        }
        
        void MapDocumentCommandFacade::performPopGroup() {
            Model::Group* previousGroup = m_editorContext->currentGroup();
            m_editorContext->popGroup();
            groupWasClosedNotifier(previousGroup);
        }

        void MapDocumentCommandFacade::performTransform(const Mat4x4& transform, const bool lockTextures) {
            const Model::NodeList& nodes = m_selectedNodes.nodes();
            const Model::NodeList parents = collectParents(nodes);
            
            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            
            Model::TransformObjectVisitor visitor(transform, lockTextures, m_worldBounds);
            Model::Node::accept(nodes.begin(), nodes.end(), visitor);
            
            invalidateSelectionBounds();
        }

        Model::EntityAttributeSnapshot::Map MapDocumentCommandFacade::performSetAttribute(const Model::AttributeName& name, const Model::AttributeValue& value) {
            const Model::AttributableNodeList attributableNodes = allSelectedAttributableNodes();
            const Model::NodeList nodes(attributableNodes.begin(), attributableNodes.end());
            const Model::NodeList parents = collectParents(nodes.begin(), nodes.end());

            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            
            Model::EntityAttributeSnapshot::Map snapshot;
            
            Model::AttributableNodeList::const_iterator it, end;
            for (it = attributableNodes.begin(), end = attributableNodes.end(); it != end; ++it) {
                Model::AttributableNode* node = *it;
                snapshot.insert(std::make_pair(node, node->attributeSnapshot(name)));
                node->addOrUpdateAttribute(name, value);
            }
            
            setEntityDefinitions(nodes);

            return snapshot;
        }
        
        Model::EntityAttributeSnapshot::Map MapDocumentCommandFacade::performRemoveAttribute(const Model::AttributeName& name) {
            const Model::AttributableNodeList attributableNodes = allSelectedAttributableNodes();
            const Model::NodeList nodes(attributableNodes.begin(), attributableNodes.end());
            const Model::NodeList parents = collectParents(nodes.begin(), nodes.end());
            
            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            
            static const Model::AttributeValue DefaultValue = "";
            Model::EntityAttributeSnapshot::Map snapshot;
            
            Model::AttributableNodeList::const_iterator it, end;
            for (it = attributableNodes.begin(), end = attributableNodes.end(); it != end; ++it) {
                Model::AttributableNode* node = *it;
                snapshot.insert(std::make_pair(node, node->attributeSnapshot(name)));
                node->removeAttribute(name);
            }
            
            setEntityDefinitions(nodes);

            return snapshot;
        }
        
        Model::EntityAttributeSnapshot::Map MapDocumentCommandFacade::performConvertColorRange(const Model::AttributeName& name, Assets::ColorRange::Type colorRange) {
            const Model::AttributableNodeList attributableNodes = allSelectedAttributableNodes();
            const Model::NodeList nodes(attributableNodes.begin(), attributableNodes.end());
            const Model::NodeList parents = collectParents(nodes.begin(), nodes.end());
            
            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            
            static const Model::AttributeValue DefaultValue = "";
            Model::EntityAttributeSnapshot::Map snapshot;

            Model::AttributableNodeList::const_iterator it, end;
            for (it = attributableNodes.begin(), end = attributableNodes.end(); it != end; ++it) {
                Model::AttributableNode* node = *it;
                
                const Model::AttributeValue& oldValue = node->attribute(name, DefaultValue);
                if (oldValue != DefaultValue) {
                    snapshot.insert(std::make_pair(node, node->attributeSnapshot(name)));
                    node->addOrUpdateAttribute(name, Model::convertEntityColor(oldValue, colorRange));
                }
            }
            
            return snapshot;
        }

        void MapDocumentCommandFacade::performRenameAttribute(const Model::AttributeName& oldName, const Model::AttributeName& newName) {
            const Model::AttributableNodeList attributableNodes = allSelectedAttributableNodes();
            const Model::NodeList nodes(attributableNodes.begin(), attributableNodes.end());
            const Model::NodeList parents = collectParents(nodes.begin(), nodes.end());
            
            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            
            Model::AttributableNodeList::const_iterator it, end;
            for (it = attributableNodes.begin(), end = attributableNodes.end(); it != end; ++it) {
                Model::AttributableNode* node = *it;
                node->renameAttribute(oldName, newName);
            }

            setEntityDefinitions(nodes);
        }
        
        void MapDocumentCommandFacade::restoreAttributes(const Model::EntityAttributeSnapshot::Map& attributes) {
            const Model::AttributableNodeList attributableNodes = MapUtils::keyList(attributes);
            const Model::NodeList nodes(attributableNodes.begin(), attributableNodes.end());
            
            const Model::NodeList parents = collectParents(nodes.begin(), nodes.end());
            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);

            Model::EntityAttributeSnapshot::Map::const_iterator it, end;
            for (it = attributes.begin(), end = attributes.end(); it != end; ++it) {
                Model::AttributableNode* node = it->first;
                assert(node->parent() == NULL || node->selected() || node->descendantSelected());
                
                const Model::EntityAttributeSnapshot& snapshot = it->second;
                snapshot.restore(node);
            }

            setEntityDefinitions(nodes);
        }

        bool MapDocumentCommandFacade::performResizeBrushes(const Vec3& normal, const Vec3& delta) {
            const Model::BrushList& selectedBrushes = m_selectedNodes.brushes();
            Model::NodeList changedNodes;
            Model::BrushFaceList faces;
            
            Model::BrushList::const_iterator bIt, bEnd;
            for (bIt = selectedBrushes.begin(), bEnd = selectedBrushes.end(); bIt != bEnd; ++bIt) {
                Model::Brush* brush = *bIt;
                Model::BrushFace* face = brush->findFace(normal);
                if (face != NULL) {
                    if (!brush->canMoveBoundary(m_worldBounds, face, delta))
                        return false;
                    
                    changedNodes.push_back(brush);
                    faces.push_back(face);
                }
            }
            
            const Model::NodeList parents = collectParents(changedNodes.begin(), changedNodes.end());
            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, changedNodes);
            
            Model::BrushFaceList::iterator fIt, fEnd;
            for (fIt = faces.begin(), fEnd = faces.end(); fIt != fEnd; ++fIt) {
                Model::BrushFace* face = *fIt;
                Model::Brush* brush = face->brush();
                assert(brush->selected());
                brush->moveBoundary(m_worldBounds, face, delta, textureLock());
            }
            
            invalidateSelectionBounds();

            return true;
        }

        void MapDocumentCommandFacade::performMoveTextures(const Vec3f& cameraUp, const Vec3f& cameraRight, const Vec2f& delta) {
            Model::BrushFaceList::const_iterator it, end;
            for (it = m_selectedBrushFaces.begin(), end = m_selectedBrushFaces.end(); it != end; ++it) {
                Model::BrushFace* face = *it;
                face->moveTexture(cameraUp, cameraRight, delta);
            }
            brushFacesDidChangeNotifier(m_selectedBrushFaces);
        }

        void MapDocumentCommandFacade::performRotateTextures(const float angle) {
            Model::BrushFaceList::const_iterator it, end;
            for (it = m_selectedBrushFaces.begin(), end = m_selectedBrushFaces.end(); it != end; ++it) {
                Model::BrushFace* face = *it;
                face->rotateTexture(angle);
            }
            brushFacesDidChangeNotifier(m_selectedBrushFaces);
        }

        void MapDocumentCommandFacade::performShearTextures(const Vec2f& factors) {
            Model::BrushFaceList::const_iterator it, end;
            for (it = m_selectedBrushFaces.begin(), end = m_selectedBrushFaces.end(); it != end; ++it) {
                Model::BrushFace* face = *it;
                face->shearTexture(factors);
            }
            brushFacesDidChangeNotifier(m_selectedBrushFaces);
        }

        void MapDocumentCommandFacade::performChangeBrushFaceAttributes(const Model::ChangeBrushFaceAttributesRequest& request) {
            const Model::BrushFaceList& faces = allSelectedBrushFaces();
            request.evaluate(faces);
            setTextures(faces);
            brushFacesDidChangeNotifier(faces);
        }

        Model::Snapshot* MapDocumentCommandFacade::performFindPlanePoints() {
            const Model::BrushList& brushes = m_selectedNodes.brushes();
            Model::Snapshot* snapshot = new Model::Snapshot(brushes.begin(), brushes.end());
            
            const Model::NodeList nodes(brushes.begin(), brushes.end());
            const Model::NodeList parents = collectParents(nodes);
            
            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            
            Model::BrushList::const_iterator it, end;
            for (it = brushes.begin(), end = brushes.end(); it != end; ++it) {
                Model::Brush* brush = *it;
                brush->findIntegerPlanePoints(m_worldBounds);
            }
            
            return snapshot;
        }

        Vec3::List MapDocumentCommandFacade::performSnapVertices(const Model::BrushVerticesMap& vertices, const size_t snapTo) {
            const Model::NodeList& nodes = m_selectedNodes.nodes();
            const Model::NodeList parents = collectParents(nodes);
            
            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);

            size_t succeededBrushCount = 0;
            size_t failedBrushCount = 0;
            size_t failedVertexCount = 0;
            
            Vec3::List newVertexPositions;
            Model::BrushVerticesMap::const_iterator it, end;
            for (it = vertices.begin(), end = vertices.end(); it != end; ++it) {
                Model::Brush* brush = it->first;
                const Vec3::List& oldPositions = it->second;
                if (brush->canSnapVertices(m_worldBounds, oldPositions, snapTo)) {
                    const Vec3::List newPositions = brush->snapVertices(m_worldBounds, oldPositions, snapTo);
                    VectorUtils::append(newVertexPositions, newPositions);
                    succeededBrushCount += 1;
                } else {
                    failedBrushCount += 1;
                    failedVertexCount += oldPositions.size();
                }
            }
            
            invalidateSelectionBounds();

            if (!newVertexPositions.empty()) {
                StringStream msg;
                msg << "Snapped " << newVertexPositions.size() << " " << StringUtils::safePlural(newVertexPositions.size(), "vertex", "vertices") << " of " << succeededBrushCount << " " << StringUtils::safePlural(succeededBrushCount, "brush", "brushes");
                info(msg.str());
            }
            if (failedVertexCount > 0) {
                StringStream msg;
                msg << "Failed to snap " << failedVertexCount << " " << StringUtils::safePlural(failedVertexCount, "vertex", "vertices") << " of " << failedBrushCount << " " << StringUtils::safePlural(failedBrushCount, "brush", "brushes");
                info(msg.str());
            }
            
            return newVertexPositions;
        }

        Vec3::List MapDocumentCommandFacade::performMoveVertices(const Model::BrushVerticesMap& vertices, const Vec3& delta) {
            const Model::NodeList& nodes = m_selectedNodes.nodes();
            const Model::NodeList parents = collectParents(nodes);
            
            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            
            Vec3::List newVertexPositions;
            Model::BrushVerticesMap::const_iterator it, end;
            for (it = vertices.begin(), end = vertices.end(); it != end; ++it) {
                Model::Brush* brush = it->first;
                const Vec3::List& oldPositions = it->second;
                const Vec3::List newPositions = brush->moveVertices(m_worldBounds, oldPositions, delta);
                VectorUtils::append(newVertexPositions, newPositions);
            }
            
            invalidateSelectionBounds();

            return newVertexPositions;
        }

        Edge3::List MapDocumentCommandFacade::performMoveEdges(const Model::BrushEdgesMap& edges, const Vec3& delta) {
            const Model::NodeList& nodes = m_selectedNodes.nodes();
            const Model::NodeList parents = collectParents(nodes);
            
            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            
            Edge3::List newEdgePositions;
            Model::BrushEdgesMap::const_iterator it, end;
            for (it = edges.begin(), end = edges.end(); it != end; ++it) {
                Model::Brush* brush = it->first;
                const Edge3::List& oldPositions = it->second;
                const Edge3::List newPositions = brush->moveEdges(m_worldBounds, oldPositions, delta);
                VectorUtils::append(newEdgePositions, newPositions);
            }
            
            return newEdgePositions;
        }

        Polygon3::List MapDocumentCommandFacade::performMoveFaces(const Model::BrushFacesMap& faces, const Vec3& delta) {
            const Model::NodeList& nodes = m_selectedNodes.nodes();
            const Model::NodeList parents = collectParents(nodes);
            
            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            
            Polygon3::List newFacePositions;
            Model::BrushFacesMap::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                Model::Brush* brush = it->first;
                const Polygon3::List& oldPositions = it->second;
                const Polygon3::List newPositions = brush->moveFaces(m_worldBounds, oldPositions, delta);
                VectorUtils::append(newFacePositions, newPositions);
            }
            
            invalidateSelectionBounds();

            return newFacePositions;
        }

        Vec3::List MapDocumentCommandFacade::performSplitEdges(const Model::BrushEdgesMap& edges, const Vec3& delta) {
            const Model::NodeList& nodes = m_selectedNodes.nodes();
            const Model::NodeList parents = collectParents(nodes);
            
            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            
            Vec3::List newVertexPositions;
            Model::BrushEdgesMap::const_iterator it, end;
            for (it = edges.begin(), end = edges.end(); it != end; ++it) {
                Model::Brush* brush = it->first;
                const Edge3::List& oldPositions = it->second;
                for (size_t i = 0; i < oldPositions.size(); ++i) {
                    const Edge3& edgePosition = oldPositions[i];
                    const Vec3 vertexPosition = brush->splitEdge(m_worldBounds, edgePosition, delta);
                    newVertexPositions.push_back(vertexPosition);
                }
            }
            
            invalidateSelectionBounds();

            return newVertexPositions;
        }

        Vec3::List MapDocumentCommandFacade::performSplitFaces(const Model::BrushFacesMap& faces, const Vec3& delta) {
            const Model::NodeList& nodes = m_selectedNodes.nodes();
            const Model::NodeList parents = collectParents(nodes);
            
            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            
            Vec3::List newVertexPositions;
            Model::BrushFacesMap::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                Model::Brush* brush = it->first;
                const Polygon3::List& oldPositions = it->second;
                for (size_t i = 0; i < oldPositions.size(); ++i) {
                    const Polygon3& facePosition = oldPositions[i];
                    const Vec3 vertexPosition = brush->splitFace(m_worldBounds, facePosition, delta);
                    newVertexPositions.push_back(vertexPosition);
                }
            }
            
            invalidateSelectionBounds();

            return newVertexPositions;
        }

        void MapDocumentCommandFacade::performRebuildBrushGeometry(const Model::BrushList& brushes) {
            const Model::NodeList nodes = VectorUtils::cast<Model::Node*>(brushes);
            const Model::NodeList parents = collectParents(nodes);
            
            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            
            Model::BrushList::const_iterator it, end;
            for (it = brushes.begin(), end = brushes.end(); it != end; ++it) {
                Model::Brush* brush = *it;
                brush->rebuildGeometry(m_worldBounds);
            }

            invalidateSelectionBounds();
        }

        void MapDocumentCommandFacade::restoreSnapshot(Model::Snapshot* snapshot) {
            if (!m_selectedNodes.empty()) {
                const Model::NodeList& nodes = m_selectedNodes.nodes();
                const Model::NodeList parents = collectParents(nodes);
                
                Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
                Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
                
                snapshot->restoreNodes(m_worldBounds);
                
                invalidateSelectionBounds();
            }
            
            const Model::BrushFaceList brushFaces = allSelectedBrushFaces();
            if (!brushFaces.empty()) {
                snapshot->restoreBrushFaces();
                setTextures(brushFaces);
                brushFacesDidChangeNotifier(brushFaces);
            }
        }

        void MapDocumentCommandFacade::performSetEntityDefinitionFile(const Assets::EntityDefinitionFileSpec& spec) {
            const Model::NodeList nodes(1, m_world);
            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            Notifier0::NotifyAfter notifyEntityDefinitions(entityDefinitionsDidChangeNotifier);
            
            // to avoid backslashes being misinterpreted as escape sequences
            const String formatted = StringUtils::replaceAll(spec.asString(), "\\", "/");
            m_world->addOrUpdateAttribute(Model::AttributeNames::EntityDefinitions, formatted);
            reloadEntityDefinitions();
        }

        void MapDocumentCommandFacade::performSetTextureCollections(const IO::Path::List& paths) {
            const Model::NodeList nodes(1, m_world);
            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            Notifier0::NotifyAfter notifyTextureCollections(textureCollectionsDidChangeNotifier);
            
            unsetTextures();
            m_game->updateTextureCollections(m_world, paths);
            reloadTextures();
            setTextures();
        }

        void MapDocumentCommandFacade::performSetMods(const StringList& mods) {
            const Model::NodeList nodes(1, m_world);
            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            Notifier0::NotifyAfter notifyMods(modsDidChangeNotifier);

            unsetEntityDefinitions();
            clearEntityModels();
            m_world->addOrUpdateAttribute(Model::AttributeNames::Mods, StringUtils::join(mods, ";"));
            updateGameSearchPaths();
            setEntityDefinitions();
            setEntityModels();
        }

        void MapDocumentCommandFacade::performSetGameEngineParameterSpecs(const ::StringMap& specs) {
            const Model::NodeList nodes(1, m_world);
            Notifier1<const Model::NodeList&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);

            m_game->setGameEngineParameterSpecs(m_world, specs);
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
            m_commandProcessor.commandDoNotifier.addObserver(commandDoNotifier);
            m_commandProcessor.commandDoneNotifier.addObserver(commandDoneNotifier);
            m_commandProcessor.commandDoFailedNotifier.addObserver(commandDoFailedNotifier);
            m_commandProcessor.commandUndoNotifier.addObserver(commandUndoNotifier);
            m_commandProcessor.commandUndoneNotifier.addObserver(commandUndoneNotifier);
            m_commandProcessor.commandUndoFailedNotifier.addObserver(commandUndoFailedNotifier);
            documentWasNewedNotifier.addObserver(this, &MapDocumentCommandFacade::documentWasNewed);
            documentWasLoadedNotifier.addObserver(this, &MapDocumentCommandFacade::documentWasLoaded);
        }
        
        void MapDocumentCommandFacade::documentWasNewed(MapDocument* document) {
            m_commandProcessor.clear();
        }
        
        void MapDocumentCommandFacade::documentWasLoaded(MapDocument* document) {
            m_commandProcessor.clear();
        }

        bool MapDocumentCommandFacade::doCanUndoLastCommand() const {
            return m_commandProcessor.hasLastCommand();
        }
        
        bool MapDocumentCommandFacade::doCanRedoNextCommand() const {
            return m_commandProcessor.hasNextCommand();
        }
        
        const String& MapDocumentCommandFacade::doGetLastCommandName() const {
            return m_commandProcessor.lastCommandName();
        }
        
        const String& MapDocumentCommandFacade::doGetNextCommandName() const {
            return m_commandProcessor.nextCommandName();
        }
        
        void MapDocumentCommandFacade::doUndoLastCommand() {
            m_commandProcessor.undoLastCommand();
        }
        
        void MapDocumentCommandFacade::doRedoNextCommand() {
            m_commandProcessor.redoNextCommand();
        }
        
        bool MapDocumentCommandFacade::doRepeatLastCommands() {
            return m_commandProcessor.repeatLastCommands();
        }
        
        void MapDocumentCommandFacade::doClearRepeatableCommands() {
            m_commandProcessor.clearRepeatableCommands();
        }

        void MapDocumentCommandFacade::doBeginTransaction(const String& name) {
            m_commandProcessor.beginGroup(name);
        }
        
        void MapDocumentCommandFacade::doEndTransaction() {
            m_commandProcessor.endGroup();
        }
        
        void MapDocumentCommandFacade::doRollbackTransaction() {
            m_commandProcessor.rollbackGroup();
        }

        bool MapDocumentCommandFacade::doSubmit(Command::Ptr command) {
            return m_commandProcessor.submitCommand(command);
        }

        bool MapDocumentCommandFacade::doSubmitAndStore(UndoableCommand::Ptr command) {
            return m_commandProcessor.submitAndStoreCommand(command);
        }
    }
}
