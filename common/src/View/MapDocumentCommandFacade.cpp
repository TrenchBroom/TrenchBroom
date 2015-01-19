/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
#include "Model/CollectSelectableBrushFacesVisitor.h"
#include "Model/CollectSelectableNodesVisitor.h"
#include "Model/EditorContext.h"
#include "Model/Entity.h"
#include "Model/Group.h"
#include "Model/Issue.h"
#include "Model/Snapshot.h"
#include "Model/TransformObjectVisitor.h"
#include "Model/World.h"
#include "View/Selection.h"

namespace TrenchBroom {
    namespace View {
        class NodeChangeNotifier {
        public:
            typedef Notifier1<const Model::NodeList&> Notifier;
        private:
            Notifier& m_didChange;
            const Model::NodeList& m_nodes;
        public:
            NodeChangeNotifier(Notifier& willChange, Notifier& didChange, const Model::NodeList& nodes) :
            m_didChange(didChange),
            m_nodes(nodes) {
                willChange(nodes);
            }
            
            ~NodeChangeNotifier() {
                m_didChange(m_nodes);
            }
        };
        
        class NodeRemoveNotifier {
        public:
            typedef Notifier1<const Model::NodeList&> Notifier;
        private:
            Notifier& m_wereRemoved;
            const Model::NodeList& m_nodes;
        public:
            NodeRemoveNotifier(Notifier& willBeRemoved, Notifier& wereRemoved, const Model::NodeList& nodes) :
            m_wereRemoved(wereRemoved),
            m_nodes(nodes) {
                willBeRemoved(nodes);
            }
            
            ~NodeRemoveNotifier() {
                m_wereRemoved(m_nodes);
            }
        };
        
        MapDocumentSPtr MapDocumentCommandFacade::newMapDocument() {
            return MapDocumentSPtr(new MapDocumentCommandFacade());
        }

        MapDocumentCommandFacade::MapDocumentCommandFacade() :
        m_commandProcessor(this) {
            m_commandProcessor.commandDoNotifier.addObserver(commandDoNotifier);
            m_commandProcessor.commandDoneNotifier.addObserver(commandDoneNotifier);
            m_commandProcessor.commandDoFailedNotifier.addObserver(commandDoFailedNotifier);
            m_commandProcessor.commandUndoNotifier.addObserver(commandUndoNotifier);
            m_commandProcessor.commandUndoneNotifier.addObserver(commandUndoneNotifier);
            m_commandProcessor.commandUndoFailedNotifier.addObserver(commandUndoFailedNotifier);
        }

        void MapDocumentCommandFacade::performSelect(const Model::NodeList& nodes) {
            selectionWillChangeNotifier();

            Model::NodeList selected;
            selected.reserve(nodes.size());
            
            Model::CollectNodesWithDescendantSelectionCountVisitor visitor(0);

            Model::NodeList::const_iterator it, end;
            for (it = nodes.begin(), end = nodes.end(); it != end; ++it) {
                Model::Node* node = *it;
                if (!node->selected() && m_editorContext->selectable(node)) {
                    node->escalate(visitor);
                    node->select();
                    selected.push_back(node);
                }
            }

            const Model::NodeList& partiallySelected = visitor.nodes();
            
            m_selectedNodes.addNodes(selected);
            m_partiallySelectedNodes.addNodes(partiallySelected);
            
            Selection selection;
            selection.addSelectedNodes(selected);
            selection.addPartiallySelectedNodes(partiallySelected);
            
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
            
            Model::NodeList deselected;
            deselected.reserve(nodes.size());
            
            Model::CollectNodesWithDescendantSelectionCountVisitor visitor(0);
            
            Model::NodeList::const_iterator it, end;
            for (it = nodes.begin(), end = nodes.end(); it != end; ++it) {
                Model::Node* node = *it;
                if (node->selected()) {
                    node->deselect();
                    deselected.push_back(node);
                    node->escalate(visitor);
                }
            }
            
            const Model::NodeList& partiallyDeselected = visitor.nodes();
            
            m_selectedNodes.removeNodes(deselected);
            m_selectedNodes.removeNodes(partiallyDeselected);
            
            Selection selection;
            selection.addDeselectedNodes(deselected);
            selection.addPartiallyDeselectedNodes(partiallyDeselected);
            
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

            Model::NodeList::const_iterator it, end;
            for (it = m_selectedNodes.begin(), end = m_selectedNodes.end(); it != end; ++it) {
                Model::Node* node = *it;
                node->deselect();
            }
            
            Selection selection;
            selection.addDeselectedNodes(m_selectedNodes.nodes());
            selection.addPartiallyDeselectedNodes(m_partiallySelectedNodes.nodes());

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
            NodeChangeNotifier notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            
            Model::NodeList addedNodes;
            Model::ParentChildrenMap::const_iterator it, end;
            for (it = nodes.begin(), end = nodes.end(); it != end; ++it) {
                Model::Node* parent = it->first;
                const Model::NodeList& children = it->second;
                parent->addChildren(children);
                VectorUtils::append(addedNodes, children);
            }
            
            setEntityDefinitions(addedNodes);
            setEntityModels(addedNodes);
            setTextures(addedNodes);

            nodesWereAddedNotifier(addedNodes);
            return addedNodes;
        }
        
