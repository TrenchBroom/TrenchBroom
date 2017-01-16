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

#include "SelectionTool.h"

#include "CollectionUtils.h"
#include "Preferences.h"
#include "PreferenceManager.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/EditorContext.h"
#include "Model/Entity.h"
#include "Model/Group.h"
#include "Model/HitAdapter.h"
#include "Model/HitQuery.h"
#include "Model/Node.h"
#include "Model/NodeVisitor.h"
#include "Model/PickResult.h"
#include "Renderer/RenderContext.h"
#include "View/InputState.h"
#include "View/Grid.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace View {
        SelectionTool::SelectionTool(MapDocumentWPtr document) :
        ToolControllerBase(),
        Tool(true),
        m_document(document) {}
        
        Tool* SelectionTool::doGetTool() {
            return this;
        }
        
        bool SelectionTool::doMouseClick(const InputState& inputState) {
            if (!handleClick(inputState))
                return false;
            
            MapDocumentSPtr document = lock(m_document);
            const Model::EditorContext& editorContext = document->editorContext();
            if (isFaceClick(inputState)) {
                const Model::Hit& hit = firstHit(inputState, Model::Brush::BrushHit);
                if (hit.isMatch()) {
                    Model::BrushFace* face = Model::hitToFace(hit);
                    if (editorContext.selectable(face)) {
                        if (isMultiClick(inputState)) {
                            const bool objects = document->hasSelectedNodes();
                            if (objects) {
                                const Model::Brush* brush = face->brush();
                                if (brush->selected()) {
                                    document->deselect(face);
                                } else {
                                    Transaction transaction(document, "Select Brush Face");
                                    document->convertToFaceSelection();
                                    document->select(face);
                                }
                            } else {
                                if (face->selected())
                                    document->deselect(face);
                                else
                                    document->select(face);
                            }
                        } else {
                            Transaction transaction(document, "Select Brush Face");
                            document->deselectAll();
                            document->select(face);
                        }
                    }
                } else {
                    document->deselectAll();
                }
            } else {
                const Model::Hit& hit = firstHit(inputState, Model::Group::GroupHit | Model::Entity::EntityHit | Model::Brush::BrushHit);
                if (hit.isMatch()) {
                    Model::Node* node = Model::hitToNode(hit);
                    if (editorContext.selectable(node)) {
                        if (isMultiClick(inputState)) {
                            if (node->selected()) {
                                document->deselect(node);
                            } else {
                                Transaction transaction(document, "Select Object");
                                if (document->hasSelectedBrushFaces())
                                    document->deselectAll();
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
            if (!handleClick(inputState))
                return false;
            
            MapDocumentSPtr document = lock(m_document);
            const Model::EditorContext& editorContext = document->editorContext();
            if (isFaceClick(inputState)) {
                const Model::Hit& hit = firstHit(inputState, Model::Brush::BrushHit);
                if (hit.isMatch()) {
                    Model::BrushFace* face = Model::hitToFace(hit);
                    if (editorContext.selectable(face)) {
                        const Model::Brush* brush = face->brush();
                        if (isMultiClick(inputState)) {
                            document->select(brush->faces());
                        } else {
                            Transaction transaction(document, "Select Brush Faces");
                            document->deselectAll();
                            document->select(brush->faces());
                        }
                    }
                }
            } else {
                const Model::Hit& hit = firstHit(inputState, Model::Group::GroupHit | Model::Brush::BrushHit | Model::Entity::EntityHit);
                if (hit.isMatch()) {
                    if (hit.type() == Model::Group::GroupHit) {
                        Model::Group* group = Model::hitToGroup(hit);
                        if (editorContext.selectable(group))
                            document->openGroup(group);
                    } else {
                        const Model::Node* node = Model::hitToNode(hit);
                        if (editorContext.selectable(node)) {
                            const Model::Node* container = node->parent();
                            const Model::NodeArray& siblings = container->children();
                            if (isMultiClick(inputState)) {
                                document->select(siblings);
                            } else {
                                Transaction transaction(document, "Select Brushes");
                                document->deselectAll();
                                document->select(siblings);
                            }
                        }
                    }
                } else if (document->currentGroup() != NULL) {
                    document->closeGroup();
                }
            }
            
            return true;
        }
        
        bool SelectionTool::handleClick(const InputState& inputState) const {
            if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                return false;
            
            MapDocumentSPtr document = lock(m_document);
            if (!document->editorContext().canChangeSelection())
                return false;
            return true;
        }
        
        bool SelectionTool::isFaceClick(const InputState& inputState) const {
            return inputState.modifierKeysDown(ModifierKeys::MKShift);
        }
        
        bool SelectionTool::isMultiClick(const InputState& inputState) const {
            return inputState.modifierKeysDown(ModifierKeys::MKCtrlCmd);
        }
        
        const Model::Hit& SelectionTool::firstHit(const InputState& inputState, const Model::Hit::HitType type) const {
            return inputState.pickResult().query().pickable().type(type).occluded().first();
        }
        
        void SelectionTool::doMouseScroll(const InputState& inputState) {
            if (inputState.checkModifierKeys(MK_Yes, MK_Yes, MK_No))
                adjustGrid(inputState);
            else if (inputState.checkModifierKeys(MK_Yes, MK_No, MK_No))
                drillSelection(inputState);
        }
        
        void SelectionTool::adjustGrid(const InputState& inputState) {
            const float factor = pref(Preferences::CameraMouseWheelInvert) ? -1.0f : 1.0f;;
            MapDocumentSPtr document = lock(m_document);
            Grid& grid = document->grid();
            if (factor * inputState.scrollY() < 0.0f)
                grid.incSize();
            else if (factor * inputState.scrollY() > 0.0f)
                grid.decSize();
        }
        
        template <typename I>
        I findFirstSelected(I it, I end) {
            while (it != end) {
                Model::Node* node = Model::hitToNode(*it);
                if (node->selected())
                    break;
                ++it;
            }
            return it;
        }
        
        template <typename I>
        std::pair<Model::Node*, Model::Node*> findSelectionPair(I it, I end, const Model::EditorContext& editorContext) {
            static Model::Node* const NullNode = NULL;
            
            const I first = findFirstSelected(it, end);
            if (first == end)
                return std::make_pair(NullNode, NullNode);
            
            I next = first; ++next;
            while (next != end) {
                Model::Node* node = Model::hitToNode(*next);
                if (editorContext.selectable(node))
                    break;
                ++next;
            }
            
            if (next == end)
                return std::make_pair(Model::hitToNode(*first), NullNode);
            return std::make_pair(Model::hitToNode(*first), Model::hitToNode(*next));
        }
        
        void SelectionTool::drillSelection(const InputState& inputState) {
            const Model::Hit::List hits = inputState.pickResult().query().pickable().type(Model::Group::GroupHit | Model::Entity::EntityHit | Model::Brush::BrushHit).occluded().all();
            
            MapDocumentSPtr document = lock(m_document);
            const Model::EditorContext& editorContext = document->editorContext();
            
            const bool forward = (inputState.scrollY() > 0.0f) != (pref(Preferences::CameraMouseWheelInvert));
            const std::pair<Model::Node*, Model::Node*> nodePair = forward ? findSelectionPair(std::begin(hits), std::end(hits), editorContext) : findSelectionPair(hits.rbegin(), hits.rend(), editorContext);
            
            Model::Node* selectedNode = nodePair.first;
            Model::Node* nextNode = nodePair.second;
            if (nextNode != NULL) {
                Transaction transaction(document, "Drill Selection");
                document->deselect(selectedNode);
                document->select(nextNode);
            }
        }
        
        bool SelectionTool::doStartMouseDrag(const InputState& inputState) {
            if (!handleClick(inputState) || !isMultiClick(inputState))
                return false;
            
            MapDocumentSPtr document = lock(m_document);
            const Model::EditorContext& editorContext = document->editorContext();
            
            if (isFaceClick(inputState)) {
                const Model::Hit& hit = firstHit(inputState, Model::Brush::BrushHit);
                if (!hit.isMatch())
                    return false;
                
                Model::BrushFace* face = Model::hitToFace(hit);
                if (editorContext.selectable(face)) {
                    
                    document->beginTransaction("Drag Select Brush Faces");
                    if (document->hasSelection() && !document->hasSelectedBrushFaces())
                        document->deselectAll();
                    
                    if (!face->selected())
                        document->select(face);
                    
                    return true;
                }
            } else {
                const Model::Hit& hit = firstHit(inputState, Model::Group::GroupHit | Model::Entity::EntityHit | Model::Brush::BrushHit);
                if (!hit.isMatch())
                    return false;
                
                Model::Node* node = Model::hitToNode(hit);
                if (editorContext.selectable(node)) {
                    document->beginTransaction("Drag Select Objects");
                    if (document->hasSelection() && !document->hasSelectedNodes())
                        document->deselectAll();
                    
                    if (!node->selected())
                        document->select(node);
                    
                    return true;
                }
            }
            
            return false;
        }
        
        bool SelectionTool::doMouseDrag(const InputState& inputState) {
            MapDocumentSPtr document = lock(m_document);
            const Model::EditorContext& editorContext = document->editorContext();
            if (document->hasSelectedBrushFaces()) {
                const Model::Hit& hit = firstHit(inputState, Model::Brush::BrushHit);
                if (hit.isMatch()) {
                    Model::BrushFace* face = Model::hitToFace(hit);
                    if (!face->selected() && editorContext.selectable(face))
                        document->select(face);
                }
            } else {
                assert(document->hasSelectedNodes());
                const Model::Hit& hit = firstHit(inputState, Model::Group::GroupHit | Model::Entity::EntityHit | Model::Brush::BrushHit);
                if (hit.isMatch()) {
                    Model::Node* node = Model::hitToNode(hit);
                    if (!node->selected() && editorContext.selectable(node))
                        document->select(node);
                }
            }
            return true;
        }
        
        void SelectionTool::doEndMouseDrag(const InputState& inputState) {
            MapDocumentSPtr document = lock(m_document);
            document->commitTransaction();
        }
        
        void SelectionTool::doCancelMouseDrag() {
            MapDocumentSPtr document = lock(m_document);
            document->cancelTransaction();
        }
        
        void SelectionTool::doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {
            MapDocumentSPtr document = lock(m_document);
            const Model::Hit& hit = firstHit(inputState, Model::Group::GroupHit | Model::Entity::EntityHit | Model::Brush::BrushHit);
            if (hit.isMatch() && Model::hitToNode(hit)->selected())
                renderContext.setShowSelectionGuide();
        }
        
        bool SelectionTool::doCancel() {
            // closing the current group is handled in MapViewBase
            return false;
        }
    }
}
