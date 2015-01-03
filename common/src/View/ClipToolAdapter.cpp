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

#include "ClipToolAdapter.h"

#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/HitAdapter.h"
#include "Model/HitQuery.h"
#include "Model/PickResult.h"
#include "Renderer/RenderContext.h"
#include "View/ClipTool.h"
#include "View/InputState.h"

namespace TrenchBroom {
    namespace View {
        ClipToolAdapter::ClipToolAdapter(ClipTool* tool) :
        ToolAdapterBase(),
        m_tool(tool) {}
        
        ClipToolAdapter::~ClipToolAdapter() {}

        Tool* ClipToolAdapter::doGetTool() {
            return m_tool;
        }
        
        void ClipToolAdapter::doPick(const InputState& inputState, Model::PickResult& pickResult) {
            m_tool->pick(inputState.pickRay(), inputState.camera(), pickResult);
        }
        
        
        bool ClipToolAdapter::doMouseClick(const InputState& inputState) {
            if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft) ||
                !inputState.checkModifierKeys(MK_No, MK_No, MK_DontCare))
                return false;

            const Model::Hit& hit = inputState.pickResult().query().pickable().type(Model::Brush::BrushHit).occluded().first();
            if (hit.isMatch()) {
                const Model::BrushFace* face = Model::hitToFace(hit);
                const Renderer::Camera& camera = inputState.camera();
                if (inputState.modifierKeysPressed(ModifierKeys::MKShift))
                    m_tool->setClipPoints(hit.hitPoint(), face, camera);
                else
                    m_tool->addClipPoint(hit.hitPoint(), face, camera);
                return true;
            }
            return false;
        }
        
        bool ClipToolAdapter::doStartMouseDrag(const InputState& inputState) {
            if (inputState.mouseButtons() != MouseButtons::MBLeft ||
                inputState.modifierKeys() != ModifierKeys::MKNone)
                return false;
            
            return m_tool->beginDragClipPoint(inputState.pickResult());
        }
        
        bool ClipToolAdapter::doMouseDrag(const InputState& inputState) {
            const Model::Hit& hit = inputState.pickResult().query().pickable().type(Model::Brush::BrushHit).occluded().first();
            if (hit.isMatch()) {
                const Model::BrushFace* face = Model::hitToFace(hit);
                m_tool->dragClipPoint(hit.hitPoint(), face);
            }
            return true;
        }
        
        void ClipToolAdapter::doEndMouseDrag(const InputState& inputState) {}
        void ClipToolAdapter::doCancelMouseDrag() {}
        
        void ClipToolAdapter::doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {
            renderContext.setHideSelection();
            renderContext.setForceHideSelectionGuide();
        }
        
        void ClipToolAdapter::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            m_tool->renderBrushes(renderContext, renderBatch);
            
            if (!inputState.modifierKeysPressed(ModifierKeys::MKShift)) {
                m_tool->renderClipPoints(renderContext, renderBatch);

                if (dragging())
                    m_tool->renderDragHighlight(renderContext, renderBatch);
                else
                    m_tool->renderHighlight(inputState.pickResult(), renderContext, renderBatch);
            }
        }
        
        bool ClipToolAdapter::doCancel() {
            return m_tool->resetClipper();
        }
    }
}