        Model::ParentChildrenMap MapDocumentCommandFacade::performRemoveNodes(const Model::NodeList& nodes) {
            Model::ParentChildrenMap removedNodes = parentChildrenMap(nodes);
            addEmptyNodes(removedNodes);
            
            const Model::NodeList parents = collectParents(removedNodes);
            NodeChangeNotifier notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            
            const Model::NodeList allChildren = collectChildren(removedNodes);
            NodeRemoveNotifier notifyChildren(nodesWillBeRemovedNotifier, nodesWereRemovedNotifier, allChildren);
            
            Model::ParentChildrenMap::const_iterator it, end;
            for (it = removedNodes.begin(), end = removedNodes.end(); it != end; ++it) {
                Model::Node* parent = it->first;
                const Model::NodeList& children = it->second;
                parent->removeChildren(children.begin(), children.end());
            }
            
            return removedNodes;
        }

        Model::ParentChildrenMap MapDocumentCommandFacade::performReparentNodes(const Model::ParentChildrenMap& nodes) {
            Model::NodeList nodesToNotify;
            Model::CollectUniqueNodesVisitor visitor;

            Model::ParentChildrenMap::const_iterator pcIt, pcEnd;
            Model::NodeList::const_iterator nIt, nEnd;
            
            for (pcIt = nodes.begin(), pcEnd = nodes.end(); pcIt != pcEnd; ++pcIt) {
                Model::Node* newParent = pcIt->first;
                newParent->acceptAndEscalate(visitor);
                
                const Model::NodeList& children = pcIt->second;
                Model::Node::escalate(children.begin(), children.end(), visitor);
                VectorUtils::append(nodesToNotify, children);
            }
            
            const Model::NodeList parentsToNodify = visitor.nodes();
            NodeChangeNotifier notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parentsToNodify);
            NodeChangeNotifier notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodesToNotify);

            Model::ParentChildrenMap result;
            for (pcIt = nodes.begin(), pcEnd = nodes.end(); pcIt != pcEnd; ++pcIt) {
                Model::Node* newParent = pcIt->first;
                const Model::NodeList& children = pcIt->second;
                
                for (nIt = children.begin(), nEnd = children.end(); nIt != nEnd; ++nIt) {
                    Model::Node* child = *nIt;
                    Model::Node* oldParent = child->parent();
                    assert(oldParent != NULL);
                    
                    result[oldParent].push_back(child);
                    oldParent->removeChild(child);
                    newParent->addChild(child);
                }
            }
            
