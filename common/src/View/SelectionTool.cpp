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
#include "Model/BrushFace.h"
#include "Model/BrushNode.h"
#include "Model/EditorContext.h"
#include "Model/GroupNode.h"
#include "Model/Hit.h"
#include "Model/HitAdapter.h"
#include "Model/HitFilter.h"
#include "Model/ModelUtils.h"
#include "Model/Node.h"
#include "Renderer/RenderContext.h"
#include "View/DragTracker.h"
#include "View/InputState.h"
#include "View/Grid.h"
#include "View/MapDocument.h"

#include <kdl/memory_utils.h>

#include <algorithm>
#include <unordered_set>
#include <vector>

namespace TrenchBroom {
    namespace View {
        Model::Node* findOutermostClosedGroupOrNode(Model::Node* node) {
            Model::GroupNode* group = findOutermostClosedGroup(node);
            if (group != nullptr) {
                return group;
            }

            return node;
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

        static bool isFaceClick(const InputState& inputState) {
            return inputState.modifierKeysDown(ModifierKeys::MKShift);
        }

        static bool isMultiClick(const InputState& inputState) {
            return inputState.modifierKeysDown(ModifierKeys::MKCtrlCmd);
        }

        static const Model::Hit& firstHit(const InputState& inputState, const Model::HitType::Type typeMask) {
            using namespace Model::HitFilters;
            return inputState.pickResult().first(type(typeMask));
        }

        static std::vector<Model::Node*> collectSelectableChildren(const Model::EditorContext& editorContext, const Model::Node* node) {
            return Model::collectSelectableNodes(node->children(), editorContext);
        }

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

        bool SelectionTool::doMouseClick(const InputState& inputState) {
            if (!handleClick(inputState)) {
                return false;
            }

            auto document = kdl::mem_lock(m_document);
            const auto& editorContext = document->editorContext();
            if (isFaceClick(inputState)) {
                const auto& hit = firstHit(inputState, Model::BrushNode::BrushHitType);
                if (const auto faceHandle = Model::hitToFaceHandle(hit)) {
                    const auto* brush = faceHandle->node();
                    const auto& face = faceHandle->face();;
                    if (editorContext.selectable(brush, face)) {
                        if (isMultiClick(inputState)) {
                            const auto objects = document->hasSelectedNodes();
                            if (objects) {
                                if (brush->selected()) {
                                    document->deselect(*faceHandle);
                                } else {
                                    Transaction transaction(document, "Select Brush Face");
                                    document->convertToFaceSelection();
                                    document->select(*faceHandle);
                                }
                            } else {
                                if (face.selected()) {
                                    document->deselect(*faceHandle);
                                } else {
                                    document->select(*faceHandle);
                                }
                            }
                        } else {
                            Transaction transaction(document, "Select Brush Face");
                            document->deselectAll();
                            document->select(*faceHandle);
                        }
                    }
                } else {
                    document->deselectAll();
                }
            } else {
                const auto& hit = firstHit(inputState, Model::nodeHitType());
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
                if (const auto faceHandle = Model::hitToFaceHandle(hit)) {
                    auto* brush = faceHandle->node();
                    const auto& face = faceHandle->face();
                    if (editorContext.selectable(brush, face)) {
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
                const auto& hit = firstHit(inputState, Model::nodeHitType());
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
            if (!inputState.checkModifierKeys(MK_DontCare, MK_No, MK_DontCare)) {
                return false;
            }

            auto document = kdl::mem_lock(m_document);
            return document->editorContext().canChangeSelection();
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

        /**
         * Returns a pair where:
         *  - first is the first node in the given list that's currently selected
         *  - second is the next selectable node in the list
         */
        template <typename I>
        std::pair<Model::Node*, Model::Node*> findSelectionPair(I it, I end, const Model::EditorContext& editorContext) {
            const auto first = std::find_if(it, end, [](const auto* node) { return node->selected(); });
            if (first == end) {
                return {nullptr, nullptr};
            }

            const auto next = std::find_if(std::next(first), end, [&](const auto* node) { return editorContext.selectable(node); });
            if (next == end) {
                return {*first, nullptr};
            }

            return {*first, *next};
        }

        void SelectionTool::drillSelection(const InputState& inputState) {
            using namespace Model::HitFilters;
            const auto hits = inputState.pickResult().all(type(Model::nodeHitType()));

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

        namespace {
            class PaintSelectionDragTracker : public DragTracker {
            private:
                std::shared_ptr<MapDocument> m_document;
            public:
                PaintSelectionDragTracker(std::shared_ptr<MapDocument> document) :
                m_document{std::move(document)} {}

                bool drag(const InputState& inputState) override {
                    const auto& editorContext = m_document->editorContext();
                    if (m_document->hasSelectedBrushFaces()) {
                        const auto& hit = firstHit(inputState, Model::BrushNode::BrushHitType);
                        if (const auto faceHandle = Model::hitToFaceHandle(hit)) {
                            const auto* brush = faceHandle->node();
                            const auto& face = faceHandle->face();
                            if (!face.selected() && editorContext.selectable(brush, face)) {
                                m_document->select(*faceHandle);
                            }
                        }
                    } else {
                        assert(m_document->hasSelectedNodes());
                        const auto& hit = firstHit(inputState, Model::nodeHitType());
                        if (hit.isMatch()) {
                            auto* node = findOutermostClosedGroupOrNode(Model::hitToNode(hit));
                            if (!node->selected() && editorContext.selectable(node)) {
                                m_document->select(node);
                            }
                        }
                    }
                    return true;
                }

                void end(const InputState&) override {
                    m_document->commitTransaction();
                }

                void cancel() override {
                    m_document->cancelTransaction();
                }
            };
        }

        std::unique_ptr<DragTracker> SelectionTool::acceptMouseDrag(const InputState& inputState) {
            if (!handleClick(inputState) || !isMultiClick(inputState)) {
                return nullptr;
            }

            auto document = kdl::mem_lock(m_document);
            const auto& editorContext = document->editorContext();

            if (isFaceClick(inputState)) {
                const auto& hit = firstHit(inputState, Model::BrushNode::BrushHitType);
                if (const auto faceHandle = Model::hitToFaceHandle(hit)) {
                    const auto* brush = faceHandle->node();
                    const auto& face = faceHandle->face();
                    if (editorContext.selectable(brush, face)) {
                        document->startTransaction("Drag Select Brush Faces");
                        if (document->hasSelection() && !document->hasSelectedBrushFaces()) {
                            document->deselectAll();
                        }
                        if (!face.selected()) {
                            document->select(*faceHandle);
                        }

                        return std::make_unique<PaintSelectionDragTracker>(std::move(document));
                    }
                }
            } else {
                const auto& hit = firstHit(inputState, Model::nodeHitType());
                if (!hit.isMatch()) {
                    return nullptr;
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

                    return std::make_unique<PaintSelectionDragTracker>(std::move(document));
                }
            }

            return nullptr;
        }

        void SelectionTool::doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {
            auto document = kdl::mem_lock(m_document);
            const auto& hit = firstHit(inputState, Model::nodeHitType());
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
