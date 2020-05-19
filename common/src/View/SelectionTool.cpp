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

#include "SelectionTool.h"

#include "Preferences.h"
#include "PreferenceManager.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceHandle.h"
#include "Model/BrushNode.h"
#include "Model/CollectSelectableNodesVisitor.h"
#include "Model/EditorContext.h"
#include "Model/EntityNode.h"
#include "Model/FindGroupVisitor.h"
#include "Model/GroupNode.h"
#include "Model/HitAdapter.h"
#include "Model/HitQuery.h"
#include "Model/Node.h"
#include "Renderer/RenderContext.h"
#include "View/InputState.h"
#include "View/Grid.h"
#include "View/MapDocument.h"

#include <kdl/memory_utils.h>

#include <unordered_set>
#include <vector>

namespace TrenchBroom {
    namespace View {
        SelectionTool::SelectionTool(std::weak_ptr<MapDocument> document) :
        ToolControllerBase(),
        Tool(true),
        m_document(document) {}

        Tool* SelectionTool::doGetTool() {
            return this;
        }

        const Tool* SelectionTool::doGetTool() const {
            return this;
        }

        Model::Node* findOutermostClosedGroupOrNode(Model::Node* node) {
            Model::GroupNode* group = findOutermostClosedGroup(node);
            if (group != nullptr) {
                return group;
            }

            return node;
        }

        bool SelectionTool::doMouseClick(const InputState& inputState) {
            if (!handleClick(inputState)) {
                return false;
            }

            auto document = kdl::mem_lock(m_document);
            const auto& editorContext = document->editorContext();
            if (isFaceClick(inputState)) {
                const auto& hit = firstHit(inputState, Model::BrushNode::BrushHitType);
                if (hit.isMatch()) {
                    auto* brush = Model::hitToBrush(hit);
                    auto* face = Model::hitToFace(hit);
                    if (editorContext.selectable(face)) {
                        if (isMultiClick(inputState)) {
                            const auto objects = document->hasSelectedNodes();
                            if (objects) {
                                if (brush->selected()) {
                                    document->deselect({ brush, face });
                                } else {
                                    Transaction transaction(document, "Select Brush Face");
                                    document->convertToFaceSelection();
                                    document->select({ brush, face });
                                }
                            } else {
                                if (face->selected()) {
                                    document->deselect({ brush, face });
                                } else {
                                    document->select({ brush, face });
                                }
                            }
                        } else {
                            Transaction transaction(document, "Select Brush Face");
                            document->deselectAll();
                            document->select({ brush, face });
                        }
                    }
                } else {
                    document->deselectAll();
                }
            } else {
                const auto& hit = firstHit(inputState, Model::EntityNode::EntityHitType | Model::BrushNode::BrushHitType);
                if (hit.isMatch()) {
                    auto* node = findOutermostClosedGroupOrNode(Model::hitToNode(hit));
                    if (editorContext.selectable(node)) {
                        if (isMultiClick(inputState)) {
                            if (node->selected()) {
                                document->deselect(node);
                            } else {
                                Transaction transaction(document, "Select Object");
                                if (document->hasSelectedBrushFaces()) {
                                    document->deselectAll();
                                }
                                document->select(node);
                            }
                        } else {
                            Transaction transaction(document, "Select Object");
                            document->deselectAll();
                            document->select(node);
                        }
                    }
                } else {
                    document->deselectAll();
                }
            }

            return true;
        }

