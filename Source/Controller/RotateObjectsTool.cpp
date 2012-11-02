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

#include "RotateObjectsTool.h"
#include "Controller/Command.h"
#include "Model/EditStateManager.h"
#include "Model/MapDocument.h"
#include "Renderer/RotateObjectsHandleFigure.h"

namespace TrenchBroom {
    namespace Controller {
        void RotateObjectsTool::updateHits(InputEvent& event) {
            Model::EditStateManager& editStateManager = documentViewHolder().document().editStateManager();
            if (editStateManager.selectionMode() == Model::EditStateManager::SMNone ||
                editStateManager.selectionMode() == Model::EditStateManager::SMFaces)
                return;
            
            Model::RotateObjectsHandleHit* hit = m_handle.pick(event.ray);
            if (hit != NULL)
                event.pickResult->add(*hit);
        }
        
        bool RotateObjectsTool::suppressOtherFeedback(InputEvent& event) {
            return m_handle.hit();
        }

        bool RotateObjectsTool::updateFeedback(InputEvent& event) {
            return m_handle.updated();
        }

        bool RotateObjectsTool::handleBeginPlaneDrag(InputEvent& event, Plane& dragPlane, Vec3f& initialDragPoint) {
            if (event.mouseButtons != MouseButtons::MBLeft ||
                event.modifierKeys() != ModifierKeys::MKNone)
                return false;
            
            Model::EditStateManager& editStateManager = documentViewHolder().document().editStateManager();
            if (editStateManager.selectionMode() == Model::EditStateManager::SMNone ||
                editStateManager.selectionMode() == Model::EditStateManager::SMFaces)
                return false;
            
            Model::RotateObjectsHandleHit* hit = static_cast<Model::RotateObjectsHandleHit*>(event.pickResult->first(Model::HitType::RotateObjectsHandleHit, true, documentViewHolder().view().filter()));
            if (hit == NULL)
                return false;
            
            switch (hit->hitArea()) {
                case Model::RotateObjectsHandleHit::HAXAxis:
                    dragPlane = Plane::planeContainingVector(hit->hitPoint(), Vec3f::PosX, event.ray.origin);
                    break;
                case Model::RotateObjectsHandleHit::HAYAxis:
                    dragPlane = Plane::planeContainingVector(hit->hitPoint(), Vec3f::PosY, event.ray.origin);
                    break;
                case Model::RotateObjectsHandleHit::HAZAxis:
                    dragPlane = Plane::planeContainingVector(hit->hitPoint(), Vec3f::PosZ, event.ray.origin);
                    break;
            }
            
            initialDragPoint = hit->hitPoint();
            m_handle.lock();
            
            const Model::EntityList& entities = editStateManager.selectedEntities();
            const Model::BrushList& brushes = editStateManager.selectedBrushes();
            BeginCommandGroup(Controller::Command::makeObjectActionName(wxT("Rotate"), entities, brushes));
            
            return true;
        }
        
        bool RotateObjectsTool::handlePlaneDrag(InputEvent& event, const Vec3f& lastMousePoint, const Vec3f& curMousePoint, Vec3f& referencePoint) {
            return true;
        }
        
        void RotateObjectsTool::handleEndPlaneDrag(InputEvent& event) {
            EndCommandGroup();
            m_handle.unlock();
            
            m_handle.pick(event.ray);
        }
        
        void RotateObjectsTool::handleChangeEditState(const Model::EditStateChangeSet& changeSet) {
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
                    m_handleFigure = new Renderer::RotateObjectsHandleFigure(m_handle, 64.0f);
                    addFigure(m_handleFigure);
                }
                Vec3f position = Model::MapObject::center(entities, brushes);
                m_handle.setPosition(position);
            }
        }

        RotateObjectsTool::RotateObjectsTool(View::DocumentViewHolder& documentViewHolder, InputController& inputController) :
        DragTool(documentViewHolder, inputController),
        m_handleFigure(NULL) {}
    }
}