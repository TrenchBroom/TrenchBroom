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

#ifndef __TrenchBroom__ClipTool__
#define __TrenchBroom__ClipTool__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/ModelTypes.h"
#include "Model/Picker.h"
#include "Renderer/ClipperRenderer.h"
#include "View/Clipper.h"
#include "View/Tool.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace View {
        class ClipTool : public ToolImpl<ActivationPolicy, PickingPolicy, MousePolicy, MouseDragPolicy, NoDropPolicy, RenderPolicy> {
        private:
            static const Model::Hit::HitType HandleHit;

            Clipper m_clipper;
            Renderer::ClipperRenderer m_renderer;
            Model::EntityBrushesMap m_frontBrushes;
            Model::EntityBrushesMap m_backBrushes;
            size_t m_dragPointIndex;
        public:
            ClipTool(MapDocumentWPtr document, ControllerWPtr controller, const Renderer::Camera& camera);
            
            bool canToggleClipSide() const;
            void toggleClipSide();
            bool canPerformClip() const;
            void performClip();
            bool canDeleteLastClipPoint() const;
            void deleteLastClipPoint();
        private:
            bool initiallyActive() const;
            bool doActivate(const InputState& inputState);
            bool doDeactivate(const InputState& inputState);
            
            void doPick(const InputState& inputState, Model::PickResult& pickResult);

            bool doMouseUp(const InputState& inputState);

            bool doStartMouseDrag(const InputState& inputState);
            bool doMouseDrag(const InputState& inputState);
            void doEndMouseDrag(const InputState& inputState);
            void doCancelMouseDrag(const InputState& inputState);

            void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const;
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext);
            
            Vec3 clipPoint(const Model::Hit& hit) const;
            void updateBrushes();
            void clearAndDelete(Model::EntityBrushesMap& brushes);
        };
    }
}

#endif /* defined(__TrenchBroom__ClipTool__) */
