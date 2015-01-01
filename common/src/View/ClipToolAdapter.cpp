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

#include "Model/HitAdapter.h"
#include "Model/ModelHitFilters.h"
#include "View/InputState.h"

namespace TrenchBroom {
    namespace View {
        ClipToolAdapter::ClipToolAdapter(ClipTool* tool) :
        ToolAdapterBase(),
        m_tool(tool) {}
        
        ClipToolAdapter::~ClipToolAdapter() {}

        Tool* doGetTool();
        
        void ClipToolAdapter::doPick(const InputState& inputState, Hits& hits) {
            const Hit hit = doPick(inputState.pickRay());
            if (hit.isMatch())
                hits.addHit(hit);
        }
        
        
        bool ClipToolAdapter::doMouseClick(const InputState& inputState) {
            if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft) ||
                !inputState.checkModifierKeys(MK_No, MK_No, MK_DontCare))
                return false;

            const Hit& hit = Model::findFirst(inputState.hits(), Model::Brush::BrushHit, document()->filter(), true);
        }
        
        bool ClipToolAdapter::doStartMouseDrag(const InputState& inputState) {
        }
        
        bool ClipToolAdapter::doMouseDrag(const InputState& inputState) {
        }
        
        void ClipToolAdapter::doEndMouseDrag(const InputState& inputState) {
        }
        
        void ClipToolAdapter::doCancelMouseDrag() {
        }
        
        void ClipToolAdapter::doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {
        }
        
        void ClipToolAdapter::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
        }
        
        bool ClipToolAdapter::doCancel() {
            return m_tool->resetClipper();
        }
    }
}
