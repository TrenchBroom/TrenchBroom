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
        bool MoveObjectsTool::handleMouseMoved(InputEvent& event) {
            m_currentRay = event.ray;
            if (m_handleFigure != NULL) {
                m_currentHit = m_handleFigure->pick(event.ray);
                m_handleFigure->setHitType(m_currentHit.type());
                updateViews();
            }
            
         	return false;
        }
        
        bool MoveObjectsTool::handleBeginPlaneDrag(InputEvent& event, Plane& dragPlane, Vec3f& initialDragPoint) {
            if (event.mouseButtons != MouseButtons::MBLeft ||
                event.modifierKeys() != ModifierKeys::MKNone)
                return false;

            if (m_handleFigure == NULL)
                return false;
            
            m_currentHit = m_handleFigure->pick(event.ray);
            switch (m_currentHit.type()) {
                case HandleHit::TXAxis:
                    dragPlane = Plane::planeContainingVector(m_currentHit.hitPoint(), Vec3f::PosX, event.ray.origin);
                    break;
                case HandleHit::TYAxis:
                    dragPlane = Plane::planeContainingVector(m_currentHit.hitPoint(), Vec3f::PosY, event.ray.origin);
                    break;
                case HandleHit::TZAxis:
                    dragPlane = Plane::planeContainingVector(m_currentHit.hitPoint(), Vec3f::PosZ, event.ray.origin);
                    break;
                case HandleHit::TXYPlane:
                    dragPlane = Plane::horizontalDragPlane(m_currentHit.hitPoint());
                    break;
                case HandleHit::TXZPlane:
                    dragPlane = Plane::verticalDragPlane(m_currentHit.hitPoint(), Vec3f::PosY);
                    break;
                case HandleHit::TYZPlane:
                    dragPlane = Plane::verticalDragPlane(m_currentHit.hitPoint(), Vec3f::PosX);
                    break;
                 default:
                    return false;
            }
            initialDragPoint = m_currentHit.hitPoint();
            m_totalDelta = Vec3f::Null;
            
            Model::EditStateManager& editStateManager = documentViewHolder().document().editStateManager();
            const Model::EntityList& entities = editStateManager.selectedEntities();
            const Model::BrushList& brushes = editStateManager.selectedBrushes();
            BeginCommandGroup(Controller::Command::makeObjectActionName(wxT("Move"), entities, brushes));
            
            m_handleFigure->setLocked(true);
            return true;
        }
        
        bool MoveObjectsTool::handlePlaneDrag(InputEvent& event, const Vec3f& lastMousePoint, const Vec3f& curMousePoint, Vec3f& referencePoint) {

            Vec3f delta = curMousePoint - referencePoint;
            switch (m_currentHit.type()) {
                case HandleHit::TXAxis:
                    delta.y = delta.z = 0.0f;
                    break;
                case HandleHit::TYAxis:
                    delta.x = delta.z = 0.0f;
                    break;
                case HandleHit::TZAxis:
                    delta.x = delta.y = 0.0f;
                    break;
                default:
                    break;
            }
            
            Utility::Grid& grid = documentViewHolder().document().grid();
            delta = grid.snap(delta);
            if (delta.null())
                return true;
            
            m_handleFigure->setPosition(m_handleFigure->position() + delta);
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
            m_handleFigure->setLocked(false);
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
                Vec3f position = Model::MapObject::center(entities, brushes);
                if (m_handleFigure == NULL) {
                    m_handleFigure = new Renderer::MoveObjectsHandleFigure(64.0f, 32.0f);
                    addFigure(m_handleFigure);
                }

                m_handleFigure->setHitType(m_handleFigure->pick(m_currentRay).type());
                m_handleFigure->setPosition(position);
            }
            
            updateViews();
        }
        
        MoveObjectsTool::MoveObjectsTool(View::DocumentViewHolder& documentViewHolder) :
        DragTool(documentViewHolder),
        m_currentHit(HandleHit::noHit()),
        m_handleFigure(NULL) {}
    }
}
