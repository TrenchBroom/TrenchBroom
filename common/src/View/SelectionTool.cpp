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

#include "SelectionTool.h"

#include "CollectionUtils.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/Entity.h"
#include "Model/Group.h"
#include "Model/HitAdapter.h"
#include "Model/HitQuery.h"
#include "Model/Node.h"
#include "Model/NodeVisitor.h"
#include "Model/PickResult.h"
#include "Renderer/RenderContext.h"
#include "View/InputState.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace View {
        SelectionTool::SelectionTool(MapDocumentWPtr document) :
        ToolAdapterBase(),
        Tool(true),
        m_document(document) {}

        Tool* SelectionTool::doGetTool() {
            return this;
        }

        bool SelectionTool::doMouseClick(const InputState& inputState) {
            if (!handleClick(inputState))
                return false;
            
            MapDocumentSPtr document = lock(m_document);
            if (isFaceClick(inputState)) {
                const Model::Hit& hit = firstHit(inputState, Model::Brush::BrushHit);
                if (hit.isMatch()) {
                    Model::BrushFace* face = Model::hitToFace(hit);
                    if (isMultiClick(inputState)) {
                        const bool objects = document->hasSelectedNodes();
                        if (objects) {
                            const Model::Brush* brush = face->brush();
                            if (brush->selected()) {
                                document->deselect(face);
                            } else {
                                Transaction transaction(document, "Select face");
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
                        Transaction transaction(document, "Select face");
                        document->deselectAll();
                        document->select(face);
                    }
                } else {
                    document->deselectAll();
                }
            } else {
                const Model::Hit& hit = firstHit(inputState, Model::Group::GroupHit | Model::Entity::EntityHit | Model::Brush::BrushHit);
                if (hit.isMatch()) {
                    Model::Node* node = Model::hitToNode(hit);
                    if (isMultiClick(inputState)) {
                        if (node->selected())
                            document->deselect(node);
                        else
                            document->select(node);
                    } else {
                        Transaction transaction(document, "Select object");
                        document->deselectAll();
                        document->select(node);
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
            if (isFaceClick(inputState)) {
                const Model::Hit& hit = firstHit(inputState, Model::Brush::BrushHit);
                if (hit.isMatch()) {
                    Model::BrushFace* face = Model::hitToFace(hit);
                    const Model::Brush* brush = face->brush();
                    if (isMultiClick(inputState)) {
                        document->select(brush->faces());
                    } else {
                        Transaction transaction(document, "Select faces");
                        document->deselectAll();
                        document->select(brush->faces());
                    }
                }
            } else {
                const Model::Hit& hit = firstHit(inputState, Model::Brush::BrushHit);
                if (hit.isMatch()) {
                    const Model::Brush* brush = Model::hitToBrush(hit);
                    const Model::Node* container = brush->container();
                    const Model::NodeList& siblings = container->children();
                    if (isMultiClick(inputState)) {
                        document->select(siblings);
                    } else {
                        Transaction transaction(document, "Select brushes");
                        document->deselectAll();
                        document->select(siblings);
                    }
                }
            }
            
            return true;
        }
        
        bool SelectionTool::handleClick(const InputState& inputState) const {
            return inputState.mouseButtonsPressed(MouseButtons::MBLeft);
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

        bool SelectionTool::doStartMouseDrag(const InputState& inputState) {
            if (!handleClick(inputState) || !isMultiClick(inputState))
                return false;
            
            MapDocumentSPtr document = lock(m_document);
            if (isFaceClick(inputState)) {
                const Model::Hit& hit = firstHit(inputState, Model::Brush::BrushHit);
                if (!hit.isMatch())
                    return false;
                
                document->beginTransaction("Drag select faces");
                if (document->hasSelection() && !document->hasSelectedBrushFaces())
                    document->deselectAll();
                
                Model::BrushFace* face = Model::hitToFace(hit);
                if (!face->selected())
                    document->select(face);
                
                return true;
            } else {
                const Model::Hit& hit = firstHit(inputState, Model::Group::GroupHit | Model::Entity::EntityHit | Model::Brush::BrushHit);
                if (!hit.isMatch())
                    return false;
                
                document->beginTransaction("Drag select objects");
                if (document->hasSelection() && !document->hasSelectedNodes())
                    document->deselectAll();
                
                Model::Node* node = Model::hitToNode(hit);
                if (!node->selected())
                    document->select(node);
                
                return true;
            }
        }
        
        bool SelectionTool::doMouseDrag(const InputState& inputState) {
            MapDocumentSPtr document = lock(m_document);
            if (document->hasSelectedBrushFaces()) {
                const Model::Hit& hit = firstHit(inputState, Model::Brush::BrushHit);
                if (hit.isMatch()) {
                    Model::BrushFace* face = Model::hitToFace(hit);
                    if (!face->selected())
                        document->select(face);
                }
            } else {
                assert(document->hasSelectedNodes());
                const Model::Hit& hit = firstHit(inputState, Model::Group::GroupHit | Model::Entity::EntityHit | Model::Brush::BrushHit);
                if (hit.isMatch()) {
                    Model::Node* node = Model::hitToNode(hit);
                    if (!node->selected())
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
            MapDocumentSPtr document = lock(m_document);
            if (document->hasSelection()) {
                document->deselectAll();
                return true;
            }
            return false;
        }
    }
}
