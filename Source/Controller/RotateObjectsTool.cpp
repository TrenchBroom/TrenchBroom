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
#include "Controller/RotateObjectsCommand.h"
#include "Model/EditStateManager.h"
#include "Renderer/ApplyMatrix.h"
#include "Renderer/AxisFigure.h"
#include "Renderer/Camera.h"
#include "Renderer/CircleFigure.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RingFigure.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/VertexArray.h"
#include "Utility/Console.h"
#include "Utility/Grid.h"
#include "Utility/Preferences.h"
#include "Utility/VecMath.h"

#include <cassert>

using namespace TrenchBroom::VecMath;

namespace TrenchBroom {
    namespace Controller {
        void RotateObjectsTool::updateHandlePosition(InputState& inputState) {
            Model::EditStateManager& editStateManager = document().editStateManager();
            if (!editStateManager.hasSelectedObjects())
                return;

            Vec3f position = document().grid().referencePoint(editStateManager.bounds());
            m_rotateHandle.setPosition(position);
        }

        bool RotateObjectsTool::handleActivate(InputState& inputState) {
            updateHandlePosition(inputState);
            return true;
        }

        bool RotateObjectsTool::handleIsModal(InputState& inputState) {
            return true;
        }

        void RotateObjectsTool::handlePick(InputState& inputState) {
            Model::RotateHandleHit* hit = m_rotateHandle.pick(inputState.pickRay());
            if (hit != NULL)
                inputState.pickResult().add(hit);
        }

        void RotateObjectsTool::handleRender(InputState& inputState, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext) {
            Model::EditStateManager& editStateManager = document().editStateManager();
            if (editStateManager.selectedEntities().empty() && editStateManager.selectedBrushes().empty())
                return;

            Model::RotateHandleHit* hit = NULL;
            if (m_rotateHandle.locked())
                hit = m_rotateHandle.lastHit();
            else
                hit = static_cast<Model::RotateHandleHit*>(inputState.pickResult().first(Model::HitType::RotateHandleHit, true, view().filter()));

            m_rotateHandle.render(hit, vbo, renderContext, m_angle);
        }

        void RotateObjectsTool::handleUpdate(const Command& command, InputState& inputState) {
            if (active()) {
                switch (command.type()) {
                    case Controller::Command::LoadMap:
                    case Controller::Command::ClearMap:
                    case Controller::Command::MoveObjects:
                    case Controller::Command::RotateObjects:
                    case Controller::Command::FlipObjects:
                    case Controller::Command::ResizeBrushes:
                    case Controller::Command::MoveVertices:
                    case Controller::Command::SnapVertices:
                    case Controller::Command::ChangeEditState:
                    case Controller::Command::ChangeGrid:
                        updateHandlePosition(inputState);
                        break;
                    default:
                        break;
                }
            }
        }

        bool RotateObjectsTool::handleStartDrag(InputState& inputState) {
            if (inputState.mouseButtons() != MouseButtons::MBLeft ||
                inputState.modifierKeys() != ModifierKeys::MKNone)
                return false;

            Model::EditStateManager& editStateManager = document().editStateManager();
            const Model::EntityList& entities = editStateManager.selectedEntities();
            const Model::BrushList& brushes = editStateManager.selectedBrushes();
            if (entities.empty() && brushes.empty())
                return false;

            Model::RotateHandleHit* hit = static_cast<Model::RotateHandleHit*>(inputState.pickResult().first(Model::HitType::RotateHandleHit, true, view().filter()));

            if (hit == NULL)
                return false;

            Vec3f test = hit->hitPoint() - m_rotateHandle.position();
            switch (hit->hitArea()) {
                case Model::RotateHandleHit::HAXAxis:
                    m_axis = Vec3f::PosX;
                    m_invert = ((test.dot(Vec3f::PosX) > 0.0f) == (test.dot(Vec3f::PosY) > 0.0f));
                    break;
                case Model::RotateHandleHit::HAYAxis:
                    m_axis = Vec3f::PosY;
                    m_invert = ((test.dot(Vec3f::PosX) > 0.0f) != (test.dot(Vec3f::PosY) > 0.0f));
                    break;
                case Model::RotateHandleHit::HAZAxis:
                    m_axis = Vec3f::PosZ;
                    m_invert = false;
                    break;
            }

            m_startX = inputState.x();
            m_startY = inputState.y();
            m_angle = 0.0f;
            m_center = m_rotateHandle.position();
            m_rotateHandle.lock();
            beginCommandGroup(Controller::Command::makeObjectActionName(wxT("Rotate"), entities, brushes));

            return true;
        }

        bool RotateObjectsTool::handleDrag(InputState& inputState) {
            int delta = 0;
            if (m_axis == Vec3f::PosZ) {
                delta = -(inputState.x() - m_startX);
            } else {
                delta = inputState.y() - m_startY;
                if (m_invert)
                    delta *= -1;
            }

            m_angle = static_cast<float>(delta) / 200.0f * Math<float>::Pi;

            Utility::Grid& grid = document().grid();
            m_angle = grid.snapAngle(m_angle);

            m_ignoreObjectsChange = true;
            rollbackCommandGroup();

            if (m_angle != 0.0f) {
                Model::EditStateManager& editStateManager = document().editStateManager();
                const Model::EntityList& entities = editStateManager.selectedEntities();
                const Model::BrushList& brushes = editStateManager.selectedBrushes();
                RotateObjectsCommand* command = RotateObjectsCommand::rotate(document(), entities, brushes, m_axis, m_angle, false, m_center, document().textureLock());
                submitCommand(command);
            }

            m_ignoreObjectsChange = false;
            return true;
        }

        void RotateObjectsTool::handleEndDrag(InputState& inputState) {
            endCommandGroup();
            m_rotateHandle.unlock();
            m_angle = 0.0f;
        }

        RotateObjectsTool::RotateObjectsTool(View::DocumentViewHolder& documentViewHolder, InputController& inputController, float axisLength, float ringRadius, float ringThickness) :
        Tool(documentViewHolder, inputController, true),
        m_angle(0.0f),
        m_ignoreObjectsChange(false),
        m_rotateHandle(RotateHandle(axisLength, ringRadius, ringThickness)){}
    }
}
