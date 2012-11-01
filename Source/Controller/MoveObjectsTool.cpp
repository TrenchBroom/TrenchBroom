/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "MoveObjectsTool.h"

#include "Controller/Command.h"
#include "Controller/MoveObjectsCommand.h"
#include "Model/EditStateManager.h"
#include "Model/MapObject.h"
#include "Model/Picker.h"
#include "View/DocumentViewHolder.h"
#include "Utility/Console.h"
#include "Utility/Grid.h"

namespace TrenchBroom {
    namespace Controller {
        void MoveObjectsTool::updateHits(InputEvent& event) {
            Model::EditStateManager& editStateManager = documentViewHolder().document().editStateManager();
            if (editStateManager.selectionMode() == Model::EditStateManager::SMNone ||
                editStateManager.selectionMode() == Model::EditStateManager::SMFaces)
                return;
            
            Model::MoveObjectsHandleHit* hit = m_handle.pick(event.ray);
            if (hit != NULL)
                event.pickResult->add(*hit);
        }
        
        bool MoveObjectsTool::handleMouseMoved(InputEvent& event) {
            if (m_handleFigure != NULL)
                updateViews();
            return false;
        }

        bool MoveObjectsTool::handleBeginPlaneDrag(InputEvent& event, Plane& dragPlane, Vec3f& initialDragPoint) {
            if (event.mouseButtons != MouseButtons::MBLeft ||
                event.modifierKeys() != ModifierKeys::MKNone)
                return false;

            Model::EditStateManager& editStateManager = documentViewHolder().document().editStateManager();
            if (editStateManager.selectionMode() == Model::EditStateManager::SMNone ||
                editStateManager.selectionMode() == Model::EditStateManager::SMFaces)
                return false;

            Model::MoveObjectsHandleHit* hit = static_cast<Model::MoveObjectsHandleHit*>(event.pickResult->first(Model::HitType::MoveObjectsHandleHit, true, documentViewHolder().view().filter()));
            if (hit == NULL)
                return false;

            switch (hit->hitArea()) {
                case Model::MoveObjectsHandleHit::HAXAxis:
                    dragPlane = Plane::planeContainingVector(hit->hitPoint(), Vec3f::PosX, event.ray.origin);
                    m_restrictToAxis = RXAxis;
                    break;
                case Model::MoveObjectsHandleHit::HAYAxis:
                    dragPlane = Plane::planeContainingVector(hit->hitPoint(), Vec3f::PosY, event.ray.origin);
                    m_restrictToAxis = RYAxis;
                    break;
                case Model::MoveObjectsHandleHit::HAZAxis:
                    dragPlane = Plane::planeContainingVector(hit->hitPoint(), Vec3f::PosZ, event.ray.origin);
                    m_restrictToAxis = RZAxis;
                    break;
                case Model::MoveObjectsHandleHit::HAXYPlane:
                    dragPlane = Plane::horizontalDragPlane(hit->hitPoint());
                    m_restrictToAxis = RNone;
                    break;
                case Model::MoveObjectsHandleHit::HAXZPlane:
                    dragPlane = Plane::verticalDragPlane(hit->hitPoint(), Vec3f::PosY);
                    m_restrictToAxis = RNone;
                    break;
                case Model::MoveObjectsHandleHit::HAYZPlane:
                    dragPlane = Plane::verticalDragPlane(hit->hitPoint(), Vec3f::PosX);
                    m_restrictToAxis = RNone;
                    break;
            }

            initialDragPoint = hit->hitPoint();
            m_totalDelta = Vec3f::Null;
            m_handle.lock();
            
            const Model::EntityList& entities = editStateManager.selectedEntities();
            const Model::BrushList& brushes = editStateManager.selectedBrushes();
            BeginCommandGroup(Controller::Command::makeObjectActionName(wxT("Move"), entities, brushes));
            
            return true;
        }
        
        bool MoveObjectsTool::handlePlaneDrag(InputEvent& event, const Vec3f& lastMousePoint, const Vec3f& curMousePoint, Vec3f& referencePoint) {
            Vec3f delta = curMousePoint - referencePoint;
            switch (m_restrictToAxis) {
                case RXAxis:
                    delta.y = delta.z = 0.0f;
                    break;
                case RYAxis:
                    delta.x = delta.z = 0.0f;
                    break;
                case RZAxis:
                    delta.x = delta.y = 0.0f;
                    break;
                default:
                    break;
            }
            
            Utility::Grid& grid = documentViewHolder().document().grid();
            delta = grid.snap(delta);
            if (delta.null())
                return true;
            
            m_handle.setPosition(m_handle.position() + delta);
            updateViews();
            
            Model::EditStateManager& editStateManager = documentViewHolder().document().editStateManager();
            const Model::EntityList& entities = editStateManager.selectedEntities();
            const Model::BrushList& brushes = editStateManager.selectedBrushes();
            Controller::MoveObjectsCommand* command = Controller::MoveObjectsCommand::moveObjects(documentViewHolder().document(), entities, brushes, delta, documentViewHolder().document().textureLock());
            postCommand(command);
            
            referencePoint += delta;
            m_totalDelta += delta;
            return true;
        }
        
        void MoveObjectsTool::handleEndPlaneDrag(InputEvent& event) {
            if (m_totalDelta.null())
                CancelCommandGroup();
            else
                EndCommandGroup();
            m_handle.unlock();
            
            m_handle.pick(event.ray);
            updateViews();
        }
        
        void MoveObjectsTool::handleChangeEditState(const Model::EditStateChangeSet& changeSet) {
            Model::EditStateManager& editStateManager = documentViewHolder().document().editStateManager();
            const Model::EntityList& entities = editStateManager.selectedEntities();
            const Model::BrushList& brushes = editStateManager.selectedBrushes();
            
            if (entities.empty() && brushes.empty()) {
                if (m_handleFigure != NULL) {
                    deleteFigure(m_handleFigure);
                    m_handleFigure = NULL;
                }
            } else {
                if (m_handleFigure == NULL) {
                    m_handleFigure = new Renderer::MoveObjectsHandleFigure(m_handle);
                    addFigure(m_handleFigure);
                }
                Vec3f position = Model::MapObject::center(entities, brushes);
                m_handle.setPosition(position);
            }
            
            updateViews();
        }
        
        MoveObjectsTool::MoveObjectsTool(View::DocumentViewHolder& documentViewHolder, InputController& inputController) :
        DragTool(documentViewHolder, inputController),
        m_handleFigure(NULL) {}
    }
}