        bool SelectionTool::doMouseDoubleClick(const InputState& inputState) {
            if (!handleClick(inputState)) {
                return false;
            }

            auto document = kdl::mem_lock(m_document);
            const auto& editorContext = document->editorContext();
            if (isFaceClick(inputState)) {
                const auto& hit = firstHit(inputState, Model::BrushNode::BrushHitType);
                if (hit.isMatch()) {
                    auto* face = Model::hitToFace(hit);
                    if (editorContext.selectable(face)) {
                        auto* brush = Model::hitToBrush(hit);
                        if (isMultiClick(inputState)) {
                            if (document->hasSelectedNodes()) {
                                document->convertToFaceSelection();
                            }
                            document->select(Model::toHandles(brush));
                        } else {
                            Transaction transaction(document, "Select Brush Faces");
                            document->deselectAll();
                            document->select(Model::toHandles(brush));
                        }
                    }
                }
            } else {
                const auto inGroup = document->currentGroup() != nullptr;
                const auto& hit = firstHit(inputState, Model::BrushNode::BrushHitType | Model::EntityNode::EntityHitType);
                if (hit.isMatch()) {
                    const auto hitInGroup = inGroup && hit.isMatch() && Model::hitToNode(hit)->isDescendantOf(document->currentGroup());
                    if (!inGroup || hitInGroup) {
                        // If the hit node is inside a closed group, treat it as a hit on the group insted
                        auto* group = findOutermostClosedGroup(Model::hitToNode(hit));
                        if (group != nullptr) {
                            if (editorContext.selectable(group)) {
                                document->openGroup(group);
                            }
                        } else {
                            const auto* node = Model::hitToNode(hit);
                            if (editorContext.selectable(node)) {
                                const auto* container = node->parent();
                                const auto siblings = collectSelectableChildren(editorContext, container);
                                if (isMultiClick(inputState)) {
                                    if (document->hasSelectedBrushFaces()) {
                                        document->deselectAll();
                                    }
                                    document->select(siblings);
                                } else {
                                    Transaction transaction(document, "Select Brushes");
                                    document->deselectAll();
                                    document->select(siblings);
                                }
                            }
                        }
                    } else if (inGroup) {
                        document->closeGroup();
                    }
                } else if (inGroup) {
                    document->closeGroup();
                }
            }

            return true;
        }

        bool SelectionTool::handleClick(const InputState& inputState) const {
            if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft)) {
                return false;
            }

