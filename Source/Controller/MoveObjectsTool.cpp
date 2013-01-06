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
#include "Model/MapDocument.h"
#include "Model/MapObject.h"
#include "Model/ModelUtils.h"
#include "Renderer/ApplyMatrix.h"
#include "Renderer/AxisFigure.h"
#include "Renderer/Camera.h"
#include "Renderer/CircleFigure.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/VertexArray.h"
#include "Utility/Grid.h"

#include <cassert>

namespace TrenchBroom {
    namespace Controller {
        void MoveObjectsTool::updateHandlePosition(InputState& inputState) {
            Model::EditStateManager& editStateManager = document().editStateManager();
            const Model::EntityList& entities = editStateManager.selectedEntities();
            const Model::BrushList& brushes = editStateManager.selectedBrushes();
            if (entities.empty() && brushes.empty())
                return;
            
            Vec3f position = referencePoint(entities, brushes, document().grid());
            m_moveHandle.setPosition(position);
        }

        bool MoveObjectsTool::handleIsModal(InputState& inputState) {
            if (dragType() == DTDrag)
                return true;
            
            Model::MoveHandleHit* hit = static_cast<Model::MoveHandleHit*>(inputState.pickResult().first(Model::HitType::MoveHandleHit, true, view().filter()));
            return hit != NULL;
        }

        void MoveObjectsTool::handlePick(InputState& inputState) {
            Model::EditStateManager& editStateManager = document().editStateManager();
            if (editStateManager.selectedEntities().empty() && editStateManager.selectedBrushes().empty())
                return;

            Model::MoveHandleHit* hit = m_moveHandle.pick(inputState.pickRay());
            if (hit != NULL)
                inputState.pickResult().add(hit);

            if (!m_moveHandle.locked())
                m_moveHandle.setLastHit(hit);
        }

        void MoveObjectsTool::handleRender(InputState& inputState, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext) {
            Model::EditStateManager& editStateManager = document().editStateManager();
            if (editStateManager.selectedEntities().empty() && editStateManager.selectedBrushes().empty())
                return;
            
            Model::MoveHandleHit* hit = static_cast<Model::MoveHandleHit*>(inputState.pickResult().first(Model::HitType::MoveHandleHit, true, view().filter()));
            m_moveHandle.render(hit, vbo, renderContext);
        }
        
        bool MoveObjectsTool::handleStartPlaneDrag(InputState& inputState, Plane& plane, Vec3f& initialPoint) {
            if (inputState.mouseButtons() != MouseButtons::MBLeft ||
                inputState.modifierKeys() != ModifierKeys::MKNone)
                return false;
            
            Model::EditStateManager& editStateManager = document().editStateManager();
            const Model::EntityList& entities = editStateManager.selectedEntities();
            const Model::BrushList& brushes = editStateManager.selectedBrushes();
            if (entities.empty() && brushes.empty())
                return false;
            
            Model::MoveHandleHit* hit = static_cast<Model::MoveHandleHit*>(inputState.pickResult().first(Model::HitType::MoveHandleHit, true, view().filter()));
            
            if (hit == NULL)
                return false;
            
            switch (hit->hitArea()) {
                case Model::MoveHandleHit::HAXAxis:
                    plane = Plane::planeContainingVector(hit->hitPoint(), Vec3f::PosX, inputState.pickRay().origin);
                    m_restrictToAxis = MoveHandle::RXAxis;
                    break;
                case Model::MoveHandleHit::HAYAxis:
                    plane = Plane::planeContainingVector(hit->hitPoint(), Vec3f::PosY, inputState.pickRay().origin);
                    m_restrictToAxis = MoveHandle::RYAxis;
                    break;
                case Model::MoveHandleHit::HAZAxis:
                    plane = Plane::planeContainingVector(hit->hitPoint(), Vec3f::PosZ, inputState.pickRay().origin);
                    m_restrictToAxis = MoveHandle::RZAxis;
                    break;
                case Model::MoveHandleHit::HAXYPlane:
                    plane = Plane::horizontalDragPlane(hit->hitPoint());
                    m_restrictToAxis = MoveHandle::RNone;
                    break;
                case Model::MoveHandleHit::HAXZPlane:
                    plane = Plane::verticalDragPlane(hit->hitPoint(), Vec3f::PosY);
                    m_restrictToAxis = MoveHandle::RNone;
                    break;
                case Model::MoveHandleHit::HAYZPlane:
                    plane = Plane::verticalDragPlane(hit->hitPoint(), Vec3f::PosX);
                    m_restrictToAxis = MoveHandle::RNone;
                    break;
            }
            
            initialPoint = hit->hitPoint();
            m_totalDelta = Vec3f::Null;
            m_moveHandle.lock();
            
            beginCommandGroup(Controller::Command::makeObjectActionName(wxT("Move"), entities, brushes));
            
            return true;
        }
        
        bool MoveObjectsTool::handlePlaneDrag(InputState& inputState, const Vec3f& lastPoint, const Vec3f& curPoint, Vec3f& refPoint) {
            Vec3f delta = curPoint - refPoint;
            switch (m_restrictToAxis) {
                case MoveHandle::RXAxis:
                    delta.y = delta.z = 0.0f;
                    break;
                case MoveHandle::RYAxis:
                    delta.x = delta.z = 0.0f;
                    break;
                case MoveHandle::RZAxis:
                    delta.x = delta.y = 0.0f;
                    break;
                default:
                    break;
            }
            
            Utility::Grid& grid = document().grid();
            delta = grid.snap(delta);
            if (delta.null())
                return true;
            
            m_moveHandle.setPosition(m_moveHandle.position() + delta);
            
            Model::EditStateManager& editStateManager = document().editStateManager();
            const Model::EntityList& entities = editStateManager.selectedEntities();
            const Model::BrushList& brushes = editStateManager.selectedBrushes();
            Controller::MoveObjectsCommand* command = Controller::MoveObjectsCommand::moveObjects(document(), entities, brushes, delta, document().textureLock());
            submitCommand(command);
            
            refPoint += delta;
            m_totalDelta += delta;
            return true;
        }
        
        void MoveObjectsTool::handleEndPlaneDrag(InputState& inputState) {
            if (m_totalDelta.null())
                discardCommandGroup();
            else
                endCommandGroup();
            m_moveHandle.unlock();
        }
        
        void MoveObjectsTool::handleObjectsChange(InputState& inputState) {
            updateHandlePosition(inputState);
        }
        
        void MoveObjectsTool::handleEditStateChange(InputState& inputState, const Model::EditStateChangeSet& changeSet) {
            updateHandlePosition(inputState);
        }

        void MoveObjectsTool::handleGridChange(InputState& inputState) {
            updateHandlePosition(inputState);
        }
        
        MoveObjectsTool::MoveObjectsTool(View::DocumentViewHolder& documentViewHolder, float axisLength, float planeRadius) :
        PlaneDragTool(documentViewHolder, true),
        m_moveHandle(MoveHandle(axisLength, planeRadius)) {}
    }
}
