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

#include "RotateObjectsTool.h"

#include "Model/ModelTypes.h"
#include "Model/Object.h"
#include "Renderer/Camera.h"
#include "View/InputState.h"
#include "View/MapDocument.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        const Model::Hit::HitType RotateObjectsTool::HandleHit = Model::Hit::freeHitType();

        RotateObjectsTool::RotateObjectsTool(BaseTool* next, MapDocumentPtr document, ControllerPtr controller) :
        Tool(next, document, controller) {}

        bool RotateObjectsTool::initiallyActive() const {
            return false;
        }
        
        bool RotateObjectsTool::doActivate(const InputState& inputState) {
            if (!document()->hasSelectedObjects())
                return false;
            resetHandlePosition();
            updateHandleAxes(inputState);
            return true;
        }
        
        bool RotateObjectsTool::doDeactivate(const InputState& inputState) {
            return true;
        }
        
        void RotateObjectsTool::doPick(const InputState& inputState, Model::PickResult& pickResult) {
            updateHandleAxes(inputState);
        }
        
        void RotateObjectsTool::doMouseMove(const InputState& inputState) {
            updateHandleAxes(inputState);
        }
        
        bool RotateObjectsTool::doStartMouseDrag(const InputState& inputState) {
            updateHandleAxes(inputState);
            return false;
        }
        
        bool RotateObjectsTool::doMouseDrag(const InputState& inputState) {
            updateHandleAxes(inputState);
            return true;
        }
        
        void RotateObjectsTool::doEndMouseDrag(const InputState& inputState) {
            updateHandleAxes(inputState);
        }
            
        void RotateObjectsTool::doCancelMouseDrag(const InputState& inputState) {
            updateHandleAxes(inputState);
        }
        
        void RotateObjectsTool::doRender(const InputState& inputState, Renderer::RenderContext& renderContext) {
            updateHandleAxes(inputState);
            m_handle.renderHandle(renderContext);
        }

        void RotateObjectsTool::resetHandlePosition() {
            const Model::ObjectList& objects = document()->selectedObjects();
            assert(!objects.empty());
            const BBox3 bounds = Model::Object::bounds(objects);
            m_handle.setPosition(bounds.center());
        }

        void RotateObjectsTool::updateHandleAxes(const InputState& inputState) {
            m_handle.updateAxes(inputState.camera().position());
        }
    }
}