            return result;
        }

        void MapDocumentCommandFacade::performTransform(const Mat4x4& transform, const bool lockTextures) {
            const Model::NodeList& nodes = m_selectedNodes.nodes();
            const Model::NodeList parents = collectParents(nodes);
            
            NodeChangeNotifier notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            NodeChangeNotifier notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            
            Model::TransformObjectVisitor visitor(transform, lockTextures, m_worldBounds);
            Model::Node::accept(nodes.begin(), nodes.end(), visitor);
            
            invalidateSelectionBounds();
        }

        Model::EntityAttribute::Map MapDocumentCommandFacade::performSetAttribute(const Model::AttributeName& name, const Model::AttributeValue& value) {
            const Model::AttributableNodeList attributableNodes = allSelectedAttributableNodes();
            const Model::NodeList nodes(attributableNodes.begin(), attributableNodes.end());
            const Model::NodeList parents = collectParents(nodes.begin(), nodes.end());

            NodeChangeNotifier notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            NodeChangeNotifier notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            
            static const Model::AttributeValue DefaultValue = "";
            Model::EntityAttribute::Map snapshot;
            
            Model::AttributableNodeList::const_iterator it, end;
            for (it = attributableNodes.begin(), end = attributableNodes.end(); it != end; ++it) {
                Model::AttributableNode* node = *it;
                
                const Model::AttributeValue& oldValue = node->attribute(name, DefaultValue);
                node->addOrUpdateAttribute(name, value);
                
                if (oldValue != DefaultValue)
                    snapshot[node] = Model::EntityAttribute(name, oldValue);
            }
            
            setEntityDefinitions(nodes);

            return snapshot;
        }
        
        Model::EntityAttribute::Map MapDocumentCommandFacade::performRemoveAttribute(const Model::AttributeName& name) {
            const Model::AttributableNodeList attributableNodes = allSelectedAttributableNodes();
            const Model::NodeList nodes(attributableNodes.begin(), attributableNodes.end());
            const Model::NodeList parents = collectParents(nodes.begin(), nodes.end());
            
            NodeChangeNotifier notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            NodeChangeNotifier notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            
            static const Model::AttributeValue DefaultValue = "";
            Model::EntityAttribute::Map snapshot;
            
            Model::AttributableNodeList::const_iterator it, end;
            for (it = attributableNodes.begin(), end = attributableNodes.end(); it != end; ++it) {
                Model::AttributableNode* node = *it;
                
                const Model::AttributeValue& oldValue = node->attribute(name, DefaultValue);
                node->removeAttribute(name);
                
                if (oldValue != DefaultValue)
                    snapshot[node] = Model::EntityAttribute(name, oldValue);
            }
            
            setEntityDefinitions(nodes);

            return snapshot;
        }
        
        Model::EntityAttribute::Map MapDocumentCommandFacade::performConvertColorRange(const Model::AttributeName& name, Model::ColorRange::Type colorRange) {
            const Model::AttributableNodeList attributableNodes = allSelectedAttributableNodes();
            const Model::NodeList nodes(attributableNodes.begin(), attributableNodes.end());
            const Model::NodeList parents = collectParents(nodes.begin(), nodes.end());
            
            NodeChangeNotifier notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            NodeChangeNotifier notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            
            static const Model::AttributeValue DefaultValue = "";
            Model::EntityAttribute::Map snapshot;

            Model::AttributableNodeList::const_iterator it, end;
            for (it = attributableNodes.begin(), end = attributableNodes.end(); it != end; ++it) {
                Model::AttributableNode* node = *it;
                
                const Model::AttributeValue& oldValue = node->attribute(name, DefaultValue);
                if (oldValue != DefaultValue) {
                    snapshot[node] = Model::EntityAttribute(name, oldValue);
                    const Model::AttributeValue newValue = Model::convertEntityColor(oldValue, colorRange);
                    node->addOrUpdateAttribute(name, newValue);
                }
            }
            
            return snapshot;
        }

        void MapDocumentCommandFacade::performRenameAttribute(const Model::AttributeName& oldName, const Model::AttributeName& newName) {
            const Model::AttributableNodeList attributableNodes = allSelectedAttributableNodes();
            const Model::NodeList nodes(attributableNodes.begin(), attributableNodes.end());
            const Model::NodeList parents = collectParents(nodes.begin(), nodes.end());
            
            NodeChangeNotifier notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            NodeChangeNotifier notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            
            Model::AttributableNodeList::const_iterator it, end;
            for (it = attributableNodes.begin(), end = attributableNodes.end(); it != end; ++it) {
                Model::AttributableNode* node = *it;
                node->renameAttribute(oldName, newName);
            }

            setEntityDefinitions(nodes);
        }
        
        void MapDocumentCommandFacade::restoreAttributes(const Model::EntityAttribute::Map& attributes) {
            const Model::AttributableNodeList attributableNodes = MapUtils::keyList(attributes);
            const Model::NodeList nodes(attributableNodes.begin(), attributableNodes.end());
            
            const Model::NodeList parents = collectParents(nodes.begin(), nodes.end());
            NodeChangeNotifier notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            NodeChangeNotifier notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);

            Model::EntityAttribute::Map::const_iterator it, end;
            for (it = attributes.begin(), end = attributes.end(); it != end; ++it) {
                Model::AttributableNode* node = it->first;
                const Model::EntityAttribute& attr = it->second;
                
                assert(node->selected() || node->descendantSelected());
                node->addOrUpdateAttribute(attr.name(), attr.value());
            }

            setEntityDefinitions(nodes);
        }

        bool MapDocumentCommandFacade::performResizeBrushes(const Model::BrushFaceList& faces, const Vec3& delta) {
            Model::NodeList nodes;
            
            Model::BrushFaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                Model::BrushFace* face = *it;
                Model::Brush* brush = face->brush();
                assert(brush->selected());
                
                if (!brush->canMoveBoundary(m_worldBounds, face, delta))
                    return false;
                
                nodes.push_back(brush);
            }
            
            const Model::NodeList parents = collectParents(nodes.begin(), nodes.end());
            NodeChangeNotifier notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            NodeChangeNotifier notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            
            for (it = faces.begin(), faces.end(); it != end; ++it) {
                Model::BrushFace* face = *it;
                Model::Brush* brush = face->brush();
                brush->moveBoundary(m_worldBounds, face, delta, textureLock());
            }
            
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
            const Model::BrushFaceList& faces = selectedBrushFaces();
            request.evaluate(faces);
            setTextures(faces);
            brushFacesDidChangeNotifier(faces);
        }

        Model::Snapshot* MapDocumentCommandFacade::performFindPlanePoints() {
            const Model::BrushList& brushes = m_selectedNodes.brushes();
            Model::Snapshot* snapshot = new Model::Snapshot(brushes.begin(), brushes.end());
            
            const Model::NodeList nodes(brushes.begin(), brushes.end());
            const Model::NodeList parents = collectParents(nodes);
            
            NodeChangeNotifier notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            NodeChangeNotifier notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            
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
            
            NodeChangeNotifier notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            NodeChangeNotifier notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);

            Vec3::List newVertexPositions;
            Model::BrushVerticesMap::const_iterator it, end;
            for (it = vertices.begin(), end = vertices.end(); it != end; ++it) {
                Model::Brush* brush = it->first;
                const Vec3::List& oldPositions = it->second;
                const Vec3::List newPositions = brush->snapVertices(m_worldBounds, oldPositions, snapTo);
                VectorUtils::append(newVertexPositions, newPositions);
            }
            
            return newVertexPositions;
        }

        Vec3::List MapDocumentCommandFacade::performMoveVertices(const Model::BrushVerticesMap& vertices, const Vec3& delta) {
            const Model::NodeList& nodes = m_selectedNodes.nodes();
            const Model::NodeList parents = collectParents(nodes);
            
            NodeChangeNotifier notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            NodeChangeNotifier notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            
            Vec3::List newVertexPositions;
            Model::BrushVerticesMap::const_iterator it, end;
            for (it = vertices.begin(), end = vertices.end(); it != end; ++it) {
                Model::Brush* brush = it->first;
                const Vec3::List& oldPositions = it->second;
                const Vec3::List newPositions = brush->moveVertices(m_worldBounds, oldPositions, delta);
                VectorUtils::append(newVertexPositions, newPositions);
            }
            
            return newVertexPositions;
        }

        Edge3::List MapDocumentCommandFacade::performMoveEdges(const Model::BrushEdgesMap& edges, const Vec3& delta) {
            const Model::NodeList& nodes = m_selectedNodes.nodes();
            const Model::NodeList parents = collectParents(nodes);
            
            NodeChangeNotifier notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            NodeChangeNotifier notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            
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
            
            NodeChangeNotifier notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            NodeChangeNotifier notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            
            Polygon3::List newFacePositions;
            Model::BrushFacesMap::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                Model::Brush* brush = it->first;
                const Polygon3::List& oldPositions = it->second;
                const Polygon3::List newPositions = brush->moveFaces(m_worldBounds, oldPositions, delta);
                VectorUtils::append(newFacePositions, newPositions);
            }
            
            return newFacePositions;
        }

        Vec3::List MapDocumentCommandFacade::performSplitEdges(const Model::BrushEdgesMap& edges, const Vec3& delta) {
            const Model::NodeList& nodes = m_selectedNodes.nodes();
            const Model::NodeList parents = collectParents(nodes);
            
            NodeChangeNotifier notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            NodeChangeNotifier notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            
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
            
            return newVertexPositions;
        }

        Vec3::List MapDocumentCommandFacade::performSplitFaces(const Model::BrushFacesMap& faces, const Vec3& delta) {
            const Model::NodeList& nodes = m_selectedNodes.nodes();
            const Model::NodeList parents = collectParents(nodes);
            
            NodeChangeNotifier notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            NodeChangeNotifier notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            
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
            
            return newVertexPositions;
        }

        void MapDocumentCommandFacade::performRebuildBrushGeometry(const Model::BrushList& brushes) {
            const Model::NodeList nodes = VectorUtils::cast<Model::Node*>(brushes);
            const Model::NodeList parents = collectParents(nodes);
            
            NodeChangeNotifier notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            NodeChangeNotifier notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            
            Model::BrushList::const_iterator it, end;
            for (it = brushes.begin(), end = brushes.end(); it != end; ++it) {
                Model::Brush* brush = *it;
                brush->rebuildGeometry(m_worldBounds);
            }
        }

        void MapDocumentCommandFacade::restoreSnapshot(Model::Snapshot* snapshot) {
            const Model::NodeList& nodes = m_selectedNodes.nodes();
            const Model::NodeList parents = collectParents(nodes);
            
            NodeChangeNotifier notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            NodeChangeNotifier notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            
            snapshot->restoreNodes(m_worldBounds);

            invalidateSelectionBounds();
        }

        void MapDocumentCommandFacade::performSetEntityDefinitionFile(const Assets::EntityDefinitionFileSpec& spec) {
            NodeChangeNotifier notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, Model::NodeList(1, m_world));
            m_world->addOrUpdateAttribute(Model::AttributeNames::EntityDefinitions, spec.asString());
            entityDefinitionsDidChangeNotifier();
        }

        void MapDocumentCommandFacade::performAddExternalTextureCollections(const StringList& names) {
            NodeChangeNotifier notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, Model::NodeList(1, m_world));
            addExternalTextureCollections(names);
            setTextures();
            updateExternalTextureCollectionProperty();
            textureCollectionsDidChangeNotifier();
        }
        
        void MapDocumentCommandFacade::performRemoveExternalTextureCollections(const StringList& names) {
            NodeChangeNotifier notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, Model::NodeList(1, m_world));
            unsetTextures();
            
            StringList::const_iterator it, end;
            for (it = names.begin(), end = names.end(); it != end; ++it) {
                const String& name = *it;
                m_textureManager->removeExternalTextureCollection(name);
            }
            
            setTextures();
            updateExternalTextureCollectionProperty();
            textureCollectionsDidChangeNotifier();
        }
        
        void MapDocumentCommandFacade::performMoveExternalTextureCollectionUp(const String& name) {
            NodeChangeNotifier notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, Model::NodeList(1, m_world));
            m_textureManager->moveExternalTextureCollectionUp(name);
            setTextures();
            updateExternalTextureCollectionProperty();
            textureCollectionsDidChangeNotifier();
        }
        
        void MapDocumentCommandFacade::performMoveExternalTextureCollectionDown(const String& name) {
            NodeChangeNotifier notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, Model::NodeList(1, m_world));
            m_textureManager->moveExternalTextureCollectionDown(name);
            setTextures();
            updateExternalTextureCollectionProperty();
            textureCollectionsDidChangeNotifier();
        }

        void MapDocumentCommandFacade::performSetMods(const StringList& mods) {
            NodeChangeNotifier notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, Model::NodeList(1, m_world));
            unsetEntityDefinitions();
            m_world->addOrUpdateAttribute(Model::AttributeNames::Mods, StringUtils::join(mods, ";"));
            setEntityDefinitions();
            modsDidChangeNotifier();
        }

        Model::NodeList MapDocumentCommandFacade::collectParents(const Model::NodeList& nodes) const {
            return collectParents(nodes.begin(), nodes.end());
        }
        
        Model::NodeList MapDocumentCommandFacade::collectParents(const Model::ParentChildrenMap& nodes) const {
            Model::CollectUniqueNodesVisitor visitor;
            Model::ParentChildrenMap::const_iterator it, end;
            for (it = nodes.begin(), end = nodes.end(); it != end; ++it) {
                Model::Node* parent = it->first;
                parent->acceptAndEscalate(visitor);
            }
            return visitor.nodes();
        }

        Model::NodeList MapDocumentCommandFacade::collectChildren(const Model::ParentChildrenMap& nodes) const {
            Model::NodeList result;
            Model::ParentChildrenMap::const_iterator it, end;
            for (it = nodes.begin(), end = nodes.end(); it != end; ++it) {
                const Model::NodeList& children = it->second;
                VectorUtils::append(result, children);
            }
            return result;
        }

        Model::ParentChildrenMap MapDocumentCommandFacade::parentChildrenMap(const Model::NodeList& nodes) const {
            Model::ParentChildrenMap result;
            
            Model::NodeList::const_iterator it, end;
            for (it = nodes.begin(), end = nodes.end(); it != end; ++it) {
                Model::Node* node = *it;
                Model::Node* parent = node->parent();
                assert(parent != NULL);
                result[parent].push_back(node);
            }
            
            return result;
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

        bool MapDocumentCommandFacade::doSubmit(UndoableCommand* command) {
            return m_commandProcessor.submitAndStoreCommand(command);
        }
    }
}