            auto document = kdl::mem_lock(m_document);
            return document->editorContext().canChangeSelection();
        }

        bool SelectionTool::isFaceClick(const InputState& inputState) const {
            return inputState.modifierKeysDown(ModifierKeys::MKShift);
        }

        bool SelectionTool::isMultiClick(const InputState& inputState) const {
            return inputState.modifierKeysDown(ModifierKeys::MKCtrlCmd);
        }

        const Model::Hit& SelectionTool::firstHit(const InputState& inputState, const Model::HitType::Type type) const {
            return inputState.pickResult().query().pickable().type(type).occluded().first();
        }

        std::vector<Model::Node*> SelectionTool::collectSelectableChildren(const Model::EditorContext& editorContext, const Model::Node* node) const {
            Model::CollectSelectableNodesVisitor collect(editorContext);
            Model::Node::accept(std::begin(node->children()), std::end(node->children()), collect);
            return collect.nodes();
        }

        void SelectionTool::doMouseScroll(const InputState& inputState) {
            if (inputState.checkModifierKeys(MK_Yes, MK_Yes, MK_No)) {
                adjustGrid(inputState);
            } else if (inputState.checkModifierKeys(MK_Yes, MK_No, MK_No)) {
                drillSelection(inputState);
            }
        }

        void SelectionTool::adjustGrid(const InputState& inputState) {
            const auto factor = pref(Preferences::CameraMouseWheelInvert) ? -1.0f : 1.0f;
            auto document = kdl::mem_lock(m_document);
            auto& grid = document->grid();
            if (factor * inputState.scrollY() < 0.0f) {
                grid.incSize();
            } else if (factor * inputState.scrollY() > 0.0f) {
                grid.decSize();
            }
        }

        template <typename I>
        I findFirstSelected(I it, I end) {
            while (it != end) {
                auto* node = *it;
                if (node->selected()) {
                    break;
                }
                ++it;
            }
            return it;
        }

        /**
         * Returns a pair where:
         *  - first is the first node in the given list that's currently selected
         *  - second is the next selectable node in the list
         */
        template <typename I>
        std::pair<Model::Node*, Model::Node*> findSelectionPair(I it, I end, const Model::EditorContext& editorContext) {
            static Model::Node* const NullNode = nullptr;

            const auto first = findFirstSelected(it, end);
            if (first == end) {
                return std::make_pair(NullNode, NullNode);
            }

            auto next = std::next(first);
            while (next != end) {
                auto* node = *next;
                if (editorContext.selectable(node)) {
                    break;
                }
                ++next;
            }

            if (next == end) {
                return std::make_pair(*first, NullNode);
            } else {
                return std::make_pair(*first, *next);
            }
        }

        std::vector<Model::Node*> hitsToNodesWithGroupPicking(const std::vector<Model::Hit>& hits) {
            std::vector<Model::Node*> hitNodes;
            std::unordered_set<Model::Node*> duplicateCheck;

            for (const auto& hit : hits) {
                Model::Node* node = findOutermostClosedGroupOrNode(Model::hitToNode(hit));
                if (!duplicateCheck.insert(node).second) {
                    continue;
                }

                // Note that the order of the input hits are preserved, although duplicates later in the list are dropped
                hitNodes.push_back(node);
            }

            return hitNodes;
        }

        void SelectionTool::drillSelection(const InputState& inputState) {
            const auto hits = inputState.pickResult().query().pickable().type(Model::EntityNode::EntityHitType | Model::BrushNode::BrushHitType).occluded().all();

            // Hits may contain multiple brush/entity hits that are inside closed groups. These need to be converted
            // to group hits using findOutermostClosedGroupOrNode() and multiple hits on the same Group need to be collapsed.
            const std::vector<Model::Node*> hitNodes = hitsToNodesWithGroupPicking(hits);

            auto document = kdl::mem_lock(m_document);
            const auto& editorContext = document->editorContext();

            const auto forward = (inputState.scrollY() > 0.0f) != (pref(Preferences::CameraMouseWheelInvert));
            const auto nodePair = forward ? findSelectionPair(std::begin(hitNodes), std::end(hitNodes), editorContext) : findSelectionPair(hitNodes.rbegin(), hitNodes.rend(), editorContext);

            auto* selectedNode = nodePair.first;
            auto* nextNode = nodePair.second;

            if (nextNode != nullptr) {
                Transaction transaction(document, "Drill Selection");
                document->deselect(selectedNode);
                document->select(nextNode);
            }
        }

        bool SelectionTool::doStartMouseDrag(const InputState& inputState) {
            if (!handleClick(inputState) || !isMultiClick(inputState)) {
                return false;
            }

            auto document = kdl::mem_lock(m_document);
            const auto& editorContext = document->editorContext();

            if (isFaceClick(inputState)) {
                const auto& hit = firstHit(inputState, Model::BrushNode::BrushHitType);
                if (!hit.isMatch()) {
                    return false;
                }

                auto* face = Model::hitToFace(hit);
                if (editorContext.selectable(face)) {
                    document->startTransaction("Drag Select Brush Faces");
                    if (document->hasSelection() && !document->hasSelectedBrushFaces()) {
                        document->deselectAll();
                    }
                    if (!face->selected()) {
                        auto* brush = Model::hitToBrush(hit);
                        document->select({ brush, face });
                    }

                    return true;
                }
            } else {
                const auto& hit = firstHit(inputState, Model::EntityNode::EntityHitType | Model::BrushNode::BrushHitType);
                if (!hit.isMatch()) {
                    return false;
                }

                auto* node = findOutermostClosedGroupOrNode(Model::hitToNode(hit));
                if (editorContext.selectable(node)) {
                    document->startTransaction("Drag Select Objects");
                    if (document->hasSelection() && !document->hasSelectedNodes()) {
                        document->deselectAll();
                    }
                    if (!node->selected()) {
                        document->select(node);
                    }
                    return true;
                }
            }

            return false;
        }

        bool SelectionTool::doMouseDrag(const InputState& inputState) {
            auto document = kdl::mem_lock(m_document);
            const auto& editorContext = document->editorContext();
            if (document->hasSelectedBrushFaces()) {
                const auto& hit = firstHit(inputState, Model::BrushNode::BrushHitType);
                if (hit.isMatch()) {
                    auto* face = Model::hitToFace(hit);
                    if (!face->selected() && editorContext.selectable(face)) {
                        auto* brush = Model::hitToBrush(hit);
                        document->select({ brush, face });
                    }
                }
            } else {
                assert(document->hasSelectedNodes());
                const auto& hit = firstHit(inputState, Model::EntityNode::EntityHitType | Model::BrushNode::BrushHitType);
                if (hit.isMatch()) {
                    auto* node = findOutermostClosedGroupOrNode(Model::hitToNode(hit));
                    if (!node->selected() && editorContext.selectable(node)) {
                        document->select(node);
                    }
                }
            }
            return true;
        }

        void SelectionTool::doEndMouseDrag(const InputState&) {
            auto document = kdl::mem_lock(m_document);
            document->commitTransaction();
        }

        void SelectionTool::doCancelMouseDrag() {
            auto document = kdl::mem_lock(m_document);
            document->cancelTransaction();
        }

        void SelectionTool::doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {
            auto document = kdl::mem_lock(m_document);
            const auto& hit = firstHit(inputState, Model::EntityNode::EntityHitType | Model::BrushNode::BrushHitType);
            if (hit.isMatch()) {
                Model::Node* node = findOutermostClosedGroupOrNode(Model::hitToNode(hit));

                if (node->selected()) {
                    renderContext.setShowSelectionGuide();
                }
            }
        }

        bool SelectionTool::doCancel() {
            // closing the current group is handled in MapViewBase
            return false;
        }
    }
}
