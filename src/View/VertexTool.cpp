/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "VertexTool.h"

#include "Model/Brush.h"
#include "Model/HitAdapter.h"
#include "Model/HitFilters.h"
#include "Model/Picker.h"
#include "Model/Object.h"
#include "Model/SelectionResult.h"
#include "View/InputState.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace View {
        const FloatType VertexTool::MaxVertexDistance = 0.25;
        
        VertexTool::VertexTool(BaseTool* next, MapDocumentWPtr document, ControllerWPtr controller, MovementRestriction& movementRestriction) :
        MoveTool(next, document, controller, movementRestriction),
        m_mode(VMMove),
        m_changeCount(0) {}

        bool VertexTool::doHandleMove(const InputState& inputState) const {
            return false;
        }
        
        Vec3 VertexTool::doGetMoveOrigin(const InputState& inputState) const {
            return Vec3::Null;
        }
        
        String VertexTool::doGetActionName(const InputState& inputState) const {
            return "";
        }
        
        bool VertexTool::doStartMove(const InputState& inputState) {
            return false;
        }
        
        Vec3 VertexTool::doSnapDelta(const InputState& inputState, const Vec3& delta) const {
            return delta;
        }
        
        MoveResult VertexTool::doMove(const Vec3& delta) {
            return Conclude;
        }
        
        void VertexTool::doEndMove(const InputState& inputState) {
        }
        
        bool VertexTool::initiallyActive() const {
            return false;
        }
        
        bool VertexTool::doActivate(const InputState& inputState) {
            m_mode = VMMove;
            m_handleManager.clear();
            m_handleManager.addBrushes(document()->selectedBrushes());
            m_changeCount = 0;
            document()->selectionDidChangeNotifier.addObserver(this, &VertexTool::selectionDidChange);
            
            return true;
        }
        
        bool VertexTool::doDeactivate(const InputState& inputState) {
            document()->selectionDidChangeNotifier.removeObserver(this, &VertexTool::selectionDidChange);
            m_handleManager.clear();
            
            /*
            if (m_changeCount > 0) {
                RebuildBrushGeometryCommand* command = RebuildBrushGeometryCommand::rebuildGeometry(document(), document().editStateManager().selectedBrushes(), m_changeCount);
                submitCommand(command);
             }
             */
            return true;
        }
        
        void VertexTool::doPick(const InputState& inputState, Model::PickResult& pickResult) {
            m_handleManager.pick(inputState.pickRay(), pickResult, m_mode == VMSplit);
        }
        
        bool VertexTool::doMouseDown(const InputState& inputState) {
            const Model::Hit::HitType any = VertexHandleManager::VertexHandleHit | VertexHandleManager::EdgeHandleHit | VertexHandleManager::FaceHandleHit;
            const Model::PickResult::FirstHit first = Model::firstHit(inputState.pickResult(), any, true);
            return first.matches && inputState.mouseButtonsPressed(MouseButtons::MBLeft);
        }
        
        bool VertexTool::doMouseUp(const InputState& inputState) {
            const Model::Hit::HitType any = VertexHandleManager::VertexHandleHit | VertexHandleManager::EdgeHandleHit | VertexHandleManager::FaceHandleHit;
            const Model::PickResult::FirstHit first = Model::firstHit(inputState.pickResult(), any, true);
            return first.matches && inputState.mouseButtonsPressed(MouseButtons::MBLeft);
        }

        void VertexTool::doRender(const InputState& inputState, Renderer::RenderContext& renderContext) {
            m_handleManager.render(renderContext, m_mode == VMSplit);
        }

        void VertexTool::selectionDidChange(const Model::SelectionResult& selection) {
            Model::ObjectSet::const_iterator it, end;

            const Model::ObjectSet& selectedObjects = selection.selectedObjects();
            for (it = selectedObjects.begin(), end = selectedObjects.end(); it != end; ++it) {
                Model::Object* object = *it;
                if (object->type() == Model::Object::OTBrush) {
                    Model::Brush* brush = static_cast<Model::Brush*>(object);
                    m_handleManager.addBrush(brush);
                }
            }
            
            const Model::ObjectSet& deselectedObjects = selection.deselectedObjects();
            for (it = deselectedObjects.begin(), end = deselectedObjects.end(); it != end; ++it) {
                Model::Object* object = *it;
                if (object->type() == Model::Object::OTBrush) {
                    Model::Brush* brush = static_cast<Model::Brush*>(object);
                    m_handleManager.removeBrush(brush);
                }
            }
        }
    }
}
